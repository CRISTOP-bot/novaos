#include <nova/gdt.h>
#include <nova/console.h>
#include <nova/types.h>
struct desc { uint16_t limit; uint64_t base; } __attribute__((packed));
static uint64_t gdt[3] __attribute__((aligned(8)));
static struct desc gdtr;
void gdt_init(void){ gdt[0]=0; gdt[1]=0x00af9a000000ffffULL; gdt[2]=0x00af92000000ffffULL; gdtr.limit=sizeof(gdt)-1;gdtr.base=(uint64_t)gdt; __asm__ volatile("lgdt %0; mov $0x10,%%ax; mov %%ax,%%ds; mov %%ax,%%es; mov %%ax,%%ss; pushq $0x08; lea 1f(%%rip),%%rax; pushq %%rax; lretq; 1:"::"m"(gdtr):"rax","memory"); }
