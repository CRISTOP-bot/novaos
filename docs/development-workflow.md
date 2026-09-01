# Workflow de desarrollo de NovaOS

NovaOS usa **trunk-based development**: `main` debe permanecer estable y cada cambio entra mediante un Pull Request pequeño.

## Ramas

Usa ramas cortas con un único objetivo:

```text
feat/vfs
feat/syscalls
fix/page-fault
test/processes
docs/abi
```

```sh
git switch main
git pull
git switch -c feat/nombre-corto
```

No hagas push directo a `main`. Activa en GitHub la protección de rama y exige que pase `NovaOS CI` antes del merge.

## Commits

Prefiere commits pequeños y descriptivos:

```text
arch: add GDT helpers
mm: map user page
proc: assign initial PID
ci: validate debug build
```

Un commit debe poder revisarse y revertirse sin arrastrar cambios no relacionados.

## Validación local

Antes de abrir el PR:

```sh
make clean
make limine
make kernel
make image
make test
make DEBUG=1 BUILD=build-debug KERNEL=build-debug/novaos.elf kernel
```

`make test` valida el ELF, construye la ISO y arranca QEMU con salida serial. Una compilación exitosa por sí sola no valida una milestone.

## CI obligatorio

GitHub Actions ejecuta:

```text
shell/repository checks
        ↓
release + debug build
        ↓
ELF / ABI validation
        ↓
QEMU smoke test
        ↓
controlled exception test
```

Los jobs publican `novaos.elf`, `novaos.iso`, símbolos de debug, secciones y logs seriales como artifacts, incluso cuando fallan.

## Milestones

Cada milestone necesita:

1. implementación;
2. prueba reproducible;
3. marcador serial `NOVAOS_*_OK`;
4. validación en QEMU y CI;
5. documentación de la ABI o decisión técnica cuando aplique.

Al completarla, crea un tag:

```sh
git tag -a m2.2-processes -m "NovaOS M2.2: minimal processes"
git push origin m2.2-processes
```

Tags sugeridos: `m3.0-virtual-memory`, `m4.0-vfs` y `m5.0-userland`.

## Configuraciones

- **DEBUG:** `-g -O0`, assertions y diagnósticos.
- **RELEASE:** `-O2`, sin instrumentación adicional.

Toda nueva syscall o cambio de layout debe actualizar `docs/abi.md`. Toda decisión que afecte la arquitectura debe registrarse en `docs/decisions.md`.
