#include <nova/mm/pmm.h>

/* Fixed, kernel-owned bitmap: 1 bit per page, covering physical memory below 64 GiB. */
#define PMM_MAX_PAGES (1ULL << 24)
#define PMM_BITMAP_BYTES (PMM_MAX_PAGES / 8ULL)
static uint8_t bitmap[PMM_BITMAP_BYTES];
extern char __kernel_start, __kernel_end;
static uint64_t total_pages_count, used_pages_count;
static uint64_t alloc_count_value, free_count_value, failed_alloc_count_value;
static bool initialized;

static bool add_overflows(uint64_t a, uint64_t b) { return b > (~0ULL - a); }
static uint64_t page_index(uint64_t address) { return address / NOVA_PAGE_SIZE; }
static void bitmap_set(uint64_t i) { bitmap[i >> 3] |= (uint8_t)(1U << (i & 7)); }
static void bitmap_clear(uint64_t i) { bitmap[i >> 3] &= (uint8_t)~(1U << (i & 7)); }
static bool bitmap_test(uint64_t i) { return (bitmap[i >> 3] & (uint8_t)(1U << (i & 7))) != 0; }
static void mark_used(uint64_t i) { if (!bitmap_test(i)) { bitmap_set(i); used_pages_count++; } }
static bool align_up(uint64_t value, uint64_t *out) {
    uint64_t add = NOVA_PAGE_SIZE - 1;
    if (add_overflows(value, add)) return false;
    *out = (value + add) & ~(NOVA_PAGE_SIZE - 1);
    return true;
}
static void release_range(uint64_t base, uint64_t length) {
    uint64_t end, first, last, i;
    if (!length || add_overflows(base, length) || !align_up(base, &first)) return;
    end = base + length;
    last = end & ~(NOVA_PAGE_SIZE - 1);
    if (first >= last) return;
    first = page_index(first); last = page_index(last);
    if (first >= PMM_MAX_PAGES) return;
    if (last > PMM_MAX_PAGES) last = PMM_MAX_PAGES;
    for (i = first; i < last; i++) if (bitmap_test(i)) { bitmap_clear(i); total_pages_count++; }
}
static void reserve_range(uint64_t base, uint64_t length) {
    uint64_t end, first, last, i;
    if (!length || add_overflows(base, length) || !align_up(base, &first)) return;
    end = base + length; last = end & ~(NOVA_PAGE_SIZE - 1);
    if (first >= last || page_index(first) >= PMM_MAX_PAGES) return;
    first = page_index(first); last = page_index(last); if (last > PMM_MAX_PAGES) last = PMM_MAX_PAGES;
    for (i = first; i < last; i++) mark_used(i);
}

void pmm_init(const struct nova_boot_info *boot) {
    uint64_t i, kernel_start, kernel_end;
    for (i = 0; i < PMM_BITMAP_BYTES; i++) bitmap[i] = 0xff;
    total_pages_count = 0; used_pages_count = 0; alloc_count_value = 0; free_count_value = 0; failed_alloc_count_value = 0;
    if (!boot) { initialized = false; return; }
    for (i = 0; i < boot->memory_region_count && i < NOVA_MAX_MEMORY_REGIONS; i++)
        if (boot->memory_regions[i].type == NOVA_MEM_USABLE)
            release_range(boot->memory_regions[i].base, boot->memory_regions[i].length);
    /* Limine gives both bases. Reserve the linked kernel image in physical space. */
    if (boot->kernel_virtual_base && boot->kernel_physical_base &&
        (uint64_t)(uintptr_t)&__kernel_start >= boot->kernel_virtual_base &&
        (uint64_t)(uintptr_t)&__kernel_end >= (uint64_t)(uintptr_t)&__kernel_start) {
        uint64_t start_offset = (uint64_t)(uintptr_t)&__kernel_start - boot->kernel_virtual_base;
        uint64_t end_offset = (uint64_t)(uintptr_t)&__kernel_end - boot->kernel_virtual_base;
        if (add_overflows(boot->kernel_physical_base, start_offset) || add_overflows(boot->kernel_physical_base, end_offset)) return;
        kernel_start = boot->kernel_physical_base + start_offset;
        kernel_end = boot->kernel_physical_base + end_offset;
        if (kernel_end >= kernel_start) reserve_range(kernel_start, kernel_end - kernel_start);
    }
    initialized = true;
}
void *pmm_alloc_page(void) {
    uint64_t i;
    if (!initialized) { failed_alloc_count_value++; return NULL; }
    for (i = 0; i < PMM_MAX_PAGES; i++) if (!bitmap_test(i)) { bitmap_set(i); used_pages_count++; alloc_count_value++; return (void *)(uintptr_t)(i * NOVA_PAGE_SIZE); }
    failed_alloc_count_value++; return NULL;
}
void pmm_free_page(void *address) {
    uint64_t a = (uint64_t)(uintptr_t)address, i;
    if (!initialized || (a & (NOVA_PAGE_SIZE - 1)) || a >= PMM_MAX_PAGES * NOVA_PAGE_SIZE) return;
    i = page_index(a); if (bitmap_test(i)) { bitmap_clear(i); if (used_pages_count) used_pages_count--; free_count_value++; }
}
uint64_t pmm_total_pages(void) { return total_pages_count; }
uint64_t pmm_used_pages(void) { return used_pages_count; }
uint64_t pmm_free_pages(void) { return total_pages_count - used_pages_count; }
uint64_t pmm_alloc_count(void) { return alloc_count_value; }
uint64_t pmm_free_count(void) { return free_count_value; }
uint64_t pmm_failed_alloc_count(void) { return failed_alloc_count_value; }

bool pmm_self_test(const struct nova_boot_info *boot) {
    void *pages[4]; uint64_t i, physical, virtual, *word;
    for (i = 0; i < 4; i++) { pages[i] = pmm_alloc_page(); if (!pages[i]) return false; if (((uint64_t)(uintptr_t)pages[i] & (NOVA_PAGE_SIZE - 1)) != 0) return false; if (i && pages[i] == pages[i-1]) return false; }
    physical = (uint64_t)(uintptr_t)pages[0];
    if (add_overflows(boot->hhdm_offset, physical)) return false;
    virtual = boot->hhdm_offset + physical; word = (uint64_t *)(uintptr_t)virtual; *word = 0xAAAAAAAAAAAAAAAAULL; if (*word != 0xAAAAAAAAAAAAAAAAULL) return false;
    for (i = 0; i < 4; i++) pmm_free_page(pages[i]);
    return pmm_alloc_page() != NULL;
}
extern char __kernel_start, __kernel_end;
