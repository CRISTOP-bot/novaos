## Qué cambia

<!-- Describe el cambio en una o dos frases. -->

## Milestone / área

- [ ] M0 — boot
- [ ] M1 — memoria
- [ ] M2 — privilegios, procesos o syscalls
- [ ] M3 — address spaces y programas
- [ ] M4 — VFS y libc
- [ ] Documentación / tooling

## Validación local

- [ ] `make kernel`
- [ ] `make image`
- [ ] `make test`
- [ ] `make DEBUG=1 BUILD=build-debug KERNEL=build-debug/novaos.elf kernel`
- [ ] Verifiqué la salida serial y los marcadores de runtime

## ABI y seguridad

- [ ] No cambié la ABI pública
- [ ] Si cambié la ABI, actualicé `docs/abi.md`
- [ ] No introduje dependencias dinámicas
- [ ] Añadí o actualicé pruebas

## Evidencia

<!-- Añade marcadores, logs o explica por qué no aplica. -->
