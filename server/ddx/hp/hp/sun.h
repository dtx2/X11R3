/*

Copyright (c) 1986, 1987 by Hewlett-Packard Company
Copyright (c) 1986, 1987 by the Massachusetts Institute of Technology

Permission to use, copy, modify, and distribute this
software and its documentation for any purpose and without
fee is hereby granted, provided that the above copyright
notice appear in all copies and that both that copyright
notice and this permission notice appear in supporting
documentation, and that the name of M.I.T. not be used in
advertising or publicity pertaining to distribution of the
software without specific, written prior permission.

HEWLETT-PACKARD MAKES NO WARRANTY OF ANY KIND WITH REGARD
TO THIS SOFWARE, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR 
PURPOSE.  Hewlett-Packard shall not be liable for errors 
contained herein or direct, indirect, special, incidental or 
consequential damages in connection with the furnishing, 
performance, or use of this material.

This software is not subject to any license of the American
Telephone and Telegraph Company or of the Regents of the
University of California.

*/
/*-
 * sun.h --
 *	Internal declarations for the sun ddx interface
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
 *	"$XConsortium: sun.h,v 1.5 88/09/06 15:24:59 jim Exp $ SPRITE (Berkeley)"
 */
#ifndef _SUN_H_
#define _SUN_H_

#include    <errno.h>
#include    <sys/param.h>
#include    <sys/types.h>
#ifdef SYSV
#include    <time.h>
#else
#include    <sys/time.h>
#endif	/* SYSV */
#include    <sys/file.h>

#ifdef	sun

#include    <sys/fcntl.h>
#include    <sys/signal.h>
#include    <sundev/kbd.h>
#include    <sundev/kbio.h>
#include    <sundev/msio.h>
#include    <sun/fbio.h>

/*
  * SUN_WINDOWS is now defined (or not) by the Makefile
  * variable $(SUNWINDOWSFLAGS) in server/Makefile.
  */

#ifdef SUN_WINDOWS
#include    <varargs.h>
#include    <sys/ioctl.h>
#include    <stdio.h>
#include    <pixrect/pixrect_hs.h>
#include    <sunwindow/rect.h>
#include    <sunwindow/rectlist.h>
#include    <sunwindow/pixwin.h>
#include    <sunwindow/win_screen.h>
#include    <sunwindow/win_input.h>
#include    <sunwindow/cms.h>
#include    <sunwindow/win_struct.h>
#else 
/* already included by sunwindow/win_input.h */
#include    <sundev/vuid_event.h>
#endif SUN_WINDOWS

#endif	sun

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
#include    "gc.h"
#include    "gcstruct.h"
#include    "region.h"
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

#ifdef sun
/*
 * Data private to any sun keyboard.
 *	GetEvents reads any events which are available for the keyboard
 *	ProcessEvent processes a single event and gives it to DIX
 *	DoneEvents is called when done handling a string of keyboard
 *	    events or done handling all events.
 *	devPrivate is private to the specific keyboard.
 *	map_q is TRUE if the event queue for the keyboard is memory mapped.
 */
typedef struct kbPrivate {
    int	    	  type;           	/* Type of keyboard */
    int	    	  fd;	    	    	/* Descriptor open to device */
    Firm_event	  *(*GetEvents)();  	/* Function to read events */
    void    	  (*ProcessEvent)();	/* Function to process an event */
    void    	  (*DoneEvents)();  	/* Function called when all events */
					/* have been handled. */
    pointer 	  devPrivate;	    	/* Private to keyboard device */
    Bool	  map_q;		/* TRUE if fd has a mapped event queue */
    int		  offset;		/* to be added to device keycodes */
} KbPrivRec, *KbPrivPtr;

#endif sun

#define	MIN_KEYCODE	8	/* necessary to avoid the mouse buttons */

/*
 * Data private to any sun pointer device.
 *	GetEvents, ProcessEvent and DoneEvents have uses similar to the
 *	    keyboard fields of the same name.
 *	pScreen is the screen the pointer is on (only valid if it is the
 *	    main pointer device).
 *	x and y are absolute coordinates on that screen (they may be negative)
 */
typedef struct ptrPrivate {
#ifdef sun
    int	    	  fd;	    	    	/* Descriptor to device */
    Firm_event 	  *(*GetEvents)(); 	/* Function to read events */
    void    	  (*ProcessEvent)();	/* Function to process an event */
    void    	  (*DoneEvents)();  	/* When all the events have been */
					/* handled, this function will be */
					/* called. */
#endif sun
    short   	  x,	    	    	/* Current X coordinate of pointer */
		  y,	    	    	/* Current Y coordinate */
		  z;	    	    	/* Current Z coordinate */
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
    PixmapPtr		fgPixels;   /* source as a byte-per-pixel glyph */
    PixmapPtr		bgPixels;   /* mask   as a byte-per-pixel glyph */
} CrPrivRec, *CrPrivPtr;

/*
 * Frame-buffer-private info.
 *	fd  	  	file opened to the frame buffer device.
 *	info	  	description of the frame buffer -- type, height, depth,
 *	    	  	width, etc.
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
 *	mapped	  	flag set true by the driver when the frame buffer has
 *	    	  	been mapped in.
 *	parent	  	set true if the frame buffer is actually a SunWindows
 *	    	  	window.
 *	fbPriv	  	Data private to the frame buffer type.
 */
typedef struct {
    pointer 	  	fb; 	    /* Frame buffer itself */
    GCPtr   	  	pGC;	    /* GC for realizing cursors */

    void    	  	(*GetImage)();
    Bool	      	(*CreateGC)();/* GC Creation function previously in the
				       * Screen structure */
    Bool	      	(*CreateWindow)();
    Bool		(*ChangeWindowAttributes)();
    unsigned int  	*(*GetSpans)();
    void		(*EnterLeave)();
    Bool    	  	mapped;	    /* TRUE if frame buffer already mapped */
    Bool		parent;	    /* TRUE if fd is a SunWindows window */
    int	    	  	fd; 	    /* Descriptor open to frame buffer */
#ifdef sun
    struct fbtype 	info;	    /* Frame buffer characteristics */
    pointer 	  	fbPriv;	    /* Frame-buffer-dependent data */
#endif sun
} fbFd;

/*
 * Data describing each type of frame buffer. The probeProc is called to
 * see if such a device exists and to do what needs doing if it does. devName
 * is the expected name of the device in the file system. Note that this only
 * allows one of each type of frame buffer. This may need changing later.
 */
typedef enum {
	neverProbed, probedAndSucceeded, probedAndFailed
} SunProbeStatus;

typedef struct _sunFbDataRec {
    Bool    (*probeProc)();	/* probe procedure for this fb */
    char    *devName;		/* device filename */
    SunProbeStatus probeStatus;	/* TRUE if fb has been probed successfully */
} sunFbDataRec;

extern sunFbDataRec sunFbData[];
/*
 * Cursor functions
 */
extern void 	  sunInitCursor();
extern Bool 	  sunRealizeCursor();
extern Bool 	  sunUnrealizeCursor();
extern Bool 	  sunDisplayCursor();
extern Bool 	  sunSetCursorPosition();
extern void 	  sunCursorLimits();
extern void 	  sunPointerNonInterestBox();
extern void 	  sunConstrainCursor();
extern void 	  sunRecolorCursor();
extern Bool	  sunCursorLoc();
extern void 	  sunRemoveCursor();
extern void	  sunRestoreCursor();

/*
 * Initialization
 */
extern void 	  sunScreenInit();
extern int  	  sunOpenFrameBuffer();

/*
 * GC Interceptions
 */
extern GCPtr	  sunCreatePrivGC();
extern Bool	  sunCreateGC();
extern Bool	  sunCreateWindow();
extern Bool	  sunChangeWindowAttributes();

extern void 	  sunGetImage();
extern unsigned int *sunGetSpans();

extern int	  isItTimeToYield;
extern int  	  sunCheckInput;    /* Non-zero if input is available */

extern fbFd 	  sunFbs[];
extern Bool	  screenSaved;		/* True is screen is being saved */

extern int  	  lastEventTime;    /* Time (in ms.) of last event */
extern void 	  SetTimeSinceLastInputEvent();
extern void	ErrorF();

/*-
 * TVTOMILLI(tv)
 *	Given a struct timeval, convert its time into milliseconds...
 */
#define TVTOMILLI(tv)	(((tv).tv_usec/1000)+((tv).tv_sec*1000))

#ifdef SUN_WINDOWS
extern int windowFd;
#endif SUN_WINDOWS

#endif _SUN_H_
