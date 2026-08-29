#ifndef NOVA_SYSCALL_H
#define NOVA_SYSCALL_H
#include <nova/types.h>
#include <nova/vfs.h>
#define NOVA_SYS_GETPID 1ULL
#define NOVA_SYS_WRITE 2ULL
#define NOVA_SYS_EXIT 3ULL
#define NOVA_SYS_OPEN 4ULL
#define NOVA_SYS_CLOSE 5ULL
#define NOVA_SYS_READ 6ULL
#define NOVA_SYS_LSEEK 7ULL
#define NOVA_SYS_STAT 8ULL
#define NOVA_SYS_ENOSYS NOVA_VFS_ENOSYS
#define NOVA_SYS_EBADF NOVA_VFS_EBADF
#define NOVA_SYS_EFAULT NOVA_VFS_EFAULT
#define NOVA_SYS_EINVAL NOVA_VFS_EINVAL
#define NOVA_SYS_ENOENT NOVA_VFS_ENOENT
#define NOVA_SYS_EEXIST NOVA_VFS_EEXIST
#define NOVA_SYS_ENOMEM NOVA_VFS_ENOMEM
int64_t syscall_dispatch(uint64_t,uint64_t,uint64_t,uint64_t);
void syscall_interrupt(uint64_t *saved);
bool syscall_self_test(void);
bool syscall_exit_seen(void);
void syscall_reset_test_state(void);
#endif
