[![NovaOS CI](../../actions/workflows/novaos-tests.yml/badge.svg)](../../actions/workflows/novaos-tests.yml)

# NovaOS

Sistema operativo experimental x86_64, independiente de Linux, orientado a ejecutar software C real mediante una ABI propia y documentada.

> Estado: M0 implementation: COMPLETE. M0 runtime validation: PENDING (este entorno no tiene GCC, Make, QEMU, xorriso ni GDB).

## Objetivos de la primera milestone

`BOOT → kernel → memoria → scheduler → syscall → ELF loader → proceso user → libc → /init → shell → hello`

La primera versión usará:

- x86_64 y ABI de CPU System V AMD64;
- ELF64, inicialmente `ET_EXEC` no-PIE;
- Limine como bootloader primario, con una capa de boot que permita añadir GRUB Multiboot2;
- QEMU + GDB como entorno de prueba;
- kernel freestanding y userspace hosted;
- Newlib upstream separada del glue específico de NovaOS;
- initramfs cpio `newc` antes de diseñar un filesystem persistente.

## Construcción

La imagen de M0 se construye con el toolchain host (`gcc`/`ld`) en modo freestanding. El cross-toolchain de userspace sigue siendo una milestone posterior. Antes de construir hay que obtener Limine:

```sh
make limine
make   # cross-binutils, GCC y sysroot
make libc        # Newlib + glue NovaOS
make kernel
make userland
make image
make run
make debug
make debug-check
```

Ningún target debe usar accidentalmente headers o librerías del host. El toolchain tendrá sysroot propio y el kernel se compilará con flags freestanding separados.

## Documentación

- [Arquitectura](docs/architecture.md)
- [ABI y syscalls](docs/abi.md)
- [Toolchain y libc](docs/toolchain.md)
- [Roadmap](docs/roadmap.md)
- [Decisiones](docs/decisions.md)

## Principios

NovaOS no pretende ser Linux, no implementa una ABI Linux y no promete POSIX completo. POSIX se incorporará por etapas, con pruebas que demuestren cada interfaz. El código propio, los ports y los componentes externos se mantienen separados para poder actualizar upstream sin reescrituras indiscriminadas.

## Estado real de M0

Implementado: entrada `_start`, stack BSS, adaptador de memoria Limine, UART COM1, abstracción de consola, panic, CPUID, GDT cargada con `lgdt`, IDT cargada con `lidt`, handlers para excepciones principales, linker script, imagen ISO y smoke test QEMU.

No implementado todavía: PMM, paging propio, scheduler, IRQ, procesos, ring 3, syscalls, ELF loader, VFS, libc y userspace.
