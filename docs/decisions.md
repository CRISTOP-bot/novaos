# Decisiones técnicas de NovaOS

## ADR-0001: kernel monolítico modular

Se mantiene un kernel monolítico pequeño para reducir mecanismos antes de ejecutar software real. Las interfaces internas separan memoria, tareas, procesos y syscalls.

## ADR-0002: Limine primero

Limine se usa como bootloader de desarrollo y el kernel consume una estructura interna `boot_info`, evitando acoplar el resto del sistema al protocolo.

## ADR-0003: ABI propia

NovaOS no copia la ABI de Linux. La interfaz experimental actual usa `INT 0x80`, números propios y validación explícita de argumentos. Su especificación vive en [abi.md](abi.md).

## ADR-0004: scheduler y procesos incrementales

El scheduler inicial es single-core y cooperativo. Los procesos actuales son una abstracción mínima de PID + tarea; todavía no tienen address spaces independientes.

## ADR-0005: validación reproducible

Una milestone sólo se marca completa cuando build, ELF checks, QEMU, salida serial y GitHub Actions pasan. Compilar no es suficiente.

## Estado

M0–M2.3 están verificadas. M2.4 (VFS mínimo en memoria) pasa build, ELF checks, QEMU y salida serial localmente y está pendiente de confirmación en GitHub Actions antes de declararse completada.
