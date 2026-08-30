.section .text
.global _start
.type _start,@function
_start:
    mov $2, %rax
    mov $1, %rdi
    lea prompt(%rip), %rsi
    mov $(prompt_end-prompt), %rdx
    int $0x80
    mov %rsp, %rsi
    sub $128, %rsi
    mov $6, %rax
    xor %rdi, %rdi
    mov $128, %rdx
    int $0x80
    mov %rax, %rdx
    mov $2, %rax
    mov $1, %rdi
    int $0x80
    mov $3, %rax
    xor %rdi, %rdi
    int $0x80
1:  jmp 1b
.section .rodata
prompt: .ascii "NovaOS read test\nType something: "
prompt_end:
