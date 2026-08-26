#include <nova/mm/paging.h>
#include <nova/mm/pmm.h>

#define ENTRY_ADDRESS_MASK 0x000ffffffffff000ULL
#define TABLE_ENTRIES 512ULL
#define PAGE_2M (2ULL * 1024ULL * 1024ULL)
#define PAGING_TEST_VA 0xffff900000000000ULL

static uint64_t root_phys;
static uint64_t hhdm;
static bool active;
extern char __kernel_start, __kernel_end;

static bool add_overflow(uint64_t a, uint64_t b) { return b > (~0ULL - a); }
static uint64_t *table(uint64_t physical) { if (add_overflow(hhdm, physical)) return NULL; return (uint64_t *)(uintptr_t)(hhdm + physical); }
static uint64_t index_for(uint64_t va, unsigned level) { return (va >> (12 + level * 9)) & 0x1ff; }
static uint64_t alloc_table(void) {
    void *page = pmm_alloc_page(); uint64_t physical = (uint64_t)(uintptr_t)page; uint64_t *memory;
    if (!page || add_overflow(hhdm, physical)) return 0;
    memory = table(physical); if (!memory) return 0;
    for (uint64_t i = 0; i < TABLE_ENTRIES; i++) memory[i] = 0;
    return physical;
}
static uint64_t next_table(uint64_t *parent, uint64_t index) {
    uint64_t entry = parent[index] & ENTRY_ADDRESS_MASK;
    if (!entry) { entry = alloc_table(); if (!entry) return 0; parent[index] = entry | NOVA_PAGE_PRESENT | NOVA_PAGE_WRITABLE; }
    return entry;
}

bool paging_map_page(uint64_t va, uint64_t pa, uint64_t flags) {
    uint64_t *pml4, *pdpt, *pd, *pt, p;
    if (!active || (va & (NOVA_PAGE_SIZE - 1)) || (pa & (NOVA_PAGE_SIZE - 1))) return false;
    pml4 = table(root_phys); if (!pml4) return false;
    p = next_table(pml4, index_for(va, 3)); if (!p) return false; pdpt = table(p);
    p = next_table(pdpt, index_for(va, 2)); if (!p) return false; pd = table(p);
    p = next_table(pd, index_for(va, 1)); if (!p) return false; pt = table(p);
    if (pt[index_for(va, 0)] & NOVA_PAGE_PRESENT) return false;
    pt[index_for(va, 0)] = (pa & ENTRY_ADDRESS_MASK) | (flags & (NOVA_PAGE_WRITABLE | NOVA_PAGE_USER | NOVA_PAGE_NO_EXECUTE)) | NOVA_PAGE_PRESENT;
    return true;
}
bool paging_unmap_page(uint64_t va) {
    uint64_t *pml4, *pdpt, *pd, *pt, p;
    if (!active || (va & (NOVA_PAGE_SIZE - 1))) return false;
    pml4 = table(root_phys); if (!pml4 || !(pml4[index_for(va,3)] & NOVA_PAGE_PRESENT)) return false; pdpt = table(pml4[index_for(va,3)] & ENTRY_ADDRESS_MASK);
    if (!(pdpt[index_for(va,2)] & NOVA_PAGE_PRESENT)) return false; pd = table(pdpt[index_for(va,2)] & ENTRY_ADDRESS_MASK);
    if (!(pd[index_for(va,1)] & NOVA_PAGE_PRESENT)) return false; pt = table(pd[index_for(va,1)] & ENTRY_ADDRESS_MASK);
    p = index_for(va,0); if (!(pt[p] & NOVA_PAGE_PRESENT)) return false; pt[p] = 0;
    __asm__ volatile("invlpg (%0)" :: "r"((void *)(uintptr_t)va) : "memory"); return true;
}
bool paging_translate(uint64_t va, uint64_t *pa) {
    uint64_t *pml4, *pdpt, *pd, *pt, p;
    if (!active || !pa) return false; pml4 = table(root_phys); if (!pml4) return false;
    if (!(pml4[index_for(va,3)] & NOVA_PAGE_PRESENT)) return false; pdpt = table(pml4[index_for(va,3)] & ENTRY_ADDRESS_MASK);
    if (!(pdpt[index_for(va,2)] & NOVA_PAGE_PRESENT)) return false; pd = table(pdpt[index_for(va,2)] & ENTRY_ADDRESS_MASK);
    if (!(pd[index_for(va,1)] & NOVA_PAGE_PRESENT)) return false; pt = table(pd[index_for(va,1)] & ENTRY_ADDRESS_MASK);
    p = pt[index_for(va,0)]; if (!(p & NOVA_PAGE_PRESENT)) return false; *pa = (p & ENTRY_ADDRESS_MASK) | (va & (NOVA_PAGE_SIZE - 1)); return true;
}

static bool map_range(uint64_t va, uint64_t pa, uint64_t length, uint64_t flags) {
    uint64_t end, v, p;
    if (!length || add_overflow(va, length) || add_overflow(pa, length)) return false;
    end = va + length; for (v = va & ~(NOVA_PAGE_SIZE - 1), p = pa & ~(NOVA_PAGE_SIZE - 1); v < end; v += NOVA_PAGE_SIZE, p += NOVA_PAGE_SIZE)
        if (!paging_map_page(v, p, flags)) return false;
    return true;
}

bool paging_init(const struct nova_boot_info *boot) {
    uint64_t i, start, end, kernel_start, kernel_end;
    if (!boot || !boot->hhdm_offset) return false; hhdm = boot->hhdm_offset;
    root_phys = alloc_table(); if (!root_phys) return false; active = true;
    /* Keep every physical region visible through the Limine HHDM. */
    for (i = 0; i < boot->memory_region_count && i < NOVA_MAX_MEMORY_REGIONS; i++) {
        if (!boot->memory_regions[i].length || add_overflow(boot->memory_regions[i].base, boot->memory_regions[i].length)) continue;
        start = boot->memory_regions[i].base & ~(NOVA_PAGE_SIZE - 1); end = (boot->memory_regions[i].base + boot->memory_regions[i].length + NOVA_PAGE_SIZE - 1) & ~(NOVA_PAGE_SIZE - 1);
        if (end > start && !map_range(hhdm + start, start, end - start, NOVA_PAGE_WRITABLE | NOVA_PAGE_NO_EXECUTE)) return false;
    }
    kernel_start = (uint64_t)(uintptr_t)&__kernel_start; kernel_end = (uint64_t)(uintptr_t)&__kernel_end;
    if (kernel_end <= kernel_start || !map_range(kernel_start, boot->kernel_physical_base, kernel_end - kernel_start, NOVA_PAGE_WRITABLE)) return false;
    __asm__ volatile("mov %0, %%cr3" :: "r"(root_phys) : "memory"); return true;
}
bool paging_self_test(void) {
    void *page = pmm_alloc_page(); uint64_t physical, translated; volatile uint64_t *memory;
    if (!page) return false; physical = (uint64_t)(uintptr_t)page;
    if (!paging_map_page(PAGING_TEST_VA, physical, NOVA_PAGE_WRITABLE | NOVA_PAGE_NO_EXECUTE)) return false;
    if (!paging_translate(PAGING_TEST_VA + 7, &translated) || translated != physical + 7) return false;
    memory = (volatile uint64_t *)(uintptr_t)PAGING_TEST_VA; *memory = 0x1122334455667788ULL;
    if (*memory != 0x1122334455667788ULL || !paging_unmap_page(PAGING_TEST_VA) || paging_translate(PAGING_TEST_VA, &translated)) return false;
    pmm_free_page(page); return true;
}
