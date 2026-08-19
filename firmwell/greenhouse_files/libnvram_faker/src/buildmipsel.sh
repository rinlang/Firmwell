#!/bin/sh
# Build libnvram-faker.so for MIPSEL little-endian (glibc/musl/uclibc).
# Override compilers with MIPSEL_GLIBC_CC, MIPSEL_MUSL_CC, MIPSEL_UCLIBC_CC.
set -e

SRC_DIR=$(cd "$(dirname "$0")" && pwd)
OUT_DIR=$(dirname "$SRC_DIR")/lib

build_one() {
    libc="$1"
    cc="$2"
    ar="$3"
    echo "=== Building mipsel/$libc with $cc ==="
    make -C "$SRC_DIR" clean >/dev/null 2>&1 || true
    make -C "$SRC_DIR" CC="$cc" AR="$ar"
    mkdir -p "$OUT_DIR/mipsel/$libc"
    cp "$SRC_DIR/libnvram-faker.so" "$OUT_DIR/mipsel/$libc/libnvram-faker.so"
    echo "-> $OUT_DIR/mipsel/$libc/libnvram-faker.so"
}

MIPSEL_GLIBC_CC=${MIPSEL_GLIBC_CC:-mipsel-linux-gnu-gcc}
MIPSEL_MUSL_CC=${MIPSEL_MUSL_CC:-mipsel-linux-musl-gcc}
MIPSEL_UCLIBC_CC=${MIPSEL_UCLIBC_CC:-mipsel-linux-uclibc-gcc}

build_one glibc  "$MIPSEL_GLIBC_CC"  "${MIPSEL_GLIBC_CC%-gcc}-ar"
build_one musl   "$MIPSEL_MUSL_CC"   "${MIPSEL_MUSL_CC%-gcc}-ar"
build_one uclibc "$MIPSEL_UCLIBC_CC" "${MIPSEL_UCLIBC_CC%-gcc}-ar"
