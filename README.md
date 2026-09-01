[![NovaOS CI](https://github.com/CRISTOP-bot/novaos/actions/workflows/novaos-tests.yml/badge.svg)](https://github.com/CRISTOP-bot/novaos/actions/workflows/novaos-tests.yml)

# NovaOS

NovaOS es un kernel experimental **x86_64 freestanding**, construido desde cero para aprender y verificar los mecanismos internos de un sistema operativo.

> Estado actual: M0–M2.2 completadas y verificadas con GitHub Actions, QEMU y salida serial. M2.3 (syscalls) sigue en desarrollo y no se considera validada.

## Milestones verificadas

- **M0:** boot, CPU, GDT, IDT y excepciones.
- **M1.0:** gestor físico de páginas (PMM).
- **M1.1:** paging de cuatro niveles.
- **M1.2:** heap del kernel (`kmalloc`, `kfree`, `kcalloc`).
- **M1.3:** diagnósticos de memoria.
- **M2.0:** TSS, `LTR`, Ring 3 y retorno controlado.
- **M2.1:** scheduler cooperativo mínimo de tareas kernel.
- **M2.2:** abstracción mínima proceso → tarea → scheduler.
- **M2.3:** ABI experimental de syscalls mediante `INT 0x80` (en validación).

## Marcadores de runtime

Las pruebas seriales verifican, en orden:

```text
NOVAOS_PMM_OK
NOVAOS_PAGING_OK
NOVAOS_HEAP_OK
NOVAOS_MEMORY_OK
NOVAOS_RING3_OK
NOVAOS_SCHEDULER_OK
NOVAOS_PROCESS_OK
NOVAOS_BOOT_OK
```

`NOVAOS_SYSCALL_OK` sólo se añadirá cuando la ruta Ring 3 → `INT 0x80` → dispatcher → retorno haya pasado QEMU y CI de forma reproducible.

## Construcción y pruebas

Requisitos habituales: GCC, binutils, Git, xorriso y QEMU.

```sh
make limine
make kernel
make image
make test
make run
```

`make test` construye el ELF/ISO, ejecuta las validaciones ELF y arranca QEMU con el smoke test serial. GitHub Actions ejecuta la misma cadena en un entorno limpio.

## Arquitectura

```text
Limine
  ↓
x86_64 entry / GDT / IDT / TSS
  ↓
PMM → paging → heap
  ↓
procesos mínimos → scheduler cooperativo
  ↓
Ring 3 → INT 0x80 → syscalls experimentales
```

El proyecto todavía no implementa un userspace completo, loader ELF, VFS, filesystem ni libc.

## Documentación

- [Arquitectura](docs/architecture.md)
- [ABI y syscalls](docs/abi.md)
- [Roadmap](docs/roadmap.md)
- [Decisiones técnicas](docs/decisions.md)
- [Workflow de desarrollo](docs/development-workflow.md)

## Principios

- correctness > simplicidad > rendimiento;
- cada milestone debe demostrar ejecución real en QEMU;
- no copiar automáticamente la ABI de Linux;
- mantener los componentes pequeños, freestanding y verificables;
- no declarar una milestone completa sólo porque compile.

## Licencia

La licencia y los componentes de terceros se documentarán antes de distribuir una release. Limine se obtiene mediante el script de toolchain; revisa sus archivos y licencia correspondiente antes de redistribuir artefactos.
