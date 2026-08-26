#ifndef NOVA_SCHEDULER_H
#define NOVA_SCHEDULER_H
#include <nova/types.h>
bool scheduler_init(void);
void scheduler_yield(void);
bool scheduler_self_test(void);
#endif
