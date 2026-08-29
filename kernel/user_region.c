#include <nova/user_region.h>
#include <nova/mm/paging.h>
#include <nova/mm/pmm.h>
#include <nova/mm/heap.h>

#define USER_LIMIT 0x00007fffffffffffULL
#define STACK_BASE 0x00007fff00000000ULL
#define STACK_PAGES 8ULL

static bool range_ok(uint64_t start, uint64_t length) {
    return length && !(start & (NOVA_PAGE_SIZE - 1)) && !(length & (NOVA_PAGE_SIZE - 1)) &&
           start <= USER_LIMIT && length <= USER_LIMIT && start <= USER_LIMIT - (length - 1);
}

bool nova_user_region_contains(const struct nova_user_region *r, uint64_t address, uint64_t length) {
    if (!r || !length || address < r->virtual_start || address > USER_LIMIT ||
        length > r->virtual_start + r->length - address) return false;
    return true;
}

struct nova_user_region *nova_user_region_map(struct nova_address_space *space, uint64_t start, uint64_t length, uint64_t flags) {
    struct nova_user_region *r, *q;
    uint64_t i, phys, page_flags = NOVA_PAGE_USER;
    if (!space || !range_ok(start, length) || !(flags & NOVA_USER_REGION_READ) ||
        (flags & ~(NOVA_USER_REGION_READ | NOVA_USER_REGION_WRITE | NOVA_USER_REGION_EXEC))) return NULL;
    for (q = space->regions; q; q = q->next)
        if (start < q->virtual_start + q->length && q->virtual_start < start + length) return NULL;
    r = kmalloc(sizeof(*r)); if (!r) return NULL;
    r->physical_pages = kmalloc(sizeof(uint64_t) * (length / NOVA_PAGE_SIZE));
    if (!r->physical_pages) { kfree(r); return NULL; }
    r->virtual_start = start; r->length = length; r->flags = flags; r->page_count = length / NOVA_PAGE_SIZE;
    if (flags & NOVA_USER_REGION_WRITE) page_flags |= NOVA_PAGE_WRITABLE;
    if (!(flags & NOVA_USER_REGION_EXEC)) page_flags |= NOVA_PAGE_NO_EXECUTE;
    for (i = 0; i < r->page_count; i++) {
        void *page = pmm_alloc_page();
        if (!page) goto fail;
        phys = (uint64_t)(uintptr_t)page; r->physical_pages[i] = phys;
        if (!paging_root_map_page(space->root_physical, start + i * NOVA_PAGE_SIZE, phys, page_flags)) { pmm_free_page(page); goto fail; }
    }
    r->next = space->regions; space->regions = r; return r;
fail:
    while (i) { --i; paging_root_unmap_page(space->root_physical, start + i * NOVA_PAGE_SIZE); pmm_free_page((void *)(uintptr_t)r->physical_pages[i]); }
    kfree(r->physical_pages); kfree(r); return NULL;
}

bool nova_user_region_unmap(struct nova_address_space *space, struct nova_user_region *region) {
    struct nova_user_region **p;
    uint64_t i;
    if (!space || !region) return false;
    for (p = &space->regions; *p && *p != region; p = &(*p)->next);
    if (!*p) return false;
    *p = region->next;
    for (i = 0; i < region->page_count; i++) { paging_root_unmap_page(space->root_physical, region->virtual_start + i * NOVA_PAGE_SIZE); pmm_free_page((void *)(uintptr_t)region->physical_pages[i]); }
    if (space->user_stack == region) { space->user_stack = NULL; space->initial_rsp = 0; }
    kfree(region->physical_pages); kfree(region); return true;
}

struct nova_user_region *nova_user_stack_create(struct nova_address_space *space) {
    struct nova_user_region *r = nova_user_region_map(space, STACK_BASE, STACK_PAGES * NOVA_PAGE_SIZE, NOVA_USER_REGION_READ | NOVA_USER_REGION_WRITE);
    if (r) { space->user_stack = r; space->initial_rsp = STACK_BASE + r->length - 16; }
    return r;
}
uint64_t nova_user_stack_initial_rsp(const struct nova_address_space *space) { return space ? space->initial_rsp : 0; }
void nova_user_region_cleanup(struct nova_address_space *space) { while (space && space->regions) nova_user_region_unmap(space, space->regions); }

bool nova_user_region_self_test(void) {
    struct nova_address_space *space = nova_address_space_create();
    struct nova_user_region *stack, *code, *data;
    uint64_t pa, rsp; struct nova_page_info info;
    bool ok = false;
    if (!space) return false;
    stack = nova_user_stack_create(space);
    code = nova_user_region_map(space, 0x500000, NOVA_PAGE_SIZE, NOVA_USER_REGION_READ | NOVA_USER_REGION_EXEC);
    data = nova_user_region_map(space, 0x510000, NOVA_PAGE_SIZE, NOVA_USER_REGION_READ | NOVA_USER_REGION_WRITE);
    rsp = nova_user_stack_initial_rsp(space);
    if (stack && code && data && nova_user_region_contains(stack, rsp - 1, 1) &&
        !nova_user_region_contains(stack, STACK_BASE - 1, 1) && rsp > STACK_BASE &&
        paging_root_translate_info(space->root_physical, rsp - 1, &pa, &info) && info.user && info.writable && !info.executable &&
        paging_root_translate_info(space->root_physical, 0x500000, &pa, &info) && info.user && !info.writable && info.executable &&
        paging_root_translate_info(space->root_physical, 0x510000, &pa, &info) && info.user && info.writable && !info.executable) ok = true;
    nova_address_space_destroy(space);
    return ok;
}
