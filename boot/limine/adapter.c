#include <limine.h>
#include <nova/boot.h>
__attribute__((used,section(".limine_requests_start"))) static volatile uint64_t start_marker[]={0xf6b8f4b39de7d1aeULL,0x0a82e883a6b2e3d4ULL};
__attribute__((used,section(".limine_requests"))) static volatile uint64_t limine_base_revision[]={0xf9562b2d5c95a6c8ULL,0x6a7b384944536bdcULL,3};
__attribute__((used,section(".limine_requests"))) static volatile struct limine_memmap_request memmap_request={.id=LIMINE_MEMMAP_REQUEST,.revision=0};
__attribute__((used,section(".limine_requests"))) static volatile struct limine_hhdm_request hhdm_request={.id=LIMINE_HHDM_REQUEST,.revision=0};
__attribute__((used,section(".limine_requests"))) static volatile struct limine_kernel_address_request kernel_address_request={.id=LIMINE_KERNEL_ADDRESS_REQUEST,.revision=0};
__attribute__((used,section(".limine_requests_end"))) static volatile uint64_t end_marker[]={0xadc0e0531bb10d03ULL,0x9572709f31764c62ULL};
void boot_limine_init(struct nova_boot_info *out){ out->bootloader_name="Limine";out->bootloader_version="protocol adapter";out->kernel_physical_base=0;out->kernel_virtual_base=0;out->hhdm_offset=0;out->memory_region_count=0; if (hhdm_request.response) out->hhdm_offset=hhdm_request.response->offset; if (kernel_address_request.response) { out->kernel_physical_base=kernel_address_request.response->physical_base; out->kernel_virtual_base=kernel_address_request.response->virtual_base; } if(!memmap_request.response)return; for(uint64_t i=0;i<memmap_request.response->entry_count && i<NOVA_MAX_MEMORY_REGIONS;i++){struct limine_memmap_entry*e=memmap_request.response->entries[i];out->memory_regions[i]=(struct nova_memory_region){e->base,e->length,(uint32_t)e->type};out->memory_region_count++;} }
