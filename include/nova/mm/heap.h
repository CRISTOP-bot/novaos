#ifndef NOVA_MM_HEAP_H
#define NOVA_MM_HEAP_H
#include <nova/types.h>

#define NOVA_HEAP_ALIGNMENT 16ULL

bool heap_init(void);
void *kmalloc(size_t size);
void kfree(void *ptr);
void *kcalloc(size_t count, size_t size);
bool heap_self_test(void);
uint64_t heap_bytes_used(void);
uint64_t heap_bytes_mapped(void);
#endif
