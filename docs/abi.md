# ABI de NovaOS (v0)

Este documento es normativo para userspace. Cambiarlo requiere versionado y pruebas de compatibilidad.

## Convenciones

- CPU: System V AMD64.
- Binarios: ELF64 little-endian, machine `EM_X86_64`.
- Syscalls: instrucción `syscall`.
- Entrada: `rax = número`, argumentos en `rdi, rsi, rdx, r10, r8, r9`.
- Retorno: `rax >= 0` éxito; `rax = -errno` error. Los wrappers de libc convierten esto a `-1` y asignan `errno`.
- Tipos ABI: enteros y tamaños de 64 bits donde corresponda; los punteros son user virtual addresses de 64 bits y nunca se pueden asumir válidos.
- Registros preservados y stack siguen System V AMD64. El kernel no preserva estado de libc ni TLS todavía.

El número de syscall es estable una vez publicado. No se reutilizan números retirados. `sys_abi_info` devuelve versión mayor/menor y capacidades; una aplicación debe comprobar capacidades cuando dependa de una extensión.

## Tabla inicial propuesta

| Nº | Nombre | Firma conceptual | Estado |
|---:|---|---|---|
| 0 | `sys_exit` | `exit(int status)` | M0 |
| 1 | `sys_write` | `write(fd, buf, len)` | M0 |
| 2 | `sys_read` | `read(fd, buf, len)` | M2 |
| 3 | `sys_open` | `open(path, flags, mode)` | M2 |
| 4 | `sys_close` | `close(fd)` | M2 |
| 5 | `sys_seek` | `seek(fd, offset, whence)` | M2 |
| 6 | `sys_stat` | `stat(path, out)` | M2 |
| 7 | `sys_mmap` | `mmap(addr, len, prot, flags, fd, off)` | M1 |
| 8 | `sys_munmap` | `munmap(addr, len)` | M1 |
| 9 | `sys_brk` | `brk(addr)` | M2 |
| 10 | `sys_spawn` | `spawn(path, argv, envp, actions)` | M1 |
| 11 | `sys_exec` | `exec(path, argv, envp)` | M2 |
| 12 | `sys_waitpid` | `waitpid(pid, status, options)` | M2 |
| 13 | `sys_getpid` | `getpid()` | M1 |
| 14 | `sys_getppid` | `getppid()` | M2 |
| 15 | `sys_dup` | `dup(fd)` | M2 |
| 16 | `sys_dup2` | `dup2(old, new)` | M2 |
| 17 | `sys_pipe` | `pipe(out_pair)` | M2 |
| 18 | `sys_ioctl` | `ioctl(fd, request, arg)` | M2 |
| 19 | `sys_clock_gettime` | `clock_gettime(id, out)` | M1 |
| 20 | `sys_nanosleep` | `nanosleep(req, rem)` | M2 |
| 21 | `sys_abi_info` | `abi_info(out)` | M1 |

`spawn` sustituye inicialmente a `fork`; `fork` sólo se añadirá cuando exista COW y tests de herencia de descriptores. Los números y firmas de esta tabla son una propuesta v0 que se congela antes de distribuir el primer SDK.

## Errores

Se reservará un conjunto pequeño y POSIX-like (`EINVAL`, `EFAULT`, `ENOENT`, `EACCES`, `EBADF`, `ENOMEM`, `EINTR`, `E2BIG`, `ENOSYS`, `ECHILD`, `ENOSPC`). Sus valores numéricos se publicarán en `include/nova/errno.h`; no se copiarán los valores de Linux por compatibilidad accidental.

## ELF y proceso inicial

M0 acepta `ET_EXEC`, `EM_X86_64`, `ELFCLASS64`, little-endian y `PT_LOAD`. No acepta PIE, `PT_INTERP`, relocaciones dinámicas ni shared libraries. Se comprueban headers, overflow, rangos, alineamiento, permisos y entry point. Se construye un stack user alineado a 16 bytes con:

```text
argc, argv[], NULL, envp[], NULL, auxv terminada en AT_NULL
```

Los valores de `argv` y `envp` se copian a memoria user. `AT_PHDR`, `AT_PHNUM` y `AT_ENTRY` se añadirán sólo con una especificación formal del auxv.

## Compatibilidad

La compatibilidad POSIX se implementa encima de esta ABI, no al revés. Cada función libc debe indicar si es ISO C, POSIX parcial o extensión NovaOS. No se afirma compatibilidad POSIX completa hasta que exista una suite relevante que lo demuestre.
