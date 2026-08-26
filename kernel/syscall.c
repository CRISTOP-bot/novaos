#include <nova/syscall.h>
#include <nova/console.h>
#include <nova/mm/paging.h>
static volatile bool exit_seen, getpid_seen, write_seen, unknown_seen, badptr_seen;
static bool valid_user(uint64_t p,uint64_t n){if(!n||p>0x00007fffffffffffULL||n>0x1000||p>0x00007fffffffffffULL-n)return false;for(uint64_t x=p&~0xfffULL,end=p+n;x<end;x+=0x1000){uint64_t pa;if(!paging_translate(x,&pa))return false;}return true;}
int64_t syscall_dispatch(uint64_t n,uint64_t a1,uint64_t a2,uint64_t a3){switch(n){case NOVA_SYS_GETPID:getpid_seen=true;return 1;case NOVA_SYS_WRITE:if(a1!=1)return NOVA_SYS_EBADF;if(!valid_user(a2,a3)){badptr_seen=true;return NOVA_SYS_EFAULT;}for(uint64_t i=0;i<a3;i++){char z[2]={((const char*)(uintptr_t)a2)[i],0};console_write(z);}write_seen=true;return (int64_t)a3;case NOVA_SYS_EXIT:exit_seen=true;return 0;default:unknown_seen=true;return NOVA_SYS_ENOSYS;}}
void syscall_interrupt(uint64_t *s){s[0]=(uint64_t)syscall_dispatch(s[0],s[7],s[8],s[2]);}
bool syscall_exit_seen(void){return exit_seen;}
bool syscall_self_test(void){return getpid_seen&&write_seen&&unknown_seen&&badptr_seen&&exit_seen;}
