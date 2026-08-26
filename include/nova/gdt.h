#ifndef NOVA_GDT_H
#define NOVA_GDT_H
#include <nova/types.h>
void gdt_init(void);
void gdt_set_tss(uint64_t base, uint32_t limit);
void gdt_load_tss(uint64_t base, uint32_t limit);
#endif
