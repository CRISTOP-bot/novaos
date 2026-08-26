#ifndef NOVA_SYSCALL_H
#define NOVA_SYSCALL_H
#include <nova/types.h>
#define NOVA_SYS_GETPID 1ULL
#define NOVA_SYS_WRITE 2ULL
#define NOVA_SYS_EXIT 3ULL
#define NOVA_SYS_ENOSYS (-1LL)
#define NOVA_SYS_EBADF (-2LL)
#define NOVA_SYS_EFAULT (-3LL)
#define NOVA_SYS_EINVAL (-4LL)
int64_t syscall_dispatch(uint64_t,uint64_t,uint64_t,uint64_t);
void syscall_interrupt(uint64_t *saved);
bool syscall_self_test(void);
bool syscall_exit_seen(void);
#endif
