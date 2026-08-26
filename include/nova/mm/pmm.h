#ifndef NOVA_MM_PMM_H
#define NOVA_MM_PMM_H
#include <nova/boot.h>
#define NOVA_PAGE_SIZE 4096ULL
void pmm_init(const struct nova_boot_info *boot_info);
void *pmm_alloc_page(void);
void pmm_free_page(void *address);
uint64_t pmm_total_pages(void);
uint64_t pmm_used_pages(void);
uint64_t pmm_free_pages(void);
uint64_t pmm_alloc_count(void);
uint64_t pmm_free_count(void);
uint64_t pmm_failed_alloc_count(void);
bool pmm_self_test(const struct nova_boot_info *boot_info);
#endif
