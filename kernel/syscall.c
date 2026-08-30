#include <nova/syscall.h>
#include <nova/console.h>
#include <nova/process.h>
#include <nova/user_memory.h>
#include <nova/input.h>

static volatile bool exit_seen, getpid_seen, write_seen, read_seen, unknown_seen, badptr_seen;

int64_t syscall_dispatch(uint64_t n, uint64_t a1, uint64_t a2, uint64_t a3) {
    struct nova_process *current;
    uint8_t buffer[NOVA_WRITE_MAX + 1];
    switch (n) {
    case NOVA_SYS_GETPID:
        current = nova_process_current();
        getpid_seen = true;
        return current ? (int64_t)current->pid : NOVA_SYS_EINVAL;
    case NOVA_SYS_WRITE:
        if (a1 != 1 || !a3 || a3 > NOVA_WRITE_MAX) return a1 != 1 ? NOVA_SYS_EBADF : NOVA_SYS_EINVAL;
        if (!nova_copy_from_user(buffer, (const void *)(uintptr_t)a2, (size_t)a3)) {
            badptr_seen = true;
            return NOVA_SYS_EFAULT;
        }
        buffer[a3] = 0;
        console_write((const char *)buffer);
        write_seen = true;
        return (int64_t)a3;
    case NOVA_SYS_READ: {
        uint8_t readbuf[NOVA_READ_MAX]; uint64_t count = 0; char c;
        if (a1 != 0) return NOVA_SYS_EBADF;
        if (!a3 || a3 > NOVA_READ_MAX) return NOVA_SYS_EINVAL;
        while (count < a3 && nova_input_pop(&c)) readbuf[count++] = (uint8_t)c;
        if (count && !nova_copy_to_user((void *)(uintptr_t)a2, readbuf, count)) return NOVA_SYS_EFAULT;
        read_seen = true; return (int64_t)count;
    }
    case NOVA_SYS_EXIT:
        current = nova_process_current();
        if (!current || !nova_process_exit(current, a1)) return NOVA_SYS_EINVAL;
        exit_seen = true;
        return 0;
    default:
        unknown_seen = true;
        return NOVA_SYS_ENOSYS;
    }
}

void syscall_interrupt(uint64_t *s) {
    /* isr128 pushes r15..r8, rdi, rsi, rbp..rax; see interrupts/entry.S. */
    s[0] = (uint64_t)syscall_dispatch(s[0], s[6], s[5], s[2]);
}
bool syscall_exit_seen(void) { return exit_seen; }
bool syscall_read_seen(void) { return read_seen; }
void syscall_reset_test_state(void) { exit_seen = false; getpid_seen = false; write_seen = false; unknown_seen = false; badptr_seen = false; }
bool syscall_self_test(void) {
    console_printf("[NovaOS] syscall flags pid=%x write=%x unknown=%x badptr=%x exit=%x\n",
        getpid_seen, write_seen, unknown_seen, badptr_seen, exit_seen);
    return getpid_seen && write_seen && unknown_seen && badptr_seen && exit_seen;
}
