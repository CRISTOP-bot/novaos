#ifndef NOVA_INITRAMFS_H
#define NOVA_INITRAMFS_H
#include <nova/types.h>
#define NOVA_INITRAMFS_MAGIC 0x53465241564f4e41ULL
#define NOVA_INITRAMFS_VERSION 1U
#define NOVA_INITRAMFS_PATH_MAX 64
struct nova_initramfs_header { uint64_t magic; uint32_t version,count; uint64_t header_size,data_size; } __attribute__((packed));
struct nova_initramfs_entry { char path[NOVA_INITRAMFS_PATH_MAX]; uint64_t offset,size; } __attribute__((packed));
bool nova_initramfs_mount(const void *image,size_t size);
bool nova_initramfs_self_test(void);
bool nova_init_execute(void);
#endif
