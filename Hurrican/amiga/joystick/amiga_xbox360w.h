/* amiga_xbox360w.h - reads the Xbox 360 pads of xbox360w.class for the SDL
 * joystick drivers of the Hurrican Amiga port (SDL 1.2 and SDL2).
 *
 * xbox360w.class publishes the full state of up to four pads in a public
 * semaphore (xbox360w.h). When the class is running, the drivers list its
 * first two pads as joysticks of their own, "Xbox 360 Controller 1" and
 * "... 2", after the two lowlevel.library ports, which stay as they were
 * (through the class's ReadJoyPort patch a pad also shows up there, as a
 * CD32 pad). The Xbox joysticks have XInput's button order:
 *
 *   buttons  0 A  1 B  2 X  3 Y  4 LB  5 RB  6 Back  7 Start  8 LS  9 RS
 *           10 Guide  11 LT  12 RT  (the triggers also as buttons)
 *   axes     0 left X  1 left Y  2 right X  3 right Y  4 LT  5 RT
 *            (Y down is positive, as SDL has it)
 *   hat      0: the D-pad, or the left stick beyond the dead zone
 *
 * MIT License, Copyright (c) 2026 Serkan DURSUN
 */
#ifndef AMIGA_XBOX360W_H
#define AMIGA_XBOX360W_H

#include <exec/types.h>
#include <exec/semaphores.h>
#include <proto/exec.h>
#include "xbox360w.h"

#define AXB_NUMPADS    2        /* listed as SDL joysticks */
#define AXB_NUMBUTTONS 13
#define AXB_NUMAXES    6
#define AXB_STICK_DEAD 12000    /* hat from the left stick beyond this */
#define AXB_TRIGGER_ON 64       /* trigger as a button beyond this (of 255) */

static const char *const axb_names[AXB_NUMPADS] = {
    "Xbox 360 Controller 1", "Xbox 360 Controller 2"
};

struct axb_state {
    int   connected;
    ULONG buttons;              /* bit n = SDL button n */
    WORD  axes[AXB_NUMAXES];
    UBYTE hat_up, hat_down, hat_left, hat_right;
};

/* Copies pad 'slot' (0..). Returns 0 if the class is not running, else 1,
 * with st->connected telling whether that pad is on. st may be NULL to
 * only ask whether the class is running. */
static int axb_read(int slot, struct axb_state *st)
{
    static const UWORD order[11] = {
        XB360W_A, XB360W_B, XB360W_X, XB360W_Y, XB360W_LB, XB360W_RB,
        XB360W_BACK, XB360W_START, XB360W_LS, XB360W_RS, XB360W_GUIDE
    };
    struct XB360WState *xs;
    struct XB360WPad pad;
    int i, ok = 0;

    Forbid();
    xs = (struct XB360WState *)FindSemaphore((STRPTR)XB360W_SEMNAME);
    if (xs)
        ObtainSemaphoreShared(&xs->sem);
    Permit();
    if (!xs)
        return 0;
    if (xs->version >= XB360W_VERSION) {
        ok = 1;
        if (slot >= 0 && slot < xs->numpads && slot < XB360W_NUMPADS)
            pad = xs->pad[slot];
        else
            pad.connected = 0;
    }
    ReleaseSemaphore(&xs->sem);

    if (!ok || !st)
        return ok;

    st->connected = pad.connected;
    st->buttons = 0;
    st->hat_up = st->hat_down = st->hat_left = st->hat_right = 0;
    for (i = 0; i < AXB_NUMAXES; i++)
        st->axes[i] = 0;
    if (!pad.connected)
        return 1;

    for (i = 0; i < 11; i++)
        if (pad.buttons & order[i])
            st->buttons |= 1UL << i;
    if (pad.lt > AXB_TRIGGER_ON) st->buttons |= 1UL << 11;
    if (pad.rt > AXB_TRIGGER_ON) st->buttons |= 1UL << 12;

    st->axes[0] = pad.lx;
    st->axes[1] = (pad.ly == -32768) ? 32767 : -pad.ly;
    st->axes[2] = pad.rx;
    st->axes[3] = (pad.ry == -32768) ? 32767 : -pad.ry;
    st->axes[4] = (WORD)(pad.lt * 128 + pad.lt / 2);    /* 0..32767 */
    st->axes[5] = (WORD)(pad.rt * 128 + pad.rt / 2);

    st->hat_up    = (pad.buttons & XB360W_UP)    || pad.ly >  AXB_STICK_DEAD;
    st->hat_down  = (pad.buttons & XB360W_DOWN)  || pad.ly < -AXB_STICK_DEAD;
    st->hat_left  = (pad.buttons & XB360W_LEFT)  || pad.lx < -AXB_STICK_DEAD;
    st->hat_right = (pad.buttons & XB360W_RIGHT) || pad.lx >  AXB_STICK_DEAD;
    return 1;
}

#endif
