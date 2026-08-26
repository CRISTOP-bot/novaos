#ifndef NOVA_PROCESS_H
#define NOVA_PROCESS_H
#include <nova/types.h>
struct nova_task { uint64_t id, rsp, state; uint8_t stack[4096] __attribute__((aligned(16))); };
struct nova_process { uint64_t pid, state; struct nova_task *task; struct nova_process *next; };
#define NOVA_PROCESS_NEW 1ULL
#define NOVA_PROCESS_READY 2ULL
#define NOVA_PROCESS_RUNNING 3ULL
#define NOVA_PROCESS_TERMINATED 4ULL
bool nova_process_init(void);
struct nova_process *nova_process_create(void);
bool nova_process_exit(struct nova_process *process);
void nova_process_destroy(struct nova_process *process);
bool process_self_test(void);
#endif
