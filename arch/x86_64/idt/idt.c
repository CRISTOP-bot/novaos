#include <nova/idt.h>
#include <nova/console.h>
#include <nova/panic.h>
#include <nova/types.h>
#include "../../../drivers/serial/serial.h"
struct idt_gate { uint16_t off0, sel; uint8_t ist, flags; uint16_t off1; uint32_t off2, zero; } __attribute__((packed));
struct idtr { uint16_t limit; uint64_t base; } __attribute__((packed));
static struct idt_gate idt[256] __attribute__((aligned(16))); static struct idtr idtr;
extern void isr0(void);extern void isr6(void);extern void isr8(void);extern void isr13(void);extern void isr14(void);extern void isr_default(void); extern void isr128(void);
static void set_gate(int n,void(*fn)(void),uint8_t flags){uint64_t x=(uint64_t)fn;idt[n]=(struct idt_gate){x,8,0,flags,x>>16,x>>32,0};}
void idt_init(void){for(int i=0;i<256;i++)set_gate(i,isr_default,0x8e); set_gate(0,isr0,0x8e);set_gate(6,isr6,0x8e);set_gate(8,isr8,0x8e);set_gate(13,isr13,0x8e);set_gate(14,isr14,0x8e);idtr.limit=sizeof(idt)-1;idtr.base=(uint64_t)idt;__asm__ volatile("lidt %0"::"m"(idtr));}
void idt_install_ring3_gate(void){set_gate(128,isr128,0xee);}
void exception_init(void){ console_write("[NovaOS] exceptions initialized\n"); }
struct frame {uint64_t vector,error,rip,cs,rflags,rsp,ss;};
void exception_dispatch(struct frame *f){ uint64_t rsp=f->rsp,ss=f->ss; if((f->cs&3)==0){rsp=0;ss=0;} const char*n="UNKNOWN";if(f->vector==0)n="DIVIDE ERROR";else if(f->vector==6)n="INVALID OPCODE";else if(f->vector==8)n="DOUBLE FAULT";else if(f->vector==13)n="GENERAL PROTECTION";else if(f->vector==14){n="PAGE FAULT";uint64_t cr2;__asm__ volatile("mov %%cr2,%0":"=r"(cr2));serial_printf("[NovaOS] CR2=%x\n",cr2);}panic_exception(n,f->vector,f->error,f->rip,f->cs,f->rflags,rsp,ss); }
extern void ring3_return_point(void);
void ring3_return_dispatch(struct frame *f){ if((f->cs&3)==3){ f->rip=(uint64_t)(uintptr_t)ring3_return_point; f->cs=0x08; f->rflags=(f->rflags & ~(3ULL << 12)) | 0x202; f->rsp=ring3_resume_stack; f->ss=0x10; } }
