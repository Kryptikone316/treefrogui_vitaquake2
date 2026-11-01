#!/bin/sh
# Builds vitaquake2_libretro.so for the Data Frog SF3500 (MIPS32r2 74Kc, hard float, no GPU).
#
#   MIPS_PREFIX=/path/to/mips-mti-linux-gnu- [SYSROOT=/path/to/sysroot] [GAME=rogue|xatrix|zaero] ./build_sf3500.sh
#
# -ffast-math and -mdspr2 are deliberately NOT used.  With them the software renderer's alias
# span loop (R_PolysetDrawSpans8_Opaque) crashed on the real device within seconds to minutes
# of the opening demo (never in QEMU); without them, 7200 frames of the same demo ran clean.
# See README-SF3500.md.
set -eu

: "${MIPS_PREFIX:?set MIPS_PREFIX to the toolchain prefix, including the trailing dash}"
HERE=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
ARCH="-mips32r2 -march=mips32r2 -mtune=74kc -mfp32 -mhard-float -mlong-calls -EL -G0 -DSF3500 -fprefetch-loop-arrays"
[ -n "${SYSROOT:-}" ] && ARCH="$ARCH --sysroot=$SYSROOT"
STRIP=${STRIP:-${MIPS_PREFIX}strip}

# GAME picks the official add-on; each is a separate core because the game code is linked in.
EXTRA=""
OUT=vitaquake2_libretro.so
case "${GAME:-}" in
	rogue|xatrix|zaero) EXTRA="basegame=$GAME"; OUT="vitaquake2-${GAME}_libretro.so" ;;
	"") ;;
	*) echo "unknown GAME=$GAME" >&2; exit 1 ;;
esac

WRAP=$(mktemp)
trap 'rm -f "$WRAP"' EXIT HUP INT TERM
cat > "$WRAP" <<WRAPPER
#!/bin/sh
exec ${MIPS_PREFIX}gcc $ARCH "\$@" -O2
WRAPPER
chmod +x "$WRAP"

cd "$HERE"
make clean >/dev/null 2>&1 || true
make platform=unix HAVE_OPENGL=0 GL_LIB= $EXTRA CC="$WRAP" CXX="$WRAP" -j"$(nproc 2>/dev/null || echo 4)"
"$STRIP" -o "${OUT%.so}.stripped.so" "$OUT"
echo "built $HERE/${OUT%.so}.stripped.so"
