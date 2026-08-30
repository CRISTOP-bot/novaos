#include <nova/process.h>
#include <nova/mm/heap.h>

static struct nova_process *processes;
static uint64_t next_pid = 1;
static struct nova_process *kernel_process;
static struct nova_process *current_process;

static uint64_t alloc_pid(void) { return next_pid++; }
static bool registered(struct nova_process *p) {
    struct nova_process *q;
    for (q = processes; q; q = q->next) if (q == p) return true;
    return false;
}

bool nova_process_init(void) {
    nova_address_space_init();
    kernel_process = nova_process_create();
    if (!kernel_process) return false;
    nova_address_space_destroy(kernel_process->address_space);
    kernel_process->address_space = nova_address_space_kernel();
    kernel_process->state = NOVA_PROCESS_RUNNING;
    current_process = kernel_process;
    return true;
}

struct nova_process *nova_process_create(void) {
    struct nova_process *p = kmalloc(sizeof(*p));
    if (!p) return NULL;
    p->task = kmalloc(sizeof(*p->task));
    if (!p->task) { kfree(p); return NULL; }
    p->address_space = nova_address_space_create();
    if (!p->address_space) { kfree(p->task); kfree(p); return NULL; }
    p->pid = alloc_pid();
    if (!p->pid) {
        nova_address_space_destroy(p->address_space);
        kfree(p->task); kfree(p); return NULL;
    }
    p->state = NOVA_PROCESS_READY;
    p->parent = current_process;
    p->task->id = p->pid;
    p->task->rsp = 0;
    p->task->state = 1;
    p->task->process = p;
    p->task->user.rflags = 0x202;
    p->task->user.cs = 0x1b;
    p->task->user.ss = 0x23;
    p->next = processes;
    processes = p;
    return p;
}

struct nova_process *nova_process_current(void) { return current_process; }
struct nova_process *nova_process_kernel(void) { return kernel_process; }

bool nova_process_activate(struct nova_process *p) {
    if (!p || !registered(p) || p->state == NOVA_PROCESS_TERMINATED) return false;
    if (current_process && current_process->state == NOVA_PROCESS_RUNNING)
        current_process->state = NOVA_PROCESS_READY;
    current_process = p;
    p->state = NOVA_PROCESS_RUNNING;
    return nova_address_space_switch(p->address_space);
}

bool nova_process_exit(struct nova_process *p, uint64_t status) {
    if (!p || !registered(p) || p->state == NOVA_PROCESS_TERMINATED) return false;
    p->state = NOVA_PROCESS_TERMINATED;
    p->task->state = 3;
    p->task->user.rax = status;
    if (current_process == p) {
        current_process = kernel_process;
        if (!nova_address_space_switch(kernel_process->address_space)) return false;
    }
    return true;
}

void nova_process_destroy(struct nova_process *p) {
    struct nova_process **q;
    if (!p || p == kernel_process || !registered(p)) return;
    for (q = &processes; *q && *q != p; q = &(*q)->next);
    if (*q == p) *q = p->next;
    nova_address_space_destroy(p->address_space);
    kfree(p->task);
    kfree(p);
}

bool process_self_test(void) {
    struct nova_process *a, *b, *c;
    if (!kernel_process || current_process != kernel_process ||
        !kernel_process->address_space) return false;
    a = nova_process_create();
    b = nova_process_create();
    if (!a || !b || a->pid == 0 || b->pid == 0 || a->pid == b->pid ||
        !a->task || !b->task || a->task->process != a ||
        b->task->process != b || a->parent != kernel_process ||
        !registered(a) || !registered(b)) return false;
    if (!nova_process_activate(a) || nova_process_current() != a ||
        a->task->user.cs != 0x1b || a->task->user.ss != 0x23) return false;
    if (!nova_process_exit(a, 7) || a->state != NOVA_PROCESS_TERMINATED ||
        a->task->state != 3 || a->task->user.rax != 7 ||
        nova_process_current() != kernel_process) return false;
    c = nova_process_create();
    if (!c || c->pid == 0 || !c->task || c->pid == a->pid ||
        c->pid == b->pid) return false;
    nova_process_destroy(a); nova_process_destroy(b); nova_process_destroy(c);
    return true;
}
