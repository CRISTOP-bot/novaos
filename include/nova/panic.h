#ifndef NOVA_PANIC_H
#define NOVA_PANIC_H
#include <nova/types.h>
__attribute__((noreturn)) void panic(const char *message);
__attribute__((noreturn)) void panic_exception(const char *name, uint64_t vector, uint64_t error, uint64_t rip, uint64_t cs, uint64_t rflags, uint64_t rsp, uint64_t ss);
#endif
