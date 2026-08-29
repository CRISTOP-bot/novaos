#ifndef NOVA_USER_MEMORY_H
#define NOVA_USER_MEMORY_H
#include <nova/types.h>
#define NOVA_WRITE_MAX 128ULL
bool nova_copy_from_user(void *destination, const void *source, size_t length);
bool nova_user_memory_self_test(void);
#endif
