#include <nova/address_space.h>
#include <nova/mm/heap.h>

static struct nova_address_space kernel_space;
static bool initialized;

struct nova_address_space *nova_address_space_kernel(void) {
    return initialized ? &kernel_space : NULL;
}

struct nova_address_space *nova_address_space_create(void) {
    struct nova_address_space *space;
    if (!initialized) return NULL;
    space = kmalloc(sizeof(*space));
    if (!space) return NULL;
    space->root_physical = 0;
    space->flags = NOVA_ADDRESS_SPACE_KERNEL_SHARED;
    space->references = 1;
    return space;
}

void nova_address_space_destroy(struct nova_address_space *space) {
    if (!space || space == &kernel_space) return;
    kfree(space);
}

bool nova_address_space_switch(struct nova_address_space *space) {
    /* A CR3 switch is deliberately not claimed until paging owns per-process roots. */
    return space != NULL && initialized && (space == &kernel_space ||
           space->flags == NOVA_ADDRESS_SPACE_KERNEL_SHARED);
}

bool nova_address_space_self_test(void) {
    struct nova_address_space *a, *b;
    if (!initialized) return false;
    a = nova_address_space_create();
    b = nova_address_space_create();
    if (!a || !b || a == b || a->root_physical != b->root_physical ||
        !(a->flags & NOVA_ADDRESS_SPACE_KERNEL_SHARED) ||
        !nova_address_space_switch(a) || !nova_address_space_switch(b)) {
        nova_address_space_destroy(a);
        nova_address_space_destroy(b);
        return false;
    }
    nova_address_space_destroy(a);
    nova_address_space_destroy(b);
    return true;
}

void nova_address_space_init(void) {
    kernel_space.root_physical = 0;
    kernel_space.flags = NOVA_ADDRESS_SPACE_KERNEL_SHARED;
    kernel_space.references = 1;
    initialized = true;
}
