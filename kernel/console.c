#include <nova/console.h>
#include "../drivers/serial/serial.h"
void console_init(void){ serial_init(); }
void console_putchar(char c){ serial_putchar(c); }
void console_write(const char *s){ serial_write(s); }
void console_printf(const char *f,...){ __builtin_va_list a; __builtin_va_start(a,f); serial_vprintf(f,a); __builtin_va_end(a); }
