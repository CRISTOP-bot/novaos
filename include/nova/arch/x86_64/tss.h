#ifndef NOVA_ARCH_X86_64_TSS_H
#define NOVA_ARCH_X86_64_TSS_H
#include <nova/boot.h>
bool tss_init(void);
bool privilege_self_test(const struct nova_boot_info *boot_info);
#endif
