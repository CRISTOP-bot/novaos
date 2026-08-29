#ifndef NOVA_USER_REGION_H
#define NOVA_USER_REGION_H
#include <nova/address_space.h>
#define NOVA_USER_REGION_READ  (1ULL << 0)
#define NOVA_USER_REGION_WRITE (1ULL << 1)
#define NOVA_USER_REGION_EXEC  (1ULL << 2)
struct nova_user_region {
    uint64_t virtual_start;
    uint64_t length;
    uint64_t flags;
    uint64_t page_count;
    uint64_t *physical_pages;
    struct nova_user_region *next;
};
struct nova_user_region *nova_user_region_map(struct nova_address_space *space, uint64_t start, uint64_t length, uint64_t flags);
bool nova_user_region_unmap(struct nova_address_space *space, struct nova_user_region *region);
bool nova_user_region_contains(const struct nova_user_region *region, uint64_t address, uint64_t length);
struct nova_user_region *nova_user_stack_create(struct nova_address_space *space);
uint64_t nova_user_stack_initial_rsp(const struct nova_address_space *space);
void nova_user_region_cleanup(struct nova_address_space *space);
bool nova_user_region_self_test(void);
#endif
