#ifndef XHPPROTO_H
#define XHPPROTO_H

/* $XConsortium: XHPproto.h,v 1.2 88/09/06 15:25:25 jim Exp $ */
/* Definitions for HP extensions used by the server and C bindings*/

#include "Xmd.h"

#define STDEVENTS	-1
#define CORE_EVENTS     64

#define	OFF		(0L << 0)
#define	ON		(1L << 0)
#define	ABSOLUTE 	(0L << 1)
#define	RELATIVE 	(1L << 1)
#define	SYSTEM_EVENTS	(0L << 2)
#define	DEVICE_EVENTS	(1L << 2)

/* HP devices */

#define	XPOINTER		0
#define	XKEYBOARD		1
#define	XOTHER   		2

#define NUM_DEV_TYPES		14
#define MAX_LOGICAL_DEVS	9

#define NULL_DEVICE		0
#define MOUSE			1
#define TABLET			2
#define KEYBOARD		3
#define QUADRATURE		4
#define TOUCHSCREEN		5
#define TOUCHPAD		6
#define BUTTONBOX		7
#define BARCODE			8
#define ONE_KNOB    		9
#define TRACKBALL		10
#define KEYPAD   		11
#define NINE_KNOB    		12
#define ID_MODULE 		13


/* clip list structure */

typedef struct _XClip
    {
    CARD16 obs_flag B16;
    INT16  x1       B16;
    INT16  y1       B16;
    INT16  x2       B16;
    INT16  y2       B16;
    CARD16 pad	    B16;	/* pad structure to 32 bit boundary */
    } XClipRec, *XClipPtr;
#endif
