# Amiga port roadmap

## Later

### Try SDL2 instead of SDL 1.2

Build Hurrican with SDL2 on a separate branch and compare it with the SDL 1.2
build.

- Source: [SteffenHaeuser/SDL2_68k](https://github.com/SteffenHaeuser/SDL2_68k),
  which has minigl.library support. Prebuilt libnix and clib2 libraries
  (SDL2, SDL2_image, SDL2_mixer, SDL2_net, SDL2_sound, SDL2_gfx) are in
  the SDL_68k package. That package has no headers, so take them from the
  repository.
- Hurrican targets SDL2 upstream, and the `GL1` renderer works with it.
- Things to check:
  - whether SDL2_mixer plays the `.it` music (through modplug or another decoder)
  - whether a 32-bit fullscreen screen opens (SDL 1.2 needed the `SA_Depth`
    fix, see `build-deps.sh`)
  - sound, which hangs in AHI in the portable WinUAE test setup with SDL 1.2
  - input and game controllers
  - frame rate and executable size against the SDL 1.2 build
- The prebuilt SDL 1.2 in the SDL_68k package (2026-09-16) is older than
  our own build and lacks its patches; keep building SDL 1.2 from source.
