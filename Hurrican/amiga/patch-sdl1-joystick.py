#!/usr/bin/env python3
"""Fix the SDL 1.2 lowlevel.library joystick driver (joystick/SDL_sysjoystick.c).

The driver reported directions only as two axes with the value +-257 and no
hat. Hurrican walks with the hat (D-pad) by default and scales axes from SDL's
+-32767 range to +-1000 with a threshold of 500, so the player never moved
while the buttons worked. After this patch the directions are reported as
axes 0/1 with +-32767 and as hat 0, and a mouse in the port no longer turns
into joystick directions.

Usage: patch-sdl1-joystick.py <SDL1>/joystick/SDL_sysjoystick.c
"""
import re
import sys

MARK = "/* hurrican: axes +-32767 and hat 0 */"

path = sys.argv[1]
src = open(path, encoding="latin-1").read()
if MARK in src:
    sys.exit(0)

src, n = re.subn(r"joystick->nhats=0;", "joystick->nhats=1;", src)
if n != 1:
    sys.exit("patch-sdl1-joystick: nhats line not found")

block = re.compile(r"#if 1\s*\n\s*if\(data&JP_DIRECTION_MASK\).*?#endif[^\n]*\n", re.S)
new = """\t\t%s
\t\t{
\t\t\tSint16 x = 0, y = 0;
\t\t\tUint8 hat = SDL_HAT_CENTERED;

\t\t\tif((data&JP_TYPE_MASK)!=JP_TYPE_MOUSE)
\t\t\t{
\t\t\t\tif(data&JPF_JOY_UP)    { y = -32767; hat |= SDL_HAT_UP; }
\t\t\t\tif(data&JPF_JOY_DOWN)  { y =  32767; hat |= SDL_HAT_DOWN; }
\t\t\t\tif(data&JPF_JOY_LEFT)  { x = -32767; hat |= SDL_HAT_LEFT; }
\t\t\t\tif(data&JPF_JOY_RIGHT) { x =  32767; hat |= SDL_HAT_RIGHT; }
\t\t\t}
\t\t\tif(joystick->axes[0]!=x) SDL_PrivateJoystickAxis(joystick,0,x);
\t\t\tif(joystick->axes[1]!=y) SDL_PrivateJoystickAxis(joystick,1,y);
\t\t\tif(joystick->hats[0]!=hat) SDL_PrivateJoystickHat(joystick,0,hat);
\t\t}
""" % MARK
src, n = block.subn(lambda m: new, src)
if n != 1:
    sys.exit("patch-sdl1-joystick: direction block not found")

open(path, "w", encoding="latin-1").write(src)
