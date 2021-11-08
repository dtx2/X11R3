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
/* $Header: /andrew/X11/R3src/Xbeta/server/ddx/ibm/ega/RCS/egaGlyph.c,v 9.0 88/10/18 12:52:02 erik Exp $ */
/* $Source: /andrew/X11/R3src/Xbeta/server/ddx/ibm/ega/RCS/egaGlyph.c,v $ */

#ifndef lint
static char *rcsid = "$Header: /andrew/X11/R3src/Xbeta/server/ddx/ibm/ega/RCS/egaGlyph.c,v 9.0 88/10/18 12:52:02 erik Exp $";
static char sccsid[] = "@(#)egaGlyph.c	3.1 88/09/22 09:32:46";
#endif

/*
 * Software Cursor implementation for ega
 * Paquin 7/87
 * Modifed For ega -- Paul Shupak 10/87
 *
 * Depends on owning some off-screen memory in SAVELOC, etc #defined areas
 */

#include "X.h"
#include "misc.h"	/* Need typedef of "Bool" */
#include "egaProcs.h"
#include "egaVideo.h"

#include "ibmIOArch.h"

/* Declared in "egacurs.c" */
extern int egaCursorChecking ;
extern int egaCheckCursor() ;
extern void egaReplaceCursor() ;

#undef TRUE
#undef FALSE
#define TRUE 1
#define FALSE 0

void 
egaDrawGlyph( data, x, y, w, h, fg, alu, planes )
register const unsigned char *data ;
register int x ;
int y ;
register int w ;
int h ;
unsigned long int fg ;
const int alu ;
const unsigned long int planes ;

{
register const unsigned char *cptr ;
register int Maskdata;
register int shift ;
register unsigned SourceRow ;
register int tmp1 ;
register int tmp2 ;
register volatile unsigned char *dst ;
register const unsigned char *lptr ;
register unsigned int paddedByteWidth ;
int cursor_saved ;
int data_rotate_value = 0x0 ;

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
		egaDrawGlyph( data, x, y, w, h, ALLPLANES, GXinvert, planes ) ;
}

/* Check If Cursor Is In The Way */
cursor_saved = !egaCursorChecking && egaCheckCursor( x, y, w, h ) ;

/* Figure Bit Offsets & Source Address */
/* Pad To Byte Boundary */
paddedByteWidth = ( ( w + 07 ) & ~07 ) >> 3 ;

/* Setup EGA Registers */
/*
 * Set The Color in The Set/Reset Register
 */
outb( 0x3CE, 0 ) ;
outb( 0x3CF, fg ) ; /* Set/Reset Register */
outb( 0x3CE, 1 ) ;
outb( 0x3CF, planes ) ; /* Enable Set/Reset Register */
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
/*
 * Point At The Bit Mask Reg
 */
outb( 0x3CE, 8 ) ;

/* Do Left Edge */
if ( shift = x & 07 ) { /* shift is amount to shift right */
	/*
	 * For Each Line In The Cursor Data
	 */
	dst = EGABASE + BYTE_OFFSET( x, y ) ;

	/* Set-Up The Bit Mask */
	tmp2 = ( (unsigned) 0xFF ) >> shift ;
	/* Catch The Cases Where The Entire Region Is Within One Byte */
	if ( ( w -= 8 - shift ) < 0 ) {
		tmp2 &= 0xFF << -w ;
		w = 0 ;
	}
	Maskdata = tmp2 ; /* Set The Bit Mask Reg */
	for ( SourceRow = h,
	      cptr = data ;
	      SourceRow-- ;
	      dst += BYTES_PER_ROW, cptr += paddedByteWidth ) {
		/* Read To Save */
		tmp2 = *( (EgaMemoryPtr) dst ) ;
		/* Write Pattern */
                outb( 0x3CF, Maskdata & (*cptr >> shift ) ) ;
		*( (EgaMemoryPtr) dst ) = Maskdata ;
	}
	if ( w )
		x = ( x + 7 ) & ~07 ; /* Byte Align "x" */
	else {
		if ( cursor_saved )
			egaReplaceCursor() ;
		return ;
	}
}

/* Fill The Center Of The Box */
Maskdata = 0xFF ; /* Set The Bit Mask Reg */
/*
 * For Each Line In The Source Pixmap
 */
if ( ROW_OFFSET( w ) )
	for ( SourceRow = h,
	      dst = EGABASE + BYTE_OFFSET( x, y ),
	      lptr = data ;
	      SourceRow-- ;
	      dst += BYTES_PER_ROW - ROW_OFFSET( w ),
	      lptr += paddedByteWidth ) {
		/*
		 * For Each Byte Across The Pattern In The X Dimension
		 */
		tmp2 = ROW_OFFSET( w ) ;
		cptr = lptr ;
		if ( shift ) {
			register unsigned long int savebits ;

			for ( savebits = ( *cptr++ << ( 8 - shift ) ) ;
			      tmp2-- ;
			      cptr++, dst++ ) {
				/* Read To Save */
				tmp1 = *( (EgaMemoryPtr) dst ) ;
				tmp1 = *cptr ;
				/* Write Pattern */
                                outb( 0x3CF, /* Set the Bit Mask Reg */  
					savebits | ( tmp1 >> shift ) ) ;
				*( (EgaMemoryPtr) dst ) = Maskdata ;
				savebits = ( tmp1 << ( 8 - shift ) ) ;
				/*
				*( (EgaMemoryPtr) dst ) =
					( *cptr << ( 8 - shift ) )
				      | ( cptr[1] >> shift ) ;
				*/
			}
		}
		else
			for ( ;
			      tmp2-- ;
			      cptr++, dst++ ) {
				/* Read To Save */
				tmp1 = *( (EgaMemoryPtr) dst ) ;
				/* Write Pattern */
                                outb( 0x3CF, *cptr ) ;
				*( (EgaMemoryPtr) dst ) = Maskdata ;
			}
		}

/* Do Right Edge */
if ( tmp1 = BIT_OFFSET( w ) ) { /* x Now Is Byte Aligned */
	Maskdata = ( 0xFF << ( 8 - tmp1 ) ) ; /* Set The Bit Mask */
	/*
	 * For Each Line In The Source Pixmap
	 */
	cptr = data + ROW_OFFSET( w ) ;
	SourceRow = h ;
	dst = EGABASE + BYTE_OFFSET( ( x + w ), y ) ;
	if ( shift )
		for ( ;
		      SourceRow-- ;
		      dst += BYTES_PER_ROW, cptr += paddedByteWidth ) {
			/* Read To Save */
			tmp1 = *( (EgaMemoryPtr) dst ) ;
			/* Write Pattern */
                        outb( 0x3CF, /* Set The Bit Mask Reg */
		  Maskdata & ( ( *cptr << ( 8 - shift ) ) | ( cptr[1] >> shift ) ) ) ;
			*( (EgaMemoryPtr) dst ) = Maskdata;
		}
	else /* shift == 0 */
		for ( ;
		      SourceRow-- ;
		      dst += BYTES_PER_ROW, cptr += paddedByteWidth ) {
			/* Read To Save */
			tmp1 = *( (EgaMemoryPtr) dst ) ;
			/* Write Pattern */
                        outb( 0x3CF, Maskdata & *cptr ) ; /* Set The Bit Mask Reg */
			*( (EgaMemoryPtr) dst ) = Maskdata ;
		}
}

/* Disable Set/Reset Register */
outb( 0x3CE, 1);
outb( 0x3CF, 0);

if ( cursor_saved )
	egaReplaceCursor() ;

return ;
}
