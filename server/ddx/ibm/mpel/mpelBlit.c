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

/* $Header: /andrew/X11/R3src/tape/server/ddx/ibm/mpel/RCS/mpelBlit.c,v 6.2 88/10/25 01:44:35 kbg Exp $ */
/* $Source: /andrew/X11/R3src/tape/server/ddx/ibm/mpel/RCS/mpelBlit.c,v $ */

#ifndef lint
static char *rcsid = "$Header: /andrew/X11/R3src/tape/server/ddx/ibm/mpel/RCS/mpelBlit.c,v 6.2 88/10/25 01:44:35 kbg Exp $";
#endif

#include "Xmd.h"

#include "ibmTrace.h"

#include "mpelFifo.h"

/* Cursor Stuff */
extern int mpelcursorSemaphore ;
extern int mpelCheckCursor() ;
extern void mpelReplaceCursor() ;

void
mpelBitBlt( alu, rmask, wmask, x0, y0, x1, y1, w, h )
int x0, y0, x1, y1, w, h, rmask, wmask, alu ;
{
    int cursor_saved ;
    mpelVPMCopy	vpm;

    TRACE(("mpelBitBlt(0x%x,0x%x,0x%x,%d,%d,%d,%d,%d,%d)\n",
	alu,rmask,wmask,x0,y0,x1,y1,w,h));

    /* If Cursor Is In The Way Remove It */
    cursor_saved = !mpelcursorSemaphore
    	    && ( mpelCheckCursor( x0, y0, w, h )
    	      || mpelCheckCursor( x1, y1, w, h ) ) ;

    mpelSetPlaneMask(wmask);

    vpm.source.lleft.x=		x0;
    vpm.source.lleft.y=		MPEL_HEIGHT - ( y0 + h ) ;
    vpm.source.uright.x=	x0 + w - 1;
    vpm.source.uright.y=	MPEL_HEIGHT - 1 - y0 ;
    vpm.dlleft.x=		x1;
    vpm.dlleft.y=		MPEL_HEIGHT - ( y1 + h ) ;
    vpm.alu=			alu + 1;
    MPELVPMCopy(&vpm);

    if ( cursor_saved )
	mpelReplaceCursor() ;

    return;
}
