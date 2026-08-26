#ifndef NOVA_BOOT_H
#define NOVA_BOOT_H
#include <nova/types.h>
enum nova_memory_type { NOVA_MEM_USABLE = 0, NOVA_MEM_RESERVED, NOVA_MEM_ACPI_RECLAIMABLE, NOVA_MEM_ACPI_NVS, NOVA_MEM_BAD, NOVA_MEM_BOOTLOADER_RECLAIMABLE, NOVA_MEM_KERNEL_AND_MODULES, NOVA_MEM_FRAMEBUFFER };
struct nova_memory_region { uint64_t base, length; uint32_t type; };
#define NOVA_MAX_MEMORY_REGIONS 128
struct nova_boot_info { const char *bootloader_name, *bootloader_version; uint64_t kernel_physical_base, kernel_virtual_base; uint64_t memory_region_count; struct nova_memory_region memory_regions[NOVA_MAX_MEMORY_REGIONS]; };
void boot_limine_init(struct nova_boot_info *out);
#endif
