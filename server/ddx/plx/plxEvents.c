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
static char *sid_ = "@(#)plxEvents.c	1.15 09/01/88 Parallax Graphics Inc";
#endif

#define	PARALLAX_QEVENT
#include	"Xplx.h"
#include	<sys/time.h>

#define	NEED_EVENTS
#include	"Xproto.h"
#include	"input.h"

#define	PARALLAX_CMDKEYS

#ifdef	PARALLAX_CMDKEYS
#include	"../dec/lk201/keynames.h"
#define	KEY_ALLUP	179
#endif

extern vsEventQueue *queue;
extern int lastEventTime;
extern int screenIsSaved;
extern DevicePtr plxKeyboard, plxPointer;

/*
 * ProcessInputEvents()
 *
 *	processes all the pending input events
 */
void
ProcessInputEvents()
{
	register int i;
	register vsEvent *pE;
	xEvent x;
	int nowInCentiSecs, nowInMilliSecs, adjustCentiSecs;
	struct timeval tp;
	int needTime = 1;

	ifdebug(4) printf("plx ProcessInputEvents()\n");

#if defined(sun) || defined(interactive) || defined(motorola131)
	px_scupdate();
#endif
	while ((i = queue->head) != queue->tail) {
		if (screenIsSaved == SCREEN_SAVER_ON) {
			SaveScreens(SCREEN_SAVER_OFF, ScreenSaverReset);
		}

		pE = &queue->events[i];

		x.u.keyButtonPointer.rootX = PTX(pE->vse_x);
		x.u.keyButtonPointer.rootY = PTY(pE->vse_y);

		/*
		 * The following silly looking code is because the old
		 * version of the driver only delivers 16 bits worth of
		 * centiseconds. We are supposed to be keeping time in
		 * terms of 32 bits of milliseconds.
		 */
		if (needTime) {
			needTime = 0;
			gettimeofday(&tp, 0);
			nowInCentiSecs = ((tp.tv_sec * 100) + (tp.tv_usec / 10000)) & 0xFFFF;
			/* same as driver */
			nowInMilliSecs = (tp.tv_sec * 1000) + (tp.tv_usec / 1000);
			/* beware overflow */
		}

		if ((adjustCentiSecs = nowInCentiSecs - pE->vse_time) < -20000) {
			adjustCentiSecs += 0x10000;
		} else {
			if (adjustCentiSecs > 20000)
				adjustCentiSecs -= 0x10000;
		}
		x.u.keyButtonPointer.time = lastEventTime = nowInMilliSecs - adjustCentiSecs * 10;

		switch (pE->vse_device) {
		case VSE_DKB:				/* DEC Keyboard */
			switch (pE->vse_type) {
			case VSE_BUTTON:
				ifdebug(4) printf("\t%d dev,type,key,dir=%s,%s,%d,%d\n", i, "VSE_DKB", "VSE_BUTTON", pE->vse_key, pE->vse_direction);
				x.u.u.detail = pE->vse_key;
				switch (pE->vse_direction) {
				case VSE_KBTDOWN:
					x.u.u.type = KeyPress;
					(plxKeyboard->processInputProc)(&x, plxKeyboard);
					break;
				case VSE_KBTUP:
					x.u.u.type = KeyRelease;
					(plxKeyboard->processInputProc)(&x, plxKeyboard);
					break;
				case VSE_KBTRAW:
#ifdef	PARALLAX_CMDKEYS
					check_pan(pE->vse_key);
#endif
					ProcessLK201Input(&x, plxKeyboard);
					break;
				default:
					/* IGNORE */
					break;
				}
				break;
			default:
				ifdebug(4) printf("\t%d dev,type,x,y,key,dir=%s,%d,%d,%d,%d,%d\n", i, "VSE_DKB", pE->vse_type, pE->vse_x, pE->vse_y, pE->vse_key, pE->vse_direction);
				ifdebug(4) printf("\t%d dev,type,key,dir=%s,%s,%d,%d\n", i, "VSE_DKB", "UNKNOWN", pE->vse_key, pE->vse_direction);
				/* IGNORE */
				break;
			}
			break;
		case VSE_MOUSE:				/* Mouse */
			switch (pE->vse_type) {
			case VSE_BUTTON:
				ifdebug(4) printf("\t%d dev,type,x,y,key,dir=%s,%s,%d,%d,%d,%d\n", i, "VSE_MOUSE", "VSE_BUTTON", pE->vse_x, pE->vse_y, pE->vse_key, pE->vse_direction);
				if (pE->vse_direction == VSE_KBTDOWN) {
					x.u.u.type = ButtonPress;
				} else {
					x.u.u.type = ButtonRelease;
				}
				/* mouse buttons numbered from one */
				x.u.u.detail = pE->vse_key + 1;
				(*plxPointer->processInputProc)(&x, plxPointer);
				break;
			case VSE_MMOTION:
				ifdebug(4) printf("\t%d dev,type,x,y,key,dir=%s,%s,%d,%d,%d,%d\n", i, "VSE_MOUSE", "VSE_MMOTION", pE->vse_x, pE->vse_y, pE->vse_key, pE->vse_direction);
				x.u.u.type = MotionNotify;
				(*plxPointer->processInputProc)(&x, plxPointer);
				break;
			default:
				ifdebug(4) printf("\t%d dev,type,x,y,key,dir=%s,%d,%d,%d,%d,%d\n", i, "VSE_MOUSE", pE->vse_type, pE->vse_x, pE->vse_y, pE->vse_key, pE->vse_direction);
				/* IGNORE */
				break;
			}
			break;
		default:
			ifdebug(4) printf("\t%d dev,type,key,dir=%d,%d,%d,%d,%d,%d\n", i, pE->vse_device, pE->vse_type, pE->vse_key, pE->vse_direction);
			/* IGNORE */
			break;
		}

		if (i == (queue->size - 1)) {
			queue->head = 0;
		} else {
			queue->head++;
		}
	}
}

TimeSinceLastInputEvent()
{
	if (lastEventTime == 0)
		lastEventTime = GetTimeInMillis();
	return GetTimeInMillis() - lastEventTime;
}

#ifdef	PARALLAX_CMDKEYS
static
check_pan(key)
unsigned char key;
{
	static unsigned char last[4];
	static int idx = -1;		/* -1 == no seqence */

	if (key == KEY_HELP) {
		idx = 0;
		return;
	}
	if ((idx < 0) || (key == KEY_ALLUP)) {
		return;
	}

	switch (key) {
	case KEY_F17:			/* normal, top left */
		p_pan(0, 0);
		idx = -1;
		return;
	case KEY_F18:			/* top right */
		p_pan(0x300, 0);
		idx = -1;
		return;
	case KEY_F19:			/* bottom left */
		p_pan(0, 0x400);
		idx = -1;
		return;
	case KEY_F20:			/* bottom right */
		p_pan(0x300, 0x400);
		idx = -1;
		return;
	case KEY_REMOVE:		/* first of 'exit' string */
	case KEY_NEXT_SCREEN:		/* Abort server, core dump */
	case KEY_PREV_SCREEN:		/* reset all connections */
		if (idx != 0) {
			idx = -1;
			return;
		}
		last[idx++] = key;
		return;
	case KEY_MENU:			/* completion of command */
		if (idx != 1) {
			idx = -1;
			return;
		}
		switch (last[idx-1]) {
		case KEY_PREV_SCREEN:
			idx = -1;
			FatalError ("Keyboard requested exit");
			/* NOT REACHED */
			return;
		case KEY_NEXT_SCREEN:
			idx = -1;
			AutoResetServer();
			return;
		case KEY_REMOVE:
			idx = -1;
			GiveUp();
			/* NOT REACHED */
			return;
		default:
			idx = -1;
			return;
		}
	default:
		idx = -1;
		return;
	}
}
#endif
