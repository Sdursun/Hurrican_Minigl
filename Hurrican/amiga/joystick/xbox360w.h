/* xbox360w.h - full pad state published by xbox360w.class
 *
 * lowlevel.library can only carry CD32 pad bits, so the class also
 * publishes everything it reads in a public SignalSemaphore. To read it:
 *
 *   struct XB360WState *st;
 *   Forbid();
 *   if ((st = (struct XB360WState *)FindSemaphore(XB360W_SEMNAME)))
 *       ObtainSemaphoreShared(&st->sem);
 *   Permit();
 *   if (st) {
 *       if (st->version >= XB360W_VERSION) ... copy st->pad[] ...
 *       ReleaseSemaphore(&st->sem);
 *   }
 *
 * Do not keep the pointer between reads: the class removes the semaphore
 * when it is expunged.
 */
#ifndef XBOX360W_H
#define XBOX360W_H

#include <exec/types.h>
#include <exec/semaphores.h>

#define XB360W_SEMNAME  "xbox360w.state"
#define XB360W_VERSION  1
#define XB360W_NUMPADS  4

/* buttons, as sent by the controller */
#define XB360W_UP       0x0001
#define XB360W_DOWN     0x0002
#define XB360W_LEFT     0x0004
#define XB360W_RIGHT    0x0008
#define XB360W_START    0x0010
#define XB360W_BACK     0x0020
#define XB360W_LS       0x0040  /* left stick pressed */
#define XB360W_RS       0x0080  /* right stick pressed */
#define XB360W_LB       0x0100
#define XB360W_RB       0x0200
#define XB360W_GUIDE    0x0400
#define XB360W_A        0x1000
#define XB360W_B        0x2000
#define XB360W_X        0x4000
#define XB360W_Y        0x8000

struct XB360WPad {
    UBYTE connected;
    UBYTE port;                 /* lowlevel port 0..3, 0xff = not mapped */
    UWORD buttons;              /* XB360W_... */
    UBYTE lt, rt;               /* triggers 0..255 */
    UWORD pad0;
    WORD  lx, ly;               /* left stick, -32768..32767, up is positive */
    WORD  rx, ry;               /* right stick */
    ULONG packets;              /* input reports received */
};

struct XB360WState {
    struct SignalSemaphore sem; /* sem.ss_Link.ln_Name = XB360W_SEMNAME */
    UWORD version;
    UWORD numpads;
    char  name[32];             /* "Xbox 360 Controller" */
    struct XB360WPad pad[XB360W_NUMPADS];
};

#endif
