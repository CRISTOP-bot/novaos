.section .text
.global _start
.type _start,@function
.extern kmain
.extern boot_limine_init
.section .bss
.align 16
stack_bottom: .skip 16384
stack_top:
.section .text
_start:
    lea stack_top(%rip), %rsp
    and $-16, %rsp
    xor %rbp, %rbp
    lea boot_info(%rip), %rdi
    call boot_limine_init
    lea boot_info(%rip), %rdi
    call kmain
1:  cli
    hlt
    jmp 1b
.section .bss
.align 8
boot_info: .skip 8 + 8 + 8 + 8 + 8 + 8 + (128 * 24)
