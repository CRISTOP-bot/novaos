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
    mov %rsp,%rdi; add $120,%rdi; call exception_dispatch
1:  cli; hlt; jmp 1b

.global isr128
.extern ring3_return_dispatch
isr128:
    pushq $0; pushq $128
    push %r15;push %r14;push %r13;push %r12;push %r11;push %r10;push %r9;push %r8;push %rdi;push %rsi;push %rbp;push %rbx;push %rdx;push %rcx;push %rax
    mov %rsp,%rdi; add $120,%rdi; call ring3_return_dispatch
    pop %rax;pop %rcx;pop %rdx;pop %rbx;pop %rbp;pop %rsi;pop %rdi;pop %r8;pop %r9;pop %r10;pop %r11;pop %r12;pop %r13;pop %r14;pop %r15
    add $16,%rsp
    iretq
