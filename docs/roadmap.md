# Roadmap por fases

Cada fase termina con una prueba reproducible en QEMU y una nota de ABI. No se empieza la siguiente por tener código que sólo arranca una vez.

## M0 — Boot y kernel mínimo

- [ ] repositorio, formato, licencia y CI base
- [ ] Limine + linker script + `boot_info`
- [ ] serial, consola, panic, GDT/IDT y excepciones
- [ ] PMM, paging kernel y heap
- [ ] smoke test que arranca y termina con código conocido

## M1 — User mode y primera syscall

- [ ] TSS/ring 3/context switch
- [ ] thread y scheduler mínimo
- [ ] `sys_exit`, `sys_write`, `sys_getpid`, `sys_mmap`, `sys_abi_info`
- [ ] `copy_{from,to}_user` y tests de punteros inválidos
- [ ] programa estático `hello` cargado como módulo/initramfs

## M2 — ELF, procesos y VFS

- [ ] parser ELF host-side con casos válidos e inválidos
- [ ] loader `ET_EXEC` PT_LOAD con W^X
- [ ] stack argc/argv/envp
- [ ] VFS en memoria e initramfs cpio newc
- [ ] open/read/write/close/seek/stat
- [ ] spawn/exec/waitpid, fd table y pipe

## M3 — libc y userland

- [ ] sysroot y GCC/Newlib reproducibles
- [ ] crt0 y wrappers
- [ ] `/init`, shell, echo, ls, cat, pwd
- [ ] malloc, stdio, errno, environment
- [ ] tests ISO C y file APIs

## M4 — Drivers y sistema utilizable

- [ ] teclado, reloj, consola interactiva
- [ ] mkdir/rm/cp/ps/clear/uname
- [ ] servicios básicos e IPC
- [ ] FAT32 de sólo lectura
- [ ] GDB guide y kernel symbols

## M5+ — Compatibilidad incremental

1. ISO C + libc básica.
2. APIs de archivos POSIX parciales.
3. procesos y señales básicas.
4. threads/pthreads y TLS.
5. shell/utilidades y job control.
6. shared libraries/PIE/dynamic linker.
7. C++ y después port opcional de Rust.
8. SMP, red, aarch64 y riscv64.

## Criterio de éxito de la primera milestone

En una ejecución limpia de QEMU, NovaOS debe cargar `/init`; `init` debe crear/ejecutar el shell; el shell debe ejecutar `hello`; y `hello` debe imprimir exactamente `hello, NovaOS!` por el dispositivo de consola. La prueba debe detectar timeout, triple fault, retorno incorrecto y salida ausente.

## Estado de implementación (M0)

La implementación actual cubre únicamente el arranque x86_64, consola UART, CPUID, GDT, IDT, excepciones, adaptador de memory map y parada segura. La validación de runtime queda PENDING hasta ejecutar QEMU en local o CI. PMM, paging dinámico, scheduler, procesos, syscalls, ELF, VFS y userspace permanecen explícitamente fuera de M0.
