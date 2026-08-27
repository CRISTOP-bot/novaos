# Arquitectura de NovaOS

NovaOS es un kernel monolítico modular experimental para x86_64. Sus interfaces internas son pequeñas y cada etapa se valida con una ejecución reproducible en QEMU.

## Capas actuales

```text
hardware x86_64
  └─ Limine y boot_info
     └─ entry / GDT / IDT / TSS / interrupts
        └─ PMM → paging → kernel heap
           └─ diagnostics
              └─ proceso mínimo → tarea → scheduler cooperativo
                 └─ Ring 3 → INT 0x80 → syscalls experimentales
```

## Implementado y verificado

M0 a M1.3 cubren el arranque, consola serial, CPUID, excepciones, mapa de memoria, PMM, paging, heap y diagnósticos.

M2.0 incorpora TSS de x86_64, `RSP0`, segmentos de usuario, transición temporal a Ring 3 y retorno controlado.

M2.1 incorpora cambio de contexto cooperativo single-core para tareas kernel.

M2.2 incorpora la abstracción mínima de procesos, PID, tarea asociada y estados básicos.

## En desarrollo

M2.3 integra una ABI experimental de syscalls sobre `INT 0x80`: `getpid`, `write` y `exit`. La milestone no se considera completa hasta que QEMU y GitHub Actions confirmen la ruta completa.

## Seguridad y límites

- El kernel se compila freestanding con warnings tratados como errores.
- Los punteros provenientes de Ring 3 deben validarse antes de leerse.
- Los fallos de usuario deben ser errores controlados; los fallos de kernel pueden producir panic.
- No se implementan aún address spaces por proceso, ELF loader, VFS, filesystem, libc, COW ni scheduler preemptivo.
