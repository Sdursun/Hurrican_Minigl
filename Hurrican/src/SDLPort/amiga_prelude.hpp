// Force-included first into every translation unit of the AmigaOS 68k build
// (see PLATFORM AMIGA in CMakeLists.txt).

#ifndef _AMIGA_PRELUDE_HPP_
#define _AMIGA_PRELUDE_HPP_

// exec/types.h declares "typedef ... TEXT", which clashes with Hurrican's
// namespace TEXT. Pull in every system header that includes it while the
// name is renamed; their include guards keep them from being parsed again.
#define TEXT AMIGA_TEXT
#include <exec/types.h>
#include <dos/dos.h>
// Plain prototypes (linked through libnix's stubs): proto/dos.h would define
// Open(), Exit(), Read()... as macros and break Hurrican's methods of that name
#include <clib/dos_protos.h>
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_mixer.h>
#include <proto/minigl.h>
#undef TEXT

// libstdc++ for m68k-amigaos is built without the C99 math TR1 functions
// in namespace std, although libnix declares them. glm needs them.
#include <cmath>
#undef log2

namespace std {
using ::acosh;
using ::asinh;
using ::atanh;
using ::exp2;
using ::fma;
using ::round;
using ::trunc;
inline double log2(double x) { return ::log(x) * 1.4426950408889634; }
inline float log2(float x) { return static_cast<float>(::log(x) * 1.4426950408889634); }
}  // namespace std

#endif  // _AMIGA_PRELUDE_HPP_
