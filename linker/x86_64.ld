OUTPUT_FORMAT(elf64-x86-64)
ENTRY(_start)

PHDRS
{
  limine_requests PT_LOAD;
  text PT_LOAD;
  rodata PT_LOAD;
  data PT_LOAD;
}

SECTIONS
{
  /* Limine x86_64 higher-half kernel mapping. */
  . = 0xffffffff80000000;
  __kernel_start = .;

  .limine_requests : ALIGN(8)
  {
    __limine_requests_start = .;
    KEEP(*(.limine_requests_start))
    KEEP(*(.limine_requests))
    KEEP(*(.limine_requests_end))
    __limine_requests_end = .;
  } :limine_requests

  . = ALIGN(CONSTANT(MAXPAGESIZE));
  .text :
  {
    KEEP(*(.text.entry))
    *(.text .text.*)
  } :text

  . = ALIGN(CONSTANT(MAXPAGESIZE));
  .rodata :
  {
    *(.rodata .rodata.*)
  } :rodata

  . = ALIGN(CONSTANT(MAXPAGESIZE));
  .data :
  {
    *(.data .data.*)
  } :data

  .bss :
  {
    *(COMMON)
    *(.bss .bss.*)
  } :data

  . = ALIGN(CONSTANT(MAXPAGESIZE));
  __kernel_end = .;

  /DISCARD/ :
  {
    *(.eh_frame*)
    *(.note .note.*)
    *(.comment)
  }
}
