#!/bin/sh
set -eu
iso=${1:?ISO path required}; log=${2:?log path required}; qemu=${QEMU:-qemu-system-x86_64}
rm -f "$log"
"$qemu" -M q35 -m 128M -cdrom "$iso" -serial "file:$log" -display none -no-reboot >/dev/null 2>&1 &
pid=$!
cleanup(){ kill "$pid" 2>/dev/null || true; wait "$pid" 2>/dev/null || true; }
trap cleanup EXIT INT TERM
for i in $(seq 1 100); do
  if [ -f "$log" ] && grep -qi 'exception' "$log" && grep -q 'KERNEL PANIC' "$log"; then
    echo 'NovaOS exception test: PASS'
    cat "$log"
    exit 0
  fi
  if ! kill -0 "$pid" 2>/dev/null; then
    echo 'NovaOS exception test: FAIL (QEMU exited before panic markers)' >&2
    [ -f "$log" ] && cat "$log" >&2 || true
    exit 1
  fi
  sleep 0.1
done
echo 'NovaOS exception test: FAIL (timeout)' >&2
[ -f "$log" ] && cat "$log" >&2 || true
exit 1
