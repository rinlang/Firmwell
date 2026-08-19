#!/bin/sh
# Build libnvram-faker.so for x86_64 (glibc).
# Override compiler with X86_64_GLIBC_CC.
set -e

SRC_DIR=$(cd "$(dirname "$0")" && pwd)
OUT_DIR=$(dirname "$SRC_DIR")/lib

X86_64_GLIBC_CC=${X86_64_GLIBC_CC:-gcc}

echo "=== Building x86_64/glibc with $X86_64_GLIBC_CC ==="
make -C "$SRC_DIR" clean >/dev/null 2>&1 || true
make -C "$SRC_DIR" CC="$X86_64_GLIBC_CC" AR="${X86_64_GLIBC_CC%-gcc}-ar"
mkdir -p "$OUT_DIR/x86_64/glibc"
cp "$SRC_DIR/libnvram-faker.so" "$OUT_DIR/x86_64/glibc/libnvram-faker.so"
echo "-> $OUT_DIR/x86_64/glibc/libnvram-faker.so"
