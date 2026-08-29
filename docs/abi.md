# ABI experimental de NovaOS

Este documento describe la interfaz experimental de syscalls de M2.3. Aún no es una ABI estable para aplicaciones y puede cambiar antes de una release.

## Entrada

La entrada actual usa un gate IDT `INT 0x80` con DPL=3. No se utiliza `SYSCALL/SYSRET` todavía.

```text
RAX = número de syscall
RDI = argumento 1
RSI = argumento 2
RDX = argumento 3
RAX = valor de retorno
```

La transición conserva el contexto necesario del interrupt stub y vuelve mediante `IRETQ`. El kernel no promete compatibilidad con la ABI de Linux.

## Números

| Número | Nombre | Forma | Estado |
|---:|---|---|---|
| 1 | `SYS_GETPID` | `getpid()` | experimental |
| 2 | `SYS_WRITE` | `write(fd, buffer, length)` | experimental |
| 3 | `SYS_EXIT` | `exit(status)` | experimental |
| 4 | `SYS_OPEN` | `open(path)` | experimental |
| 5 | `SYS_CLOSE` | `close(fd)` | experimental |
| 6 | `SYS_READ` | `read(fd, buffer, length)` | experimental |
| 7 | `SYS_LSEEK` | `lseek(fd, offset)` | experimental |
| 8 | `SYS_STAT` | `stat(fd, struct nova_stat *)` | experimental |

Los números no deben reutilizarse una vez publicados en una versión estable.

`SYS_WRITE` mantiene `fd=1` como salida a la consola serial (copiando realmente los bytes del buffer de usuario). Con un descriptor de archivo devuelto por `SYS_OPEN` (fd >= 3) escribe en el archivo del VFS en memoria.

## Syscalls actuales

### `SYS_GETPID`

Devuelve el PID del proceso actual usando la relación interna proceso → tarea. En esta etapa los procesos siguen siendo kernel-only.

### `SYS_WRITE`

Dirige la salida a la consola serial si `fd=1`, o al archivo del VFS si `fd` es un descriptor devuelto por `SYS_OPEN`. El buffer debe estar dentro del espacio de usuario, no desbordar el rango permitido y tener sus páginas presentes.

### `SYS_OPEN`

Abre un archivo existente del VFS en memoria a partir de una ruta absoluta (`/hello.txt`) y devuelve un descriptor >= 3, o un error negativo. No crea archivos todavía (`SYS_CREATE` queda para una fase posterior).

### `SYS_CLOSE`

Cierra el descriptor y lo libera. Devuelve 0 o `-EBADF`.

### `SYS_READ`

Copia desde el archivo del descriptor hasta `length` bytes en el buffer de usuario. Devuelve los bytes leídos (0 en EOF) o un error negativo.

### `SYS_LSEEK`

Reposiciona el offset del descriptor. Devuelve el nuevo offset o un error negativo si excede el tamaño actual del archivo.

### `SYS_STAT`

Rellena `struct nova_stat` (ino, kind, size) para el descriptor abierto. Devuelve 0 o un error negativo.

### `SYS_EXIT`

Marca la ejecución de prueba como terminada y entrega el control al camino de retorno/scheduler mínimo. No hay zombies, `wait`, parent/child ni recolección avanzada.

## Errores experimentales

```text
-1  ENOSYS  syscall desconocida
-2  EBADF   descriptor no válido
-3  EFAULT  puntero o rango de usuario no válido
-4  EINVAL  argumento no válido
-5  ENOENT  ruta inexistente
-6  EEXIST  la ruta ya existe
-7  ENOMEM  sin memoria para la operación
-8  EMFILE  sin descriptores libres
```

Una syscall inválida debe devolver un error controlado, nunca provocar un kernel panic.

## Registros

El stub de entrada guarda los registros generales necesarios para ejecutar C y restaura el contexto antes de `IRETQ`. El estado FPU/SSE/AVX no forma parte de esta milestone. La interfaz de usuario definitiva y los wrappers de libc se definirán más adelante.

## VFS en memoria (M2.4)

Las syscalls de archivo operan sobre un VFS mínimo con backend `tmpfs-like` en RAM: árbol `/` con archivos regulares cuyos datos viven en el heap del kernel (`kmalloc`). Se representan `vnode` (con `ino`, `kind`, `name`, `size`, `data`), descriptores (`struct nova_file`, tabla global de 64 entradas, fd >= 3) y `stat` (`struct nova_stat`). No hay aún initramfs, filesystems persistentes, userspace de archivos ni libc.
