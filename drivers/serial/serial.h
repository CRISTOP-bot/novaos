#ifndef NOVA_SERIAL_H
#define NOVA_SERIAL_H
#include <nova/types.h>
void serial_init(void); void serial_putchar(char c); void serial_write(const char *s); void serial_vprintf(const char *fmt, __builtin_va_list ap); void serial_printf(const char *fmt, ...);
#endif
