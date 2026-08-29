#include <nova/address_space.h>
#include <nova/mm/heap.h>
#include <nova/mm/paging.h>
#include <nova/mm/pmm.h>

static struct nova_address_space kernel_space;
static bool initialized;

struct nova_address_space *nova_address_space_kernel(void) {
    return initialized ? &kernel_space : NULL;
}

struct nova_address_space *nova_address_space_create(void) {
    struct nova_address_space *space;
    if (!initialized) return NULL;
    space = kmalloc(sizeof(*space));
    if (!space) return NULL;
    if (!paging_root_create(&space->root_physical)) { kfree(space); return NULL; }
    space->flags = 0;
    space->references = 1;
    return space;
}

void nova_address_space_destroy(struct nova_address_space *space) {
    if (!space || space == &kernel_space) return;
    paging_root_destroy(space->root_physical);
    kfree(space);
}

bool nova_address_space_switch(struct nova_address_space *space) {
    if (!space || !initialized) return false;
    return paging_root_switch(space->root_physical);
}

bool nova_address_space_self_test(void) {
    struct nova_address_space *a, *b;
    void *pa, *pb;
    uint64_t xa, xb, kernel_root;
    const uint64_t test_va = 0x0000000000600000ULL;
    if (!initialized) return false;
    kernel_root = kernel_space.root_physical;
    a = nova_address_space_create(); b = nova_address_space_create();
    pa = pmm_alloc_page(); pb = pmm_alloc_page();
    if (!a || !b || !pa || !pb || a->root_physical == b->root_physical ||
        !paging_root_map_page(a->root_physical, test_va, (uint64_t)(uintptr_t)pa,
                              NOVA_PAGE_USER | NOVA_PAGE_WRITABLE) ||
        !paging_root_map_page(b->root_physical, test_va, (uint64_t)(uintptr_t)pb,
                              NOVA_PAGE_USER | NOVA_PAGE_WRITABLE) ||
        !paging_root_translate(a->root_physical, test_va, &xa) ||
        !paging_root_translate(b->root_physical, test_va, &xb) ||
        xa != (uint64_t)(uintptr_t)pa || xb != (uint64_t)(uintptr_t)pb ||
        !nova_address_space_switch(a) || paging_current_root() != a->root_physical ||
        !nova_address_space_switch(&kernel_space) || paging_current_root() != kernel_root) {
        if (pa) pmm_free_page(pa);
        if (pb) pmm_free_page(pb);
        nova_address_space_destroy(a);
        nova_address_space_destroy(b);
        return false;
    }
    pmm_free_page(pa); pmm_free_page(pb);
    nova_address_space_destroy(a); nova_address_space_destroy(b);
    return true;
}

void nova_address_space_init(void) {
    kernel_space.root_physical = paging_current_root();
    kernel_space.flags = NOVA_ADDRESS_SPACE_KERNEL_SHARED;
    kernel_space.references = 1;
    initialized = kernel_space.root_physical != 0;
}
