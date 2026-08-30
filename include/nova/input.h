#ifndef NOVA_INPUT_H
#define NOVA_INPUT_H
#include <nova/types.h>
#define NOVA_INPUT_CAPACITY 128
struct nova_input_buffer { char data[NOVA_INPUT_CAPACITY]; size_t head, tail; };
void nova_input_init(void);
bool nova_input_push(char c);
bool nova_input_pop(char *c);
bool nova_input_empty(void);
bool nova_input_full(void);
bool nova_input_self_test(void);
#endif
