#ifndef NOVA_MM_DIAGNOSTICS_H
#define NOVA_MM_DIAGNOSTICS_H
#include <nova/types.h>
struct nova_memory_stats {
    uint64_t pmm_total_pages, pmm_used_pages, pmm_free_pages;
    uint64_t pmm_allocations, pmm_frees, pmm_failed_allocations;
    uint64_t paging_mappings, paging_unmappings;
    uint64_t heap_allocations, heap_frees, heap_active_allocations;
    uint64_t heap_bytes_used, heap_bytes_mapped, heap_failed_allocations;
};
void memory_diagnostics_snapshot(struct nova_memory_stats *out);
void memory_diagnostics_print(void);
bool memory_diagnostics_self_test(void);
#endif
