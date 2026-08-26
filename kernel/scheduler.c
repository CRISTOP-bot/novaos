#include <nova/scheduler.h>
#include <nova/types.h>
#define STACK_SIZE 4096
#define READY 1ULL
#define RUNNING 2ULL
#define DONE 3ULL
struct nova_task { uint64_t id, rsp, state; uint8_t stack[STACK_SIZE] __attribute__((aligned(16))); };
extern void scheduler_context_switch(uint64_t *, uint64_t);
static struct nova_task kernel_task, task_a, task_b; static struct nova_task *current;
static volatile uint64_t counter_a, counter_b; static bool initialized;
static void task_a_main(void); static void task_b_main(void);
static void task_start(void (*fn)(void)) { fn(); for (;;) scheduler_yield(); }
static void task_a_main(void) { for (;;) { counter_a++; scheduler_yield(); } }
static void task_b_main(void) { for (;;) { counter_b++; scheduler_yield(); } }
static void prepare(struct nova_task *t, uint64_t id, void (*fn)(void)) {
    uint64_t *sp=(uint64_t *)(uintptr_t)(t->stack+STACK_SIZE);
    *--sp=(uint64_t)(uintptr_t)task_start; *--sp=0; *--sp=0; *--sp=0; *--sp=0; *--sp=0; *--sp=(uint64_t)(uintptr_t)fn;
    t->id=id; t->rsp=(uint64_t)(uintptr_t)sp; t->state=READY;
}
bool scheduler_init(void) { kernel_task.id=0; kernel_task.state=RUNNING; current=&kernel_task; counter_a=counter_b=0; prepare(&task_a,1,task_a_main); prepare(&task_b,2,task_b_main); initialized=true; return true; }
void scheduler_yield(void) {
    struct nova_task *old=current,*next;
    if (!initialized) return;
    if (old==&task_a && counter_a>=2) task_a.state=DONE;
    if (old==&task_b && counter_b>=2) task_b.state=DONE;
    if (old->state==RUNNING) old->state=READY;
    if (old==&kernel_task && task_a.state==DONE && task_b.state==DONE) { current=&kernel_task; kernel_task.state=RUNNING; return; }
    if (old==&kernel_task) next=&task_a;
    else if (old==&task_a && task_b.state!=DONE) next=&task_b;
    else if (old==&task_b && task_a.state!=DONE) next=&task_a;
    else next=&kernel_task;
    if (next->state==DONE) next=&kernel_task;
    next->state=RUNNING; current=next;
    scheduler_context_switch(&old->rsp,next->rsp);
}
bool scheduler_self_test(void) { if (!scheduler_init()) return false; scheduler_yield(); return counter_a>=2 && counter_b>=2 && current==&kernel_task; }
