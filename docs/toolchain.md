# Toolchain y libc

## Decisión

El target de distribución será `x86_64-novaos`, sin reutilizar `x86_64-linux-gnu`. El nombre identifica el contrato del sysroot; no implica compatibilidad Linux.

Orden recomendado:

1. GNU Binutils cross (`as`, `ld`, `objcopy`, `readelf`, `ar`), porque ofrece control y scripts ELF maduros.
2. GCC cross stage 1 con `libgcc`, sin headers del host.
3. Newlib port de NovaOS, usando sólo sus hooks `_read`, `_write`, `_sbrk`, `_close`, `_fstat`, `_lseek`, `_isatty`, `_exit`, etc.
4. crt0, headers ABI, syscall wrappers y linker script propios.
5. GCC final con sysroot completo.
6. Clang/LLVM como alternativa después de que el contrato esté probado.

Newlib es la elección de bootstrap porque su port de OS está explícitamente basado en una capa de syscalls y no fuerza la ABI Linux. Se separa de upstream en `ports/newlib/` y se mantienen parches mínimos y documentados. No se copia glibc ni se enlaza con bibliotecas del host.

### Alternativas evaluadas

- **musl:** excelente y más cercana a POSIX, pero su port requiere una base de kernel/threading/signal/mmap mucho más completa; candidata para una fase posterior.
- **LLVM libc:** modular y atractiva, pero su superficie y toolchain de soporte aún no son el camino más corto para bootstrap.
- **relibc:** interesante para estudiar y posiblemente portar, pero añade dependencia de Rust y decisiones de Redox que NovaOS no necesita al inicio.
- **libc propia:** sólo headers, errno, startup y wrappers mínimos temporales; no reimplementar stdio/malloc si Newlib cubre el objetivo.

## Sysroot

```text
sysroot/
├── etc/
├── lib/                 # crt y runtime esenciales
└── usr/
    ├── include/        # ABI pública NovaOS
    ├── lib/             # libc, libgcc, objetos de arranque
    └── bin/
```

El compilador se invoca con `--target=x86_64-novaos --sysroot=$NOVA_SYSROOT -ffreestanding` sólo para kernel, y sin `-ffreestanding` para userspace. El kernel nunca enlaza libc ni usa headers de userspace.

## Startup y enlaces

Userspace:

```text
crt0.S → _start → runtime_init → main(argc, argv, envp) → exit
```

`crtbegin/crtend` y `libgcc` se incorporan sólo al enlazado hosted. El linker script de userland define segmentos separados RX/R/RW y un límite de stack; el loader debe respetar esos permisos.

## Reproducibilidad

- Versiones fijadas por archivo de releases/checksums.
- Descargas a `toolchain/src`; builds fuera del árbol fuente.
- Variables `TARGET`, `PREFIX`, `SYSROOT` configurables, con defaults dentro del árbol.
- CI ejecuta la misma receta que local.
- `pkg-config`, CMake y Ninja quedan fuera del bootstrap mínimo.
