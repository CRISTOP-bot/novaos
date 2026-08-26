#!/bin/sh
set -eu
LIMINE_VERSION=${LIMINE_VERSION:-v8.6.0-binary}
root=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
dir="$root/toolchain"
mkdir -p "$dir"
if [ ! -d "$dir/limine/.git" ]; then
  git clone --depth 1 --branch "$LIMINE_VERSION" https://github.com/limine-bootloader/limine.git "$dir/limine"
else
  current=$(git -C "$dir/limine" describe --tags --exact-match 2>/dev/null || true)
  [ "$current" = "$LIMINE_VERSION" ] || { echo "Limine checkout is $current, expected $LIMINE_VERSION" >&2; exit 1; }
fi
