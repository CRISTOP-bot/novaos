#include <nova/mm/diagnostics.h>
#include <nova/mm/pmm.h>
#include <nova/mm/paging.h>
#include <nova/mm/heap.h>
#include <nova/console.h>

void memory_diagnostics_snapshot(struct nova_memory_stats *out) {
    if (!out) return;
    out->pmm_total_pages = pmm_total_pages(); out->pmm_used_pages = pmm_used_pages(); out->pmm_free_pages = pmm_free_pages();
    out->pmm_allocations = pmm_alloc_count(); out->pmm_frees = pmm_free_count(); out->pmm_failed_allocations = pmm_failed_alloc_count();
    out->paging_mappings = paging_map_count(); out->paging_unmappings = paging_unmap_count();
    out->heap_allocations = heap_alloc_count(); out->heap_frees = heap_free_count(); out->heap_active_allocations = heap_active_allocations();
    out->heap_bytes_used = heap_bytes_used(); out->heap_bytes_mapped = heap_bytes_mapped(); out->heap_failed_allocations = heap_failed_alloc_count();
}
void memory_diagnostics_print(void) {
    struct nova_memory_stats s; memory_diagnostics_snapshot(&s);
    console_write("[NovaOS] Memory diagnostics\n[NovaOS] PMM:\n");
    console_printf("  total pages: %x\n  used pages: %x\n  free pages: %x\n  allocations: %x\n  frees: %x\n  failed allocations: %x\n",s.pmm_total_pages,s.pmm_used_pages,s.pmm_free_pages,s.pmm_allocations,s.pmm_frees,s.pmm_failed_allocations);
    console_write("[NovaOS] Paging:\n"); console_printf("  mappings: %x\n  unmappings: %x\n",s.paging_mappings,s.paging_unmappings);
    console_write("[NovaOS] Heap:\n"); console_printf("  allocations: %x\n  frees: %x\n  active allocations: %x\n  bytes used: %x\n  bytes mapped: %x\n  failed allocations: %x\n",s.heap_allocations,s.heap_frees,s.heap_active_allocations,s.heap_bytes_used,s.heap_bytes_mapped,s.heap_failed_allocations);
}
static bool diag_fail(uint64_t code) { console_printf("[NovaOS] diagnostics checkpoint failed: %x\n", code); return false; }
bool memory_diagnostics_self_test(void) {
    struct nova_memory_stats before, after; void *page, *a, *b, *c; uint64_t physical, translated, map_before, unmap_before, active_before, used_before;
    memory_diagnostics_snapshot(&before); used_before = before.pmm_used_pages; map_before = before.paging_mappings; unmap_before = before.paging_unmappings;
    page = pmm_alloc_page(); if (!page || pmm_alloc_count() != before.pmm_allocations + 1 || pmm_free_pages() + 1 != before.pmm_free_pages) return diag_fail(1);
    physical = (uint64_t)(uintptr_t)page;
    if (!paging_map_page(0xffff900200000000ULL, physical, NOVA_PAGE_WRITABLE | NOVA_PAGE_NO_EXECUTE)) return diag_fail(2);
    if (paging_map_count() != map_before + 1 || !paging_translate(0xffff900200000000ULL, &translated) || translated != physical) return diag_fail(3);
    if (!paging_unmap_page(0xffff900200000000ULL) || paging_unmap_count() != unmap_before + 1 || paging_translate(0xffff900200000000ULL, &translated)) return diag_fail(4);
    pmm_free_page(page); if (pmm_free_count() != before.pmm_frees + 1 || pmm_used_pages() != used_before) return diag_fail(5);
    active_before = before.heap_active_allocations; a=kmalloc(24); b=kmalloc(128); c=kmalloc(2048); if (!a||!b||!c) return diag_fail(6);
    if (heap_alloc_count() != before.heap_allocations + 3 || heap_active_allocations() != active_before + 3 || heap_bytes_used() <= before.heap_bytes_used) return diag_fail(7);
    kfree(a); kfree(b); kfree(c); memory_diagnostics_snapshot(&after);
    if (after.heap_active_allocations != active_before || after.heap_bytes_used != before.heap_bytes_used) return diag_fail(8);
    if (after.heap_frees < before.heap_frees + 3) return diag_fail(9);
    if (kmalloc(0) != NULL) return diag_fail(10);
    kfree(NULL); kfree((void *)(uintptr_t)0x1234); kfree(a);
    return true;
}
