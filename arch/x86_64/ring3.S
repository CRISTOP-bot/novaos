.section .text.user
.global ring3_user_entry
.type ring3_user_entry,@function
ring3_user_entry:
    mov $3, %rax
    int $0x80
    ret

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
