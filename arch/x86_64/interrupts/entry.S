.section .text
.extern exception_dispatch
.macro NOERR n
.global isr\n
isr\n: pushq $0; pushq $\n; jmp isr_common
.endm
.macro ERR n
.global isr\n
isr\n: pushq $\n; jmp isr_common
.endm
NOERR 0
NOERR 6
ERR 8
ERR 13
ERR 14
.global isr_default
isr_default: pushq $0; pushq $255; jmp isr_common
isr_common:
    push %r15;push %r14;push %r13;push %r12;push %r11;push %r10;push %r9;push %r8;push %rdi;push %rsi;push %rbp;push %rbx;push %rdx;push %rcx;push %rax
    mov %rsp,%rdi
    add $120,%rdi
    call exception_dispatch
1:  cli; hlt; jmp 1b
