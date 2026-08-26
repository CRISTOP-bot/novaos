#include <nova/gdt.h>
#include <nova/types.h>

struct desc { uint16_t limit; uint64_t base; } __attribute__((packed));
static uint64_t gdt[7] __attribute__((aligned(8)));
static struct desc gdtr;

void gdt_set_tss(uint64_t base, uint32_t limit) {
    uint64_t low = 0x0000890000000000ULL | (limit & 0xffffULL) | ((base & 0xffffffULL) << 16) | ((base >> 24) & 0xffULL) << 56;
    uint64_t high = base >> 32;
    gdt[5] = low; gdt[6] = high;
}
void gdt_init(void) {
    gdt[0]=0; gdt[1]=0x00af9a000000ffffULL; gdt[2]=0x00af92000000ffffULL;
    gdt[3]=0x00affa000000ffffULL; gdt[4]=0x00cff2000000ffffULL;
    gdtr.limit=sizeof(gdt)-1; gdtr.base=(uint64_t)gdt;
    __asm__ volatile("lgdt %0; mov $0x10,%%ax; mov %%ax,%%ds; mov %%ax,%%es; mov %%ax,%%ss; pushq $0x08; lea 1f(%%rip),%%rax; pushq %%rax; lretq; 1:"::"m"(gdtr):"rax","memory");
}
void gdt_load_tss(uint64_t base, uint32_t limit) {
    gdt_set_tss(base, limit); gdtr.limit=sizeof(gdt)-1; gdtr.base=(uint64_t)gdt;
    __asm__ volatile("lgdt %0; mov $0x28, %%ax; ltr %%ax" :: "m"(gdtr) : "rax", "memory");
}

