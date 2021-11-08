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
/* $Header: /andrew/X11/R3src/Xbeta/server/ddx/ibm/ega/RCS/egaStipple.c,v 9.0 88/10/18 12:52:55 erik Exp $ */
/* $Source: /andrew/X11/R3src/Xbeta/server/ddx/ibm/ega/RCS/egaStipple.c,v $ */

#ifndef lint
static char *rcsid = "$Header: /andrew/X11/R3src/Xbeta/server/ddx/ibm/ega/RCS/egaStipple.c,v 9.0 88/10/18 12:52:55 erik Exp $";
static char sccsid[] = "@(#)egaStipple.c	3.1 88/09/22 09:32:55";
#endif

#include "X.h"
#include "pixmapstr.h"
#include "egaVideo.h"

#include "ibmIOArch.h"

#undef TRUE
#undef FALSE
#define TRUE 1
#define FALSE 0

static unsigned char getbits( x, patternWidth, lineptr )
register const int x, patternWidth ;
register const unsigned char * const lineptr ;
{
register unsigned char bits ;
register const unsigned char *cptr ;
register shift ;
register wrap ;

cptr = lineptr + ( x >> 3 ) ;
bits = *cptr ;
if ( shift = x & 7 )
	bits = ( bits << shift ) | ( cptr[1] >> ( 8 - shift ) ) ;
if ( ( wrap = x + 8 - patternWidth ) > 0 ) {
	bits &= 0xFF << wrap ;
	bits |= *lineptr >> ( 8 - wrap ) ;
}

return bits ;
}

extern void egaFillSolid() ;
/* Declared in "egacurs.c" */
extern int egaCursorChecking ;
extern int egaCheckCursor() ;
extern void egaReplaceCursor() ;

void egaFillStipple( pStipple, fg, alu, planes, x, y, w, h, xSrc, ySrc )
PixmapPtr const pStipple ;
unsigned long int fg ;
const int alu ;
const unsigned long int planes ;
register int x, y, w, h ;
const int xSrc, ySrc ;
{
register unsigned Maskdata ;
register volatile unsigned char *xDst ;
register unsigned DestinationRow ;
register unsigned int SourceRow ;
register int tmp1 ;
register int tmp2 ;
register volatile unsigned char *dst ;
register const unsigned char *mastersrc ;
register unsigned int height ;
register int NeedValX ;
register unsigned int paddedByteWidth ;
int xshift ;
int yshift ;
unsigned int width ;
int SavNeedX ;
unsigned int data_rotate_value = 0 ;
int cursor_saved ;

/* Test The Raster-Op */
{
	unsigned int invert_existing_data = FALSE ;

	switch ( alu ) {
		case GXclear:		/* 0x0 Zero 0 */
			fg = 0 ;
			break ;
		case GXinvert:		/* 0xa NOT dst */
			data_rotate_value = 0x18 ;
		case GXset:		/* 0xf 1 */
			fg = ALLPLANES ;
			break ;
		case GXnoop:		/* 0x5 dst */
			return ;
		case GXnor:		/* 0x8 NOT src AND NOT dst */
			invert_existing_data = TRUE ;
		case GXandInverted:	/* 0x4 NOT src AND dst */
			fg = ~fg ;
		case GXand:		/* 0x1 src AND dst */
			data_rotate_value = 0x8 ;
		case GXcopy:		/* 0x3 src */
			break ;
		case GXequiv:		/* 0x9 NOT src XOR dst */
			fg = ~fg ;
		case GXxor:		/* 0x6 src XOR dst */
			data_rotate_value = 0x18 ;
			break ;
		case GXandReverse:	/* 0x2 src AND NOT dst */
			invert_existing_data = TRUE ;
			data_rotate_value = 0x8 ;
			break ;
		case GXorReverse:	/* 0xb src OR NOT dst */
			invert_existing_data = TRUE ;
			data_rotate_value = 0x10 ;
			break ;
		case GXnand:		/* 0xe NOT src OR NOT dst */
			invert_existing_data = TRUE ;
		case GXorInverted:	/* 0xd NOT src OR dst */
			fg = ~fg ;
		case GXor:		/* 0x7 src OR dst */
			data_rotate_value = 0x10 ;
			break ;
		case GXcopyInverted:	/* 0xc NOT src */
			fg = ~fg ;
		default:
			break ;
	}
	if ( invert_existing_data )
		egaFillStipple( pStipple, 0xF, GXinvert, planes,
				x, y, w, h, xSrc, ySrc ) ;
}
/* Check If Cursor Is In The Way */
cursor_saved = !egaCursorChecking && egaCheckCursor( x, y, w, h ) ;

/* Setup EGA Registers */
/*
 * Put Display Into SET-RESET
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
/*
 * Set The Color in The Set/Reset Register
 */
outb( 0x3CE, 0 ) ;
outb( 0x3CF, fg ) ; /* Set/Reset Register */
/*
 * Set The Planes in The Enable-Set/Reset Register
 */
outb( 0x3CE, 1 ) ;
outb( 0x3CF, planes ) ;
/*
 * Set The Plane-Enable
 */
outb( 0x3C4, 2 ) ;
outb( 0x3C5, planes ) ; /* Map Mask Register */
/*
 * Set The Ega's Alu Function
 */
outb( 0x3CE, 3 ) ;
outb( 0x3CF, data_rotate_value ) ; /* Data Rotate Register */

/* Point At The Bit Mask Reg */
outb( 0x3CE, 8 ) ;

/* Figure Bit Offsets & Source Address */
mastersrc = (const unsigned char *) pStipple->devPrivate ;
width = pStipple->width ;
paddedByteWidth = ( ( width + 31 ) & ~31 ) >> 3 ;

if ( ( xshift = ( x - xSrc ) % width ) < 0 )
	xshift += width ;

height = pStipple->height ;
if ( ( yshift = ( y - ySrc ) % height ) < 0 )
	yshift += height ;

/* Do Left Edge */
if ( tmp1 = x & 07 ) {
	tmp2 = ( (unsigned) 0xFF ) >> tmp1 ;
	/* Catch The Cases Where The Entire Region Is Within One Byte */
	if ( ( w -= 8 - tmp1 ) < 0 ) {
		tmp2 &= 0xFF << -w ;
		w = 0 ;
	}
	Maskdata = tmp2 ; /* Set The Bit Mask Reg */
	if ( ( NeedValX = xshift - tmp1 ) < 0 )
		NeedValX += width ;
	/*
	 * For Each Line In The Source Pixmap
	 */
	SourceRow = 0, dst = EGABASE + BYTE_OFFSET( x, y ) ;
	if ( h > height )
		for ( ;
		      SourceRow < height ;
		      SourceRow++, dst += BYTES_PER_ROW ) {

			if ( ( tmp1 = ( SourceRow + yshift ) ) >= height )
				tmp1 -= height ;
			outb( 0x3CF, Maskdata 
				   & getbits( NeedValX, width,
					      mastersrc
					      + ( tmp1 * paddedByteWidth ) )
				) ; /* Set The Bit Mask Reg */
			/*
			 * For Each Time Pattern Repeats In The Y Dimension
			 */
			for ( xDst = dst, DestinationRow = /* y + */ SourceRow ;
			      DestinationRow < /* y + */ h ;
			      xDst += height * BYTES_PER_ROW,
			      DestinationRow += height ) {
				/* Read To Save */
				tmp2 = *( (EgaMemoryPtr) xDst) ;
				/* Write Pattern */
				*( (EgaMemoryPtr) xDst ) = Maskdata ;
			}
		}
	else
		for ( ;
		      SourceRow < h ;
		      SourceRow++, dst += BYTES_PER_ROW ) {

			if ( ( tmp1 = ( SourceRow + yshift ) ) >= height )
				tmp1 -= height ;
			outb( 0x3CF, Maskdata
			           & getbits( NeedValX, width,
					      mastersrc
					      + ( tmp1 * paddedByteWidth ) ) ) ;
			/* Read To Save */
			tmp2 = *( (EgaMemoryPtr) dst ) ;
			/* Write Pattern */
			*( (EgaMemoryPtr) dst ) = Maskdata ;
		}
	x = ( x + 7 ) & ~07 ;
	if ( ( NeedValX += 8 ) >= width )
		NeedValX -= width ;
}
else {
	NeedValX = xshift ;
}
/* Fill The Center Of The Box */
Maskdata = 0xFF ; /* Set The Bit Mask Reg */
/*
 * For Each Line In The Source Pixmap
 */
SavNeedX = NeedValX ;

SourceRow = 0, dst = EGABASE + BYTE_OFFSET( x, y ) ;
if ( h > height )
	for ( ;
	      SourceRow < height ;
	      SourceRow++, dst += BYTES_PER_ROW - ROW_OFFSET( w ) ) {
		/*
		 * For Each Byte Across The Pattern In The X Dimension
		 */
		tmp2 = ROW_OFFSET( w ), NeedValX = SavNeedX ;
		for ( ; tmp2-- ; dst++ ) {

			if ( ( tmp1 = ( SourceRow + yshift ) ) >= height )
				tmp1 -= height ;
			outb( 0x3CF, Maskdata
			           & getbits( NeedValX, width,
					      mastersrc
					      + ( tmp1 * paddedByteWidth ) ) ) ;
			/*
			 * For Each Time Pattern Repeats In The Y Dimension
			 */
			for ( xDst = dst, DestinationRow = /* y + */ SourceRow ;
			      DestinationRow < /* y + */ h ;
			      xDst += ( height * BYTES_PER_ROW ),
			      DestinationRow += height ) {
				/* Read To Save */
				tmp1 = *( (EgaMemoryPtr) xDst ) ;
				/* Write Pattern */
				*( (EgaMemoryPtr) xDst ) = Maskdata ;
			}
			if ( ( NeedValX += 8 ) >= width )
				NeedValX -= width ;
		}
	}
else
	for ( ;
	      SourceRow < h ;
	      SourceRow++, dst += BYTES_PER_ROW - ROW_OFFSET( w ) ) {
		/*
		 * For Each Byte Across The Pattern In The X Dimension
		 */
		tmp2 = ROW_OFFSET( w ), NeedValX = SavNeedX ;
		for ( ; tmp2-- ; dst++ ) {
			if ( ( tmp1 = ( SourceRow + yshift ) ) >= height )
				tmp1 -= height ;
				outb( 0x3CF, /* Set The Bit Mask Reg */
			              getbits( NeedValX, width,
					       mastersrc
					       + ( tmp1 * paddedByteWidth ) ) ) ;
			/* Read To Save */
			tmp1 = *( (EgaMemoryPtr) dst ) ;
			/* Write Pattern */
			*( (EgaMemoryPtr) dst ) = Maskdata ;
			if ( ( NeedValX += 8 ) >= width )
				NeedValX -= width ;
		}
	}

/* Do Right Edge */
if ( tmp1 = BIT_OFFSET( w ) ) { /* x Now Is Byte Aligned */
	Maskdata = ( 0xFF << ( 8 - tmp1 ) ) ; /* Set The Bit Mask */
	/*
	 * For Each Line In The Source Pixmap
	 */
	SourceRow = 0, dst = EGABASE + BYTE_OFFSET( ( x + w ), y ) ;
	if ( h > height )
		for ( ;
		      SourceRow < height ;
		      SourceRow++, dst += BYTES_PER_ROW ) {

			if ( ( tmp1 = ( SourceRow + yshift ) ) >= height )
				tmp1 -= height ;
			outb( 0x3CF, /* Set The Bit Mask Reg */	
			      Maskdata
			    & getbits( NeedValX, width,
				       mastersrc
				       + ( tmp1 * paddedByteWidth ) ) ) ;
			/*
			 * For Each Time Pattern Repeats In The Y Dimension
			 */
			for ( xDst = dst, DestinationRow = /* y + */ SourceRow ;
			      DestinationRow < /* y + */ h ;
			      xDst += ( height * BYTES_PER_ROW ),
			      DestinationRow += height ) {
				/* Read To Save */
				tmp1 = *( (EgaMemoryPtr) xDst ) ;
				/* Write Pattern */
				*( (EgaMemoryPtr) xDst ) = Maskdata ;
			}
		}
	else
		for ( ;
		      SourceRow < h ;
		      SourceRow++, dst += BYTES_PER_ROW ) {

			if ( ( tmp1 = ( SourceRow + yshift ) ) >= height )
				tmp1 -= height ;
			outb( 0x3CF, /* Set The Bit Map Reg */
			      Maskdata 
			    & getbits( NeedValX, width,
			      	       mastersrc
				       + ( tmp1 * paddedByteWidth ) ) ) ;
			/* Read To Save */
			tmp1 = *( (EgaMemoryPtr) dst ) ;
			/* Write Pattern */
			*( (EgaMemoryPtr) dst ) = Maskdata ;
		}
}
/* Disable Set/Reset Reg */
outb( 0x3CE, 1 ) ;
outb( 0x3CF, 0 ) ;
/* Replace Cursor If Nessesary */
if ( cursor_saved )
	egaReplaceCursor() ;

return ;
}
