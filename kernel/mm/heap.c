#include <nova/mm/heap.h>
#include <nova/mm/pmm.h>
#include <nova/mm/paging.h>

#define HEAP_BASE 0xffff900100000000ULL
#define HEAP_LIMIT 0xffffa00000000000ULL
#define BLOCK_MAGIC 0x4e4f564148454150ULL

typedef struct heap_block {
    uint64_t magic;
    uint64_t size;
    struct heap_block *prev;
    struct heap_block *next;
    uint64_t free;
} heap_block;

static heap_block *first_block;
static uint64_t heap_end, mapped_end, used_bytes;
static bool initialized;

static bool add_overflow(uint64_t a, uint64_t b) { return b > (~0ULL - a); }
static uint64_t align_size(uint64_t size) { return (size + NOVA_HEAP_ALIGNMENT - 1) & ~(NOVA_HEAP_ALIGNMENT - 1); }
static bool valid_block(heap_block *b) { return b && b->magic == BLOCK_MAGIC && (uint64_t)(uintptr_t)b >= HEAP_BASE && (uint64_t)(uintptr_t)b < heap_end; }
static bool map_more(uint64_t bytes) {
    uint64_t needed, target, pages, i, physical;
    if (!bytes || add_overflow(heap_end, bytes)) return false;
    if (heap_end + bytes > HEAP_LIMIT) return false;
    needed = heap_end + bytes; target = (needed + NOVA_PAGE_SIZE - 1) & ~(NOVA_PAGE_SIZE - 1); if (target < needed || target < mapped_end) target = mapped_end;
    pages = (target - mapped_end) / NOVA_PAGE_SIZE;
    for (i = 0; i < pages; i++) {
        void *page = pmm_alloc_page(); if (!page) return false;
        physical = (uint64_t)(uintptr_t)page;
        if (!paging_map_page(mapped_end + i * NOVA_PAGE_SIZE, physical, NOVA_PAGE_WRITABLE | NOVA_PAGE_NO_EXECUTE)) { pmm_free_page(page); return false; }
    }
    mapped_end += pages * NOVA_PAGE_SIZE; return true;
}
static bool grow(uint64_t payload) {
    uint64_t needed, old_end = heap_end; heap_block *b;
    if (add_overflow(sizeof(heap_block), payload)) return false;
    needed = sizeof(heap_block) + payload;
    if (!map_more(needed)) return false;
    heap_end = old_end + needed;
    b = (heap_block *)(uintptr_t)old_end; b->magic = BLOCK_MAGIC; b->size = payload; b->prev = NULL; b->next = NULL; b->free = 1;
    if (!first_block) first_block = b; else { heap_block *last = first_block; while (last->next) last = last->next; last->next = b; b->prev = last; }
    return true;
}
static void merge_with_next(heap_block *b) {
    heap_block *n = b->next; if (!n || !n->free) return;
    b->size += sizeof(heap_block) + n->size; b->next = n->next; if (b->next) b->next->prev = b;
}

bool heap_init(void) { first_block = NULL; heap_end = HEAP_BASE; mapped_end = HEAP_BASE; used_bytes = 0; initialized = true; return true; }
void *kmalloc(size_t requested) {
    uint64_t size, address; heap_block *b;
    if (!initialized || !requested || add_overflow((uint64_t)requested, NOVA_HEAP_ALIGNMENT - 1)) return NULL;
    size = align_size((uint64_t)requested);
    for (b = first_block; b; b = b->next) if (b->free && b->size >= size) {
        if (b->size >= size + sizeof(heap_block) + NOVA_HEAP_ALIGNMENT) {
            heap_block *split = (heap_block *)((uint8_t *)(b + 1) + size); split->magic = BLOCK_MAGIC; split->size = b->size - size - sizeof(heap_block); split->free = 1; split->prev = b; split->next = b->next; if (split->next) split->next->prev = split; b->next = split; b->size = size;
        }
        b->free = 0; used_bytes += b->size; return (void *)(b + 1);
    }
    if (!grow(size)) return NULL; b = first_block; while (b->next) b = b->next; b->free = 0; used_bytes += b->size; address = (uint64_t)(uintptr_t)(b + 1); return (void *)(uintptr_t)address;
}
void kfree(void *ptr) {
    heap_block *b; uint64_t p = (uint64_t)(uintptr_t)ptr;
    if (!initialized || !ptr || p < HEAP_BASE + sizeof(heap_block) || p >= heap_end) return;
    b = ((heap_block *)ptr) - 1; if (!valid_block(b) || (void *)(b + 1) != ptr || b->free) return;
    b->free = 1; used_bytes -= b->size; if (b->next && b->next->free) merge_with_next(b); if (b->prev && b->prev->free) merge_with_next(b->prev);
}
void *kcalloc(size_t count, size_t size) { uint64_t total; uint8_t *p; if (!count || !size || (uint64_t)size > (~0ULL / (uint64_t)count)) return NULL; total = (uint64_t)count * (uint64_t)size; p = kmalloc((size_t)total); if (!p) return NULL; for (uint64_t i = 0; i < total; i++) p[i] = 0; return p; }
uint64_t heap_bytes_used(void) { return used_bytes; }
uint64_t heap_bytes_mapped(void) { return mapped_end - HEAP_BASE; }

bool heap_self_test(void) {
    void *a, *b, *c, *d, *items[128]; uint64_t sizes[] = {1,8,16,64,256,1024,4096}; uint64_t i, j;
    a = kmalloc(16); if (!a || ((uint64_t)(uintptr_t)a & 15)) return false; *(uint64_t *)a = 0xaaaaaaaaaaaaaaaaULL; if (*(uint64_t *)a != 0xaaaaaaaaaaaaaaaaULL) return false; kfree(a); if (kmalloc(16) != a) return false;
    for (i = 0; i < 7; i++) { void *p = kmalloc(sizes[i]); if (!p || ((uint64_t)(uintptr_t)p & 15)) return false; kfree(p); }
    a=kmalloc(64); b=kmalloc(128); c=kmalloc(256); d=kmalloc(512); if (!a||!b||!c||!d||a==b||b==c||c==d) return false; kfree(b); kfree(d); kfree(a); kfree(c);
    for (i = 0; i < 128; i++) { items[i] = kmalloc(4096); if (!items[i]) return false; for (j=0;j<128;j++) if (j != i && items[i] == items[j]) return false; }
    for (i = 0; i < 128; i++) kfree(items[i]); if (!kcalloc(8, 32)) return false; return true;
}
