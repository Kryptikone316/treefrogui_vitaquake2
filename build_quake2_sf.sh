#!/bin/sh
# Build vitaquake2 (Quake II) libretro core for the Hichip MIPS handhelds.
# Software renderer only (HAVE_OPENGL=0 — our fb has no GL); runs under picoarch
# like tyrquake. Arch flags are baked into CC so the link step inherits them too.
set -e
TC="$HOME/sf3000-work/sf3000toolchain/mipsel-buildroot-linux-gnu_sdk-buildroot/opt/ext-toolchain/bin/mips-mti-linux-gnu-"
SYSROOT="$HOME/sf3000-work/sf3000toolchain/mipsel-buildroot-linux-gnu_sdk-buildroot/mipsel-buildroot-linux-gnu/sysroot"

ARCH="-mips32r2 -march=mips32r2 -mtune=24kc -mfp32 -mhard-float -mlong-calls -EL -G0 --sysroot=$SYSROOT"
CC="${TC}gcc $ARCH"
CXX="${TC}g++ $ARCH"

cd "$(dirname "$0")"
make -f Makefile clean >/dev/null 2>&1 || true
make -f Makefile platform=unix HAVE_OPENGL=0 GL_LIB= \
     CC="$CC" CXX="$CXX" -j8
echo "=== built: $(ls -la vitaquake2_libretro.so 2>/dev/null) ==="
file vitaquake2_libretro.so 2>/dev/null
