/***********************************************************
		Copyright IBM Corporation 1988

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

/* $Header$ */
/* $Source$ */

#ifndef lint
static char *rcsid = "$Header$";
static char sccsid[] = "@(#)ibmsave.c	1.1 88/09/14 00:18:13";
#endif

#include <sys/types.h>
#include <sys/time.h>

#define NEED_EVENTS

#include "X.h"
#define NEED_REPLIES
#include "Xproto.h"
#include "misc.h"
#include "scrnintstr.h"
#include "os.h"
#include "regionstr.h"
#include "windowstr.h"
#include "input.h"
#include "resource.h"
#include "colormapst.h"
#include "cursorstr.h"
#include "dixstruct.h"
#include "gc.h"
#include "gcstruct.h"
#include "servermd.h"
#include "miscstruct.h"
#include "pixmapstr.h"
#include "colormap.h"

#include "OScompiler.h"

#include "ibmKeybd.h"
#include "ibmMouse.h"

#include "ibmScreen.h"
#include "OSio.h"
#include "ibmTrace.h"

extern WindowRec WindowTable[];

/***====================================================================***/

static	WindowPtr obscuringWins[MAXSCREENS];

static void
ObscureEverything(pScreen)
ScreenPtr	pScreen;
{
WindowPtr	pWin;
unsigned	mask= CWBackPixel|CWSaveUnder;
XID		attributes[2];
int		result;

    TRACE(("ObscureEverything()\n"));
    attributes[0]= pScreen->blackPixel;
    attributes[1]= xTrue;
    pWin= obscuringWins[pScreen->myNum]= CreateWindow(
			FakeClientID(0),		/* window id */
			&WindowTable[pScreen->myNum],	/* parent */
			0,0,				/* x,y */
			pScreen->width,pScreen->height,	/* width,height */
			0,				/* border width */
			InputOutput,			/* class */
			mask, attributes,		/* attributes */
			0,				/* depth */
			(ClientPtr)NULL,		/* client */
			WindowTable[pScreen->myNum].visual,/* visual */
			&result);			/* error */
    AddResource(pWin->wid,RT_WINDOW,(pointer)pWin,DeleteWindow, RC_CORE);
    pWin->overrideRedirect= TRUE;
    MapWindow( pWin, TRUE, FALSE, FALSE, (ClientPtr)NULL);
    return;
}

/***====================================================================***/

static void
ExposeEverything(pScreen)
ScreenPtr	pScreen;
{
WindowPtr	pWin= obscuringWins[pScreen->myNum];

    TRACE(("ExposeEverything()\n"));
    FreeResource(pWin->wid,RC_NONE);
    obscuringWins[pScreen->myNum]= NULL;
    return;
}

/***====================================================================***/

void
ibmDeactivateScreens()
{
ScreenPtr 	pScreen;
void 		(*fnp)();
int		scrn;

    TRACE(("ibmDeactivateScreens()\n"));
#ifdef OS_SaveState
    OS_SaveState();
#endif
    for (scrn = 0; scrn < ibmNumScreens; scrn++) {

	pScreen = ibmScreen(scrn);
	if ((!pScreen)||(ibmScreenState(pScreen->myNum)!=SCREEN_ACTIVE))
	    continue;
	if (!pScreen) {
	    ErrorF("WSGO!!! trying to deactivate null screen\n");
	    ErrorF("to debug: ask Paquin what's going on here\n");
	    return ;
	}
	
	ObscureEverything(pScreen);
	ibmSetScreenState(pScreen->myNum,SCREEN_INACTIVE);
	/* find out what the screen wants to do vis-a-vis saving */
	fnp = ibmScreens[scrn]->ibm_SaveFunc;
	if (fnp) (*fnp)();


    }
    DontListenToAnybody();
    return;
}
	
void
ibmReactivateScreens()
{
ScreenPtr 	pScreen;
void 		(*fnp)(); /* "fnp" means "FunctionPointer" */
int		scrn;
WindowPtr	pRoot;
ColormapPtr	pCmap;


    TRACE(("ibmReactivateScreens()\n"));
#ifdef OS_RestoreState
    OS_RestoreState();
#endif

    PayAttentionToClientsAgain();
    for (scrn = 0; scrn < ibmNumScreens; scrn++) {
	pScreen = ibmScreen(scrn);
	if ((!pScreen)||(ibmScreenState(pScreen->myNum)!=SCREEN_INACTIVE))
	    continue;
		
	ibmSetScreenState(pScreen->myNum,SCREEN_ACTIVE);

	fnp = ibmScreens[scrn]->ibm_RestoreFunc;
	if (fnp)
	    (*fnp)();
	ExposeEverything(pScreen);
    }
    return;
}
