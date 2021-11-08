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

/* $Header: /andrew/X11/R3src/tape/server/ddx/ibm/mpel/RCS/mpelCursor.c,v 6.2 88/10/25 01:45:39 kbg Exp $ */
/* $Source: /andrew/X11/R3src/tape/server/ddx/ibm/mpel/RCS/mpelCursor.c,v $ */

#ifndef lint
static char *rcsid = "$Header: /andrew/X11/R3src/tape/server/ddx/ibm/mpel/RCS/mpelCursor.c,v 6.2 88/10/25 01:45:39 kbg Exp $" ;
#endif

/*
 * xcursor.c -- software cursor
 */

#include "X.h"
#include "miscstruct.h"
#include "scrnintstr.h"
#include "cursorstr.h"
#include "maskbits.h"

#include "OScompiler.h"

#include "ibmScreen.h"

#include "ibmTrace.h"

#include "mpelHdwr.h"
#include "mpelFifo.h"

#define	CURSOR_MAX_SIZE		( ( 32 / 8 ) * 32 )
#define	CURRENT_WIDTH		( curCursor->width > 16 ? 32 : 16 )
#define	CURRENT_HEIGHT		MIN( curCursor->height, 32 )

/* Global Cursor State Semaphore */
int mpelcursorSemaphore = 0 ;

static int screen_index ;
static short c_x ;
static short c_y ;
static unsigned long int c_fg =1 ;
static unsigned long int c_bg =0 ;
static int cursor_not_displayed = 1 ;
static int cursor_is_valid = FALSE ;

static unsigned short int forePtrn[ CURSOR_MAX_SIZE / 2 ] ;
static unsigned short int backPtrn[ CURSOR_MAX_SIZE / 2 ] ;

/*** ==================================================================***/

/*
 * mpelPutCursorOn( x, y )
 */

static void
mpelPutCursorOn( x, y )
    register short int x, y ;
{
    register CursorPtr curCursor ;
    register unsigned int byteSize, w ;
    register unsigned short int *fSrc ;
    register unsigned short int *bSrc ;
    unsigned short int tempFore[CURSOR_MAX_SIZE] ;
    unsigned short int tempBack[CURSOR_MAX_SIZE] ;
    mpelVPMBLTDestination saveBlit ;
    mpelBLTImmedWColorExpansion	cursorBlit ;

    TRACE( ( "mpelPutCursorOn(x=%d,y=%d)\n", x, y )) ;

    if ( ibmScreenState( screen_index )!=SCREEN_ACTIVE )
	return;
    curCursor = ibmCurrentCursor( screen_index ) ;
    fSrc = (unsigned short int *) forePtrn ;
    bSrc = (unsigned short int *) backPtrn ;

    /* Save The Existing Image */
    saveBlit.destaddr = mpelAddr( MPEL_CURSOR_SAVE ) ;
    saveBlit.source.lleft.x = x ;
    saveBlit.source.lleft.y = MPEL_HEIGHT - ( y + CURRENT_HEIGHT ) ;
    saveBlit.source.uright.x = x + CURRENT_WIDTH - 1 ;
    saveBlit.source.uright.y = MPEL_HEIGHT - 1 - y ;
    saveBlit.comp = 0x0001 ;

    MPELSetPlaneMask( 0 ) ;
    MPELVPMBLTDest( &saveBlit ) ;

    if ( x > MPEL_WIDTH - CURRENT_WIDTH ) {
	int i ;
	unsigned short int *fDst, *bDst ;
	unsigned short int mask ;

	mask = endtab[ ( MPEL_WIDTH - x ) & 0xF ] >> 16 ;
	w = ( ( ( MPEL_WIDTH - x ) + 0xF ) >> 4 ) << 4 ;
	fDst = tempFore ;
	bDst = tempBack ;
	for ( i = CURRENT_HEIGHT ; i-- ; ) {
	    *fDst = *fSrc++ ;
	    *bDst = *bSrc++ ;
	    if ( CURRENT_WIDTH > 16 ) {
		if ( w > 16 ) {
		    *++fDst = *fSrc ;
		    *++bDst = *bSrc ;
		}
		fSrc++ ;
		bSrc++ ;
	    }
	    *fDst++ &= mask ;
	    *bDst++ &= mask ;
	}
	fSrc = tempFore ;
	bSrc = tempBack ;
    }
    else {
	w = CURRENT_WIDTH ;
    }

    cursorBlit.dest.lleft.x = x ;
    cursorBlit.dest.lleft.y = MPEL_HEIGHT - ( y + CURRENT_HEIGHT ) ;
    cursorBlit.dest.uright.x = x + w - 1 ;
    cursorBlit.dest.uright.y = MPEL_HEIGHT - 1 - y ;
    cursorBlit.alu = GXcopy+1 ;
    cursorBlit.color = c_fg ;
    byteSize = ( w >> 3 ) * CURRENT_HEIGHT ;

    MPELBLTImmedWColorExpansion( byteSize, &cursorBlit ) ;
    MPELSendData( byteSize, fSrc ) ;

    cursorBlit.color = c_bg ;
    MPELBLTImmedWColorExpansion( byteSize, &cursorBlit ) ;
    MPELSendData( byteSize, bSrc ) ;

    /* Cursor Is Now Active */
    cursor_not_displayed = 0 ;

    return ;
}

/*** ==================================================================***/

int
mpelRemoveCursor()
{
    if ( ( cursor_not_displayed == 0 ) && 
	 ( ibmScreenState( screen_index )==SCREEN_ACTIVE ) ) {
	register CursorPtr curCursor ;
	mpelSrcBLTVPM blt ;

	curCursor = ibmCurrentCursor( screen_index ) ;
	blt.srcaddr = mpelAddr( MPEL_CURSOR_SAVE ) ;
	blt.dest.lleft.x = c_x ;
	blt.dest.lleft.y = MPEL_HEIGHT - ( c_y + CURRENT_HEIGHT ) ;
	blt.dest.uright.x = c_x + CURRENT_WIDTH - 1 ;
	blt.dest.uright.y = MPEL_HEIGHT - 1 - c_y ;
	blt.bpixel = 0x0008 ;
	blt.alu = GXcopy+1 ;

	MPELSetPlaneMask( 0 ) ;
	MPELSrcBLTVPM( &blt ) ;
    }

    return cursor_not_displayed = 1 ;
}

/*** ==================================================================***/

void
mpelShowCursor( x, y )
    register short x, y ;
{
register CursorPtr curCursor = ibmCurrentCursor( screen_index ) ;

    if ( ibmScreenState( screen_index )!=SCREEN_ACTIVE ) {
	c_x= x-curCursor->xhot;
	c_y= y-curCursor->yhot;
    }
    if ( cursor_not_displayed == 0 ) {
	(void) mpelRemoveCursor() ;
	x -= curCursor->xhot ;
	y -= curCursor->yhot ;
    }
    c_x = x ;
    c_y = y ;
    if ( cursor_is_valid )
	mpelPutCursorOn( x, y ) ;
    return ;
}

/*** ================================================================== ***/

/* check if the cursor is in this rectangle.  if so, remove and return TRUE
    else return FALSE */
int
mpelCheckCursor( x, y, lx, ly )
register const int x, y, lx, ly ;
{
register CursorPtr curCursor = ibmCurrentCursor( screen_index ) ;

if ( !mpelcursorSemaphore && !cursor_not_displayed 
  && (ibmScreenState( screen_index )==SCREEN_ACTIVE)
  && !( ( x >= ( c_x + CURRENT_WIDTH ) )
     || ( y >= ( c_y + CURRENT_HEIGHT ) )
     || ( ( x + lx ) <= c_x )
     || ( ( y + ly ) <= c_y ) ) )
	return mpelRemoveCursor() ;
else
	return FALSE ;
/*NOTREACHED*/
}

/*** ==================================================================***/

void mpelReplaceCursor()
{

if ( cursor_not_displayed && !mpelcursorSemaphore &&
	ibmScreenState( screen_index )==SCREEN_ACTIVE )
	mpelShowCursor( c_x, c_y ) ;

return ;
}

/*** ==================================================================***/

/*ARGSUSED*/
void mpelRecolorCursor( pVisual )
    register VisualPtr	pVisual ;
{
    register CursorPtr curCursor = ibmCurrentCursor( screen_index ) ;
    TRACE( ( "mpelRecolorCursor( 0x%x )\n", pVisual )) ;

    mpelFindColor( &c_fg,
		   curCursor->foreRed,
		   curCursor->foreGreen,
		   curCursor->foreBlue,
		   pVisual ) ;
    mpelFindColor( &c_bg,
		   curCursor->backRed,
		   curCursor->backGreen,
		   curCursor->backBlue,
		   pVisual ) ;
    return ;
}

/* ************************************************************************** */

void
mpelCursorInit( index )
register int index ;
{
    TRACE( ( "mpelCursorInit()\n" )) ;

    ibmCursorShow( index ) = mpelShowCursor ;

    ibmCurrentCursor( index ) = NULL ;
    cursor_is_valid = FALSE ;

    screen_index = index ;

    c_x = 0 ;
    c_y = 0 ;
    mpelcursorSemaphore = 0 ;
    cursor_not_displayed = 1 ;
    return ;
}

/*** ============================================================***/

Bool
mpelRealizeCursor( pScr, pCurs )
    register ScreenPtr	pScr ;
    register CursorPtr	pCurs ;
{
    register unsigned long int *pFG, *pBG ;
    register unsigned long int *psrcImage ;
    register unsigned long int *psrcMask ;
    register int i ;
    register unsigned long int endbits ;
    int srcWidth ;
    int srcHeight ;
    int srcRealWidth ;

    TRACE( ( "mpelRealizeCursor(pScr=0x%x,pCurs=0x%x)\n", pScr, pCurs) ) ;
    if ( !( pCurs->devPriv[ pScr->myNum ] = (pointer) Xalloc( 256 ) ) )
	{
	ErrorF( "mpelRealizeCursor: can't malloc\n" ) ;
	return FALSE ;
	}
    pFG = (unsigned long int *) pCurs->devPriv[pScr->myNum] ;
    bzero( (char *) pFG, 256 ) ;
    pBG = (unsigned long int *) pFG + 32 ; /* words */
    psrcImage = (unsigned long int *) pCurs->source ;
    psrcMask = (unsigned long int *) pCurs->mask ;
    srcRealWidth = ( pCurs->width + 31 ) / 32 ;

    srcWidth = MIN( pCurs->width, 32 ) ;
    srcHeight = MIN( pCurs->height, 32 ) ;

    endbits = -1 << ( 32 - srcWidth ) ;

    for ( i = srcHeight ; i-- ; )
	{
	*pFG++ = (*psrcMask & endbits) & *psrcImage ;
	*pBG++ = (*psrcMask & endbits) & ~ *psrcImage ;
	psrcImage = psrcImage + srcRealWidth ;
	psrcMask = psrcMask + srcRealWidth ;
	}

    if ( ! ibmCurrentCursor( pScr->myNum ) ) {
	ibmCursorHotX( i ) = pCurs->xhot;
	ibmCursorHotY( i ) = pCurs->yhot;
	ibmCurrentCursor( pScr->myNum ) = pCurs;
	cursor_is_valid = TRUE;
    }

    TRACE( ( "exiting mpelRealizeCursor\n" )) ;
    return TRUE ;
}

/*** ============================================================***/

Bool
mpelUnrealizeCursor( pScr, pCurs )
    register ScreenPtr 	pScr ;
    register CursorPtr 	pCurs ;
{

    TRACE( ("mpelUnrealizeCursor(pScr=0x%x,pCurs=0x%x)\n", pScr, pCurs )) ;

    Xfree( (char *) pCurs->devPriv[ pScr->myNum ] ) ;
    pCurs->devPriv[ pScr->myNum ] = 0 ;
    if ( ibmCurrentCursor( pScr->myNum ) == pCurs )
	cursor_is_valid = FALSE ;
    return TRUE ;
}

void
mpelColorCursor( fg, bg )
register unsigned long int fg, bg ;
{
TRACE( ( "mpelColorCursor( %d, %d )\n", fg, bg )) ;
if ( fg > 255 || bg > 255 ) {
    ErrorF( "mpelColorCursor: bad color value(s) (fg/bg)=(%d,%d)", fg, bg ) ;
}
else {
    c_fg = fg ;
    c_bg = bg ;
}

return ;
}

/*** ============================================================***/

Bool
mpelDisplayCursor( pScr, pCurs )
    ScreenPtr	pScr ;
    CursorPtr	pCurs ;
{
    register CursorPtr curCursor = ibmCurrentCursor( pScr->myNum ) ;
    register unsigned short int  *srcFG, *srcBG, *dstFG, *dstBG ;
    register int i ;

    TRACE( ( "mpelDisplayCursor( pScr =0x%x, pCurs =0x%x )\n", pScr, pCurs )) ;

    /* cleanup old cursor */

    if ( cursor_is_valid ) {
	if ( cursor_not_displayed == 0 )
	    (void) mpelRemoveCursor() ;
	curCursor = ibmCurrentCursor( pScr->myNum ) ;
	c_x += curCursor->xhot ;
	c_y += curCursor->yhot ;
    }

    i = pScr->myNum ;
    ibmCurrentCursor( i ) = curCursor = pCurs ;
    ibmCursorHotX( i ) = pCurs->xhot ;
    ibmCursorHotY( i ) = pCurs->yhot ;

    srcFG = (unsigned short int *) pCurs->devPriv[ pScr->myNum ] ;
    srcBG = srcFG + ( CURSOR_MAX_SIZE / 2 );
    dstFG = (unsigned short int *) forePtrn ;
    dstBG = (unsigned short int *) backPtrn ;
    for ( i = CURRENT_HEIGHT ; i-- ; ) {
	*dstFG++ = *srcFG++ ;
	*dstBG++ = *srcBG++ ;
	if ( CURRENT_WIDTH > 16 ) {
	    *dstFG++ = *srcFG ;
	    *dstBG++ = *srcBG ;
	}
	srcFG++ ;
	srcBG++ ;
    }

    mpelRecolorCursor( pScr->visuals ) ;

    c_x -= curCursor->xhot ;
    c_y -= curCursor->yhot ;
    mpelPutCursorOn( c_x, c_y ) ;
    return cursor_is_valid = TRUE ;
}

/*** ==================================================================***/

void
mpelRevalidateCursor()
{
    cursor_not_displayed= FALSE;
    if (cursor_is_valid)
	mpelPutCursorOn( c_x, c_y ) ;
}

