#include <nova/types.h>
#include "serial.h"
#define COM1 0x3f8
static inline void outb(uint16_t p, uint8_t v){ __asm__ volatile("outb %0,%1" : : "a"(v),"Nd"(p)); }
static inline uint8_t inb(uint16_t p){ uint8_t v; __asm__ volatile("inb %1,%0":"=a"(v):"Nd"(p)); return v; }
void serial_init(void){ outb(COM1+1,0); outb(COM1+3,0x80); outb(COM1+0,1); outb(COM1+1,0); outb(COM1+3,3); outb(COM1+2,0xc7); outb(COM1+4,3); }
void serial_putchar(char c){ if(c=='\n') serial_putchar('\r'); while(!(inb(COM1+5)&0x20)){} outb(COM1,(uint8_t)c); }
void serial_write(const char *s){ if(!s)return; while(*s) serial_putchar(*s++); }
static void hex(uint64_t v){ const char *d="0123456789abcdef"; serial_write("0x"); for(int i=15;i>=0;i--) serial_putchar(d[(v>>(i*4))&15]); }
void serial_vprintf(const char *f,__builtin_va_list a){ for(;*f;f++){ if(*f!='%'){serial_putchar(*f);continue;} f++; if(*f=='s')serial_write(__builtin_va_arg(a,const char*)); else if(*f=='c')serial_putchar((char)__builtin_va_arg(a,int)); else if(*f=='x')hex(__builtin_va_arg(a,uint64_t)); else {serial_putchar('%');serial_putchar(*f);} } }

void serial_printf(const char *f,...){ __builtin_va_list a; __builtin_va_start(a,f); serial_vprintf(f,a); __builtin_va_end(a); }
