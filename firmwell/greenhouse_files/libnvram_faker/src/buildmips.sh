#!/bin/sh
# Build libnvram-faker.so for MIPS big-endian (glibc/musl/uclibc).
# Override compilers with MIPS_GLIBC_CC, MIPS_MUSL_CC, MIPS_UCLIBC_CC.
set -e

SRC_DIR=$(cd "$(dirname "$0")" && pwd)
OUT_DIR=$(dirname "$SRC_DIR")/lib

build_one() {
    libc="$1"
    cc="$2"
    ar="$3"
    echo "=== Building mips/$libc with $cc ==="
    make -C "$SRC_DIR" clean >/dev/null 2>&1 || true
    make -C "$SRC_DIR" CC="$cc" AR="$ar"
    mkdir -p "$OUT_DIR/mips/$libc"
    cp "$SRC_DIR/libnvram-faker.so" "$OUT_DIR/mips/$libc/libnvram-faker.so"
    echo "-> $OUT_DIR/mips/$libc/libnvram-faker.so"
}

MIPS_GLIBC_CC=${MIPS_GLIBC_CC:-mips-linux-gnu-gcc}
MIPS_MUSL_CC=${MIPS_MUSL_CC:-mips-linux-musl-gcc}
MIPS_UCLIBC_CC=${MIPS_UCLIBC_CC:-mips-linux-uclibc-gcc}

build_one glibc  "$MIPS_GLIBC_CC"  "${MIPS_GLIBC_CC%-gcc}-ar"
build_one musl   "$MIPS_MUSL_CC"   "${MIPS_MUSL_CC%-gcc}-ar"
build_one uclibc "$MIPS_UCLIBC_CC" "${MIPS_UCLIBC_CC%-gcc}-ar"
