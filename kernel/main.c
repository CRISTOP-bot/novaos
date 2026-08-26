#include <nova/boot.h>
#include <nova/console.h>
#include <nova/cpu.h>
#include <nova/gdt.h>
#include <nova/idt.h>
#include <nova/panic.h>
static const char *memory_type_name(uint32_t t){ switch(t){ case NOVA_MEM_USABLE:return "USABLE"; case NOVA_MEM_RESERVED:return "RESERVED"; case NOVA_MEM_ACPI_RECLAIMABLE:return "ACPI_RECLAIMABLE"; case NOVA_MEM_ACPI_NVS:return "ACPI_NVS"; case NOVA_MEM_BAD:return "BAD"; case NOVA_MEM_BOOTLOADER_RECLAIMABLE:return "BOOTLOADER_RECLAIMABLE"; case NOVA_MEM_KERNEL_AND_MODULES:return "KERNEL_AND_MODULES"; case NOVA_MEM_FRAMEBUFFER:return "FRAMEBUFFER"; default:return "UNKNOWN"; } }
void kmain(struct nova_boot_info *boot){ console_init(); console_write("[NovaOS] booting\n"); cpu_init(); cpu_print_info(); gdt_init(); console_write("[NovaOS] gdt initialized\n"); idt_init(); exception_init(); console_write("[NovaOS] idt initialized\n");
#ifdef NOVAOS_TEST_EXCEPTION
    __asm__ volatile("ud2");
#endif
    console_printf("[NovaOS] memory map detected, regions=%x\n",boot->memory_region_count); for(uint64_t i=0;i<boot->memory_region_count;i++) console_printf("[NovaOS] memory[%x] base=%x len=%x type=%s\n",i,boot->memory_regions[i].base,boot->memory_regions[i].length,memory_type_name(boot->memory_regions[i].type)); console_write("[NovaOS] kernel initialized\nNOVAOS_BOOT_OK\n"); for(;;)__asm__ volatile("cli; hlt"); }
