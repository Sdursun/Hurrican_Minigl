#!/bin/bash
# Packs the Aminet release: Hurrican_Minigl.lha and Hurrican_Minigl.readme in $OUT.
#
# Usage: make-aminet.sh <stripped hurrican executable> [out dir]
# Needs lha, Python 3 with Pillow (for the icon), and the sources fetched by
# build-deps.sh in $DEPS (their license files go into the archive).

set -euo pipefail
LHA=${LHA:-lha}

EXE=$1
HERE=$(cd "$(dirname "$0")" && pwd)
ROOT=$(cd "$HERE/../.." && pwd)
OUT=${2:-$ROOT/dist-aminet}
DEPS=${DEPS:-$HOME/amiga-deps}
TOOLCHAIN_SRC=${TOOLCHAIN_SRC:-$HOME/m68k-amigaos-gcc}
MINIGL_SDK=${MINIGL_SDK:-$ROOT/PiStorm3D}

STAGE=$(mktemp -d)
trap 'rm -rf "$STAGE"' EXIT
GAME=$STAGE/Hurrican
DOCS=$GAME/Docs
LIC=$DOCS/Licenses
mkdir -p "$LIC"

cp "$EXE" "$GAME/Hurrican"
chmod +x "$GAME/Hurrican"
python3 -W ignore "$HERE/make-icon.py" "$ROOT/Hurrican/Hurrican.ico" "$GAME/Hurrican.info"
# shaders are for the GL2/GL3 renderers, pvr textures for PowerVR GPUs
rsync -a --exclude shaders --exclude pvr "$ROOT/Hurrican/data" "$GAME/"
rsync -a "$ROOT/Hurrican/lang" "$GAME/"

cp "$HERE/aminet/Hurrican_Minigl.readme" "$DOCS/Hurrican_Minigl.readme"
cp "$ROOT/Hurrican/readme.txt" "$DOCS/readme.txt"
cp "$ROOT/README.txt" "$DOCS/README-source.txt"
cp "$HERE/LICENSE" "$DOCS/LICENSE-port.txt"

cp "$DEPS/SDL1/COPYING" "$LIC/SDL-LGPL-2.1.txt"
cp "$DEPS/SDL_image/COPYING" "$LIC/SDL_image.txt"
cp "$DEPS/SDL_mixer/COPYING" "$LIC/SDL_mixer.txt"
cp "$DEPS/libmodplug/COPYING" "$LIC/libmodplug.txt"
cp "$TOOLCHAIN_SRC"/projects/zlib-*/LICENSE "$LIC/zlib.txt"
cp "$TOOLCHAIN_SRC"/projects/libpng-*/LICENSE "$LIC/libpng.txt"
cp "$MINIGL_SDK/licences/License_MiniGL_Library.txt" "$LIC/MiniGL.txt"
cp "$ROOT/Hurrican/3rdparty/glm/copying.txt" "$LIC/glm.txt"

# Amiga text: LF line ends
find "$DOCS" -name '*.txt' -o -name '*.readme' | xargs sed -i 's/\r$//'

mkdir -p "$OUT"
rm -f "$OUT/Hurrican_Minigl.lha"
(cd "$STAGE" && "$LHA" aq "$OUT/Hurrican_Minigl.lha" Hurrican)
cp "$HERE/aminet/Hurrican_Minigl.readme" "$OUT/Hurrican_Minigl.readme"
ls -l "$OUT"
