.section .text.user
.global ring3_user_entry
.type ring3_user_entry,@function
ring3_user_entry:
    mov $1, %rax
    int $0x80
    mov %rax, %r12
    mov $2, %rax
    mov $1, %rdi
    lea syscall_message(%rip), %rsi
    mov $20, %rdx
    int $0x80
    mov $0xffff, %rax
    int $0x80
    mov $2, %rax
    mov $1, %rdi
    mov $0x500000, %rsi
    mov $1, %rdx
    int $0x80
    mov $3, %rax
    xor %rdi, %rdi
    int $0x80
    ret
.section .text.user
syscall_message: .ascii "NovaOS syscall write\n"

.section .text
.global ring3_enter
.type ring3_enter,@function
.extern ring3_resume_stack
ring3_enter:
    mov %rsp, ring3_resume_stack(%rip)
    pushq $0x23
    pushq %rsi
    pushfq
    pop %rax
    or $0x200, %rax
    push %rax
    pushq $0x1b
    pushq %rdi
    iretq
.global ring3_return_point
ring3_return_point:
    mov ring3_resume_stack(%rip), %rsp
    ret
