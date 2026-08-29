#include <nova/vfs.h>
#include <nova/mm/heap.h>
#include <nova/console.h>

#define NOVA_VFS_TEST_CONTENT "Hello NovaOS"

struct nova_file { bool in_use; struct nova_vnode *node; uint64_t offset; };

static struct nova_vfs { struct nova_vnode *root; uint64_t next_ino; struct nova_file files[NOVA_VFS_MAX_FILES]; } vfs;
static bool vfs_ready;

uint64_t vfs_strlen(const char *s) { uint64_t n = 0; if (!s) return 0; while (*s++) n++; return n; }
int vfs_strcmp(const char *a, const char *b) { if (!a || !b) return a ? 1 : -1; while (*a && *a == *b) { a++; b++; } return (int)(uint8_t)*a - (int)(uint8_t)*b; }
void vfs_memcpy(void *d, const void *s, size_t n) { uint8_t *dd = d; const uint8_t *ss = s; for (size_t i = 0; i < n; i++) dd[i] = ss[i]; }
void vfs_memset(void *d, int c, size_t n) { uint8_t *dd = d; for (size_t i = 0; i < n; i++) dd[i] = (uint8_t)c; }
int vfs_memcmp(const void *a, const void *b, size_t n) { const uint8_t *aa = a, *bb = b; for (size_t i = 0; i < n; i++) if (aa[i] != bb[i]) return (int)aa[i] - (int)bb[i]; return 0; }

static struct nova_vnode *vnode_alloc(const char *name, uint32_t kind, struct nova_vnode *parent) {
    struct nova_vnode *n; uint64_t len;
    if (!name) return NULL;
    len = vfs_strlen(name); if (len >= NOVA_VFS_NAME_MAX) return NULL;
    n = kmalloc(sizeof(*n)); if (!n) return NULL;
    vfs_memset(n, 0, sizeof(*n));
    vfs_memcpy(n->name, name, len); n->name[len] = 0;
    n->ino = vfs.next_ino++; n->kind = kind; n->parent = parent;
    return n;
}

static bool valid_fd(int64_t fd) { return fd >= (int64_t)NOVA_VFS_FD_MIN && (uint64_t)fd < NOVA_VFS_FD_MIN + NOVA_VFS_MAX_FILES; }
static struct nova_file *file_get(int64_t fd) {
    if (!vfs_ready || !valid_fd(fd)) return NULL;
    struct nova_file *f = &vfs.files[fd - NOVA_VFS_FD_MIN];
    return f->in_use ? f : NULL;
}

static struct nova_vnode *vfs_lookup(const char *path) {
    struct nova_vnode *c;
    if (!vfs_ready || !path || path[0] != '/') return NULL;
    if (path[1] == '\0') return vfs.root;
    for (c = vfs.root->children; c; c = c->next) if (vfs_strcmp(c->name, path + 1) == 0) return c;
    return NULL;
}

bool vfs_init(void) {
    int64_t fd;
    if (vfs_ready) return true;
    vfs_memset(&vfs, 0, sizeof(vfs)); vfs.next_ino = 1;
    vfs.root = vnode_alloc("/", NOVA_VNODE_DIR, NULL); if (!vfs.root) return false;
    vfs_ready = true;
    if (vfs_create("/hello.txt") != 0) return false;
    fd = vfs_open("/hello.txt"); if (fd < 0) return false;
    if (vfs_write(fd, NOVA_VFS_TEST_CONTENT, vfs_strlen(NOVA_VFS_TEST_CONTENT)) != (int64_t)vfs_strlen(NOVA_VFS_TEST_CONTENT)) return false;
    if (vfs_close(fd) != 0) return false;
    return true;
}

int64_t vfs_create(const char *path) {
    struct nova_vnode *n; const char *name; uint64_t i;
    if (!vfs_ready || !path || path[0] != '/') return NOVA_VFS_EINVAL;
    name = path + 1;
    if (name[0] == '\0') return NOVA_VFS_EEXIST;
    for (i = 0; name[i]; i++) if (name[i] == '/') return NOVA_VFS_EINVAL;
    if (i >= NOVA_VFS_NAME_MAX) return NOVA_VFS_EINVAL;
    if (vfs_lookup(path)) return NOVA_VFS_EEXIST;
    n = vnode_alloc(name, NOVA_VNODE_FILE, vfs.root); if (!n) return NOVA_VFS_ENOMEM;
    n->next = vfs.root->children; vfs.root->children = n;
    return 0;
}

int64_t vfs_open(const char *path) {
    struct nova_vnode *n; uint64_t i;
    if (!vfs_ready || !path) return NOVA_VFS_EFAULT;
    n = vfs_lookup(path); if (!n) return NOVA_VFS_ENOENT;
    if (n->kind != NOVA_VNODE_FILE) return NOVA_VFS_EINVAL;
    for (i = 0; i < NOVA_VFS_MAX_FILES; i++) if (!vfs.files[i].in_use) {
        vfs.files[i].in_use = true; vfs.files[i].node = n; vfs.files[i].offset = 0;
        return (int64_t)(i + NOVA_VFS_FD_MIN);
    }
    return NOVA_VFS_EMFILE;
}

int64_t vfs_close(int64_t fd) {
    struct nova_file *f = file_get(fd);
    if (!f) return NOVA_VFS_EBADF;
    f->in_use = false; f->node = NULL; f->offset = 0;
    return 0;
}

static bool file_reserve(struct nova_vnode *n, uint64_t need) {
    uint64_t ncap; uint8_t *nd;
    if (need <= n->cap) { if (n->size < need) n->size = need; return true; }
    ncap = n->cap ? n->cap : 16;
    while (ncap < need) { uint64_t prev = ncap; ncap *= 2; if (ncap < prev) return false; }
    nd = kmalloc((size_t)ncap); if (!nd) return false;
    if (n->data) { vfs_memcpy(nd, n->data, n->size); kfree(n->data); }
    vfs_memset(nd + n->size, 0, (size_t)(ncap - n->size));
    n->data = nd; n->cap = ncap;
    if (n->size < need) n->size = need;
    return true;
}

int64_t vfs_write(int64_t fd, const void *buf, uint64_t len) {
    struct nova_file *f = file_get(fd); uint64_t need;
    if (!f) return NOVA_VFS_EBADF;
    if (f->node->kind != NOVA_VNODE_FILE) return NOVA_VFS_EINVAL;
    if (!buf && len) return NOVA_VFS_EFAULT;
    if (f->offset > f->node->size || f->offset > ~0ULL - len) return NOVA_VFS_EINVAL;
    need = f->offset + len;
    if (!file_reserve(f->node, need)) return NOVA_VFS_ENOMEM;
    vfs_memcpy(f->node->data + f->offset, buf, (size_t)len);
    f->offset += len;
    return (int64_t)len;
}

int64_t vfs_read(int64_t fd, void *buf, uint64_t len) {
    struct nova_file *f = file_get(fd); uint64_t avail;
    if (!f) return NOVA_VFS_EBADF;
    if (f->node->kind != NOVA_VNODE_FILE) return NOVA_VFS_EINVAL;
    if (!buf && len) return NOVA_VFS_EFAULT;
    if (f->offset >= f->node->size) return 0;
    avail = f->node->size - f->offset; if (len > avail) len = avail;
    vfs_memcpy(buf, f->node->data + f->offset, (size_t)len);
    f->offset += len;
    return (int64_t)len;
}

int64_t vfs_seek(int64_t fd, uint64_t offset) {
    struct nova_file *f = file_get(fd);
    if (!f) return NOVA_VFS_EBADF;
    if (f->node->kind != NOVA_VNODE_FILE) return NOVA_VFS_EINVAL;
    if (offset > f->node->size) return NOVA_VFS_EINVAL;
    f->offset = offset;
    return (int64_t)offset;
}

bool vfs_stat(int64_t fd, struct nova_stat *out) {
    struct nova_file *f = file_get(fd);
    if (!f || !out) return false;
    if (f->node->kind != NOVA_VNODE_FILE) return false;
    out->ino = f->node->ino; out->kind = NOVA_VNODE_FILE; out->reserved = 0; out->size = f->node->size;
    return true;
}

bool vfs_self_test(void) {
    struct nova_stat st; char buf[NOVA_VFS_MAX_PATH]; uint64_t content_len = vfs_strlen(NOVA_VFS_TEST_CONTENT); int64_t fd;
    if (!vfs_ready) return false;
    if (vfs_create("/test.txt") != 0) return false;
    fd = vfs_open("/test.txt"); if (fd < 0) return false;
    if (vfs_write(fd, NOVA_VFS_TEST_CONTENT, content_len) != (int64_t)content_len) return false;
    if (vfs_seek(fd, 0) != 0) return false;
    if (vfs_read(fd, buf, content_len) != (int64_t)content_len || vfs_memcmp(buf, NOVA_VFS_TEST_CONTENT, content_len)) return false;
    if (!vfs_stat(fd, &st) || st.kind != NOVA_VNODE_FILE || st.size != content_len) return false;
    if (vfs_close(fd) != 0) return false;
    fd = vfs_open("/hello.txt"); if (fd < 0) return false;
    if (vfs_read(fd, buf, content_len) != (int64_t)content_len || vfs_memcmp(buf, NOVA_VFS_TEST_CONTENT, content_len)) return false;
    if (vfs_close(fd) != 0) return false;
    if (vfs_open("/does-not-exist") >= 0) return false;
    if (vfs_open("/") >= 0) return false;
    if (vfs_read(9999, buf, 1) >= 0) return false;
    if (vfs_write(9999, buf, 1) >= 0) return false;
    if (vfs_seek(9999, 0) >= 0) return false;
    if (vfs_close(9999) >= 0) return false;
    if (vfs_stat(9999, &st)) return false;
    if (vfs_create("/test.txt") >= 0) return false;
    if (vfs_create("/a/b") >= 0) return false;
    return true;
}
