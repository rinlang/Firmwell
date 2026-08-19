#!/bin/sh
# Build libnvram-faker.so for all target/libc combinations.
# Adjust the CC/AR variables to match the cross toolchains installed on your system.
set -e

SRC_DIR="$(cd "$(dirname "$0")" && pwd)"
OUT_DIR="$(dirname "$SRC_DIR")/lib"

build_one() {
    arch="$1"
    libc="$2"
    cc="$3"
    echo "=== Building $arch/$libc with $cc ==="
    make -C "$SRC_DIR" clean >/dev/null 2>&1 || true
    make -C "$SRC_DIR" CC="$cc" AR="${cc%-gcc}-ar" >/dev/null
    mkdir -p "$OUT_DIR/$arch/$libc"
    cp "$SRC_DIR/libnvram-faker.so" "$OUT_DIR/$arch/$libc/libnvram-faker.so"
    echo "-> $OUT_DIR/$arch/$libc/libnvram-faker.so"
}

# Examples (replace with actual toolchain prefixes on your machine):
# build_one arm glibc   arm-linux-gnueabi-gcc
# build_one arm musl    arm-linux-musleabi-gcc
# build_one arm uclibc  arm-linux-uclibcgnueabi-gcc
# build_one mips glibc  mips-linux-gnu-gcc
# build_one mips musl   mips-linux-musl-gcc
# build_one mips uclibc mips-linux-uclibc-gcc
# build_one mipsel glibc mipsel-linux-gnu-gcc
# build_one mipsel musl  mipsel-linux-musl-gcc
# build_one mipsel uclibc mipsel-linux-uclibc-gcc
# build_one x86 glibc   i686-linux-gnu-gcc
# build_one x86 uclibc  i686-linux-uclibc-gcc
# build_one x86_64 glibc gcc

echo "Edit build_all.sh to set the correct cross compiler names, then run it."
