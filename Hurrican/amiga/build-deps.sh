#!/bin/bash
# Builds the static 68k libraries Hurrican needs for AmigaOS 3.x / PiStorm3D:
#   zlib, libpng (via the toolchain), MiniGL SDK, SDL 1.2 (MiniGL backend),
#   libmodplug, SDL_image 1.2 (PNG/BMP) and SDL_mixer 1.2 (WAV + modplug)
#
# Requires an installed AmigaPorts m68k-amigaos-gcc toolchain.
# Usage: build-deps.sh [toolchain-source-dir]

set -euo pipefail

PREFIX=${PREFIX:-/opt/amiga}
SDK=${SDK:-$PREFIX/hurrican-sdk}
DEPS=${DEPS:-$HOME/amiga-deps}
TOOLCHAIN_SRC=${1:-$HOME/m68k-amigaos-gcc}
JOBS=${JOBS:-$(nproc)}

export PATH=$PREFIX/bin:$PATH
CC=m68k-amigaos-gcc
CXX=m68k-amigaos-g++
AR=m68k-amigaos-ar
CPU="-m68040 -mhard-float -noixemul"
OPT="-O2 -fomit-frame-pointer"
HERE=$(cd "$(dirname "$0")" && pwd)

mkdir -p "$DEPS" "$SDK/include" "$SDK/lib"

clone() { # dir url [branch]
    [ -d "$DEPS/$1" ] && return
    git clone --depth 1 ${3:+-b $3} "$2" "$DEPS/$1"
}

echo "== zlib + libpng (toolchain targets)"
make -C "$TOOLCHAIN_SRC" zlib libpng PREFIX="$PREFIX"

echo "== AHI / CyberGraphX / Picasso96 SDKs (needed by SDL 1.2)"
for sdk in ahi cgx p96; do
    make -C "$TOOLCHAIN_SRC" sdk="$sdk" PREFIX="$PREFIX"
done

echo "== MiniGL SDK (PiStorm3D)"
# MINIGL_SDK: the SDK/ folder of a PiStorm3D release archive
# (minigl-shared-library/include, backend/include, lib/libminigl.a);
# without it the headers come from the source repository
rm -rf "$SDK/include/mgl" "$SDK/include/v3d"
if [ -n "${MINIGL_SDK:-}" ]; then
    cp -r "$MINIGL_SDK/minigl-shared-library/include/." "$SDK/include/"
    cp "$MINIGL_SDK/lib/libminigl.a" "$SDK/lib/"
    V3D_INCLUDE="$MINIGL_SDK/backend/include"
else
    clone minigl https://github.com/SteffenHaeuser/MiniGL_Library_68k.git MiniGL_PiStormV3D
    cp -r "$DEPS/minigl/include/." "$SDK/include/"
    cp "$DEPS/minigl/lib/libminigl.a" "$SDK/lib/"
    clone minigl-backend https://github.com/SteffenHaeuser/MiniGL_Library_68k.git PiStorm3D
    V3D_INCLUDE="$DEPS/minigl-backend/backend/include"
fi
# mgl/context.h and mgl/vertexbuffer.h include the V3D backend headers by a
# path relative to the source tree; ship them in include/v3d instead
mkdir -p "$SDK/include/v3d"
cp "$V3D_INCLUDE/"*.h "$SDK/include/v3d/"
sed -i 's|"\.\./\.\./\.\./backend/include/|"../v3d/|' "$SDK/include/mgl/"*.h

echo "== SDL 1.2 (MiniGL backend)"
clone SDL1 https://github.com/SteffenHaeuser/SDL1_MiniGL_Library.git
(
    cd "$DEPS/SDL1"
    find . -name '*.o' -delete
    # NDK 3.2 declares TimerBase as struct Device *
    sed -i -e 's/struct Library \*\s*TimerBase;/struct Device *TimerBase;/' \
           -e 's/TimerBase\(\s*\)= (struct Library \*)/TimerBase\1= (struct Device *)/' \
           timer/amigaos/SDL_systimer.c
    # Intuition takes at most 24 as SA_Depth; a 32-bit RTG mode is opened
    # with depth 24 and chosen by SA_DisplayID. With 32 OpenScreenTags fails
    # and SDL falls back to a window on Workbench ("Screen cant open")
    sed -i 's/SA_Depth,bpp,/SA_Depth,(bpp > 24 ? 24 : bpp),/' video/amigaos/SDL_cgxvideo.c
    # Our joystick driver: lowlevel.library with a hat and +-32767 axes (the
    # original's +-257 never moved Hurrican's player), and Xbox 360 pads
    # through xbox360w.class (see the file)
    cp "$HERE"/joystick/sdl1/SDL_sysjoystick.c "$HERE"/joystick/amiga_xbox360w.h \
       "$HERE"/joystick/xbox360w.h joystick/
    make -f Makefile.cross static-libnix CC="$CC" AR="$AR" \
        GCCFLAGS="$OPT -fpermissive -I. -I./inc -I./include -I./include/SDL -I$SDK/include -mhard-float -mcpu=68040"
)
cp "$DEPS/SDL1/libSDL_libnix.a" "$SDK/lib/libSDL.a"
# SDL 1.2 calls kprintf() for diagnostics (e.g. "Screen cant open"), which
# libnix's libdebug only provides as KPrintF() on the serial port; send them
# to stderr instead so they show up in the Shell the game was started from
cat > "$DEPS/SDL1/kprintf_stub.c" <<'EOF'
#include <stdarg.h>
#include <stdio.h>
void kprintf(const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    fputs("SDL: ", stderr);
    vfprintf(stderr, fmt, ap);
    va_end(ap);
}
EOF
$CC $CPU $OPT -c "$DEPS/SDL1/kprintf_stub.c" -o "$DEPS/SDL1/kprintf_stub.o"
$AR rs "$SDK/lib/libSDL.a" "$DEPS/SDL1/kprintf_stub.o"
mkdir -p "$SDK/include/SDL"
cp "$DEPS/SDL1/include/SDL/"*.h "$SDK/include/SDL/"

echo "== libmodplug"
clone libmodplug https://github.com/Konstanty/libmodplug.git
cmake -S "$DEPS/libmodplug" -B "$DEPS/libmodplug/build-amiga" \
    -DCMAKE_TOOLCHAIN_FILE="$HERE/../cmake/m68k-amigaos.cmake" \
    -DCMAKE_BUILD_TYPE=Release -DBUILD_SHARED_LIBS=OFF -DCMAKE_POLICY_VERSION_MINIMUM=3.5 \
    -DCMAKE_INSTALL_PREFIX="$SDK"
cmake --build "$DEPS/libmodplug/build-amiga" -j "$JOBS"
cmake --install "$DEPS/libmodplug/build-amiga"

build_static() { # name outlib cflags sources...
    local name=$1 out=$2 flags=$3; shift 3
    local obj="$DEPS/$name/build-amiga"
    rm -rf "$obj"; mkdir -p "$obj"
    for src in "$@"; do
        $CC $CPU $OPT $flags -I"$SDK/include" -I"$SDK/include/SDL" \
            -c "$DEPS/$name/$src" -o "$obj/$(basename "${src%.c}").o"
    done
    $AR rcs "$SDK/lib/$out" "$obj"/*.o
}

echo "== SDL_image 1.2 (PNG, BMP)"
clone SDL_image https://github.com/libsdl-org/SDL_image.git SDL-1.2
# IMG.c references every format; the disabled ones compile to stubs
build_static SDL_image libSDL_image.a "-DLOAD_PNG -DLOAD_BMP" IMG.c \
    IMG_bmp.c IMG_gif.c IMG_jpg.c IMG_lbm.c IMG_pcx.c IMG_png.c IMG_pnm.c \
    IMG_tga.c IMG_tif.c IMG_webp.c IMG_xcf.c IMG_xpm.c IMG_xv.c
cp "$DEPS/SDL_image/SDL_image.h" "$SDK/include/SDL/"

echo "== SDL_mixer 1.2 (WAV, modplug)"
clone SDL_mixer https://github.com/libsdl-org/SDL_mixer.git SDL-1.2
build_static SDL_mixer libSDL_mixer.a \
    "-DWAV_MUSIC -DMODPLUG_MUSIC -I$SDK/include/libmodplug" \
    mixer.c music.c effect_position.c effect_stereoreverse.c effects_internal.c \
    load_aiff.c load_voc.c wavestream.c music_modplug.c
cp "$DEPS/SDL_mixer/SDL_mixer.h" "$SDK/include/SDL/"

echo "== done, SDK in $SDK"
ls -l "$SDK/lib"
