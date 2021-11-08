/*
 *   Copyright (c) 1987, 88 by
 *   PARALLAX GRAPHICS, INCORPORATED, Santa Clara, California.
 *   All rights reserved
 *
 *   This software is furnished on an as-is basis, and may be used and copied
 *   only with the inclusion of the above copyright notice.
 *
 *   The information in this software is subject to change without notice.
 *   No committment is made as to the usability or reliability of this
 *   software.
 *
 *   Parallax Graphics, Inc.
 *   2500 Condensa Street
 *   Santa Clara, California  95051
 */

#ifndef lint
static char *sid_ = "@(#)plxKb.c	1.12 09/01/88 Parallax Graphics Inc";
#endif

#define	PARALLAX_QEVENT
#include	"Xplx.h"
#include	<sys/file.h>
#include	<sys/time.h>

#define NEED_EVENTS
#include "Xproto.h"
#include "cursorstr.h"
#include "input.h"

void plxChangeKeyboardControl();
void plxBell();

extern vsEventQueue *queue;
extern DevicePtr plxKeyboard, plxPointer;

extern int plx_wfd;

int
plxKeybdProc(pDev, onoff, argc, argv)
DevicePtr pDev;
int onoff, argc;
char *argv[];
{
	BYTE map[MAP_LENGTH];
	KeySymsRec keySyms;
	CARD8 modMap[MAP_LENGTH];
	int fd;

	ifdebug(4) printf("plxKeybdProc() onoff=%d\n", onoff);

	switch (onoff) {
	case DEVICE_INIT:
		plxKeyboard = pDev;
		fd = px_kbd_init();
		pDev->devicePrivate = (pointer)&queue;
		GetLK201Mappings(&keySyms, modMap);
		InitKeyboardDeviceStruct(plxKeyboard, &keySyms, modMap, plxBell, plxChangeKeyboardControl);
		Xfree(keySyms.map);
		break;
	case DEVICE_ON:
		plxKbdInit();
		SetLKAutoRepeat(FALSE);
		pDev->on = TRUE;
		AddEnabledDevice(plx_wfd);
		break;
	case DEVICE_OFF:
		pDev->on = FALSE;
		/* RemoveEnabledDevice(plx_wfd); */
		break;
	case DEVICE_CLOSE:
		break;
	}
	return Success;
}

/*
 * begin lk201 code.
 */

#define MAX_LED 4

#define	LED_1			0x81	/* led bits */
#define	LED_2			0x82
#define	LED_3			0x84
#define	LED_4			0x88
#define	LED_ALL			0x8f

#define	LK_AUTODOWN		0x82
#define	LK_DEFAULTS		0xd3	/* reset (some) default settings */
#define	LK_DIV_0		0x00	/* division 0 */
#define	LK_DIV_1		0x08	/* division 1 */
#define	LK_DIV_10		0x50	/* division 10 */
#define	LK_DIV_11		0x58	/* division 11 */
#define	LK_DIV_12		0x60	/* division 12 */
#define	LK_DIV_13		0x68	/* division 13 */
#define	LK_DIV_14		0x70	/* division 14 */
#define	LK_DIV_15		0x78	/* division 15 */
#define	LK_DIV_2		0x10	/* division 2 */
#define	LK_DIV_3		0x18	/* division 3 */
#define	LK_DIV_4		0x20	/* division 4 */
#define	LK_DIV_5		0x28	/* division 5 */
#define	LK_DIV_6		0x30	/* division 6 */
#define	LK_DIV_7		0x38	/* division 7 */
#define	LK_DIV_8		0x40	/* division 8 */
#define	LK_DIV_9		0x48	/* division 9 */
#define	LK_DOWN			0x80

#define	LK_DISABLE_CLICK	0x99	/* disable keyclick entirely */
#define	LK_DISABLE_LED		0x11	/* turn off led */
#define	LK_DISABLE_REPEAT	0xe1
#define	LK_ENABLE_BELL		0x23	/* enable bell / set volume */
#define	LK_ENABLE_CLICK		0x1b	/* enable keyclick / set volume */
#define	LK_ENABLE_KBD		0x8b	/* keyboard enable */
#define	LK_ENABLE_LED		0x13	/* light led */
#define	LK_ENABLE_REPEAT	0xe3	/* global auto repeat enable */

#define	LK_INPUT_ERROR		0xb6	/* garbage command to keyboard */
#define	LK_KDOWN_ERROR		0x3d	/* key down on powerup error */
#define	LK_OUTPUT_ERROR		0xb5	/* keystrokes lost during inhibit */
#define	LK_POWER_ERROR		0x3e	/* keyboard failure on powerup test */

#define	LK_LAST_PARAM		0x80	/* or'ed in with last byte */
#define	LK_RING_BELL		0xa7	/* ring keyboard bell */
#define	LK_UPDOWN		0x86	/* bits for setting lk201 modes */

unsigned char plxlk201buffer[16];

void
plxBell(loud, pDevice)
int loud;
DevicePtr pDevice;
{
	/*
	 * the lk201 volume is between:
	 *	7 (quiet but audible)
	 * and
	 *	0 (loud)
	 */
	plxlk201buffer[0] = LK_ENABLE_BELL;
	plxlk201buffer[1] = (7 - ((loud / 14) & 7)) | LK_LAST_PARAM;
	plxlk201buffer[2] = LK_RING_BELL | LK_LAST_PARAM;

	px_kbd_write(plxlk201buffer, 3);
}

/*
 * used by the ddx/dec/lk201 code
 */
SetLockLED(on)
Bool on;
{
	plxlk201buffer[0] = on ? LK_ENABLE_LED : LK_DISABLE_LED;
	plxlk201buffer[1] = LED_3 | LK_LAST_PARAM;

	px_kbd_write(plxlk201buffer, 2);
}

void
ChangeLED(led, on)
int led;
Bool on ;
{
	plxlk201buffer[0] = on ? LK_ENABLE_LED : LK_DISABLE_LED;
	switch (led) {
	case 1:
		plxlk201buffer[1] = LED_1 | LK_LAST_PARAM;
		break;
	case 2:
		plxlk201buffer[1] = LED_2 | LK_LAST_PARAM;
		break;
	case 3:
		plxlk201buffer[1] = LED_3 | LK_LAST_PARAM;
		break;
	case 4:
		plxlk201buffer[1] = LED_4 | LK_LAST_PARAM;
		break;
	default:
		return;
	}
	px_kbd_write(plxlk201buffer, 2);
}

void
plxChangeKeyboardControl(pDevice, ctrl)
DevicePtr pDevice;
register KeybdCtrl *ctrl;
{
	register int i;

	if (ctrl->click == 0) {
		plxlk201buffer[0] = LK_DISABLE_CLICK | LK_LAST_PARAM;
		px_kbd_write(plxlk201buffer, 1);
	} else {
		int volume = ctrl->click - 1;

		if (volume > 7)
			volume = 7;
		plxlk201buffer[0] = LK_ENABLE_CLICK;
		plxlk201buffer[1] = (7 - volume) | LK_LAST_PARAM;
		px_kbd_write(plxlk201buffer, 2);
	}

	/*
	 * ctrl->bell: the DIX layer handles the base volume for the bell
	 *
	 * ctrl->bell_pitch: as far as I can tell, you can't set this on lk201
	 *
	 * ctrl->bell_duration: as far as I can tell, you can't set this
	 */

	/*
	 * LEDs
	 */
	for (i=1;i<=MAX_LED;i++)
		ChangeLED(i, (ctrl->leds & (1 << (i-1))));

	/*
	 * ctrl->autoRepeat: I'm turning it all on or all off.
	 */
	SetLKAutoRepeat(ctrl->autoRepeat);
}

/*
 * from ddx/dec/lk201/lk201.c
 */
extern unsigned char *AutoRepeatLKMode();
extern unsigned char *UpDownLKMode();

SetLKAutoRepeat(on)
Bool on;
{
	register unsigned char *divsets;

	divsets = on ? AutoRepeatLKMode() : UpDownLKMode();

	while (*divsets) {
		plxlk201buffer[0] = *divsets++ | LK_LAST_PARAM;
		px_kbd_write(plxlk201buffer, 1);
	}

	plxlk201buffer[0] = on ? LK_ENABLE_REPEAT | LK_LAST_PARAM : LK_DISABLE_REPEAT | LK_LAST_PARAM;
	px_kbd_write(plxlk201buffer, 1);
}

char plxKbdInitString[] = {		/* reset any random keyboard stuff */
	LK_DEFAULTS,			/* reset to keyboard defaults */
	LK_AUTODOWN | LK_DIV_1 , LK_AUTODOWN | LK_DIV_2,
	LK_AUTODOWN | LK_DIV_3 , LK_DOWN     | LK_DIV_4,
	LK_UPDOWN   | LK_DIV_5 , LK_UPDOWN   | LK_DIV_6,
	LK_AUTODOWN | LK_DIV_7 , LK_AUTODOWN | LK_DIV_8,
	LK_AUTODOWN | LK_DIV_9 , LK_AUTODOWN | LK_DIV_10,
	LK_AUTODOWN | LK_DIV_11, LK_AUTODOWN | LK_DIV_12,
	LK_DOWN     | LK_DIV_13, LK_AUTODOWN | LK_DIV_14,
	LK_ENABLE_REPEAT,		/* we want autorepeat by default */
	LK_ENABLE_CLICK,		/* keyclick */
	0x84,				/* keyclick volume */
	LK_ENABLE_KBD,			/* the keyboard itself */
	LK_ENABLE_BELL,			/* keyboard bell */
	0x84,				/* bell volume */
	LK_RING_BELL,			/* signal we're alive */
	LK_DISABLE_LED,			/* keyboard leds */
	LED_ALL,
};

plxKbdInit()
{
	px_kbd_write(plxKbdInitString, sizeof(plxKbdInitString));
}
