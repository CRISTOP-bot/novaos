ENTRY(_start)
SECTIONS {
  . = 1M;
  __kernel_start = .;
  .text : ALIGN(4K) { KEEP(*(.text.entry)) *(.text .text.*) }
  .limine_requests : ALIGN(8) { __limine_requests_start = .; KEEP(*(.limine_requests_start)) KEEP(*(.limine_requests)) KEEP(*(.limine_requests_end)) __limine_requests_end = .; }
  .rodata : ALIGN(4K) { *(.rodata .rodata.*) }
  .data : ALIGN(4K) { *(.data .data.*) }
  .bss : ALIGN(4K) { *(COMMON) *(.bss .bss.*) }
  . = ALIGN(4K);
  __kernel_end = .;
  /DISCARD/ : { *(.eh_frame) *(.comment) }
}
