# Roadmap por fases

Una milestone sólo se completa con build, ELF checks, QEMU, salida serial y GitHub Actions en verde.

## M0 — Boot y kernel mínimo ✅

Boot x86_64 con Limine, consola UART, CPUID, GDT, IDT, excepciones, linker script, ISO y smoke test.

## M1 — Memoria ✅

- M1.0: PMM.
- M1.1: paging de cuatro niveles.
- M1.2: heap del kernel.
- M1.3: diagnósticos de memoria.

## M2 — Privilegios y ejecución ✅ / en desarrollo

- M2.0: TSS, Ring 3 y retorno controlado ✅
- M2.1: scheduler cooperativo kernel-only ✅
- M2.2: procesos mínimos y PID ✅
- M2.3: syscalls mínimas mediante `INT 0x80` ✅
- M2.4: VFS mínimo en memoria (`vnode`, descriptores, backend `tmpfs-like`) + syscalls `open/close/read/write/lseek/stat` — **en validación en CI**

## M3 — Address spaces y programas

- address space por proceso;
- validación robusta de copia user/kernel;
- loader ELF64 `ET_EXEC`;
- stack inicial de userspace;
- `spawn` y proceso inicial.

## M4 — Filesystems y libc

- initramfs y backend de archivo real sobre el VFS (ext2/FAT después);
- descriptores por proceso;
- sysroot y libc freestanding/hosted;
- `/init`, shell y utilidades pequeñas.

## M5+ — Evolución

Fork/COW, señales, threads, scheduler preemptivo, drivers adicionales, networking, SMP y otras arquitecturas sólo después de que las bases anteriores tengan pruebas reproducibles.
