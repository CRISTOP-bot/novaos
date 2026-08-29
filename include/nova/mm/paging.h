#ifndef NOVA_MM_PAGING_H
#define NOVA_MM_PAGING_H
#include <nova/boot.h>
#define NOVA_PAGE_PRESENT (1ULL << 0)
#define NOVA_PAGE_WRITABLE (1ULL << 1)
#define NOVA_PAGE_USER (1ULL << 2)
#define NOVA_PAGE_NO_EXECUTE (1ULL << 63)
struct nova_page_info { bool present; bool user; bool writable; bool executable; };
bool paging_init(const struct nova_boot_info *boot_info);
bool paging_map_page(uint64_t virtual_address, uint64_t physical_address, uint64_t flags);
bool paging_unmap_page(uint64_t virtual_address);
bool paging_translate(uint64_t virtual_address, uint64_t *physical_address);
bool paging_translate_info(uint64_t virtual_address, uint64_t *physical_address, struct nova_page_info *info);
bool paging_self_test(void);
uint64_t paging_map_count(void);
uint64_t paging_unmap_count(void);
uint64_t paging_current_root(void);
bool paging_root_create(uint64_t *root_physical);
void paging_root_destroy(uint64_t root_physical);
bool paging_root_switch(uint64_t root_physical);
bool paging_root_map_page(uint64_t root_physical, uint64_t virtual_address, uint64_t physical_address, uint64_t flags);
bool paging_root_translate(uint64_t root_physical, uint64_t virtual_address, uint64_t *physical_address);
bool paging_root_translate_info(uint64_t root_physical, uint64_t virtual_address, uint64_t *physical_address, struct nova_page_info *info);
#endif
