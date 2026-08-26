.section .text
.global scheduler_context_switch
.type scheduler_context_switch,@function
scheduler_context_switch:
    push %rbp
    push %rbx
    push %r12
    push %r13
    push %r14
    push %r15
    mov %rsp,(%rdi)
    mov %rsi,%rsp
    pop %r15
    pop %r14
    pop %r13
    pop %r12
    pop %rbx
    pop %rbp
    ret
