#include <nova/syscall.h>
#include <nova/console.h>
#include <nova/mm/paging.h>
#include <nova/process.h>

static volatile bool exit_seen, getpid_seen, write_seen, unknown_seen, badptr_seen;
static bool valid_user(uint64_t p, uint64_t n) {
    if (!n || p > 0x00007fffffffffffULL || n > 0x1000 ||
        p > 0x00007fffffffffffULL - n) return false;
    for (uint64_t x = p & ~0xfffULL, end = p + n; x < end; x += 0x1000) {
        uint64_t pa;
        if (!paging_translate(x, &pa)) {
            console_printf("[NovaOS] syscall user range unmapped: p=%x len=%x va=%x\n", p, n, x);
            return false;
        }
    }
    return true;
}

int64_t syscall_dispatch(uint64_t n, uint64_t a1, uint64_t a2, uint64_t a3) {
    struct nova_process *current;
    switch (n) {
    case NOVA_SYS_GETPID:
        current = nova_process_current();
        getpid_seen = true;
        return current ? (int64_t)current->pid : NOVA_SYS_EINVAL;
    case NOVA_SYS_WRITE:
        if (a1 != 1) return NOVA_SYS_EBADF;
        if (!valid_user(a2, a3)) { badptr_seen = true; return NOVA_SYS_EFAULT; }
        console_write("NovaOS syscall write PASS\n");
        write_seen = true;
        return (int64_t)a3;
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
bool syscall_self_test(void) {
    console_printf("[NovaOS] syscall flags pid=%x write=%x unknown=%x badptr=%x exit=%x\n",
        getpid_seen, write_seen, unknown_seen, badptr_seen, exit_seen);
    return getpid_seen && write_seen && unknown_seen && badptr_seen && exit_seen;
}
