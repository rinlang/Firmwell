#!/bin/sh
# Build libnvram-faker.so for ARM (glibc/musl/uclibc).
# Override compilers with ARM_GLIBC_CC, ARM_MUSL_CC, ARM_UCLIBC_CC.
set -e

SRC_DIR=$(cd "$(dirname "$0")" && pwd)
OUT_DIR=$(dirname "$SRC_DIR")/lib

build_one() {
    libc="$1"
    cc="$2"
    ar="$3"
    echo "=== Building arm/$libc with $cc ==="
    make -C "$SRC_DIR" clean >/dev/null 2>&1 || true
    make -C "$SRC_DIR" CC="$cc" AR="$ar"
    mkdir -p "$OUT_DIR/arm/$libc"
    cp "$SRC_DIR/libnvram-faker.so" "$OUT_DIR/arm/$libc/libnvram-faker.so"
    echo "-> $OUT_DIR/arm/$libc/libnvram-faker.so"
}

ARM_GLIBC_CC=${ARM_GLIBC_CC:-arm-linux-gnueabi-gcc}
ARM_MUSL_CC=${ARM_MUSL_CC:-arm-linux-musleabi-gcc}
ARM_UCLIBC_CC=${ARM_UCLIBC_CC:-arm-linux-uclibcgnueabi-gcc}

build_one glibc  "$ARM_GLIBC_CC"  "${ARM_GLIBC_CC%-gcc}-ar"
build_one musl   "$ARM_MUSL_CC"   "${ARM_MUSL_CC%-gcc}-ar"
build_one uclibc "$ARM_UCLIBC_CC" "${ARM_UCLIBC_CC%-gcc}-ar"
