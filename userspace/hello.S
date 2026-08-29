.section .text
.global _start
.type _start,@function
_start:
    mov $2, %rax
    mov $1, %rdi
    lea message(%rip), %rsi
    mov $21, %rdx
    int $0x80
    mov $3, %rax
    xor %rdi, %rdi
    int $0x80
1:  jmp 1b
.section .rodata
message: .ascii "NovaOS syscall write\n"
