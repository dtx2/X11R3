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
/* $Header: /andrew/X11/R3src/Xbeta/server/ddx/ibm/ega/RCS/egaColorImg.c,v 9.0 88/10/18 12:51:41 erik Exp $ */
/* $Source: /andrew/X11/R3src/Xbeta/server/ddx/ibm/ega/RCS/egaColorImg.c,v $ */

#ifndef lint
static char *rcsid = "$Header: /andrew/X11/R3src/Xbeta/server/ddx/ibm/ega/RCS/egaColorImg.c,v 9.0 88/10/18 12:51:41 erik Exp $";
#endif

#include "egaVideo.h"
#include "X.h"

#undef TRUE
#undef FALSE
#define TRUE 1
#define FALSE 0

extern void egaFillSolid() ;
/* Declared in "egacurs.c" */
extern int egaCursorChecking ;
extern int egaCheckCursor() ;
extern void egaReplaceCursor() ;

void egaDrawColorImage( x, y, w, h, data, alu, planes )
int x, y ;
register int w, h ;
register const unsigned char *data ;
const int alu, planes ;
{
register unsigned long int tmp ;
register const unsigned char *src ;
register volatile unsigned char *dst ;
register int Pixel_Count ;
register unsigned int Mask ;
register unsigned int InitialMask ;
register const unsigned char *DataPtr ;
register volatile unsigned char *StartByte ;
register int Line_Count ;
register int RowIncrement ;
unsigned int invert_source_data = FALSE ;
int cursor_saved ;

{
	unsigned int invert_existing_data = FALSE ;
	unsigned int data_rotate_value = 0 ;

	switch ( alu ) {
		case GXclear:		/* 0x0 Zero 0 */
		case GXinvert:		/* 0xa NOT dst */
		case GXset:		/* 0xf 1 */
			egaFillSolid( 0xF, alu, planes, x, y, w, h ) ;
		case GXnoop:		/* 0x5 dst */
			return ;
		case GXnor:		/* 0x8 NOT src AND NOT dst */
			invert_existing_data = TRUE ;
		case GXandInverted:	/* 0x4 NOT src AND dst */
			invert_source_data = TRUE ;
		case GXand:		/* 0x1 src AND dst */
			data_rotate_value = 0x08 ;
			break ;
		case GXequiv:		/* 0x9 NOT src XOR dst */
			invert_source_data = TRUE ;
		case GXxor:		/* 0x6 src XOR dst */
			data_rotate_value = 0x18 ;
			break ;
		case GXandReverse:	/* 0x2 src AND NOT dst */
			invert_existing_data = TRUE ;
			data_rotate_value = 0x08 ;
			break ;
		case GXnand:		/* 0xe NOT src OR NOT dst */
			invert_source_data = TRUE ;
			invert_existing_data = TRUE ;
		case GXorReverse:	/* 0xb src OR NOT dst */
			data_rotate_value = 0x10 ;
			break ;
		case GXorInverted:	/* 0xd NOT src OR dst */
			invert_source_data = TRUE ;
		case GXor:		/* 0x7 src OR dst */
			data_rotate_value = 0x10 ;
			break ;
		case GXcopyInverted:	/* 0xc NOT src */
			invert_source_data = TRUE ;
		case GXcopy:		/* 0x3 src */
		default:
			break ;
	}
	cursor_saved = !egaCursorChecking && egaCheckCursor( x, y, w, h ) ;
	if ( invert_existing_data )
		egaFillSolid( 0xF, GXinvert, planes, x, y, w, h ) ;
	/* Setup EGA Registers */
	outb( 0x3C4, 2 ) ;
	outb( 0x3C5, planes ) ;			/* Set Write Enable Planes */
	outb( 0x3CE, 3 ) ;
	outb( 0x3CF, data_rotate_value ) ;	/* Set Raster Op */
	outb( 0x3CE, 5 ) ;
	outb( 0x3CF, 2 ) ;			/* Set Write Mode 2 */
	/* Point To Bit Mask Register */
	outb( 0x3CE, 8 ) ;
	
}

StartByte = (volatile unsigned char *)
	( EGABASE + ( y * BYTES_PER_ROW ) + ROW_OFFSET( x ) ) ;
InitialMask = 0x80 >> BIT_OFFSET( x ) ;
RowIncrement = w + 3 & ~3 ;

for ( Line_Count = h, DataPtr = data ;
      Line_Count-- ;
      DataPtr += RowIncrement, StartByte += BYTES_PER_ROW ) {
	for ( dst = StartByte, src = DataPtr,
	      Pixel_Count = w, Mask = InitialMask ;
	      Pixel_Count-- ;
	      src++ ) {
		outb( 0x3CF, Mask ) ;
		/* Read To Load ega Data Latches */
		tmp = *( (EgaMemoryPtr) dst ) ;
		tmp = *src ;
		if ( invert_source_data )
			*( (EgaMemoryPtr) dst ) = ~tmp ;
		else
			*( (EgaMemoryPtr) dst ) = tmp ;
		if ( Mask & 1 ) {
			Mask = 0x80 ;
			dst++ ;
		}
		else
			Mask >>= 1 ;
	}
}
if ( cursor_saved )
	egaReplaceCursor() ;

return ;
}

static unsigned long int read8Z( screen_ptr )
register volatile char *screen_ptr ;
{
register unsigned int i ;
register unsigned int j ;

/* Read One Byte At A Time to get
 *	i ==	[ Plane 3 ] [ Plane 2 ] [ Plane 1 ] [ Plane 0 ]
 * into a single register
 */
outb( 0x3CF, 3 ) ;
i = *( (EgaMemoryPtr) screen_ptr ) << 8 ;
outb( 0x3CF, 2 ) ;
i |= *( (EgaMemoryPtr) screen_ptr ) ;
i <<= 8 ;
outb( 0x3CF, 1 ) ;
i |= *( (EgaMemoryPtr) screen_ptr ) ;
i <<= 8 ;
outb( 0x3CF, 0 ) ;
i |= *( (EgaMemoryPtr) screen_ptr ) ;

/* Push Bits To Get
 * j ==	[Pixel 0][Pixel 1][Pixel 2][Pixel 3][Pixel 4][Pixel 5][Pixel 6][Pixel 7]
 * into one register
 */
j = ( i & 0x1 ) << 4 ;
i >>= 1 ;
j |= i & 0x1 ;
j <<= 4 ;
i >>= 1 ;
j |= i & 0x1 ;
j <<= 4 ;
i >>= 1 ;
j |= i & 0x1 ;
j <<= 4 ;
i >>= 1 ;
j |= i & 0x1 ;
j <<= 4 ;
i >>= 1 ;
j |= i & 0x1 ;
j <<= 4 ;
i >>= 1 ;
j |= i & 0x1 ;
j <<= 4 ;
i >>= 1 ;
j |= i & 0x1 ;

j |= i & 0x2 ;
i >>= 1 ;
j |= ( i & 0x2 ) << 4 ;
i >>= 1 ;
j |= ( i & 0x2 ) << 8 ;
i >>= 1 ;
j |= ( i & 0x2 ) << 12 ;
i >>= 1 ;
j |= ( i & 0x2 ) << 16 ;
i >>= 1 ;
j |= ( i & 0x2 ) << 20 ;
i >>= 1 ;
j |= ( i & 0x2 ) << 24 ;
i >>= 1 ;
j |= ( i & 0x2 ) << 28 ;

j |= i & 0x4 ;
i >>= 1 ;
j |= ( i & 0x4 ) << 4 ;
i >>= 1 ;
j |= ( i & 0x4 ) << 8 ;
i >>= 1 ;
j |= ( i & 0x4 ) << 12 ;
i >>= 1 ;
j |= ( i & 0x4 ) << 16 ;
i >>= 1 ;
j |= ( i & 0x4 ) << 20 ;
i >>= 1 ;
j |= ( i & 0x4 ) << 24 ;
i >>= 1 ;
j |= ( i & 0x4 ) << 28 ;

j |= i & 0x8 ;
i >>= 1 ;
j |= ( i & 0x8 ) << 4 ;
i >>= 1 ;
j |= ( i & 0x8 ) << 8 ;
i >>= 1 ;
j |= ( i & 0x8 ) << 12 ;
i >>= 1 ;
j |= ( i & 0x8 ) << 16 ;
i >>= 1 ;
j |= ( i & 0x8 ) << 20 ;
i >>= 1 ;
j |= ( i & 0x8 ) << 24 ;
i >>= 1 ;
j |= ( i & 0x8 ) << 28 ;

return j ;
}

void egaReadColorImage( x, y, lx, ly, data )
int x, y ;
register int lx, ly ;
register unsigned char *data ;
{
register volatile unsigned char *src ;
register volatile unsigned char *s1ptr ;
register unsigned long int tmp ;
register int dx ;
register int lcnt ;
register int skip ;
register int center_width ;
register int ignore ;
int pad ;
int cursor_saved ;

cursor_saved = !egaCursorChecking && egaCheckCursor( x, y, lx, ly ) ;

/* Setup EGA Registers */
outb( 0x3CE, 5 ) ;
outb( 0x3CF, 0 ) ;
outb( 0x3CE, 4 ) ;

skip = BIT_OFFSET( x ) ;
ignore = BIT_OFFSET( x + lx ) ;
pad = ( skip - ignore ) & 0x3 ;
src = (volatile unsigned char *) EGABASE + BYTE_OFFSET( x, y ) ;
center_width = ROW_OFFSET( x + lx ) - ROW_OFFSET( ( x + 0x7 ) & ~0x7 ) ;

if ( center_width < 0 )
	for ( ;
	      ly-- ;
	      src += BYTES_PER_ROW ) {
		for ( dx = skip, tmp = read8Z( src ) ;
		      dx < ignore ;
		      dx++ )
			*data++ = 0xF & ( tmp >> ( 32 - ( dx << 2 ) ) ) ;
		data += pad ;
	}
else
	for ( ;
	      ly-- ;
	      src += BYTES_PER_ROW ) {
		s1ptr = src ;
		if ( dx = skip )
			for ( tmp = read8Z( s1ptr++ ) ; dx < 8 ; dx++ )
				*data++ = 0xF & ( tmp >> ( 32 - ( dx << 2 ) ) ) ;
		for ( lcnt = center_width ; lcnt-- ; )
			for ( tmp = read8Z( s1ptr++ ), dx = 0 ; dx < 8 ; dx++ )
				*data++ = ( tmp >> ( 32 - ( dx << 2 ) ) ) & 0xF ;
		if ( ignore )
			for ( tmp = read8Z( s1ptr ), dx = 0 ; dx < ignore ; dx++ )
				*data++ = 0xF & ( tmp >> ( 32 - ( dx << 2 ) ) ) ;
		data += pad ;
	}

if ( cursor_saved )
	egaReplaceCursor() ;

return ;
}
