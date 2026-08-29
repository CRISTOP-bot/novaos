#!/bin/sh
set -eu
export LC_ALL=C
elf=${1:?ELF path required}
readelf -h "$elf" | grep -q 'Class:[[:space:]]*ELF64'
readelf -h "$elf" | grep -q 'Machine:[[:space:]]*Advanced Micro Devices X86-64'
readelf -l "$elf" | grep -qv 'Requesting program interpreter' || { echo 'NovaOS kernel must not use a dynamic linker' >&2; exit 1; }
readelf -s "$elf" | grep -q '[[:space:]]_start$'
readelf -s "$elf" | grep -q '[[:space:]]kmain$'
echo 'NovaOS ELF validation: PASS'
