#ifndef NOVA_VFS_H
#define NOVA_VFS_H
#include <nova/types.h>

#define NOVA_VFS_NAME_MAX 64
#define NOVA_VFS_MAX_FILES 64
#define NOVA_VFS_MAX_PATH 128
#define NOVA_VFS_FD_MIN 3ULL

#define NOVA_VFS_ENOSYS (-1LL)
#define NOVA_VFS_EBADF (-2LL)
#define NOVA_VFS_EFAULT (-3LL)
#define NOVA_VFS_EINVAL (-4LL)
#define NOVA_VFS_ENOENT (-5LL)
#define NOVA_VFS_EEXIST (-6LL)
#define NOVA_VFS_ENOMEM (-7LL)
#define NOVA_VFS_EMFILE (-8LL)

#define NOVA_VNODE_DIR 1U
#define NOVA_VNODE_FILE 2U

struct nova_vnode {
    uint64_t ino;
    uint32_t kind;
    char name[NOVA_VFS_NAME_MAX];
    uint64_t size;
    uint64_t cap;
    uint8_t *data;
    struct nova_vnode *parent;
    struct nova_vnode *next;
    struct nova_vnode *children;
};

struct nova_stat {
    uint64_t ino;
    uint32_t kind;
    uint32_t reserved;
    uint64_t size;
};

bool vfs_init(void);
int64_t vfs_create(const char *path);
int64_t vfs_create_path(const char *path);
int64_t vfs_open(const char *path);
int64_t vfs_close(int64_t fd);
int64_t vfs_read(int64_t fd, void *buf, uint64_t len);
int64_t vfs_write(int64_t fd, const void *buf, uint64_t len);
int64_t vfs_seek(int64_t fd, uint64_t offset);
bool vfs_stat(int64_t fd, struct nova_stat *out);
bool vfs_self_test(void);

uint64_t vfs_strlen(const char *s);
int vfs_strcmp(const char *a, const char *b);
void vfs_memcpy(void *dst, const void *src, size_t n);
void vfs_memset(void *dst, int c, size_t n);
int vfs_memcmp(const void *a, const void *b, size_t n);
#endif
