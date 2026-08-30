#include <nova/input.h>
#include <nova/console.h>
#include <nova/types.h>

static struct nova_input_buffer input;
static inline uint8_t inb(uint16_t p){uint8_t v;__asm__ volatile("inb %1,%0":"=a"(v):"Nd"(p));return v;}
static inline void outb(uint16_t p,uint8_t v){__asm__ volatile("outb %0,%1"::"a"(v),"Nd"(p));}
void nova_input_init(void){input.head=input.tail=0;}
bool nova_input_empty(void){return input.head==input.tail;}
bool nova_input_full(void){return ((input.head+1)%NOVA_INPUT_CAPACITY)==input.tail;}
bool nova_input_push(char c){if(nova_input_full())return false;input.data[input.head]=c;input.head=(input.head+1)%NOVA_INPUT_CAPACITY;return true;}
bool nova_input_pop(char*c){if(!c||nova_input_empty())return false;*c=input.data[input.tail];input.tail=(input.tail+1)%NOVA_INPUT_CAPACITY;return true;}
static const char normal[128]={ [2]='1',[3]='2',[4]='3',[5]='4',[6]='5',[7]='6',[8]='7',[9]='8',[10]='9',[11]='0',[12]='-',[13]='=',[14]='\b',[15]='\t',[16]='q',[17]='w',[18]='e',[19]='r',[20]='t',[21]='y',[22]='u',[23]='i',[24]='o',[25]='p',[26]='[',[27]=']',[28]='\n',[30]='a',[31]='s',[32]='d',[33]='f',[34]='g',[35]='h',[36]='j',[37]='k',[38]='l',[39]=';',[40]='\'', [41]='`',[43]='\\',[44]='z',[45]='x',[46]='c',[47]='v',[48]='b',[49]='n',[50]='m',[51]=',',[52]='.',[53]='/' ,[57]=' '};
void nova_keyboard_irq(void){uint8_t sc=inb(0x60);if(sc&0x80)return;if(sc<128&&normal[sc])nova_input_push(normal[sc]);outb(0x20,0x20);}
void nova_keyboard_init(void){uint8_t v;/* 8259 remap: IRQ1 becomes IDT vector 33. */outb(0x20,0x11);outb(0xa0,0x11);outb(0x21,0x20);outb(0xa1,0x28);outb(0x21,0x04);outb(0xa1,0x02);outb(0x21,0x01);outb(0xa1,0x01);outb(0x21,0xfd);outb(0xa1,0xff);outb(0x64,0xae);for(int i=0;i<1000;i++){v=inb(0x64);if(!(v&2))break;}outb(0x60,0xf4);nova_input_init();__asm__ volatile("sti");}
bool nova_input_self_test(void){char c;struct nova_input_buffer saved=input;nova_input_init();if(!nova_input_empty()||nova_input_pop(&c)||!nova_input_push('a')||!nova_input_push('\n')||!nova_input_pop(&c)||c!='a'||!nova_input_pop(&c)||c!='\n'||!nova_input_empty()){input=saved;return false;}for(size_t i=0;i<NOVA_INPUT_CAPACITY-1;i++)if(!nova_input_push('x')){input=saved;return false;}if(!nova_input_full()||nova_input_push('y')){input=saved;return false;}input=saved;return true;}
