/***********************************************************
Copyright 1987, 1988 by Digital Equipment Corporation, Maynard, Massachusetts,
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

#define  NEED_EVENTS

#include <sys/types.h>
#include "Ultrix2.0inc.h"
#include <stdio.h>
#include "X.h"		/* for "Device" */
#include "Xproto.h"

#include "scrnintstr.h"
#include "cursorstr.h"
#include "input.h"

#include "qdreg.h"
#include "qdioctl.h"
#include "qevent.h"
#include "qduser.h"
#include "libtl/tlsg.h"

#include "regionstr.h"

extern int  qdGetMotionEvents();
extern void qdChangePointerControl(), qdChangeKeyboardControl(), qdBell();

/* ifdef Vaxstar */
extern short *DMAdev_reg;
extern int	Nplanes;
extern int	Vaxstar;
/* endif Vaxstar */

/*
 * shorthand to a struct in qdmap
 */
extern vsCursor *	mouse;
extern vsBox *		mbox;		/* limits for unreported motion */

extern int		fd_qdss;

/*
Bool qdRealizeCursor(), qdUnrealizeCursor(), qdDisplayCursor();
Bool qdSetCursorPosition();
void qdCursorLimits();
void qdPointerNonInterestBox();
void qdConstrainCursor();
 */
extern void miRecolorCursor();

CursorRec CurrentCurs;
CursorRec LastAlignCurs;/* also used by ProcessInputEvents(), in qdss_io.c */

void
QDAlignCursor( x, y)
    unsigned int	x;	/* top-left corner of sprite */
    unsigned int	y;	/* top-left corner of sprite */
{
    vsCursor    cursor;

    cursor.x = x + LastAlignCurs.xhot - CurrentCurs.xhot;
    cursor.y = y + LastAlignCurs.yhot - CurrentCurs.yhot;
    if ( ioctl( fd_qdss, QD_POSCURSOR, &cursor) < 0)
        ErrorF( "error warping cursor\n");

    LastAlignCurs = CurrentCurs;
}

Bool
qdSetCursorPosition( pScr, newx, newy, generateEvent)
    ScreenPtr		pScr;		/* NOT USED */
    unsigned int	newx;
    unsigned int	newy;
    Bool        generateEvent;
{
    vsCursor	cursor;
    xEvent      motion;
    
    extern DevicePtr	qdPointer;
    extern vsEventQueue *queue;          /* shorthand to a struct in qdmap */
    extern int		lastEventTime;


    cursor.x = newx - CurrentCurs.xhot;
    cursor.y = newy - CurrentCurs.yhot;
    if ( ioctl(fd_qdss, QD_POSCURSOR, &cursor) < 0)
    {
	ErrorF( "error warping cursor\n");
	return FALSE;
    }
    if (generateEvent)
    {
        if (queue->head != queue->tail)
            ProcessInputEvents();
        motion.u.keyButtonPointer.rootX = newx;
        motion.u.keyButtonPointer.rootY = newy;
        motion.u.keyButtonPointer.time = lastEventTime;
        motion.u.u.type = MotionNotify;
        (* qdPointer->processInputProc)( &motion, qdPointer);
    }
    return TRUE;
}

/*
 * note that this no longer calls qdSetCursorPosition, instead
 * ProcessInputEvents reads all the events in queue, then calls QDAlignCursor.
 *
 * Cursor updating (reading the queue) is still slow; perhaps
 * we should wait until QDAlignCursor time to install the new bitmap.
 */
Bool
qdDisplayCursor( pScr, pCurs, x, y)
    ScreenPtr	pScr;
    CursorPtr	pCurs;
    int		x, y;
{
/* ifdef Vaxstar */
    u_short red,green,blue;
    u_short color;
    short * vdac_sav;
    short * eight_planes;
/* endif         */

    xColorItem	colors[2];
    extern struct dga   *Dga;
    register struct dga   *dga = Dga;	/* required to be register */

    CurrentCurs = *pCurs;

    /*
     * load cursor colors
     */

    if (!Vaxstar)
    {
#if NPLANES==24
        colors[0].pixel = 254 | (254 << 8) | (254 << 16);
        colors[1].pixel = 255 | (255 << 8) | (255 << 16);
#else	/* NPLANES == 8 */
        colors[0].pixel = 254;
        colors[1].pixel = 255;
#endif
        colors[0].flags = DoRed|DoGreen|DoBlue;
        colors[0].red = pCurs->backRed;
        colors[0].green = pCurs->backGreen;
        colors[0].blue = pCurs->backBlue;
        colors[1].flags = DoRed|DoGreen|DoBlue;
        colors[1].red = pCurs->foreRed;
        colors[1].green = pCurs->foreGreen;
        colors[1].blue = pCurs->foreBlue;

        dga->csr &= ~CURS_ENB;
        qdStoreColors( NULL, 2, colors);
    }
    else  /*Vaxstar*/
    {
        if (Nplanes != 8)  /*The VDAC*/
    	{
    	    /* load the background color */
    	    blue = (pCurs->backBlue >> 12);
    	    red = (pCurs->backRed >> 12);
    	    green = (pCurs->backGreen >> 12);
    	    color = (green << 8);
    	    color |= (blue << 4);
    	    color |= red;
    	    vdac_sav = DMAdev_reg;
    	    vdac_sav += 33; /* See Page 9 of the VDAC func spec - VKB */
    	    *vdac_sav++ = color;
    	    /* load the foreground color */
    	    blue = (pCurs->foreBlue >> 12);
    	    red = (pCurs->foreRed >> 12);
    	    green = (pCurs->foreGreen >> 12);
    	    color = (green << 8);
    	    color |= (blue << 4);
    	    color |= red;
    	    vdac_sav++;
    	    *vdac_sav = color;
    	}
        else                /*The Brooktree*/
    	{
    	    /* load the background color */
    	    eight_planes = DMAdev_reg;
    	    *eight_planes = 1;
    	    eight_planes += 3;
    	    *eight_planes = (pCurs->backRed >> 8);
    	    *eight_planes = (pCurs->backGreen >> 8);
    	    *eight_planes = (pCurs->backBlue >> 8);
    	    /* load the foreground color */
    	    eight_planes = DMAdev_reg;
    	    *eight_planes = 1;
    	    eight_planes += 3;
    	    *eight_planes = (pCurs->foreRed >> 8);
    	    *eight_planes = (pCurs->foreGreen >> 8);
    	    *eight_planes = (pCurs->foreBlue >> 8);
    	}
    }

    ioctl( fd_qdss, QD_WTCURSOR, (short *)pCurs->devPriv[ pScr->myNum]);

    if (!Vaxstar)
	dga->csr |= CURS_ENB;
}

void
qdPointerNonInterestBox( pScr, pBox)
    ScreenPtr	pScr;
    BoxPtr	pBox;
{
    mbox->left = pBox->x1;
    mbox->right = pBox->x2;
    mbox->top = pBox->y1;
    mbox->bottom = pBox->y2;
}

/*
 * Let DIX do this, as qdss driver is incapable of constraining the sprite
 * to any bounds other than the physical screen.
 */
void
qdConstrainCursor( pScr, pBox)
    ScreenPtr	pScr;
    BoxPtr	pBox;
{
}

/*
 * Qd cursor top-left corner cannot go to negative coordinates
 * (at least the driver thinks not).
 */
void
qdCursorLimits( pScr, pCurs, pHotBox, pTopLeftBox)
    ScreenPtr	pScr;
    CursorPtr	pCurs;
    BoxPtr	pHotBox;
    BoxPtr	pTopLeftBox;	/* return value */
{
    pTopLeftBox->x1 = max( pHotBox->x1 - pCurs->xhot, 0);
    pTopLeftBox->y1 = max( pHotBox->y1 - pCurs->yhot, 0);
    pTopLeftBox->x2 = min( pHotBox->x2 - pCurs->xhot, 1023);
    pTopLeftBox->y2 = min( pHotBox->y2 - pCurs->yhot, 863);
}

Bool
qdRealizeCursor( pScr, pCurs)
    ScreenPtr	pScr;
    CursorPtr	pCurs;	/* a SERVER-DEPENDENT cursor */
{
    short *	phardbits;	/* qdss-defined */
    int *	pservbits;	/* server-defined
				   assumes serverscanlinepad = sizeof(int) */
    int		i;
    unsigned int widthmask = (1<<pCurs->width)-1;

    pCurs->devPriv[pScr->myNum] = (pointer)Xalloc( 32*sizeof(short));
    bzero((char *)pCurs->devPriv[pScr->myNum], 32*sizeof(short));

    /*
     * Munge the SERVER-DEPENDENT, device-independent cursor bits into
     * what the device wants.
     * Assumes that the DIX cursor format is padded to int boundaries.
     * Mask bits first, then source.
     */
    for ( i=0,
	      phardbits = &((short *)pCurs->devPriv[ pScr->myNum])[0],
	      pservbits = (int *)pCurs->mask;
	  i < pCurs->height && i < 16;
	  i++
	)
	*phardbits++ = *pservbits++ & widthmask; /* clip to cursor width */
    for ( i=0,
	      phardbits = &((short *)pCurs->devPriv[ pScr->myNum])[16],
	      pservbits = (int *)pCurs->source;
	  i < pCurs->height && i < 16;
	  i++
	)
	*phardbits++ = *pservbits++ & widthmask; /* clip to cursor width */
}

Bool
qdUnrealizeCursor( pScr, pCurs)
    ScreenPtr	pScr;
    CursorPtr	pCurs;
{
    Xfree( pCurs->devPriv[ pScr->myNum]);
}
