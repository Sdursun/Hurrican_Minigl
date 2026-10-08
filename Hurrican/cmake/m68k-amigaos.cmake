# CMake toolchain file for AmigaOS 3.x 68k (AmigaPorts/bebbo m68k-amigaos-gcc)
#
#   cmake -DCMAKE_TOOLCHAIN_FILE=../cmake/m68k-amigaos.cmake -DPLATFORM=AMIGA ..

set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR m68k)

set(AMIGA_TOOLCHAIN_PREFIX "/opt/amiga" CACHE PATH "m68k-amigaos-gcc install prefix")

set(CMAKE_C_COMPILER   ${AMIGA_TOOLCHAIN_PREFIX}/bin/m68k-amigaos-gcc)
set(CMAKE_CXX_COMPILER ${AMIGA_TOOLCHAIN_PREFIX}/bin/m68k-amigaos-g++)
set(CMAKE_AR           ${AMIGA_TOOLCHAIN_PREFIX}/bin/m68k-amigaos-ar)
set(CMAKE_RANLIB       ${AMIGA_TOOLCHAIN_PREFIX}/bin/m68k-amigaos-ranlib)

# PiStorm/Emu68 presents a 68040 with FPU; libnix runtime (no ixemul.library)
set(AMIGA_CPU_FLAGS "-m68040 -mhard-float -noixemul")
set(CMAKE_C_FLAGS_INIT   "${AMIGA_CPU_FLAGS}")
set(CMAKE_CXX_FLAGS_INIT "${AMIGA_CPU_FLAGS}")
set(CMAKE_EXE_LINKER_FLAGS_INIT "${AMIGA_CPU_FLAGS}")

set(CMAKE_FIND_ROOT_PATH ${AMIGA_TOOLCHAIN_PREFIX}/m68k-amigaos)
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

# Linking a test executable needs the full runtime, a static library is enough
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)
