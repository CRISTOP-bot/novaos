# Decisiones técnicas iniciales

## ADR-0001: kernel monolítico modular

Se elige para reducir el número de mecanismos necesarios antes de ejecutar C real. Las interfaces internas separan módulos y permiten extraer servicios más adelante.

## ADR-0002: Limine primero, GRUB como adaptador

Limine ofrece un protocolo moderno, módulos y una integración sencilla con QEMU. Se oculta detrás de `boot_info` para no acoplar el kernel. GRUB Multiboot2 se conserva como opción de compatibilidad, no como dependencia del diseño.

## ADR-0003: Newlib para bootstrap

Newlib minimiza el trabajo de port inicial y no impone la syscall ABI de Linux. Se mantiene upstream casi intacta. Musl se reevalúa cuando mmap, procesos, señales, TLS y pthreads estén suficientemente maduros.

## ADR-0004: spawn antes que fork

`fork` sin COW es costoso y propenso a errores; no se finge una semántica parcial. `spawn` permite construir una cadena init → shell → programa con una especificación más pequeña.

## ADR-0005: initramfs antes que filesystem persistente

Permite probar VFS, loader y userland sin que un driver de disco o el diseño de un filesystem bloqueen el objetivo principal.

## ADR-0006: ABI propia, no ABI Linux

La ergonomía POSIX se ofrece desde libc donde sea razonable, pero los números, tipos, errores y semántica del kernel pertenecen a NovaOS. Las extensiones se versionan y nunca reutilizan números retirados.

## Estado de implementación (M0)

La implementación actual cubre únicamente el arranque x86_64, consola UART, CPUID, GDT, IDT, excepciones, adaptador de memory map y parada segura. La validación de runtime queda PENDING hasta ejecutar QEMU en local o CI. PMM, paging dinámico, scheduler, procesos, syscalls, ELF, VFS y userspace permanecen explícitamente fuera de M0.
