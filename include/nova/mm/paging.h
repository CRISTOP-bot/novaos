#ifndef NOVA_MM_PAGING_H
#define NOVA_MM_PAGING_H
#include <nova/boot.h>
#define NOVA_PAGE_PRESENT (1ULL << 0)
#define NOVA_PAGE_WRITABLE (1ULL << 1)
#define NOVA_PAGE_USER (1ULL << 2)
#define NOVA_PAGE_NO_EXECUTE (1ULL << 63)
bool paging_init(const struct nova_boot_info *boot_info);
bool paging_map_page(uint64_t virtual_address, uint64_t physical_address, uint64_t flags);
bool paging_unmap_page(uint64_t virtual_address);
bool paging_translate(uint64_t virtual_address, uint64_t *physical_address);
bool paging_self_test(void);
uint64_t paging_map_count(void);
uint64_t paging_unmap_count(void);
#endif
