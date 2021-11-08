/************************************************************ 
Copyright 1988 by Apple Computer, Inc, Cupertino, California
			All Rights Reserved

Permission to use, copy, modify, and distribute this software
for any purpose and without fee is hereby granted, provided
that the above copyright notice appear in all copies.

APPLE MAKES NO WARRANTY OR REPRESENTATION, EITHER EXPRESS,
OR IMPLIED, WITH RESPECT TO THIS SOFTWARE, ITS QUALITY,
PERFORMANCE, MERCHANABILITY, OR FITNESS FOR A PARTICULAR
PURPOSE. AS A RESULT, THIS SOFTWARE IS PROVIDED "AS IS,"
AND YOU THE USER ARE ASSUMING THE ENTIRE RISK AS TO ITS
QUALITY AND PERFORMANCE. IN NO EVENT WILL APPLE BE LIABLE 
FOR DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL
DAMAGES RESULTING FROM ANY DEFECT IN THE SOFTWARE.

THE WARRANTY AND REMEDIES SET FORTH ABOVE ARE EXCLUSIVE
AND IN LIEU OF ALL OTHERS, ORAL OR WRITTEN, EXPRESS OR
IMPLIED.

************************************************************/
/*-
 * macII.h --
 *	Internal declarations for the macII ddx interface
 *
 * Copyright (c) 1987 by the Regents of the University of California
 *
 * Permission to use, copy, modify, and distribute this
 * software and its documentation for any purpose and without
 * fee is hereby granted, provided that the above copyright
 * notice appear in all copies.  The University of California
 * makes no representations about the suitability of this
 * software for any purpose.  It is provided "as is" without
 * express or implied warranty.
 *
 * $XConsortium: macII.h,v 1.11 88/09/06 14:41:59 jim Exp $
 */
#ifndef _MACII_H_
#define _MACII_H_

#define USE_TOD_CLOCK

#ifdef macII
#define gettimeofday(time, timezone) _gettimeofday(time)
#endif

#include    <errno.h>
#include    <sys/param.h>
#include    <sys/types.h>
#ifdef USE_TOD_CLOCK
#include    <sys/time.h>
#else
#include    <sys/times.h>
#endif USE_TOD_CLOCK
#include    <sys/file.h>
#include    <sys/signal.h>
#include    <sys/stropts.h>
#include    <sys/video.h>

#include    "X.h"
#include    "Xproto.h"
#include    "scrnintstr.h"
#include    "screenint.h"
#ifdef NEED_EVENTS
#include    "inputstr.h"
#endif NEED_EVENTS
#include    "input.h"
#include    "cursorstr.h"
#include    "cursor.h"
#include    "pixmapstr.h"
#include    "pixmap.h"
#include    "windowstr.h"
#include    "gc.h"
#include    "gcstruct.h"
#include    "regionstr.h"
#include    "colormap.h"
#include    "miscstruct.h"
#include    "dix.h"
#include    "mfb.h"
#include    "mi.h"

/*
 * MAXEVENTS is the maximum number of events the mouse and keyboard functions
 * will read on a given call to their GetEvents vectors.
 */
#define MAXEVENTS 	32

/*
 * Data private to any macII keyboard.
 *	ProcessEvent processes a single event and gives it to DIX
 *	DoneEvents is called when done handling a string of keyboard
 *	    events or done handling all events.
 *	devPrivate is private to the specific keyboard.
 */
#define KEY_DETAIL(e) 	((e) & 0x7f)
#define KEY_UP(e)     	((e) & 0x80)

#define MS_LEFT 	1
#define MS_MIDDLE 	2
#define MS_RIGHT 	3

#define MOUSE_ESCAPE 	0x7e			/* from <sys/video.h> */
#define PSEUDO_MIDDLE_1 0x3b 			/* Left Arrow Key */
#define PSEUDO_RIGHT_1 	0x3c 			/* Right Arrow Key */
#define PSEUDO_MIDDLE_2 PSEUDO_MIDDLE_1		/* extra defs just in case */
#define PSEUDO_RIGHT_2 	PSEUDO_RIGHT_1

#define IS_MIDDLE_KEY(c) 			\
	((KEY_DETAIL(c) == PSEUDO_MIDDLE_1) || 	\
	 (KEY_DETAIL(c) == PSEUDO_MIDDLE_2))

#define IS_RIGHT_KEY(c) 			\
      	((KEY_DETAIL(c) == PSEUDO_RIGHT_1) || 	\
	 (KEY_DETAIL(c) == PSEUDO_RIGHT_2))

#define IS_MOUSE_KEY(c)     			\
	(IS_MIDDLE_KEY(c) || IS_RIGHT_KEY(c))


#define KBTYPE_MACII 	0

typedef struct kbPrivate {
    int	    	  type;           	/* Type of keyboard */
    void    	  (*ProcessEvent)();	/* Function to process an event */
    void    	  (*DoneEvents)();  	/* Function called when all events */
					/* have been handled. */
    pointer 	  devPrivate;	    	/* Private to keyboard device */
    int		  offset;		/* to be added to device keycodes */
    KeybdCtrl     *ctrl;                /* Current control structure (for
                                         * keyclick, bell duration, auto-
                                         * repeat, etc.) */
} KbPrivRec, *KbPrivPtr;

#define	MIN_KEYCODE	8	/* necessary to avoid the mouse buttons */

/*
 * Data private to any macII pointer device.
 *	ProcessEvent and DoneEvents have uses similar to the
 *	    keyboard fields of the same name.
 *	pScreen is the screen the pointer is on (only valid if it is the
 *	    main pointer device).
 *	x and y are absolute coordinates on that screen (they may be negative)
 */
typedef struct ptrPrivate {
    void    	  (*ProcessEvent)();	/* Function to process an event */
    void    	  (*DoneEvents)();  	/* When all the events have been */
					/* handled, this function will be */
					/* called. */
    short   	  x,	    	    	/* Current X coordinate of pointer */
		  y;	    	    	/* Current Y coordinate */
    ScreenPtr	  pScreen;  	    	/* Screen pointer is on */
    pointer    	  devPrivate;	    	/* Field private to device */
} PtrPrivRec, *PtrPrivPtr;

/*
 * Cursor-private data
 *	screenBits	saves the contents of the screen before the cursor
 *	    	  	was placed in the frame buffer.
 *	source	  	a bitmap for placing the foreground pixels down
 *	srcGC	  	a GC for placing the foreground pixels down.
 *	    	  	Prevalidated for the cursor's screen.
 *	invSource 	a bitmap for placing the background pixels down.
 *	invSrcGC  	a GC for placing the background pixels down.
 *	    	  	Also prevalidated for the cursor's screen Pixmap.
 *	temp	  	a temporary pixmap for low-flicker cursor motion --
 *	    	  	exists to avoid the overhead of creating a pixmap
 *	    	  	whenever the cursor must be moved.
 *	fg, bg	  	foreground and background pixels. For a color display,
 *	    	  	these are allocated once and the rgb values changed
 *	    	  	when the cursor is recolored.
 *	scrX, scrY	the coordinate on the screen of the upper-left corner
 *	    	  	of screenBits.
 *	state	  	one of CR_IN, CR_OUT and CR_XING to track whether the
 *	    	  	cursor is in or out of the frame buffer or is in the
 *	    	  	process of going from one state to the other.
 */
typedef enum {
    CR_IN,		/* Cursor in frame buffer */
    CR_OUT,		/* Cursor out of frame buffer */
    CR_XING	  	/* Cursor in flux */
} CrState;

typedef struct crPrivate {
    PixmapPtr  	        screenBits; /* Screen before cursor put down */
    PixmapPtr  	        source;     /* Cursor source (foreground bits) */
    GCPtr   	  	srcGC;	    /* Foreground GC */
    PixmapPtr  	        invSource;  /* Cursor source inverted (background) */
    GCPtr   	  	invSrcGC;   /* Background GC */
    PixmapPtr  	        temp;	    /* Temporary pixmap for merging screenBits
				     * and the sources. Saves creation time */
    Pixel   	  	fg; 	    /* Foreground color */
    Pixel   	  	bg; 	    /* Background color */
    int	    	  	scrX,	    /* Screen X coordinate of screenBits */
			scrY;	    /* Screen Y coordinate of screenBits */
    CrState		state;      /* Current state of the cursor */
} CrPrivRec, *CrPrivPtr;

/*
 * Frame-buffer-private info.
 *	fb  	  	pointer to the mapped image of the frame buffer. Used
 *	    	  	by the driving routines for the specific frame buffer
 *	    	  	type.
 *	pGC 	  	A GC for realizing cursors.
 *	GetImage  	Original GetImage function for this screen.
 *	CreateGC  	Original CreateGC function
 *	CreateWindow	Original CreateWindow function
 *	ChangeWindowAttributes	Original function
 *	GetSpans  	GC function which needs to be here b/c GetSpans isn't
 *	    	  	called with the GC as an argument...
 *	slot
 *	default_depth
 *	installedMap
 *	info	  	description of the frame buffer -- type, height, depth,
 *	    	  	width, etc.
 *	fbPriv	  	Data private to the frame buffer type.
 */

#define FBTYPE_MACII 0

typedef struct video_data fbtype;

typedef struct {
    pointer 	  	fb; 	    /* Frame buffer itself */
    GCPtr   	  	pGC;	    /* GC for realizing cursors */

    void    	  	(*GetImage)();
    Bool	      	(*CreateGC)();/* GC Creation function previously in the
				       * Screen structure */
    Bool	      	(*CreateWindow)();
    Bool		(*ChangeWindowAttributes)();
    unsigned int  	*(*GetSpans)();
    int			slot;
    int			default_depth;
    ColormapPtr		installedMap;
    fbtype 		info;	    /* Frame buffer characteristics */
    pointer 	  	fbPriv;	    /* Frame-buffer-dependent data */
} fbFd;

extern fbFd 	  macIIFbs[];

/*
 * Data describing each type of frame buffer. The probeProc is called to
 * see if such a device exists and to do what needs doing if it does. devName
 * is the expected name of the device in the file system. 
 */
typedef enum {
	neverProbed, probedAndSucceeded, probedAndFailed
} macIIProbeStatus;

typedef struct _macIIFbDataRec {
    Bool    (*probeProc)();	/* probe procedure for this fb */
    char    *devName;		/* device filename */
    macIIProbeStatus probeStatus;	/* TRUE if fb has been probed successfully */
} macIIFbDataRec;

extern macIIFbDataRec macIIFbData[];
/*
 * Cursor functions
 */
extern void 	  macIIInitCursor();
extern Bool 	  macIIRealizeCursor();
extern Bool 	  macIIUnrealizeCursor();
extern Bool 	  macIIDisplayCursor();
extern Bool 	  macIISetCursorPosition();
extern void 	  macIICursorLimits();
extern void 	  macIIPointerNonInterestBox();
extern void 	  macIIConstrainCursor();
extern void 	  macIIRecolorCursor();
extern Bool	  macIICursorLoc();
extern void 	  macIIRemoveCursor();
extern void	  macIIRestoreCursor();
extern void	  macIIMoveCursor();

/*
 * Initialization
 */
extern void 	  macIIScreenInit();
extern int  	  macIIOpenFrameBuffer();

/*
 * GC Interceptions
 */
extern GCPtr	  macIICreatePrivGC();
extern Bool	  macIICreateGC();
extern Bool	  macIICreateWindow();
extern Bool	  macIIChangeWindowAttributes();

extern void 	  macIIGetImage();
extern unsigned int *macIIGetSpans();

extern int	  isItTimeToYield;
extern int  	  macIICheckInput;    /* Non-zero if input is available */

extern int  	  lastEventTime;    /* Time (in ms.) of last event */
extern void 	  SetTimeSinceLastInputEvent();

#define AUTOREPEAT_INITIATE     (200)           /* milliseconds */
#define AUTOREPEAT_DELAY        (50)           /* milliseconds */
/*
 * We signal autorepeat events with the unique id AUTOREPEAT_EVENTID.
 */
#define AUTOREPEAT_EVENTID      (0x7d)          /* AutoRepeat id */

extern int	autoRepeatKeyDown;		/* TRUE if key down */
extern int	autoRepeatReady;		/* TRUE if time out */
extern int	autoRepeatDebug;		/* TRUE if debugging */
extern long 	autoRepeatLastKeyDownTv;
extern long 	autoRepeatDeltaTv;

#ifdef USE_TOD_CLOCK
/*-
 * TVTOMILLI(tv)
 *	Given a struct timeval, convert its time into milliseconds...
 */
#define TVTOMILLI(tv)	(((tv).tv_usec/1000)+((tv).tv_sec*1000))

#define tvminus(tv, tv1, tv2) /* tv = tv1 - tv2 */ \
              if ((tv1).tv_usec < (tv2).tv_usec) { \
                      (tv1).tv_usec += 1000000; \
                      (tv1).tv_sec -= 1; \
              } \
              (tv).tv_usec = (tv1).tv_usec - (tv2).tv_usec; \
              (tv).tv_sec = (tv1).tv_sec - (tv2).tv_sec;

#define tvplus(tv, tv1, tv2)  /* tv = tv1 + tv2 */ \
              (tv).tv_sec = (tv1).tv_sec + (tv2).tv_sec; \
              (tv).tv_usec = (tv1).tv_usec + (tv2).tv_usec; \
              if ((tv).tv_usec > 1000000) { \
                      (tv).tv_usec -= 1000000; \
                      (tv).tv_sec += 1; \
              }
#endif USE_TOD_CLOCK

#endif _MACII_H_
