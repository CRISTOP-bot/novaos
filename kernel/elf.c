#include <nova/elf.h>
#include <nova/user_region.h>
#include <nova/mm/paging.h>
#include <nova/mm/pmm.h>
#include <nova/mm/heap.h>

#define USER_LIMIT 0x00007fffffffffffULL
static bool bounds(size_t size, uint64_t off, uint64_t len) { return off <= size && len <= (uint64_t)size - off; }
static bool add_ok(uint64_t a,uint64_t b,uint64_t *out){if(b>~0ULL-a)return false;*out=a+b;return true;}
static uint64_t down(uint64_t x){return x & ~(NOVA_PAGE_SIZE-1);}
static bool up(uint64_t x,uint64_t *out){if(x>~0ULL-(NOVA_PAGE_SIZE-1))return false;*out=(x+NOVA_PAGE_SIZE-1)&~(NOVA_PAGE_SIZE-1);return true;}

static bool validate(const void *image,size_t size,const struct nova_elf64_header **eh,const struct nova_elf64_phdr **ph){
 const struct nova_elf64_header *h=(const struct nova_elf64_header *)image;
 if(!image||size<sizeof(*h)||h->ident[0]!=0x7f||h->ident[1]!='E'||h->ident[2]!='L'||h->ident[3]!='F'||h->ident[4]!=NOVA_ELFCLASS64||h->ident[5]!=NOVA_ELFDATA2LSB||h->ident[6]!=NOVA_EI_VERSION||h->type!=NOVA_ET_EXEC||h->machine!=NOVA_EM_X86_64||h->version!=NOVA_EI_VERSION||h->ehsize!=sizeof(*h)||h->phentsize!=sizeof(struct nova_elf64_phdr)||!h->phnum||!bounds(size,h->phoff,(uint64_t)h->phnum*h->phentsize))return false;
 if(h->phnum > 128 || h->entry>USER_LIMIT)return false;
 *eh=h;*ph=(const struct nova_elf64_phdr *)((const uint8_t *)image+h->phoff);return true;
}

struct nova_process *nova_process_create_from_elf(const void *image,size_t size){
 const struct nova_elf64_header *h;const struct nova_elf64_phdr *ph;struct nova_process *p;struct nova_user_region **regions;uint64_t i,end,start,flags,j;bool loaded=false;
 if(!validate(image,size,&h,&ph)){return NULL;}
 p=nova_process_create();if(!p){return NULL;}
 regions=kmalloc(sizeof(*regions)*h->phnum);if(!regions){nova_process_destroy(p);return NULL;}for(i=0;i<h->phnum;i++)regions[i]=NULL;
 for(i=0;i<h->phnum;i++){
  if(ph[i].type!=NOVA_PT_LOAD)continue;
  if(ph[i].filesz>ph[i].memsz||!bounds(size,ph[i].offset,ph[i].filesz)||!add_ok(ph[i].vaddr,ph[i].memsz,&end)||end>USER_LIMIT||!up(end,&end)||ph[i].vaddr>end||(ph[i].align>1&&(ph[i].align&(ph[i].align-1)))){goto fail;}
  start=down(ph[i].vaddr);flags=NOVA_USER_REGION_READ;if(ph[i].flags&NOVA_PF_W)flags|=NOVA_USER_REGION_WRITE;if(ph[i].flags&NOVA_PF_X)flags|=NOVA_USER_REGION_EXEC;
  if(end<=start||(regions[i]=nova_user_region_map(p->address_space,start,end-start,flags))==NULL){goto fail;}
 }
 if(!nova_user_stack_create(p->address_space)){goto fail;}
 for(i=0;i<h->phnum;i++)if(ph[i].type==NOVA_PT_LOAD){
  for(j=0;j<ph[i].memsz;j++){
   uint64_t va=ph[i].vaddr+j; uint64_t page=(va-regions[i]->virtual_start)/NOVA_PAGE_SIZE; uint64_t in=va & (NOVA_PAGE_SIZE-1);
   uint8_t *dst=(uint8_t *)paging_physical_pointer(regions[i]->physical_pages[page]);
   if(!dst){goto fail;}
   dst[in]=(j<ph[i].filesz)?((const uint8_t *)image)[ph[i].offset+j]:0;
  }
 }
 for(i=0;i<h->phnum;i++)if(ph[i].type==NOVA_PT_LOAD&&nova_user_region_contains(regions[i],h->entry,1)&&(regions[i]->flags&NOVA_USER_REGION_EXEC))loaded=true;
 if(!loaded){goto fail;}
 p->task->user.rip=h->entry;p->task->user.rsp=nova_user_stack_initial_rsp(p->address_space);p->task->user.rflags=0x202;p->task->user.cs=0x1b;p->task->user.ss=0x23;
 kfree(regions);return p;
fail:
 kfree(regions);nova_process_destroy(p);return NULL;
}

