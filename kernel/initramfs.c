#include <nova/initramfs.h>
#include <nova/console.h>
#include <nova/elf.h>
#include <nova/mm/heap.h>
#include <nova/vfs.h>
#include <nova/process.h>
#include <nova/syscall.h>

extern void ring3_enter(uint64_t rip,uint64_t rsp);
extern bool returned_from_ring3;
static bool mounted;
static bool initramfs_valid(const void *image,size_t size,const struct nova_initramfs_header **h,const struct nova_initramfs_entry **e){
 const struct nova_initramfs_header *x=image; const struct nova_initramfs_entry *q;
 if(!image||size<sizeof(*x)||x->magic!=NOVA_INITRAMFS_MAGIC||x->version!=NOVA_INITRAMFS_VERSION||!x->count||x->header_size!=sizeof(*x)+(uint64_t)x->count*sizeof(struct nova_initramfs_entry)||x->header_size>size||x->data_size>size-x->header_size)return false;
 if(x->count>64||sizeof(*x)+(uint64_t)x->count*sizeof(*q)>size)return false;
 q=(const struct nova_initramfs_entry *)((const uint8_t *)image+sizeof(*x));
 bool found=false; for(uint32_t i=0;i<x->count;i++){ if(vfs_strlen(q[i].path)>=NOVA_INITRAMFS_PATH_MAX||!q[i].size||q[i].offset<x->header_size||q[i].offset>size||q[i].size>size-q[i].offset)return false; if(!vfs_strcmp(q[i].path,"/init"))found=true; } if(!found)return false;
 *h=x;*e=q;return true;
}
bool nova_initramfs_mount(const void *image,size_t size){const struct nova_initramfs_header*h;const struct nova_initramfs_entry*e;if(mounted)return false;if(!initramfs_valid(image,size,&h,&e))return false;(void)e;const struct nova_initramfs_entry* entries=(const struct nova_initramfs_entry*)((const uint8_t*)image+sizeof(*h));for(uint32_t i=0;i<h->count;i++){if(vfs_create_path(entries[i].path)!=0)return false;int64_t fd=vfs_open(entries[i].path);if(fd<0||vfs_write(fd,(const uint8_t*)image+entries[i].offset,entries[i].size)!=(int64_t)entries[i].size){if(fd>=0)vfs_close(fd);return false;}vfs_close(fd);}mounted=true;return true;}
bool nova_initramfs_self_test(void){const uint8_t*image;size_t size;int64_t fd;struct nova_stat st;uint8_t bad[sizeof(struct nova_initramfs_header)];extern const uint8_t _binary_embedded_initramfs_start[],_binary_embedded_initramfs_end[];image=_binary_embedded_initramfs_start;size=(size_t)(_binary_embedded_initramfs_end-image);if(!nova_initramfs_mount(image,size)||vfs_open("/missing")>=0||(fd=vfs_open("/init"))<0||!vfs_stat(fd,&st)||!st.size||vfs_close(fd)!=0)return false;for(size_t i=0;i<sizeof(bad);i++)bad[i]=0;if(nova_initramfs_mount(bad,sizeof(bad)))return false;return true;}
bool nova_init_execute(void){int64_t fd=-1;struct nova_stat st;uint8_t*image;struct nova_process*p;bool ok;if(!mounted||(fd=vfs_open("/init"))<0||!vfs_stat(fd,&st)||!st.size||st.size>0x100000){if(fd>=0)vfs_close(fd);return false;}image=kmalloc((size_t)st.size);if(!image){vfs_close(fd);return false;}if(vfs_read(fd,image,st.size)!=(int64_t)st.size||vfs_close(fd)!=0){kfree(image);return false;}p=nova_process_create_from_elf(image,(size_t)st.size);kfree(image);if(!p||!nova_process_activate(p)){if(p)nova_process_destroy(p);return false;}syscall_reset_test_state();returned_from_ring3=false;ring3_enter(p->task->user.rip,p->task->user.rsp);ok=returned_from_ring3&&syscall_exit_seen();nova_process_kernel()->state=NOVA_PROCESS_RUNNING;if(!nova_address_space_switch(nova_process_kernel()->address_space))ok=false;nova_process_destroy(p);return ok;}
