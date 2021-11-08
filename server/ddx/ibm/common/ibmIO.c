/***********************************************************
Copyright 1987 by Digital Equipment Corporation, Maynard, Massachusetts,
and the Massachusetts Institute of Technology, Cambridge, Massachusetts.

                        All Rights Reserved

Permission to use, copy, modify, and distribute this software and its
documentation for any purpose and without fee is hereby granted,
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in
supporting documentation, and that the names of Digital or MIT not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.

DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.

******************************************************************/
/***********************************************************
		Copyright IBM Corporation 1987,1988

                      All Rights Reserved

Permission to use, copy, modify, and distribute this software and its
documentation for any purpose and without fee is hereby granted,
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in
supporting documentation, and that the name of IBM not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.

IBM DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
IBM BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.

******************************************************************/
/* $Header: /andrew/X11/R3src/Xbeta/server/ddx/ibm/common/RCS/ibmIO.c,v 9.0 88/10/17 14:55:13 erik Exp $ */
/* $Source: /andrew/X11/R3src/Xbeta/server/ddx/ibm/common/RCS/ibmIO.c,v $ */

#ifndef lint
static char *rcsid = "$Header: /andrew/X11/R3src/Xbeta/server/ddx/ibm/common/RCS/ibmIO.c,v 9.0 88/10/17 14:55:13 erik Exp $";
static char sccsid[] = "%W% %E% %U%";
#endif

#include <sys/types.h>
#include <sys/time.h>

#define NEED_EVENTS

#include "X.h"
#include "Xproto.h"
#include "input.h"
#include "miscstruct.h"
#include "scrnintstr.h"
#include "cursorstr.h"

#include "OScompiler.h"

#include "ibmKeybd.h"
#include "ibmMouse.h"

#include "OSio.h"
#include "OScursor.h"

#include "ibmScreen.h"
#include "ibmTrace.h"

int		lastEventTime;

#ifdef IBM_OS_HAS_X_QUEUE
XEventQueue	*ibmQueue;
extern	int	screenIsSaved;

/*****************
 * ProcessInputEvents:
 *    processes all the pending input events
 *****************/

void
ProcessInputEvents()
{
    register int    i ;
    register ibmPerScreenInfo *screenInfo ;
    register int x, y;
    register XEvent *pE ;
    xEvent e;
    int nowInCentiSecs, nowInMilliSecs, adjustCentiSecs;
    struct timeval tp;
    int needTime = 1;
    int	oldScr;
    int	setCursor;
static    int	alreadyHere;
#ifdef IBM_SPECIAL_MALLOC
    extern int ibmShouldDumpArena;
#endif /* IBM_SPECIAL_MALLOC */

/*    TRACE(("ProcessInputEvents()\n"));*/

			/* hack solution to problem when NewCurrentScreen
			 * generates an event which sends us back here
			 * before we've finished processing all this
			 * stuff.
			 */
    if (alreadyHere) {
	return;
    }
    alreadyHere= 1;

#ifdef IBM_SPECIAL_MALLOC
    if ( ibmShouldDumpArena ) {
       ibmDumpArena() ;
    }
#endif /* IBM_SPECIAL_MALLOC */

    for ( i = ibmQueue->head ;
	  i != ibmQueue->tail ;
i = ( ibmQueue->head = ( ( i == ibmQueue->size ) ? 0 : ( ibmQueue->head + 1 ) ) ) )
    {
	if ( screenIsSaved == SCREEN_SAVER_ON )
	    SaveScreens( SCREEN_SAVER_OFF, ScreenSaverReset ) ;
	pE = &ibmQueue->events[i] ;

	if ( pE->xe_device == XE_CONSOLE ) { /* virtual screen event */
		OS_ScreenStateChange( pE ) ;
	}
	else if (pE->xe_device) {

		x = (signed short) pE->xe_x ;
		y = (signed short) pE->xe_y ;
		screenInfo = ibmScreens[ oldScr = ibmCurrentScreen ] ;
		setCursor = FALSE;

		if ( ibmYWrapScreen ) {
		    while ( y < screenInfo->ibm_ScreenBounds.y1 ) {
			y +=
	screenInfo->ibm_ScreenBounds.y2 - screenInfo->ibm_ScreenBounds.y1 ;
			setCursor = TRUE ;
		    }

		    while ( y > screenInfo->ibm_ScreenBounds.y2 ) {
			y -=
	screenInfo->ibm_ScreenBounds.y2 - screenInfo->ibm_ScreenBounds.y1 ;
			setCursor = TRUE ;
		    }
		}

		while ( x < screenInfo->ibm_ScreenBounds.x1 ) {
		    if ( screenInfo == ibmScreens[ 0 ] ) {
			/* Already At First Possible Screen */
			if ( ibmXWrapScreen ) {
			    x -= screenInfo->ibm_ScreenBounds.x1 ; /* x < 0 */
			    screenInfo =
				ibmScreens[ ibmCurrentScreen = ibmNumScreens - 1 ] ;
			    x += screenInfo->ibm_ScreenBounds.x2 ;
			}
			else {
			    x = screenInfo->ibm_ScreenBounds.x1 ;
			}
		    }
		    else {
			x -= screenInfo->ibm_ScreenBounds.x1 ; /* Now x < 0 */
			screenInfo = ibmScreens[ --ibmCurrentScreen ] ;
			x += screenInfo->ibm_ScreenBounds.x2 ;
		    }
		    setCursor = TRUE ;
		}

		while ( x > screenInfo->ibm_ScreenBounds.x2 ) {
		    if ( screenInfo == ibmScreens[ ibmNumScreens - 1 ] ) {
			/* Already At Last Possible Screen */
			if ( ibmXWrapScreen ) {
			    x -= screenInfo->ibm_ScreenBounds.x2 ; /* x > 0 */
			    screenInfo = ibmScreens[ ibmCurrentScreen = 0 ] ;
			    x += screenInfo->ibm_ScreenBounds.x1 ;
			}
			else {
			    x = screenInfo->ibm_ScreenBounds.x2 ;
			}
		    }
		    else {
			x -= screenInfo->ibm_ScreenBounds.x2 ; /* Now x > 0 */
			screenInfo = ibmScreens[ ++ibmCurrentScreen ] ;
			x += screenInfo->ibm_ScreenBounds.x1 ;
		    }
		    setCursor = TRUE ;
		}

		if ( y > screenInfo->ibm_ScreenBounds.y2 ) {
		    y = screenInfo->ibm_ScreenBounds.y2 ;
		    setCursor = TRUE ;
		}
		else if ( y < screenInfo->ibm_ScreenBounds.y1 ) {
		    y = screenInfo->ibm_ScreenBounds.y1 ;
		    setCursor = TRUE ;
		}

		if (setCursor) {
		    /* OS-DEPENDANT MACRO GOES HERE !!
		     * MACRO DEFINED IN FILE "OSio.h"
		     * TELL OS THAT CURSOR HAS MOVED
		     * TO A NEW POSTION
		     */
		    OS_TellCursorPosition( x, y ) ;
		}

		e.u.keyButtonPointer.rootX =
		  ( x -= screenInfo->ibm_ScreenBounds.x1 ) ;
		e.u.keyButtonPointer.rootY =
		  ( y -= screenInfo->ibm_ScreenBounds.y1 ) ;

		if ( oldScr != ibmCurrentScreen ) {
		    CursorPtr	prevCursor= screenInfo->ibm_CurrentCursor;
		    (*ibmHideCursor( oldScr ) )( oldScr ) ;
		    NewCurrentScreen( screenInfo->ibm_Screen, x, y ) ;
		    if ( screenInfo->ibm_CurrentCursor == prevCursor ) {
			(* screenInfo->ibm_Screen->DisplayCursor )(
					screenInfo->ibm_Screen,
					ibmCurrentCursor( oldScr ) ) ;
		    }
		}

		if (pE->xe_device==XE_MOUSE) {
		    (* screenInfo->ibm_CursorShow )( x, y ) ;
		}
	    /*
	     * The following silly looking code is because the old version of
	     * the driver only delivers 16 bits worth of centiseconds. We are
	     * supposed to be keeping time in terms of 32 bits of milliseconds.
	     */
		if (needTime)
		{
		    needTime = 0;
		    gettimeofday(&tp, 0);
		    nowInCentiSecs = ((tp.tv_sec * 100)
				   + (tp.tv_usec / 10000)) & 0xFFFF;
		/* same as driver */
		    nowInMilliSecs = (tp.tv_sec * 1000) + (tp.tv_usec / 1000);
		/* beware overflow */
		}
		if ((adjustCentiSecs = nowInCentiSecs - pE->xe_time) < -20000)
		    adjustCentiSecs += 0x10000;
		else
		    if (adjustCentiSecs > 20000)
			adjustCentiSecs -= 0x10000;
		e.u.keyButtonPointer.time = lastEventTime =
		    nowInMilliSecs - adjustCentiSecs * 10;

		if ((pE->xe_type == XE_BUTTON)&&(pE->xe_device==XE_DKB)) {
		    extern CARD16 keyModifiersList[];

		    /* 
		     * Caps Lock code courtesy of John Kubiatowicz
		     * IBM/Athena
		     */
		    if ((keyModifiersList[pE->xe_key]&LockMask)&&
						ibmLockEnabled) {
			/* Deal with shift lock toggling */
			if (pE->xe_direction == XE_KBTDOWN) {
			    ibmLockState= !ibmLockState;
			    OS_CapsLockFeedback(ibmLockState);
			    if (ibmLockState) { /* turning lock on */
				e.u.u.type= KeyPress;
				e.u.u.detail= ibmCurLockKey= pE->xe_key;
			    }
			    else {
				/* turn off same key as originally locked */
				e.u.u.type= KeyRelease;
				e.u.u.detail= ibmCurLockKey;
			    }
			    (ibmKeybd->processInputProc)(&e,ibmKeybd);
			}
		    } else {
			e.u.u.detail= pE->xe_key;
			switch (pE->xe_direction) {
			    case XE_KBTDOWN:
				e.u.u.type= KeyPress;
				(ibmKeybd->processInputProc)(&e,ibmKeybd);
				break;
			    case XE_KBTUP:
				e.u.u.type= KeyRelease;
				(ibmKeybd->processInputProc)(&e,ibmKeybd);
				break;
			    default:	/* hopefully BUTTON_RAW_TYPE */
				ErrorF("got a raw button, what do I do?\n");
				break;
			}
		    }
		}
		else if ((pE->xe_device==XE_MOUSE)||(pE->xe_device==XE_TABLET))
		{
		    if (pE->xe_type == XE_BUTTON )
		    {
			if (pE->xe_direction == XE_KBTDOWN)
			    e.u.u.type= ButtonPress;
			else
			    e.u.u.type= ButtonRelease;
			/* mouse buttons numbered from one */
			e.u.u.detail = pE->xe_key+1;
		    }
		    else
			e.u.u.type = MotionNotify;
		    (*ibmPtr->processInputProc)(&e,ibmPtr);
		}
	}
    }
    alreadyHere= 0;
    return ;
}

#endif
TimeSinceLastInputEvent()
{
/*    TRACE(("TimeSinceLastInputEvent()\n"));*/

    if (lastEventTime == 0)
	lastEventTime = GetTimeInMillis();
    return GetTimeInMillis() - lastEventTime;
}

/***==================================================================***/

ibmSaveScreen( pScreen, on )
    ScreenPtr	pScreen;
    int		on;
{
    TRACE(("ibmSaveScreen( pScreen= 0x%x, on= %d )\n",pScreen,on));

    if ( on == SCREEN_SAVER_FORCER ) {
	lastEventTime = GetTimeInMillis();
	return TRUE;
    }
    else
	return FALSE;
}
