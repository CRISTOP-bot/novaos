#include <nova/console.h>
#include <nova/cpu.h>
#include <nova/types.h>
static char vendor[13];
void cpu_init(void){ uint32_t a,b,c,d; __asm__ volatile("cpuid":"=a"(a),"=b"(b),"=c"(c),"=d"(d):"a"(0)); ((uint32_t*)vendor)[0]=b; ((uint32_t*)vendor)[1]=d; ((uint32_t*)vendor)[2]=c; vendor[12]=0; }
void cpu_print_info(void){ console_write("[NovaOS] cpu initialized, vendor="); console_write(vendor); console_write("\n"); }
