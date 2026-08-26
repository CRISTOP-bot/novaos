#!/bin/sh
set -eu
iso=${1:?ISO path required}; log=${2:?log path required}; qemu=${QEMU:-qemu-system-x86_64}
rm -f "$log"
"$qemu" -M q35 -m 128M -boot order=d -cdrom "$iso" -serial "file:$log" -display none -no-reboot >/dev/null 2>&1 &
pid=$!
cleanup(){ kill "$pid" 2>/dev/null || true; wait "$pid" 2>/dev/null || true; }
trap cleanup EXIT INT TERM
for i in $(seq 1 100); do
  if [ -f "$log" ] && grep -q 'NOVAOS_BOOT_OK' "$log" && grep -q 'NOVAOS_PMM_OK' "$log" && grep -q 'NOVAOS_PAGING_OK' "$log" && grep -q 'NOVAOS_HEAP_OK' "$log" && grep -q 'NOVAOS_MEMORY_OK' "$log" && grep -q 'NOVAOS_RING3_OK' "$log" && grep -q 'NOVAOS_SCHEDULER_OK' "$log" && grep -q 'NOVAOS_PROCESS_OK' "$log" && grep -q 'NOVAOS_SYSCALL_OK' "$log"; then
    echo 'NovaOS M0 smoke test: PASS'
    cat "$log"
    exit 0
  fi
  if ! kill -0 "$pid" 2>/dev/null; then
    echo 'NovaOS M0 smoke test: FAIL (QEMU exited before boot marker)' >&2
    [ -f "$log" ] && cat "$log" >&2 || true
    exit 1
  fi
  sleep 0.1
done
echo 'NovaOS M0 smoke test: FAIL (timeout waiting for NOVAOS_BOOT_OK)' >&2
[ -f "$log" ] && cat "$log" >&2 || true
exit 1
