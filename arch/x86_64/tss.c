#include <nova/arch/x86_64/tss.h>
#include <nova/gdt.h>
#include <nova/mm/pmm.h>
#include <nova/mm/paging.h>
#include <nova/console.h>
#include <nova/types.h>

struct tss64 { uint32_t reserved0; uint64_t rsp0,rsp1,rsp2; uint64_t reserved1; uint64_t ist[7]; uint64_t reserved2; uint16_t reserved3; uint16_t iomap; } __attribute__((packed,aligned(16)));
static struct tss64 tss __attribute__((aligned(16)));
static uint8_t rsp0_stack[16384] __attribute__((aligned(16)));
uint64_t ring3_resume_stack;
static bool returned_from_ring3;
extern void ring3_enter(uint64_t rip, uint64_t rsp);
extern void ring3_user_entry(void);
extern char __kernel_start;
extern void idt_install_ring3_gate(void);
extern uint64_t gdt_debug_entry(unsigned); extern uint64_t gdt_debug_base(void); extern uint64_t gdt_debug_limit(void);


bool tss_init(void) {
    for (uint64_t i=0;i<sizeof(tss);i++) ((uint8_t *)&tss)[i]=0;
    tss.rsp0=(uint64_t)(uintptr_t)(rsp0_stack+sizeof(rsp0_stack)); tss.iomap=sizeof(tss);
    gdt_load_tss((uint64_t)(uintptr_t)&tss, sizeof(tss)-1); console_printf("[NovaOS] GDT base=%x limit=%x GDT3=%x GDT4=%x TSS5=%x TSS6=%x RSP0=%x\n",gdt_debug_base(),gdt_debug_limit(),gdt_debug_entry(3),gdt_debug_entry(4),gdt_debug_entry(5),gdt_debug_entry(6),tss.rsp0); idt_install_ring3_gate(); return true;
}
bool privilege_self_test(const struct nova_boot_info *boot) {
    uint64_t fn=(uint64_t)(uintptr_t)ring3_user_entry, phys, stack_phys, user_sp, user_rip;
    if (!boot || !boot->kernel_virtual_base || !boot->kernel_physical_base) return false;
    if (fn < boot->kernel_virtual_base) return false;
    phys=boot->kernel_physical_base+(fn-boot->kernel_virtual_base); phys &= ~(NOVA_PAGE_SIZE-1);
    if (!paging_map_page(0x0000000000400000ULL, phys, NOVA_PAGE_USER | NOVA_PAGE_PRESENT)) return false;
    stack_phys=(uint64_t)(uintptr_t)pmm_alloc_page(); if (!stack_phys) return false;
    if (!paging_map_page(0x0000000000401000ULL, stack_phys, NOVA_PAGE_USER | NOVA_PAGE_WRITABLE)) return false;
    user_sp=0x0000000000402000ULL; returned_from_ring3=false; ring3_enter(0x0000000000400000ULL,user_sp);
    return returned_from_ring3;
}
