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

/* $Header: /andrew/X11/R3src/Xbeta/server/ddx/ibm/ega/RCS/egaCurs.c,v 9.0 88/10/18 12:51:47 erik Exp $ */
/* $Source: /andrew/X11/R3src/Xbeta/server/ddx/ibm/ega/RCS/egaCurs.c,v $ */

#ifndef lint
static char *rcsid = "$Header: /andrew/X11/R3src/Xbeta/server/ddx/ibm/ega/RCS/egaCurs.c,v 9.0 88/10/18 12:51:47 erik Exp $";
static char sccsid[] = "@(#)egacurs.c	3.1 88/09/22 09:33:09";
#endif

/*
 * Software Cursor implementation for ega
 * Paquin 7/87
 * Modifed For ega -- Paul Shupak 10/87
 *
 * Depends on owning some off-screen memory in SAVELOC, etc #defined areas
 */

#include "X.h"
#include "Xmd.h"

#include "miscstruct.h"
#include "scrnintstr.h"
#include "pixmapstr.h"
#include "cursorstr.h"

#include "ibmScreen.h"

#include "ibmTrace.h"
#include "egaVideo.h"
#include "egaProcs.h"

#include "ibmIOArch.h"

#undef TRUE
#undef FALSE
#define TRUE 1
#define FALSE 0

/* CONSTANTS FOR NOW */
#define MAX_CURSOR_WIDTH 32
#define MAX_CURSOR_HEIGHT 32
#define CURSOR_SAVE_ROW ( MAX_OFFSCREEN_ROW - MAX_CURSOR_HEIGHT )
#define CURSOR_SAVE_COL 0

/* Global Semaphore For Low Level ( e.g. Screen Drawing ) Routines */
int egaCursorChecking = 0 ;

/* Static Cursor Variables */
static int cursor_save_x_loc ;
static int cursor_save_y_loc ;
static int cursor_save_width ;
static int cursor_save_height ;

static short c_x ;
static short c_y ;
static unsigned long int c_fg = 1 ;
static unsigned long int c_bg = 0 ;
static int active = FALSE ;
static int cursor_not_displayed = FALSE ;
static int screen_index ;

static unsigned long int cursorFG[ MAX_CURSOR_HEIGHT ] ;
static int foreground_width  = MAX_CURSOR_WIDTH ;	/* width */
static int foreground_height = MAX_CURSOR_HEIGHT ;	/* height */

static unsigned long int cursorBG[ MAX_CURSOR_HEIGHT ] ;
static int background_width  = MAX_CURSOR_WIDTH ;	/* width */
static int background_height = MAX_CURSOR_HEIGHT ;	/* height */

/* Bytewidth == 1 */
static void
fast_CursorBlt1( source, destination, height )
register volatile char *source ;
register volatile char *destination ;
register unsigned int height ;
{
register volatile int tmp ;
register const unsigned int notZero = ~0 ;
register const int row_jump = BYTES_PER_ROW - 1 ;

for ( ; height-- ; source += row_jump, destination += row_jump ) {
	tmp = *( (EgaMemoryPtr) ( source ++ ) ) ;
	( *( (EgaMemoryPtr) ( destination ++ ) ) = notZero ) ;
}
return ;
}

/* Bytewidth == 2 */
static void
fast_CursorBlt2( source, destination, height )
register volatile char *source ;
register volatile char *destination ;
register unsigned int height ;
{
register volatile int tmp ;
register const unsigned int notZero = ~0 ;
register const int row_jump = BYTES_PER_ROW - 2 ;

for ( ; height-- ; source += row_jump, destination += row_jump ) {
	tmp = *( (EgaMemoryPtr) ( source ++ ) ) ;
	( *( (EgaMemoryPtr) ( destination ++ ) ) = notZero ) ;
	tmp = *( (EgaMemoryPtr) ( source ++ ) ) ;
	( *( (EgaMemoryPtr) ( destination ++ ) ) = notZero ) ;
}
return ;
}
/* Bytewidth == 3 */
static void
fast_CursorBlt3( source, destination, height )
register volatile char *source ;
register volatile char *destination ;
register unsigned int height ;
{
register volatile int tmp ;
register const unsigned int notZero = ~0 ;
register const int row_jump = BYTES_PER_ROW - 3 ;

for ( ; height-- ; source += row_jump, destination += row_jump ) {
	tmp = *( (EgaMemoryPtr) ( source ++ ) ) ;
	( *( (EgaMemoryPtr) ( destination ++ ) ) = notZero ) ;
	tmp = *( (EgaMemoryPtr) ( source ++ ) ) ;
	( *( (EgaMemoryPtr) ( destination ++ ) ) = notZero ) ;
	tmp = *( (EgaMemoryPtr) ( source ++ ) ) ;
	( *( (EgaMemoryPtr) ( destination ++ ) ) = notZero ) ;
}
return ;
}
/* Bytewidth == 4 */
static void
fast_CursorBlt4( source, destination, height )
register volatile char *source ;
register volatile char *destination ;
register unsigned int height ;
{
register volatile int tmp ;
register const unsigned int notZero = ~0 ;
register const int row_jump = BYTES_PER_ROW - 4 ;

for ( ; height-- ; source += row_jump, destination += row_jump ) {
	tmp = *( (EgaMemoryPtr) ( source ++ ) ) ;
	( *( (EgaMemoryPtr) ( destination ++ ) ) = notZero ) ;
	tmp = *( (EgaMemoryPtr) ( source ++ ) ) ;
	( *( (EgaMemoryPtr) ( destination ++ ) ) = notZero ) ;
	tmp = *( (EgaMemoryPtr) ( source ++ ) ) ;
	( *( (EgaMemoryPtr) ( destination ++ ) ) = notZero ) ;
	tmp = *( (EgaMemoryPtr) ( source ++ ) ) ;
	( *( (EgaMemoryPtr) ( destination ++ ) ) = notZero ) ;
}
return ;
}
/* Bytewidth == 5 */
static void
fast_CursorBlt5( source, destination, height )
register volatile char *source ;
register volatile char *destination ;
register unsigned int height ;
{
register volatile int tmp ;
register const unsigned int notZero = ~0 ;
register const int row_jump = BYTES_PER_ROW - 5 ;

for ( ; height-- ; source += row_jump, destination += row_jump ) {
	tmp = *( (EgaMemoryPtr) ( source ++ ) ) ;
	( *( (EgaMemoryPtr) ( destination ++ ) ) = notZero ) ;
	tmp = *( (EgaMemoryPtr) ( source ++ ) ) ;
	( *( (EgaMemoryPtr) ( destination ++ ) ) = notZero ) ;
	tmp = *( (EgaMemoryPtr) ( source ++ ) ) ;
	( *( (EgaMemoryPtr) ( destination ++ ) ) = notZero ) ;
	tmp = *( (EgaMemoryPtr) ( source ++ ) ) ;
	( *( (EgaMemoryPtr) ( destination ++ ) ) = notZero ) ;
	tmp = *( (EgaMemoryPtr) ( source ++ ) ) ;
	( *( (EgaMemoryPtr) ( destination ++ ) ) = notZero ) ;
}
return ;
}

static void (*UnwoundBlitProcs[5])() = {
	fast_CursorBlt1,
	fast_CursorBlt2,
	fast_CursorBlt3,
	fast_CursorBlt4,
	fast_CursorBlt5
} ;

/* Only Does Special Case -- VERY Fast Blt
 * ( Width % 8 ) == 0 AND ( x0 % 8 ) == 0 AND ( x1 % 8 ) == 0
 */
static void
egaCursorBitBlt( x0, y0, x1, y1, w, h )
register int x0 ;
register int y0 ;
register int x1 ;
register int y1 ;
register int w, h ;
{
/* ******************** Ega Register Setup ******************** */
/* Disable SET/RESET Function */
outb( 0x3CE, 1 ) ;
outb( 0x3CF, 0 ) ;
/* Set Write Mode To 0 -- Read Mode To 0 */
outb( 0x3CE, 5 ) ;
outb( 0x3CF, 0 ) ;
/* Set Bit Mask */
outb( 0x3CE, 8 ) ;
outb( 0x3CF, 0xFF ) ;
/* Set Data Rotate Function To Data "AND" */
outb( 0x3CE, 3 ) ;
outb( 0x3CF, 0x8 ) ;
/* Set Map Mask */
outb( 0x3C4, 2 ) ;
outb( 0x3C5, ALLPLANES ) ;

/* Really Do It Here! */
(*UnwoundBlitProcs[ROW_OFFSET( w ) - 1])(
	(char *) EGABASE + ( BYTES_PER_ROW * y0 ) + ROW_OFFSET( x0 ),
	(char *) EGABASE + ( BYTES_PER_ROW * y1 ) + ROW_OFFSET( x1 ),
	h ) ;

return ;
}

static void
egaScreenDrawCursor( data, w, h, cursorHeight, x, y, xSrc, ySrc )
register const unsigned char *data ;
register int w ;
int const h ;
int const cursorHeight ;
int x ;
int const y ;
int const xSrc ;
int const ySrc ;
{
register unsigned int Maskdata ;
register const unsigned char *cptr ;
register int shift ;
register unsigned SourceRow ;
register int tmp1 ;
register int tmp2 ;
register volatile unsigned char *dst ;
register const unsigned char *lptr ;
register int NeedValX ;
register int SavNeedX ;
int xshift ;

#define paddedByteWidth ( MAX_CURSOR_WIDTH >> 3 ) /* Should Be 4 Bytes */

/* Figure Bit Offsets & Source Address */
if ( ( xshift = ( x - xSrc ) ) < 0 )
	xshift += MAX_CURSOR_WIDTH ;
else if ( xshift >= MAX_CURSOR_WIDTH )
	xshift -= MAX_CURSOR_WIDTH ;

if ( ( tmp1 = ( y - ySrc ) ) < 0 ) /* Now Use Tmp1 as Y-Shift */
	tmp1 += cursorHeight ;
else if ( tmp1 >= cursorHeight ) /* Now Use Tmp1 as Y-Shift */
	tmp1 -= cursorHeight ;

if ( tmp1 )
	data += ( tmp1 * paddedByteWidth ) ;

outb( 0x3CE, 8 ) ; /* Point At The Bit Mask Reg */

/* Do Left Edge */
if ( tmp1 = x & 07 ) {

	tmp2 = ( (unsigned) 0xFF ) >> tmp1 ;
	/* Catch The Cases Where The Entire Region Is Within One Byte */
	if ( ( w -= 8 - tmp1 ) < 0 ) {
		tmp2 &= 0xFF << -w ;
		w = 0 ;
	}
	Maskdata = tmp2 ;
	/*
	 * For Each Line In The Cursor Data
	 */
	SourceRow = h ;
	dst = EGABASE + BYTE_OFFSET( x, y ) ;
	if ( ( NeedValX = xshift - tmp1 ) < 0 ) {
		NeedValX += MAX_CURSOR_WIDTH ;
		cptr = data + ( NeedValX >> 3 ) - paddedByteWidth ;
	}
	else
		cptr = data + ( NeedValX >> 3 ) ;
	if ( shift = NeedValX & 7 )
		for ( ;
		      SourceRow-- ;
		      dst += BYTES_PER_ROW, cptr += paddedByteWidth ) {
			/* Read To Save */
			tmp2 = *( (EgaMemoryPtr) dst ) ;
			/* Write Pattern */
			outb( 0x3CF, /* Set The Bit Mask Reg */
	( Maskdata & ( ( *cptr << shift ) | ( cptr[1] >> ( 8 - shift ) ) ) ) ) ;
			*( (EgaMemoryPtr) dst ) = Maskdata;
		}
	else /* shift == 0 */
		for ( ;
		      SourceRow-- ;
		      dst += BYTES_PER_ROW, cptr += paddedByteWidth ) {
			/* Read To Save */
			tmp2 = *( (EgaMemoryPtr) dst ) ;
			/* Write Pattern */
			outb( 0x3CF, Maskdata & *cptr);
			*( (EgaMemoryPtr) dst ) = Maskdata ;
		}
	x = ( x + 7 ) & ~07 ;
	if ( ( SavNeedX = NeedValX + 8 ) >= MAX_CURSOR_WIDTH )
		SavNeedX -= MAX_CURSOR_WIDTH ;
}
else {
	SavNeedX = xshift ;
}
shift = SavNeedX & 0x7 ;

/* Fill The Center Of The Box */
Maskdata = 0xFF ; /* Set The Bit Mask Reg */

/*
 * For Each Line In The Source Pixmap
 */
for ( SourceRow = h, dst = EGABASE + BYTE_OFFSET( x, y ), lptr = data ;
      SourceRow-- ;
      dst += BYTES_PER_ROW - ROW_OFFSET( w ),
      lptr += paddedByteWidth ) {
	/*
	 * For Each Byte Across The Pattern In The X Dimension
	 */
	for ( tmp2 = ROW_OFFSET( w ), NeedValX = SavNeedX ;
	      tmp2-- ;
	      dst++ ) {
		/* Read To Save */
		tmp1 = *( (EgaMemoryPtr) dst ) ;
		/* Write Pattern */
		cptr = lptr + ( NeedValX >> 3 ) ;
		outb( 0x3CF, /* Set The Bit Mask Reg */
			( shift )
			? ( ( *cptr << shift ) | ( cptr[1] >> ( 8 - shift ) ) )
			: *cptr ) ;
		*( (EgaMemoryPtr) dst ) =  Maskdata ;
		if ( ( NeedValX += 8 ) >= MAX_CURSOR_WIDTH )
			NeedValX -= MAX_CURSOR_WIDTH ;
	}
}

/* Do Right Edge */
if ( tmp1 = BIT_OFFSET( w ) ) { /* x Now Is Byte Aligned */
	Maskdata = ( 0xFF << ( 8 - tmp1 ) ) ; /* Set The Bit Mask */
	/*
	 * For Each Line In The Source Pixmap
	 */
	cptr = data + ( NeedValX >> 3 ) ;
	SourceRow = h ;
	dst = EGABASE + BYTE_OFFSET( ( x + w ), y ) ;
	if ( shift = NeedValX & 7 )
		for ( ;
		      SourceRow-- ;
		      dst += BYTES_PER_ROW, cptr += paddedByteWidth ) {
			/* Read To Save */
			tmp1 = *( (EgaMemoryPtr) dst ) ;
			/* Write Pattern */
			outb( 0x3CF, /* Set The Bit Mask Reg */
	( Maskdata & ( *cptr << shift ) | ( cptr[1] >> ( 8 - shift ) ) ) ) ;
			*( (EgaMemoryPtr) dst ) = Maskdata ;
		}
	else /* shift == 0 */
		for ( ;
		      SourceRow-- ;
		      dst += BYTES_PER_ROW, cptr += paddedByteWidth ) {
			/* Read To Save */
			tmp1 = *( (EgaMemoryPtr) dst ) ;
			/* Write Pattern */
			outb( 0x3CF, Maskdata & *cptr ) ;
			*( (EgaMemoryPtr) dst ) = Maskdata ;
		}
}

return ;
#undef paddedByteWidth
}

void
egaRemoveCursor()
{
    egaCursorBitBlt( CURSOR_SAVE_COL, CURSOR_SAVE_ROW,
		     cursor_save_x_loc, cursor_save_y_loc,
		     cursor_save_width, cursor_save_height ) ;
    active = FALSE ;
return ;
}

void
egaShowCursor( x_loc, y_loc )
register const int x_loc, y_loc ;
{
register int x ;
register int y ;
register int fgwidth ;
register int bgwidth ;
register int bg_height ;
register int fg_height ;
register int hotx, hoty ;

if ( active )
	egaRemoveCursor() ;

bg_height = background_height ;
fg_height = foreground_height ;
fgwidth = foreground_width ;
bgwidth = background_width ;

x = ( hotx = ( ( c_x = x_loc ) - ibmCursorHotX( screen_index ) ) ) ;
y = ( hoty = ( ( c_y = y_loc ) - ibmCursorHotY( screen_index ) ) ) ;

/* Validate The Arguments */
if ( x < 0 ) {
	fgwidth += x ;
	bgwidth += x ;
	x = 0 ;
}
if ( x + fgwidth > ( MAX_COLUMN + 1 ) )
	fgwidth = ( MAX_COLUMN + 1 ) - x ;
if ( x + bgwidth > ( MAX_COLUMN + 1 ) )
	bgwidth = ( MAX_COLUMN + 1 ) - x ;

if ( y < 0 ) {
	bg_height += y ;
	fg_height += y ;
	y = 0 ;
}
if ( y + bg_height > ( MAX_ROW + 1 ) )
	bg_height = ( MAX_ROW + 1 ) - y ;
if ( y + fg_height > ( MAX_ROW + 1 ) )
	fg_height = ( MAX_ROW + 1 ) - y ;

/* Check If The Cursor Is On-Screen */
if ( ( ( fgwidth > 0 ) || ( bgwidth > 0 ) )
  && ( ( bg_height > 0 ) || ( fg_height > 0 ) )
  && ( x <= MAX_COLUMN )
  && ( y <= MAX_ROW ) ) {
	/* Round Edges To Use FAST Blt */
	egaCursorBitBlt( 
		   ( cursor_save_x_loc = x & ~0x7 ),
		   ( cursor_save_y_loc = y ),
		   CURSOR_SAVE_COL,
		   CURSOR_SAVE_ROW,
( cursor_save_width = ( ( ( ( bgwidth > fgwidth ) ? bgwidth : fgwidth )
			+ ( x & 0x7 ) ) + 0x7 ) & ~0x7 ),
		   ( cursor_save_height = ( bg_height > fg_height )
					? bg_height : fg_height ) ) ;
	/*
	 * Set The Plane-Enable
	 */
	outb( 0x3C4, 2 ) ;
	outb( 0x3C5, ALLPLANES ) ; /* Map Mask Register */
	/*
	 * Set The Plans in The Enable-Set/Rest Resiter
	 */
	outb( 0x3CE, 1 ) ;
	outb( 0x3CF, ALLPLANES ) ; /* Map Mask Register */
	/*
	 * Set The Ega's Alu Function
	 */
	outb( 0x3CE, 3 ) ;
	outb( 0x3CF, 0x0 ) ; /* Data Rotate Register */
	/*
	 * Put Display Into SET/RESET Write Mode
	 */
/* ******************** EGAFIX ******************** */
/* Need To Use Mode 0
 * Method Exchange The vga's method of writing
 * the bit-mask and the data
 * i.e. Write The Pattern into The Bit-Mask Register
 * Write What would have been the mask as data
 */
	outb( 0x3CE, 5 ) ;
	outb( 0x3CF, 0x0 ) ; /* Graphics Mode Register */

	if ( ( fg_height > 0 ) && ( fgwidth > 0 ) ) {
		/*
		 * Set The Color in The Set/Reset Register
		 */
		outb( 0x3CE, 0 ) ;
		outb( 0x3CF, c_fg ) ; /* Set/Reset Register */
		egaScreenDrawCursor( (unsigned char *) cursorFG,
				     fgwidth,
				     fg_height,
				     foreground_height,
				     x, y,
				     hotx, hoty ) ;
	}
	if ( ( bg_height > 0 ) && ( bgwidth > 0 ) ) {
		/*
		 * Set The Color in The Set/Reset Register
		 */
		outb( 0x3CE, 0 ) ;
		outb( 0x3CF, c_bg ) ; /* Set/Reset Register */
		egaScreenDrawCursor( (unsigned char *) cursorBG,
				     bgwidth,
				     bg_height,
				     background_height,
				     x, y,
				     hotx, hoty ) ;
	}
        /* Disable The Set/REset Register */
        outb( 0x3CE, 1);
        outb( 0x3CF, 0 ) ;
	active = TRUE ;
}

return ;
}

void egaCursorInit( index )
register int index ;
{

    screen_index = index ;
    c_x = 0 ;
    c_y = 0 ;
    ibmCursorShow( index )=	egaShowCursor ;
    ibmCurrentCursor( index )=	NULL ;
    active = FALSE ;
    egaCursorChecking = 0 ;

return ;
}

void
egaColorCursor( fg, bg )
register unsigned long int fg, bg ;
{

if ( ( fg > 0xF ) || ( bg > 0xF ) ) {
	ErrorF( "egaColorCursor: bad color value(s)(fg/bg)=(%d,%d)", fg, bg ) ;
	return ;
}
c_fg = fg ;
c_bg = bg ;

return ;
}

/***============================================================***/
/*
static void
egaStripCursor( cursorBitArray )
register unsigned long int cursorBitArray[32] ;
{
register unsigned int i ;
register unsigned long int bitMask ;
register unsigned long int *sptr ;

*/
    /* Strip The Cursor Down To Size */
    /* Remove Zero's At The Beginning */
/*
    for ( i = MAX_CURSOR_HEIGHT, sptr = curdef ;
	  i && !*sptr ;
	  i--, sptr-- ) ;
    foregroundOffsetY = MAX_CURSOR_WIDTH - i ;
*/
    /* Remove Zero's At The End */
/*
    for ( sptr = &( cursorBitArray[31] ) ;
	  !*sptr ;
	  i--, sptr-- ) ;
    foreground_height = i ;
*/
    /* Find All Used Bit Positions */
/*
    for ( bitMask = 0 ;
	  i-- ;
	  bitMask |= *sptr-- ) ;
    bitMask = ~bitMask ;
*/
    /* Leading & Trailing 0nes are Unused Bit Positions */
    /* Remove Zero's At The Beginning */
/*
    for ( i = MAX_CURSOR_WIDTH ;
	  bitMask & 0x80000000 && i-- ;
	  bitMask <<= 1 ) ;
*/
    /* Remove Zero's At The End */
/*
    for ( bitMask >>= ( foregroundHorizOffset = MAX_CURSOR_WIDTH - i ) ;
	  bitMask & 0x1 && i-- ;
	  bitMask >>= 1 ) ;
    foreground_width = i ;

*/
    /* Build Copy(s) */
/*
    for ( dptr = &cursorFG[i -1] ;
	  i-- ;
	  *dptr-- = *sptr-- ) ;

    return ;
}
*/

/***============================================================***/

int
egaDisplayCursor( pScr, pCurs )
register ScreenPtr const pScr ;
register CursorPtr const pCurs ;
{
    register const unsigned long int *curdef ;
    register const unsigned long int *sptr ;
    register unsigned long int *dptr ;
    register unsigned long int bitmask ;
    register int i ;

    TRACE( ("egaDisplayCursor( pScr = 0x%x, pCurs = 0x%x )\n", pScr, pCurs ) ) ;

    if ( active )
	egaRemoveCursor() ;
    curdef = (const unsigned long int *) pCurs->devPriv[ pScr->myNum ] ;

    ibmCursorHotX( pScr->myNum ) = pCurs->xhot ;
    ibmCursorHotY( pScr->myNum ) = pCurs->yhot ;
    ibmCurrentCursor( pScr->myNum ) = pCurs ;

    egaColorCursor( 0, 1 ) ;

    /* Strip The Cursor Down To Size */
    for ( i = MAX_CURSOR_HEIGHT, sptr = &( curdef[63] ) ;
	  i && !*sptr ;
	  i--, sptr-- ) ;
    background_height = i ;
    for ( bitmask = 0, dptr = &cursorBG[i -1] ;
	  i-- ;
	  bitmask |= ( *dptr-- = *sptr-- ) ) ;
    for ( bitmask = ~bitmask, i = MAX_CURSOR_WIDTH ;
	  bitmask & 1 ;
	  bitmask >>= 1, i-- ) ;
    background_width = i ;
    for ( i = MAX_CURSOR_HEIGHT, sptr = &( curdef[31] ) ;
	  i && !*sptr ;
	  i--, sptr-- ) ;
    foreground_height = i ;
    for ( dptr = &cursorFG[i -1], bitmask = 0 ;
	  i-- ;
	  bitmask |= ( *dptr-- = *sptr-- ) ) ;
    for ( bitmask = ~bitmask, i = MAX_CURSOR_WIDTH ;
	  bitmask & 1 ;
	  bitmask >>= 1, i-- ) ;
    foreground_width = i ;

    egaShowCursor( c_x, c_y ) ;

    return TRUE ;
}

int
egaCheckCursor( x, y, lx, ly )
register const int x, y, lx, ly ;
{
/* check if the cursor is in this rectangle.  if so, remove and return TRUE
    else return FALSE */

if ( !egaCursorChecking && active
  && !( ( x >= ( cursor_save_x_loc + cursor_save_width ) )
     || ( y >= ( cursor_save_y_loc + cursor_save_height ) )
     || ( ( x + lx ) <= cursor_save_x_loc )
     || ( ( y + ly ) <= cursor_save_y_loc ) ) ) {
	egaRemoveCursor() ;
	return cursor_not_displayed = TRUE ;
}
else
	return FALSE ;
/*NOTREACHED*/
}

void
egaReplaceCursor()
{
if ( cursor_not_displayed && !egaCursorChecking ) {
	egaShowCursor( c_x, c_y ) ;
	cursor_not_displayed = FALSE ;
}

return ;
}
