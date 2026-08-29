#ifndef NOVA_ADDRESS_SPACE_H
#define NOVA_ADDRESS_SPACE_H
#include <nova/types.h>

/* M2.3 owns the object, but paging still has one kernel root. */
#define NOVA_ADDRESS_SPACE_KERNEL_SHARED 1ULL

struct nova_address_space {
    uint64_t root_physical;
    uint64_t flags;
    uint64_t references;
};

void nova_address_space_init(void);
struct nova_address_space *nova_address_space_kernel(void);
struct nova_address_space *nova_address_space_create(void);
void nova_address_space_destroy(struct nova_address_space *space);
bool nova_address_space_switch(struct nova_address_space *space);
bool nova_address_space_self_test(void);
#endif
