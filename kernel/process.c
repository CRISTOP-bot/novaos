#include <nova/process.h>
#include <nova/mm/heap.h>
static struct nova_process *processes;
static uint64_t next_pid=1;
static struct nova_process *kernel_process;
static uint64_t alloc_pid(void) { return next_pid++; }
static bool registered(struct nova_process *p) { struct nova_process *q; for(q=processes;q;q=q->next) if(q==p) return true; return false; }
bool nova_process_init(void) { kernel_process=nova_process_create(); if(!kernel_process) return false; kernel_process->state=NOVA_PROCESS_RUNNING; return true; }
struct nova_process *nova_process_create(void) { struct nova_process *p=kmalloc(sizeof(*p)); if(!p) return NULL; p->task=kmalloc(sizeof(*p->task)); if(!p->task){kfree(p);return NULL;} p->pid=alloc_pid(); if(!p->pid){kfree(p->task);kfree(p);return NULL;} p->state=NOVA_PROCESS_READY; p->task->id=p->pid; p->task->rsp=0; p->task->state=1; p->next=processes; processes=p; return p; }
bool nova_process_exit(struct nova_process *p) { if(!p||!registered(p)||p==kernel_process) return false; p->state=NOVA_PROCESS_TERMINATED; p->task->state=3; return true; }
void nova_process_destroy(struct nova_process *p) { struct nova_process **q; if(!p||p==kernel_process||!registered(p)) return; for(q=&processes;*q && *q!=p;q=&(*q)->next); if(*q==p)*q=p->next; kfree(p->task); kfree(p); }
bool process_self_test(void) { struct nova_process *a,*b,*c; if(!kernel_process) return false; a=nova_process_create(); b=nova_process_create(); if(!a||!b||a->pid==0||b->pid==0||a->pid==b->pid||!a->task||!b->task||!registered(a)||!registered(b)) return false; if(!nova_process_exit(a)||a->state!=NOVA_PROCESS_TERMINATED||a->task->state!=3)return false; c=nova_process_create(); if(!c||c->pid==0||!c->task||c->pid==a->pid||c->pid==b->pid)return false; nova_process_destroy(a);nova_process_destroy(b);nova_process_destroy(c);return true; }
