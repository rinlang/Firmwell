#!/bin/sh
# Build libnvram-faker.so for 32-bit x86 (glibc/uclibc).
# Override compilers with X86_GLIBC_CC, X86_UCLIBC_CC.
set -e

SRC_DIR=$(cd "$(dirname "$0")" && pwd)
OUT_DIR=$(dirname "$SRC_DIR")/lib

build_one() {
    libc="$1"
    cc="$2"
    ar="$3"
    echo "=== Building x86/$libc with $cc ==="
    make -C "$SRC_DIR" clean >/dev/null 2>&1 || true
    make -C "$SRC_DIR" CC="$cc" AR="$ar"
    mkdir -p "$OUT_DIR/x86/$libc"
    cp "$SRC_DIR/libnvram-faker.so" "$OUT_DIR/x86/$libc/libnvram-faker.so"
    echo "-> $OUT_DIR/x86/$libc/libnvram-faker.so"
}

X86_GLIBC_CC=${X86_GLIBC_CC:-i686-linux-gnu-gcc}
X86_UCLIBC_CC=${X86_UCLIBC_CC:-i686-linux-uclibc-gcc}

build_one glibc  "$X86_GLIBC_CC"  "${X86_GLIBC_CC%-gcc}-ar"
build_one uclibc "$X86_UCLIBC_CC" "${X86_UCLIBC_CC%-gcc}-ar"
