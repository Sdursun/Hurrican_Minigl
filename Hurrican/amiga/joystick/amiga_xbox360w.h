/* amiga_xbox360w.h - reads the Xbox 360 pads of xbox360w.class for the SDL
 * joystick drivers of the Hurrican Amiga port (SDL 1.2 and SDL2).
 *
 * xbox360w.class maps each pad to a lowlevel.library port and publishes the
 * full pad state in a public semaphore (xbox360w.h). A pad mapped to a port
 * is opened as an "Xbox 360 Controller" with XInput's button order instead
 * of a CD32 pad with seven buttons:
 *
 *   buttons  0 A  1 B  2 X  3 Y  4 LB  5 RB  6 Back  7 Start  8 LS  9 RS
 *           10 Guide  11 LT  12 RT  (the triggers also as buttons)
 *   axes     0 left X  1 left Y  2 right X  3 right Y  4 LT  5 RT
 *            (Y down is positive, as SDL has it)
 *   hat      0: the D-pad, or the left stick beyond the dead zone
 *
 * Without the class (no semaphore) nothing changes: the port is read
 * through lowlevel.library as before.
 *
 * MIT License, Copyright (c) 2026 Serkan DURSUN
 */
#ifndef AMIGA_XBOX360W_H
#define AMIGA_XBOX360W_H

#include <exec/types.h>
#include <exec/semaphores.h>
#include <proto/exec.h>
#include "xbox360w.h"

#define AXB_NUMBUTTONS 13
#define AXB_NUMAXES    6
#define AXB_NAME       "Xbox 360 Controller"
#define AXB_STICK_DEAD 12000    /* hat from the left stick beyond this */
#define AXB_TRIGGER_ON 64       /* trigger as a button beyond this (of 255) */

struct axb_state {
    int   connected;
    ULONG buttons;              /* bit n = SDL button n */
    WORD  axes[AXB_NUMAXES];
    UBYTE hat_up, hat_down, hat_left, hat_right;
};

/* Copies the pad mapped to lowlevel port 'port'. Returns 0 if the class is
 * not running or no pad is mapped to that port; 1 if one is (connected or
 * not, see st->connected). st may be NULL to only ask. */
static int axb_read(ULONG port, struct axb_state *st)
{
    static const UWORD order[11] = {
        XB360W_A, XB360W_B, XB360W_X, XB360W_Y, XB360W_LB, XB360W_RB,
        XB360W_BACK, XB360W_START, XB360W_LS, XB360W_RS, XB360W_GUIDE
    };
    struct XB360WState *xs;
    struct XB360WPad pad;
    int i, found = 0;

    Forbid();
    xs = (struct XB360WState *)FindSemaphore((STRPTR)XB360W_SEMNAME);
    if (xs)
        ObtainSemaphoreShared(&xs->sem);
    Permit();
    if (!xs)
        return 0;
    if (xs->version >= XB360W_VERSION) {
        for (i = 0; i < xs->numpads && i < XB360W_NUMPADS; i++) {
            if (xs->pad[i].port == port) {
                pad = xs->pad[i];
                found = 1;
                break;
            }
        }
    }
    ReleaseSemaphore(&xs->sem);

    if (!found || !st)
        return found;

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
