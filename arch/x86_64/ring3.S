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
    mov $21, %rdx
    int $0x80
    mov $0xffff, %rax
    int $0x80
    mov $2, %rax
    mov $1, %rdi
    mov $0x500000, %rsi
    mov $1, %rdx
    int $0x80
    mov $4, %rax
    lea vfs_test_path(%rip), %rdi
    int $0x80
    mov %rax, %r13
    mov $6, %rax
    mov %r13, %rdi
    lea vfs_read_buf(%rip), %rsi
    mov $12, %rdx
    int $0x80
    mov %rax, %r14
    mov $5, %rax
    mov %r13, %rdi
    int $0x80
    mov $2, %rax
    mov $1, %rdi
    lea vfs_read_buf(%rip), %rsi
    mov $12, %rdx
    int $0x80
    mov $3, %rax
    xor %rdi, %rdi
    int $0x80
    ret
.section .text.user
.global syscall_message
syscall_message: .ascii "NovaOS syscall write\n"
.section .text.user
vfs_test_path: .asciz "/hello.txt"
vfs_read_buf: .skip 64
.global ring3_text_end
ring3_text_end:

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
