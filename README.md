[![CI build](https://github.com/HurricanGame/Hurrican/actions/workflows/build.yml/badge.svg)](https://github.com/HurricanGame/Hurrican/actions/workflows/build.yml)

A fork of Hurrican, freeware jump and shoot game created by Poke53280, with SDL2 enabled by default, support for libopenmpt and CRT simulation.
Also uses XDG compliant data/config paths on UNIX.
Additional userlevels from http://turricanforever.de included.

Original code by Eiswuxe (Poke53280) [[Winterworks](https://www.winterworks.de/project/hurrican/)]  
Further work by [Pickle136](https://sourceforge.net/projects/hurrican/), Stefan Schmidt ([thrimbor](https://github.com/thrimbor/Hurrican)) and Leandro Nini ([drfiemost](https://github.com/drfiemost/Hurrican))  
CRT simulation partially based on [CRT effect - Shadertoy, Unity](https://luka712.github.io/2018/07/21/CRT-effect-Shadertoy-Unity/) article from luka712's blog

![screenshot](https://github.com/HurricanGame/Hurrican/wiki/images/level1.png)

### AmigaOS 68k / MiniGL port

This repository adds a native AmigaOS 3.x 68k port that draws through
minigl.library (PiStorm3D on PiStorm, QuarkTex NG in WinUAE), built with
SDL 1.2 and the `GL1` renderer. Build instructions are in
[Hurrican/amiga/README.md](Hurrican/amiga/README.md), the release archive is
on the Releases page and on Aminet as `game/shoot/Hurrican_Minigl.lha`.

Licenses: the game and its assets keep Poke53280's terms (free, not for sale,
see [README.txt](README.txt)); the Amiga port's own code is MIT
([Hurrican/amiga/LICENSE](Hurrican/amiga/LICENSE)).

---

### Dependencies

The code depends on SDL2 (or the old deprecated SDL) with the image and mixer components, and libepoxy.
Optionally libopenmpt can be used for the music in place of the standard from SDL_mixer (see below).
A compiler with c++17 support is required.

### Building

The code can be built using cmake (tested on Linux and MinGW)

    git clone --recurse-submodules https://github.com/HurricanGame/Hurrican.git
    cd Hurrican/Hurrican
    mkdir build && cd build
    cmake -DCMAKE_BUILD_TYPE=Release ..
    cmake --build .

The following build options are available:

Predefined platforms
* -DPLATFORM=<RPI|GCW|RDU|RDU2|PANDORA> : Compile for the specified platform, setting the correct options

OpenGL Options
* -DRENDERER=GL1          : Use the OpenGL 1.X code (fixed pipline)
* -DRENDERER=GLES1        : Use the OpenGL 1.X code with ES compatible
* -DRENDERER=GL2          : Use the OpenGL 2.0 code (programable pipline) [this is the default if not specified]
* -DRENDERER=GLES2        : Use the OpenGL 2.0 code with ES compatible
* -DRENDERER=GL3          : Use the OpenGL 3.0 code (programable pipline)
* -DRENDERER=GLES3        : Use the OpenGL 3.0 code with ES compatible
* -DFBO=ON                : Add FBO support, allow screen to be scaled to arbitrary dimensions, available only with GL2 or GL3 and enabled by default
* -DDEFAULT_SCREENBPP=<16|24|32> : Set the default screen depth, 32 if not specified

Sound
* -DOPENMPT=ON            : Use the libopenmpt code for music (SDL2_mixer uses libmodplug while SDL_mixer uses the lower quality mikmod engine)

Generic
* -DFAST_RANDOM=OFF             : Use standard C random function in place of the fast [LCG](https://en.wikipedia.org/wiki/Linear_congruential_generator)
* -DFAST_TRIG=ON                : Use fast approximation for trigonometric functions
* -DDISABLE_EXCEPTIONS=ON       : Disable exception handling to reduce binary size
* -DUSE_PRECOMPILED_HEADERS=ON  : Enable pre-compiled headers for better compile time

Debug
* -DDISABLE_MEMPOOLING=ON : Bypass pooled memory manager
* -DCMAKE_BUILD_TYPE=<Asan|Ubsan>: Enable the Address or Undefined Behaviour Sanitizer

### Running

To launch Hurrican, go back under the Hurrican folder

    cd ..
    ./build/hurrican

To see the available command line options use the `--help` argument

    ./build/hurrican --help

or check online at https://github.com/HurricanGame/Hurrican/wiki/Help

---

### Credits and thanks

**Hurrican** was created by **Jörg M. Winterstein (Eiswuxe)** and the team of
**Poke53280**, and released as freeware in 2007
([winterworks](https://www.winterworks.de/project/hurrican/)). Many thanks for
the game, and for releasing its source code and assets so that it can be
ported to platforms like the Amiga.

- Graphics: Thomas "Turri" Schreiter
- Additional level design: T.R. Schmidt and Gerhard "Schlauchi" Weihrauch
- SDL/OpenGL port: Pickle136 and Scott R. Smith
- Further work: Stefan Schmidt ([thrimbor](https://github.com/thrimbor/Hurrican)),
  Leandro Nini ([drfiemost](https://github.com/drfiemost/Hurrican)) and the
  [HurricanGame](https://github.com/HurricanGame/Hurrican) contributors
- User levels from [turricanforever.de](http://turricanforever.de) by their authors
- Hurrican is inspired by Manfred Trenz's Turrican; it is not affiliated with or
  endorsed by Turrican's authors or rights holders

The AmigaOS port builds on the work of:

- Steffen "MagicSN" Haeuser: minigl.library and SDL 1.2 with its minigl.library backend
- Dennis van der Boon: PiStorm3D, the VideoCore V3D backend
- Claude Schwarz, Michal Schulz and the PiStorm and Emu68 projects
- The authors of SDL, SDL_image, SDL_mixer, libmodplug, zlib, libpng and glm
- bebbo and the AmigaPorts m68k-amigaos-gcc toolchain

AmigaOS port: Serkan DURSUN
