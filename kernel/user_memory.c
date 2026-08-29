#include <nova/user_memory.h>
#include <nova/mm/paging.h>
#include <nova/mm/pmm.h>
#include <nova/process.h>

#define NOVA_USER_LIMIT 0x00007fffffffffffULL

static bool user_range(uint64_t root, uint64_t address, uint64_t length) {
    uint64_t first, last, page;
    if (!length || address > NOVA_USER_LIMIT || length > NOVA_USER_LIMIT ||
        address > NOVA_USER_LIMIT - (length - 1)) return false;
    first = address & ~(NOVA_PAGE_SIZE - 1);
    last = (address + length - 1) & ~(NOVA_PAGE_SIZE - 1);
    for (page = first;; page += NOVA_PAGE_SIZE) {
        uint64_t physical;
        struct nova_page_info info;
        if (!paging_root_translate_info(root, page, &physical, &info) ||
            !info.present || !info.user) return false;
        if (page == last) break;
        if (page > ~0ULL - NOVA_PAGE_SIZE) return false;
    }
    return true;
}

bool nova_copy_from_user(void *destination, const void *source, size_t length) {
    struct nova_process *process = nova_process_current();
    uint64_t root, old_root, address;
    bool result = false;
    uint8_t *dst = (uint8_t *)destination;
    const uint8_t *src = (const uint8_t *)source;
    if (!destination || !source || !length || !process || !process->address_space) return false;
    root = process->address_space->root_physical;
    if (!user_range(root, (uint64_t)(uintptr_t)source, (uint64_t)length)) return false;
    old_root = paging_current_root();
    if (old_root != root && !paging_root_switch(root)) return false;
    for (address = 0; address < (uint64_t)length; address++) dst[address] = src[address];
    result = true;
    if (old_root != root && !paging_root_switch(old_root)) result = false;
    return result;
}

bool nova_user_memory_self_test(void) {
    struct nova_process *process;
    void *valid_page, *kernel_page;
    uint8_t copied[16];
    uint64_t user_va = 0x0000000000600000ULL;
    uint64_t no_user_va = 0x0000000000610000ULL;
    bool ok = false;
    process = nova_process_create();
    valid_page = pmm_alloc_page(); kernel_page = pmm_alloc_page();
    if (!process || !valid_page || !kernel_page ||
        !paging_root_map_page(process->address_space->root_physical, user_va,
                              (uint64_t)(uintptr_t)valid_page,
                              NOVA_PAGE_USER | NOVA_PAGE_WRITABLE | NOVA_PAGE_NO_EXECUTE) ||
        !paging_root_map_page(process->address_space->root_physical, no_user_va,
                              (uint64_t)(uintptr_t)kernel_page,
                              NOVA_PAGE_WRITABLE | NOVA_PAGE_NO_EXECUTE) ||
        !nova_process_activate(process)) goto cleanup;
    ((uint8_t *)(uintptr_t)user_va)[0] = 'u';
    ((uint8_t *)(uintptr_t)user_va)[1] = 's';
    ((uint8_t *)(uintptr_t)user_va)[2] = 'r';
    ((uint8_t *)(uintptr_t)user_va)[3] = '!';
    if (!nova_copy_from_user(copied, (const void *)(uintptr_t)user_va, 4) ||
        copied[0] != 'u' || copied[1] != 's' || copied[2] != 'r' || copied[3] != '!') goto cleanup;
    if (nova_copy_from_user(copied, (const void *)(uintptr_t)0x0000000000700000ULL, 1)) goto cleanup;
    if (nova_copy_from_user(copied, (const void *)(uintptr_t)0xffff800000000000ULL, 1)) goto cleanup;
    if (nova_copy_from_user(copied, (const void *)(uintptr_t)no_user_va, 1)) goto cleanup;
    if (nova_copy_from_user(copied, (const void *)(uintptr_t)(NOVA_USER_LIMIT - 1), 4)) goto cleanup;
    ok = true;
cleanup:
    if (nova_process_current() != nova_process_kernel()) nova_process_activate(nova_process_kernel());
    if (process) nova_process_destroy(process);
    if (valid_page) pmm_free_page(valid_page);
    if (kernel_page) pmm_free_page(kernel_page);
    return ok;
}

uint64_t nova_write_max(void) { return NOVA_WRITE_MAX; }
