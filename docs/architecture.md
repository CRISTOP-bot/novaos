# Arquitectura de NovaOS

## Decisión base

NovaOS comienza como un kernel monolítico modular: drivers, VFS, memoria y procesos viven en el kernel, pero dependen de interfaces internas pequeñas y testeables. No se adopta microkernel por principio: para la primera milestone añade IPC, servidores y debugging antes de aportar valor. La separación de servicios queda preparada para una evolución posterior.

El kernel no reutiliza una arquitectura de CortexOS automáticamente. Cualquier componente propio que se porte deberá pasar una revisión de licencia, invariantes, acoplamiento y compatibilidad con esta ABI.

## Capas

```text
hardware x86_64
  └─ boot protocol (Limine; GRUB2 como adaptador futuro)
     └─ arch/x86_64 (GDT, IDT, traps, paging, context switch, syscall entry)
        └─ kernel core (panic, logging, init, capabilities de objetos)
           ├─ mm (PMM, VMM, kmalloc, user-copy)
           ├─ sched (threads, run queues, timer preemption)
           ├─ proc (address spaces, processes, handles/fds)
           ├─ syscall (dispatch + validation)
           ├─ vfs/fs (VFS + initramfs cpio)
           ├─ dev/drivers (console, serial, timer, keyboard)
           └─ ipc/time/net (incremental)
              └─ userspace: init, libc, shell, utilities, services
```

## Orden de implementación

1. Boot, serial console, panic y linker script.
2. GDT/IDT, excepciones y salida de registros.
3. Mapa de memoria del bootloader, PMM y paging kernel.
4. Heap del kernel con tests host-side y assertions.
5. TSC/PIT/APIC según lo exija el scheduler; no introducir SMP aún.
6. Thread de kernel, cambio de contexto y scheduler cooperativo; luego tick preemptivo.
7. Ring 3, address space de proceso y entrada/salida segura de userspace.
8. ABI de syscalls estable: `write`, `exit`, `getpid`, `mmap`, luego I/O.
9. VFS en memoria + initramfs cpio `newc`.
10. Validación y carga ELF64 `ET_EXEC`, stack inicial y `/init`.
11. `crt0`, wrappers, Newlib, shell y utilidades mínimas.

## Memoria y seguridad

- Separar físicamente PMM, VMM y allocator del heap.
- W^X por defecto: segmentos `PT_LOAD` no son simultáneamente writable y executable.
- Validar overflow de `p_offset + p_filesz`, `p_vaddr + p_memsz`, alineamiento, rango de usuario y solapamiento de segmentos.
- `copy_from_user`/`copy_to_user` nunca desreferencian directamente un puntero user.
- Límites explícitos para argv/envp, strings, buffers y número de descriptores.
- Fallos de page fault en user terminan el proceso; en kernel provocan panic controlado.

## Procesos

La primera versión usa `spawn(path, argv, envp)` como creación primaria. `fork` se deja para cuando existan address spaces copy-on-write y una semántica clara; no se simula con una implementación insegura. `exec` reemplaza el address space y `waitpid` recoge el estado del hijo.

## VFS

La interfaz inicial expone nodos, directorios, operaciones de archivo y descriptores. El primer backend es initramfs de solo lectura. Un backend en memoria permitirá tests sin disco. FAT32 y un filesystem persistente son fases posteriores, no requisitos del loader.

## Bootloader

Se elige Limine para desarrollo: protocolo moderno, soporte x86_64/QEMU, carga de módulos y licencia permisiva. GRUB Multiboot2 queda como adaptador alternativo porque es muy maduro, pero su GPL y la complejidad de su configuración no aportan a la primera milestone. El kernel consume una estructura interna `boot_info`, no el formato de un bootloader directamente.

## Estado de implementación (M0)

La implementación actual cubre únicamente el arranque x86_64, consola UART, CPUID, GDT, IDT, excepciones, adaptador de memory map y parada segura. La validación de runtime queda PENDING hasta ejecutar QEMU en local o CI. PMM, paging dinámico, scheduler, procesos, syscalls, ELF, VFS y userspace permanecen explícitamente fuera de M0.
