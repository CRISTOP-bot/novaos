#ifndef NOVA_ELF_H
#define NOVA_ELF_H
#include <nova/types.h>
#include <nova/process.h>
#define NOVA_ELF_MAGIC 0x464c457fU
#define NOVA_ELFCLASS64 2
#define NOVA_ELFDATA2LSB 1
#define NOVA_EI_VERSION 1
#define NOVA_ET_EXEC 2
#define NOVA_EM_X86_64 62
#define NOVA_PT_LOAD 1
#define NOVA_PF_X 1
#define NOVA_PF_W 2
#define NOVA_PF_R 4
struct nova_elf64_header { uint8_t ident[16]; uint16_t type,machine; uint32_t version; uint64_t entry,phoff,shoff,flags; uint16_t ehsize,phentsize,phnum,shentsize,shnum,shstrndx; } __attribute__((packed));
struct nova_elf64_phdr { uint32_t type,flags; uint64_t offset,vaddr,paddr,filesz,memsz,align; } __attribute__((packed));
struct nova_process *nova_process_create_from_elf(const void *image, size_t size);
bool nova_elf_self_test(void);
const uint8_t *nova_embedded_elf_start(void);
size_t nova_embedded_elf_size(void);
#endif
