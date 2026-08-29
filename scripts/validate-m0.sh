#!/bin/sh
set -eu
export LC_ALL=C
command -v make >/dev/null || { echo 'make is required' >&2; exit 1; }
make kernel
readelf -h build/novaos.elf | grep -q 'ELF64'
make image
make test
