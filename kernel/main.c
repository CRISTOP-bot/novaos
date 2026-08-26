#include <nova/boot.h>
#include <nova/console.h>
#include <nova/cpu.h>
#include <nova/gdt.h>
#include <nova/idt.h>
#include <nova/mm/pmm.h>
#include <nova/mm/paging.h>
#include <nova/mm/heap.h>
#include <nova/mm/diagnostics.h>
#include <nova/arch/x86_64/tss.h>
#include <nova/panic.h>
#include <nova/scheduler.h>

static const char *memory_type_name(uint32_t t){ switch(t){ case NOVA_MEM_USABLE:return "USABLE"; case NOVA_MEM_RESERVED:return "RESERVED"; case NOVA_MEM_ACPI_RECLAIMABLE:return "ACPI_RECLAIMABLE"; case NOVA_MEM_ACPI_NVS:return "ACPI_NVS"; case NOVA_MEM_BAD:return "BAD"; case NOVA_MEM_BOOTLOADER_RECLAIMABLE:return "BOOTLOADER_RECLAIMABLE"; case NOVA_MEM_KERNEL_AND_MODULES:return "KERNEL_AND_MODULES"; case NOVA_MEM_FRAMEBUFFER:return "FRAMEBUFFER"; default:return "UNKNOWN"; } }

void kmain(struct nova_boot_info *boot){
    console_init(); console_write("[NovaOS] booting\n"); cpu_init(); cpu_print_info();
    gdt_init(); console_write("[NovaOS] gdt initialized\n"); idt_init(); exception_init(); console_write("[NovaOS] idt initialized\n");
#ifdef NOVAOS_TEST_EXCEPTION
    __asm__ volatile("ud2");
#endif
    console_printf("[NovaOS] memory map detected, regions=%x\n",boot->memory_region_count);
    for(uint64_t i=0;i<boot->memory_region_count;i++) console_printf("[NovaOS] memory[%x] base=%x len=%x type=%s\n",i,boot->memory_regions[i].base,boot->memory_regions[i].length,memory_type_name(boot->memory_regions[i].type));
    pmm_init(boot);
    console_write("[NovaOS] PMM initialized\n"); console_printf("[NovaOS] page size: %x\n",(uint64_t)NOVA_PAGE_SIZE);
    console_printf("[NovaOS] total pages: %x\n",pmm_total_pages()); console_printf("[NovaOS] used pages: %x\n",pmm_used_pages()); console_printf("[NovaOS] free pages: %x\n",pmm_free_pages());
    if (!pmm_self_test(boot)) { console_write("[NovaOS] PMM self-test: FAIL\n"); panic("PMM self-test failed"); }
    console_write("[NovaOS] PMM self-test: PASS\nNOVAOS_PMM_OK\n");
    if (!paging_init(boot)) { console_write("[NovaOS] paging initialization: FAIL\n"); panic("paging initialization failed"); }
    console_write("[NovaOS] paging initialized\n");
    if (!paging_self_test()) { console_write("[NovaOS] paging self-test: FAIL\n"); panic("paging self-test failed"); }
    console_write("[NovaOS] paging self-test: PASS\nNOVAOS_PAGING_OK\n");
    if (!heap_init()) { console_write("[NovaOS] heap initialization: FAIL\n"); panic("heap initialization failed"); }
    console_write("[NovaOS] heap initialized\n");
    if (!heap_self_test()) { console_write("[NovaOS] heap self-test: FAIL\n"); panic("heap self-test failed"); }
    console_printf("[NovaOS] heap bytes mapped: %x\n", heap_bytes_mapped());
    console_printf("[NovaOS] heap bytes used: %x\n", heap_bytes_used());
    console_write("[NovaOS] heap self-test: PASS\nNOVAOS_HEAP_OK\n");
    memory_diagnostics_print();
    if (!memory_diagnostics_self_test()) { console_write("[NovaOS] memory diagnostics self-test: FAIL\n"); panic("memory diagnostics self-test failed"); }
    console_write("[NovaOS] memory diagnostics self-test: PASS\nNOVAOS_MEMORY_OK\n");
    if (!tss_init()) { console_write("[NovaOS] TSS initialization: FAIL\n"); panic("TSS initialization failed"); }
    console_write("[NovaOS] TSS initialized and LTR loaded\n");
    if (!privilege_self_test(boot)) { console_write("[NovaOS] Ring 3 self-test: FAIL\n"); panic("Ring 3 self-test failed"); }
    console_write("[NovaOS] Ring 3 self-test: PASS\nNOVAOS_RING3_OK\n");
    if (!scheduler_self_test()) { console_write("[NovaOS] scheduler self-test: FAIL\n"); panic("scheduler self-test failed"); }
    console_write("[NovaOS] scheduler self-test: PASS\nNOVAOS_SCHEDULER_OK\n");
    console_write("[NovaOS] kernel initialized\nNOVAOS_BOOT_OK\n"); for(;;)__asm__ volatile("cli; hlt");
}
