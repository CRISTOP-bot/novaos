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

Los números no deben reutilizarse una vez publicados en una versión estable.

## Syscalls actuales

### `SYS_GETPID`

Devuelve el PID del proceso actual usando la relación interna proceso → tarea. En esta etapa los procesos siguen siendo kernel-only.

### `SYS_WRITE`

Sólo acepta `fd=1` y dirige la salida a la consola serial. El buffer debe estar dentro del espacio de usuario, no desbordar el rango permitido y tener sus páginas presentes y marcadas como `USER`. Todavía no existe VFS ni una tabla de descriptores completa.

### `SYS_EXIT`

Marca la ejecución de prueba como terminada y entrega el control al camino de retorno/scheduler mínimo. No hay zombies, `wait`, parent/child ni recolección avanzada.

## Errores experimentales

```text
-1  ENOSYS  syscall desconocida
-2  EBADF   descriptor no válido
-3  EFAULT  puntero o rango de usuario no válido
-4  EINVAL  argumento no válido
```

Una syscall inválida debe devolver un error controlado, nunca provocar un kernel panic.

## Registros

El stub de entrada guarda los registros generales necesarios para ejecutar C y restaura el contexto antes de `IRETQ`. El estado FPU/SSE/AVX no forma parte de esta milestone. La interfaz de usuario definitiva y los wrappers de libc se definirán más adelante.
