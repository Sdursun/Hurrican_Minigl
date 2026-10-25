/*
  SDL2 joystick driver for AmigaOS 3.x: lowlevel.library ports, and the
  Xbox 360 pads of xbox360w.class.

  Written for the Hurrican Amiga port (2026, Serkan DURSUN) in place of
  SDL2_68k's src/joystick/amigaos3/SDL_sysjoystick.c, which reports no
  joysticks. Same devices as the SDL 1.2 driver of the MiniGL build:

  - joystick 0 is the joystick port (lowlevel port 1), joystick 1 the mouse
    port (lowlevel port 0)
  - a joystick or CD32 pad: directions as axes 0/1 (+-32767) and hat 0,
    buttons red, blue, play, yellow, green, forward, reverse (three for a
    plain joystick); a mouse in the port gives no directions
  - a port that xbox360w.class has a pad mapped to: an "Xbox 360
    Controller" with 13 buttons, 6 axes and a hat (amiga_xbox360w.h)

  Simple DirectMedia Layer
  Copyright (C) 1997-2024 Sam Lantinga <slouken@libsdl.org>

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.
*/

#include "../../SDL_internal.h"

#if SDL_JOYSTICK_AMIGAOS3

#include <exec/types.h>
#include <libraries/lowlevel.h>
#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/lowlevel.h>

#include "SDL_joystick.h"
#include "../SDL_sysjoystick.h"
#include "../SDL_joystick_c.h"

#include "amiga_xbox360w.h"

#define OS3_NUMPORTS 2

struct Library *LowLevelBase = NULL;

static SDL_JoystickID os3_instance[OS3_NUMPORTS];

static const ULONG os3_joybut[7] = {
    JPF_BUTTON_RED, JPF_BUTTON_BLUE, JPF_BUTTON_PLAY, JPF_BUTTON_YELLOW,
    JPF_BUTTON_GREEN, JPF_BUTTON_FORWARD, JPF_BUTTON_REVERSE
};

struct joystick_hwdata {
    ULONG port;
    int xbox;
};

static ULONG port_of(int index)
{
    return index == 0 ? 1 : 0;
}

static int OS3_JoystickInit(void)
{
    int i;

    if (!LowLevelBase) {
        LowLevelBase = OpenLibrary((CONST_STRPTR)"lowlevel.library", 37);
    }
    if (!LowLevelBase) {
        return 0;   /* no joysticks, not an error */
    }
    for (i = 0; i < OS3_NUMPORTS; i++) {
        os3_instance[i] = SDL_GetNextJoystickInstanceID();
        SDL_PrivateJoystickAdded(os3_instance[i]);
    }
    return 0;
}

static int OS3_JoystickGetCount(void)
{
    return LowLevelBase ? OS3_NUMPORTS : 0;
}

static void OS3_JoystickDetect(void)
{
}

static const char *OS3_JoystickGetDeviceName(int index)
{
    if (axb_read(port_of(index), NULL)) {
        return AXB_NAME;
    }
    return index == 1 ? "Port 1 Amiga Joystick/Joypad" : "Port 2 Amiga Joystick/Joypad";
}

static const char *OS3_JoystickGetDevicePath(int index)
{
    return NULL;
}

static int OS3_JoystickGetDeviceSteamVirtualGamepadSlot(int index)
{
    return -1;
}

static int OS3_JoystickGetDevicePlayerIndex(int index)
{
    return -1;
}

static void OS3_JoystickSetDevicePlayerIndex(int index, int player)
{
}

static SDL_JoystickGUID OS3_JoystickGetDeviceGUID(int index)
{
    return SDL_CreateJoystickGUIDForName(OS3_JoystickGetDeviceName(index));
}

static SDL_JoystickID OS3_JoystickGetDeviceInstanceID(int index)
{
    return os3_instance[index];
}

static int OS3_JoystickOpen(SDL_Joystick *joy, int index)
{
    struct joystick_hwdata *hw;
    ULONG temp = 0;
    int i;

    hw = (struct joystick_hwdata *)SDL_calloc(1, sizeof(*hw));
    if (!hw) {
        return SDL_OutOfMemory();
    }
    hw->port = port_of(index);
    joy->hwdata = hw;
    joy->nhats = 1;

    if ((hw->xbox = axb_read(hw->port, NULL))) {
        joy->nbuttons = AXB_NUMBUTTONS;
        joy->naxes = AXB_NUMAXES;
        return 0;
    }

    /* lowlevel.library needs a few reads to tell a CD32 pad from a joystick */
    for (i = 0; i < 20; i++) {
        temp = ReadJoyPort(hw->port);
        Delay(1);
    }
    joy->nbuttons = ((temp & JP_TYPE_MASK) == JP_TYPE_GAMECTLR) ? 7 : 3;
    joy->naxes = 2;
    return 0;
}

static int OS3_JoystickRumble(SDL_Joystick *joy, Uint16 lo, Uint16 hi)
{
    return SDL_Unsupported();
}

static int OS3_JoystickRumbleTriggers(SDL_Joystick *joy, Uint16 lo, Uint16 hi)
{
    return SDL_Unsupported();
}

static Uint32 OS3_JoystickGetCapabilities(SDL_Joystick *joy)
{
    return 0;
}

static int OS3_JoystickSetLED(SDL_Joystick *joy, Uint8 r, Uint8 g, Uint8 b)
{
    return SDL_Unsupported();
}

static int OS3_JoystickSendEffect(SDL_Joystick *joy, const void *data, int size)
{
    return SDL_Unsupported();
}

static int OS3_JoystickSetSensorsEnabled(SDL_Joystick *joy, SDL_bool enabled)
{
    return SDL_Unsupported();
}

static Uint8 os3_hat(int up, int down, int left, int right)
{
    Uint8 hat = SDL_HAT_CENTERED;

    if (up)    hat |= SDL_HAT_UP;
    if (down)  hat |= SDL_HAT_DOWN;
    if (left)  hat |= SDL_HAT_LEFT;
    if (right) hat |= SDL_HAT_RIGHT;
    return hat;
}

/* SDL keeps the previous values and only reports changes */
static void OS3_JoystickUpdate(SDL_Joystick *joy)
{
    struct joystick_hwdata *hw = joy->hwdata;
    int i;

    if (!hw) {
        return;
    }

    if (hw->xbox) {
        struct axb_state st;

        if (!axb_read(hw->port, &st) || !st.connected) {
            SDL_zero(st);
        }
        for (i = 0; i < AXB_NUMAXES; i++) {
            SDL_PrivateJoystickAxis(joy, (Uint8)i, st.axes[i]);
        }
        SDL_PrivateJoystickHat(joy, 0, os3_hat(st.hat_up, st.hat_down, st.hat_left, st.hat_right));
        for (i = 0; i < AXB_NUMBUTTONS; i++) {
            SDL_PrivateJoystickButton(joy, (Uint8)i, (st.buttons & (1UL << i)) ? SDL_PRESSED : SDL_RELEASED);
        }
    } else {
        ULONG data = ReadJoyPort(hw->port);
        int up = 0, down = 0, left = 0, right = 0;

        if ((data & JP_TYPE_MASK) != JP_TYPE_MOUSE) {
            up = (data & JPF_JOY_UP) != 0;
            down = (data & JPF_JOY_DOWN) != 0;
            left = (data & JPF_JOY_LEFT) != 0;
            right = (data & JPF_JOY_RIGHT) != 0;
        }
        SDL_PrivateJoystickAxis(joy, 0, left ? -32767 : right ? 32767 : 0);
        SDL_PrivateJoystickAxis(joy, 1, up ? -32767 : down ? 32767 : 0);
        SDL_PrivateJoystickHat(joy, 0, os3_hat(up, down, left, right));
        /* as in SDL 1.2's driver: blue also reads as play on some pads */
        if (data & JPF_BUTTON_BLUE) {
            data &= ~JPF_BUTTON_PLAY;
        }
        for (i = 0; i < joy->nbuttons; i++) {
            SDL_PrivateJoystickButton(joy, (Uint8)i, (data & os3_joybut[i]) ? SDL_PRESSED : SDL_RELEASED);
        }
    }
}

static void OS3_JoystickClose(SDL_Joystick *joy)
{
    SDL_free(joy->hwdata);
    joy->hwdata = NULL;
}

static void OS3_JoystickQuit(void)
{
    if (LowLevelBase) {
        CloseLibrary(LowLevelBase);
        LowLevelBase = NULL;
    }
}

static SDL_bool OS3_JoystickGetGamepadMapping(int index, SDL_GamepadMapping *out)
{
    return SDL_FALSE;
}

SDL_JoystickDriver SDL_AMIGAOS3_JoystickDriver = {
    OS3_JoystickInit,
    OS3_JoystickGetCount,
    OS3_JoystickDetect,
    OS3_JoystickGetDeviceName,
    OS3_JoystickGetDevicePath,
    OS3_JoystickGetDeviceSteamVirtualGamepadSlot,
    OS3_JoystickGetDevicePlayerIndex,
    OS3_JoystickSetDevicePlayerIndex,
    OS3_JoystickGetDeviceGUID,
    OS3_JoystickGetDeviceInstanceID,
    OS3_JoystickOpen,
    OS3_JoystickRumble,
    OS3_JoystickRumbleTriggers,
    OS3_JoystickGetCapabilities,
    OS3_JoystickSetLED,
    OS3_JoystickSendEffect,
    OS3_JoystickSetSensorsEnabled,
    OS3_JoystickUpdate,
    OS3_JoystickClose,
    OS3_JoystickQuit,
    OS3_JoystickGetGamepadMapping
};

#endif /* SDL_JOYSTICK_AMIGAOS3 */
