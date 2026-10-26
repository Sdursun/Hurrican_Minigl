/*
    SDL - Simple DirectMedia Layer
    Copyright (C) 1997, 1998, 1999, 2000, 2001  Sam Lantinga

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public
    License along with this library; if not, write to the Free
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

    Sam Lantinga
    slouken@libsdl.org
*/

/* SDL 1.2 joystick driver for AmigaOS 3.x: lowlevel.library ports, and the
 * Xbox 360 pads of xbox360w.class.
 *
 * Changed for the Hurrican Amiga port (2026, Serkan DURSUN), replacing
 * SDL1_MiniGL_Library's joystick/SDL_sysjoystick.c:
 * - directions are reported as axes 0/1 with +-32767 and as hat 0 (they
 *   were two axes of +-257 and no hat, so Hurrican's player did not move);
 *   a mouse in the port no longer gives directions
 * - when xbox360w.class runs, its first two pads are joysticks 2 and 3,
 *   "Xbox 360 Controller 1/2", with 13 buttons, 6 axes and a hat, read
 *   from the class's public state (amiga_xbox360w.h); joysticks 0 and 1
 *   stay the two ports
 */

#include <stdio.h>		/* For the definition of NULL */

#include <libraries/lowlevel.h>
#include <proto/dos.h>
#include <inline/dos.h>
#include <inline/exec.h>
#include <inline/lowlevel.h>
#include <inline/graphics.h>
#include "../mydebug.h"

extern struct ExecBase *SysBase;
extern struct GfxBase *GfxBase;

#include <stdlib.h>

#include "SDL_error.h"
#include "SDL_joystick.h"
#include "SDL_sysjoystick.h"
#include "SDL_joystick_c.h"
#include <SDL_events.h>

#include "amiga_xbox360w.h"

struct Library *LowLevelBase=NULL;

static const ULONG joybut[]=
{
	JPF_BUTTON_RED,
	JPF_BUTTON_BLUE,
	JPF_BUTTON_PLAY,
	JPF_BUTTON_YELLOW,
	JPF_BUTTON_GREEN,
	JPF_BUTTON_FORWARD,
	JPF_BUTTON_REVERSE,
};

struct joystick_hwdata
{
	ULONG joystate;		/* last ReadJoyPort() value, or Xbox button bits */
	int xbox;		/* read through xbox360w.class */
};

#define NUMPORTS 2
static int numxbox;		/* Xbox pads listed after the ports */

/* SDL joystick 0 is the Amiga's joystick port (lowlevel port 1), 1 the
   mouse port (lowlevel port 0) */
static ULONG port_of(int index)
{
	return index == 0 ? 1 : 0;
}

int SDL_SYS_JoystickInit(void)
{
	if(!LowLevelBase)
		LowLevelBase=OpenLibrary("lowlevel.library",37);
	if(!LowLevelBase)
		return 0;
	numxbox = axb_read(0,NULL) ? AXB_NUMPADS : 0;
	return NUMPORTS+numxbox;
}

/* Function to get the device-dependent name of a joystick */
const char *SDL_SYS_JoystickName(int index)
{
	if(index<NUMPORTS&&LowLevelBase)
		return index == 1 ? "Port 1 Amiga Joystick/Joypad" : "Port 2 Amiga Joystick/Joypad";
	if(index<NUMPORTS+numxbox)
		return axb_names[index-NUMPORTS];

	SDL_SetError("No joystick available with that index");
	return(NULL);
}

int SDL_SYS_JoystickOpen(SDL_Joystick *joystick)
{
	ULONG temp=0,i;
	ULONG port=port_of(joystick->index);
	D(bug("Opening joystick %ld\n",joystick->index));

	if(!(joystick->hwdata=malloc(sizeof(struct joystick_hwdata))))
		return -1;
	joystick->hwdata->joystate=0L;
	joystick->nballs=0;
	joystick->nhats=1;

	if((joystick->hwdata->xbox=(joystick->index>=NUMPORTS)))
	{
		joystick->nbuttons=AXB_NUMBUTTONS;
		joystick->naxes=AXB_NUMAXES;
		return 0;
	}

/* This loop is to check if the controller is a joypad */
	for(i=0;i<20;i++)
	{
		temp=ReadJoyPort(port);
		Delay(1);
	}

	if((temp&JP_TYPE_MASK)==JP_TYPE_GAMECTLR)
		joystick->nbuttons=7;
	else
		joystick->nbuttons=3;
	joystick->naxes=2;

	return 0;
}

static void update_hat(SDL_Joystick *joystick,int up,int down,int left,int right)
{
	Uint8 hat = SDL_HAT_CENTERED;

	if(up)    hat |= SDL_HAT_UP;
	if(down)  hat |= SDL_HAT_DOWN;
	if(left)  hat |= SDL_HAT_LEFT;
	if(right) hat |= SDL_HAT_RIGHT;
	if(joystick->hats[0]!=hat) SDL_PrivateJoystickHat(joystick,0,hat);
}

static void update_xbox(SDL_Joystick *joystick)
{
	struct axb_state st;
	ULONG old=joystick->hwdata->joystate;
	int i;

	if(!axb_read(joystick->index-NUMPORTS,&st))
		st.connected=0;
	if(!st.connected)
	{
		st.buttons=0;
		st.hat_up=st.hat_down=st.hat_left=st.hat_right=0;
		for(i=0;i<AXB_NUMAXES;i++) st.axes[i]=0;
	}

	for(i=0;i<AXB_NUMAXES;i++)
		if(joystick->axes[i]!=st.axes[i]) SDL_PrivateJoystickAxis(joystick,i,st.axes[i]);
	update_hat(joystick,st.hat_up,st.hat_down,st.hat_left,st.hat_right);
	for(i=0;i<AXB_NUMBUTTONS;i++)
	{
		ULONG bit=1UL<<i;
		if((st.buttons&bit)!=(old&bit))
			SDL_PrivateJoystickButton(joystick,i,(st.buttons&bit)?SDL_PRESSED:SDL_RELEASED);
	}
	joystick->hwdata->joystate=st.buttons;
}

/* Function to update the state of a joystick - called as a device poll. */
void SDL_SYS_JoystickUpdate(SDL_Joystick *joystick)
{
	ULONG data;
	int i;

	if(joystick->hwdata->xbox)
	{
		update_xbox(joystick);
		return;
	}
	if(joystick->index>=NUMPORTS)
		return;

	data=ReadJoyPort(port_of(joystick->index));
	{
		Sint16 x = 0, y = 0;
		int up=0,down=0,left=0,right=0;

		if((data&JP_TYPE_MASK)!=JP_TYPE_MOUSE)
		{
			if(data&JPF_JOY_UP)    { y = -32767; up=1; }
			if(data&JPF_JOY_DOWN)  { y =  32767; down=1; }
			if(data&JPF_JOY_LEFT)  { x = -32767; left=1; }
			if(data&JPF_JOY_RIGHT) { x =  32767; right=1; }
		}
		if(joystick->axes[0]!=x) SDL_PrivateJoystickAxis(joystick,0,x);
		if(joystick->axes[1]!=y) SDL_PrivateJoystickAxis(joystick,1,y);
		update_hat(joystick,up,down,left,right);
	}
	for(i=0;i<joystick->nbuttons;i++)
	{
		if( (data&joybut[i]) )
		{
			if(i==1)
				data&=(~(joybut[2]));

			if(!(joystick->hwdata->joystate&joybut[i]))
				SDL_PrivateJoystickButton(joystick,i,SDL_PRESSED);
		}
		else if(joystick->hwdata->joystate&joybut[i])
			SDL_PrivateJoystickButton(joystick,i,SDL_RELEASED);
	}

	joystick->hwdata->joystate=data;
}

/* Function to close a joystick after use */
void SDL_SYS_JoystickClose(SDL_Joystick *joystick)
{
	if(joystick->hwdata)
		free(joystick->hwdata);
}

/* Function to perform any system-specific joystick related cleanup */
void SDL_SYS_JoystickQuit(void)
{
	if(LowLevelBase)
	{
		CloseLibrary(LowLevelBase);
		LowLevelBase=NULL;
		SDL_numjoysticks=0;
	}
}
