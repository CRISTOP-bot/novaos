#include <nova/panic.h>
#include <nova/console.h>
#include "../drivers/serial/serial.h"
__attribute__((noreturn)) void panic(const char *m){ console_write("\n*** NOVAOS KERNEL PANIC ***\nreason: "); console_write(m); console_write("\n"); for(;;)__asm__ volatile("cli; hlt"); }
__attribute__((noreturn)) void panic_exception(const char *n,uint64_t v,uint64_t e,uint64_t rip,uint64_t cs,uint64_t fl,uint64_t rsp,uint64_t ss){ console_printf("\n[NovaOS] exception #"); serial_printf("%x",v); console_write(" ");console_write(n);console_write("\n"); serial_printf("[NovaOS] error=%x RIP=%x CS=%x RFLAGS=%x RSP=%x SS=%x\n",e,rip,cs,fl,rsp,ss); panic("CPU exception"); }
