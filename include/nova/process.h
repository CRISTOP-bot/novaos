#ifndef NOVA_PROCESS_H
#define NOVA_PROCESS_H
#include <nova/types.h>
#include <nova/address_space.h>

struct nova_user_context {
    uint64_t rip, rsp, rflags;
    uint64_t cs, ss;
    uint64_t rax, rbx, rcx, rdx, rsi, rdi;
    uint64_t rbp, r8, r9, r10, r11, r12, r13, r14, r15;
};

struct nova_process;
struct nova_task {
    uint64_t id, rsp, state;
    struct nova_process *process;
    struct nova_user_context user;
    uint8_t stack[4096] __attribute__((aligned(16)));
};
struct nova_process {
    uint64_t pid, state;
    struct nova_address_space *address_space;
    struct nova_process *parent;
    struct nova_task *task;
    struct nova_process *next;
};
#define NOVA_PROCESS_NEW 1ULL
#define NOVA_PROCESS_READY 2ULL
#define NOVA_PROCESS_RUNNING 3ULL
#define NOVA_PROCESS_TERMINATED 4ULL

bool nova_process_init(void);
struct nova_process *nova_process_create(void);
struct nova_process *nova_process_current(void);
struct nova_process *nova_process_kernel(void);
bool nova_process_activate(struct nova_process *process);
bool nova_process_exit(struct nova_process *process, uint64_t status);
void nova_process_destroy(struct nova_process *process);
bool process_self_test(void);
#endif
