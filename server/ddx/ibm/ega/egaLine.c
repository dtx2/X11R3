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
/*
 *
 *  Hardware interface routines for IBM EGA adapter for
 *  X.11 server(s) on IBM equipment.
 *
 *  Last Modified By
 *  P. Shupak -- Oct., 1987
 *
 */

/* $Header: /andrew/X11/R3src/Xbeta/server/ddx/ibm/ega/RCS/egaLine.c,v 9.0 88/10/18 12:52:14 erik Exp $ */
/* $Source: /andrew/X11/R3src/Xbeta/server/ddx/ibm/ega/RCS/egaLine.c,v $ */

#ifndef lint
static char *rcsid = "$Header: /andrew/X11/R3src/Xbeta/server/ddx/ibm/ega/RCS/egaLine.c,v 9.0 88/10/18 12:52:14 erik Exp $" ;
static char sccsid[] = "@(#)egaline.c	3.1 88/09/22 09:33:15";
#endif

#include "X.h"
#include "egaVideo.h"

#include "ibmIOArch.h"

extern void egaFillSolid( ) ;

void egaHorzLine( color, alu, mask, x, y, w )
unsigned long int color ;
int alu ;
unsigned long int mask ;
int x, y, w ;
{
egaFillSolid( color, alu, mask, x, y, w, 1 ) ;
return ;
}

void egaVertLine( color, alu, mask, x, y, h )
unsigned long int color ;
int alu ;
unsigned long int mask ;
int x, y, h ;
{
egaFillSolid( color, alu, mask, x, y, 1, h ) ;
return ;
}

/* X increasing */
static void fast_x_line_right( et, e1, e2, len, y_direction, ptx, pty )
register int et ;
register const int e1 ;
register const int e2 ;
register unsigned int len ;
const int y_direction ;
const int ptx ;
const int pty ;
{
register volatile tmp ;
register unsigned volatile char *addr ;
register unsigned const long int y_increment =
		y_direction > 0 ? BYTES_PER_ROW : - BYTES_PER_ROW ;
register unsigned char mask ;
register unsigned char single_bit ;

#define SINGLE_STEP \
if ( et < 0 ) { \
	mask |= single_bit ; \
	et += e1 ; \
} \
else { \
	tmp = *( (EgaMemoryPtr) addr ) ; \
	et += e2 ; \
	outb( 0x3CF, mask ) ; \
	*( (EgaMemoryPtr) addr ) = mask ; \
	mask = single_bit ; \
	addr += y_increment ; \
}

/* Set-Up And Set Initial Mask Value */
addr = EGABASE + BYTE_OFFSET( ptx, pty ) ;
mask = single_bit = ( 0x80 >> ( tmp = BIT_OFFSET( ptx ) ) ) ;
tmp = ( tmp + 1 ) & 7 ;
if ( !( single_bit >>= 1 ) ) {
	tmp = *( (EgaMemoryPtr) addr ) ;
	single_bit = 0x80 ;
	outb( 0x3CF, mask ) ;
	*( (EgaMemoryPtr) addr++ ) = mask ;
	tmp = mask = 0 ;
}
if ( ( 8 - tmp ) > --len ) {
	tmp = len ;
	len = 8 ;
}
else if ( tmp ) {
	len += tmp ;	/* Round-Up */
	tmp = 8 - tmp ;
}

branchPoint:
switch ( tmp ) {
	Loop_Top:
	/* */
	/* First -- Leftmost -- Most-Significant Bit In Byte */
	case 0 : SINGLE_STEP ; single_bit >>= 1 ;
	case 7 : SINGLE_STEP ; single_bit >>= 1 ;
	case 6 : SINGLE_STEP ; single_bit >>= 1 ;
	case 5 : SINGLE_STEP ; single_bit >>= 1 ;
	case 4 : SINGLE_STEP ; single_bit >>= 1 ;
	case 3 : SINGLE_STEP ; single_bit >>= 1 ;
	case 2 : SINGLE_STEP ; single_bit >>= 1 ;
	/* Last -- Rightmost -- Least-Significant Bit In Byte */
	case 1 : SINGLE_STEP ;
	/* Flush buffered mask */
	tmp = *( (EgaMemoryPtr) addr ) ;
	len -= 8 ;
	outb( 0x3CF, mask ) ;
	*( (EgaMemoryPtr) addr ) = mask ;
	if ( len > 7 ) {
		mask = 0 ;
		addr++ ;
		single_bit = 0x80 ;
		goto Loop_Top ;
	}
	else if ( len ) {
		tmp = len ;
		mask = 0 ;
		addr++ ;
		len = 8 ;
		single_bit = 0x80 ;
		goto branchPoint ;
	}
	else
		return ;
}
#undef SINGLE_STEP
/*NOTREACHED*/
}

/* X decreasing */
static void fast_x_line_left( et, e1, e2, len, y_direction, ptx, pty )
register int et ;
register const int e1 ;
register const int e2 ;
register unsigned int len ;
const int y_direction ;
const int ptx ;
const int pty ;
{
register volatile tmp ;
register unsigned volatile char *addr ;
register unsigned const long int y_increment =
		y_direction > 0 ? BYTES_PER_ROW : - BYTES_PER_ROW ;
register unsigned char mask ;
register unsigned single_bit ;

#define SINGLE_STEP \
if ( et < 0 ) { \
	mask |= single_bit ; \
	et += e1 ; \
} \
else { \
	tmp = *( (EgaMemoryPtr) addr ) ; \
	et += e2 ; \
	outb( 0x3CF, mask ) ; \
	*( (EgaMemoryPtr) addr ) = mask ; \
	mask = single_bit ; \
	addr += y_increment ; \
}

/* Set-Up And Set Initial Mask Value */
addr = EGABASE + BYTE_OFFSET( ptx, pty ) ;
mask = single_bit = ( 0x01 << ( tmp = 7 - BIT_OFFSET( ptx ) ) ) ;
tmp = ( tmp + 1 ) & 7 ;
if ( ( single_bit <<= 1 ) == 0x100 ) {
	tmp = *( (EgaMemoryPtr) addr ) ;
	single_bit = 0x01 ;
	outb( 0x3CF, mask ) ;
	*( (EgaMemoryPtr) addr-- ) = mask ;
	tmp = mask = 0 ;
}
if ( ( 8 - tmp ) > --len ) {
	tmp = len ;
	len = 8 ;
}
else if ( tmp ) {
	len += tmp ;	/* Round-Up */
	tmp = 8 - tmp ;
}

branchPoint:
switch ( tmp ) {
	Loop_Top:
	/* */
	/* First -- Rightmost -- Least-Significant Bit In Byte */
	case 0 : SINGLE_STEP ; single_bit <<= 1 ;
	case 7 : SINGLE_STEP ; single_bit <<= 1 ;
	case 6 : SINGLE_STEP ; single_bit <<= 1 ;
	case 5 : SINGLE_STEP ; single_bit <<= 1 ;
	case 4 : SINGLE_STEP ; single_bit <<= 1 ;
	case 3 : SINGLE_STEP ; single_bit <<= 1 ;
	case 2 : SINGLE_STEP ; single_bit <<= 1 ;
	/* Last -- Leftmost -- Most-Significant Bit In Byte */
	case 1 : SINGLE_STEP ;
	/* Flush buffered mask */
	tmp = *( (EgaMemoryPtr) addr ) ;
	len -= 8 ;
	outb( 0x3CF, mask ) ;
	*( (EgaMemoryPtr) addr ) = mask ;
	if ( len > 7 ) {
		mask = 0 ;
		addr-- ;
		single_bit = 0x01 ;
		goto Loop_Top ;
	}
	else if ( len ) {
		tmp = len ;
		mask = 0 ;
		addr-- ;
		len = 8 ;
		single_bit = 0x01 ;
		goto branchPoint ;
	}
	else
		return ;
}
#undef SINGLE_STEP
/*NOTREACHED*/
}

/* X increasing */
static void fast_y_line_right( et, e1, e2, len, y_direction, ptx, pty )
register int et ;
register const int e1 ;
register const int e2 ;
register unsigned int len ;
const int y_direction ;
const int ptx ;
const int pty ;
{
register volatile tmp ;
register unsigned single_bit ;
register unsigned volatile char *addr ;
register unsigned const long int y_increment =
		y_direction > 0 ? BYTES_PER_ROW : - BYTES_PER_ROW ;

#define SINGLE_STEP \
addr += y_increment ; \
if ( et < 0 ) { \
	tmp = *( (EgaMemoryPtr) addr ) ; \
	et += e1 ; \
} \
else { \
	et += e2 ; \
	if ( single_bit >>= 1 ) { \
		tmp = *( (EgaMemoryPtr) addr ) ; \
	} \
	else { \
		tmp = *( (EgaMemoryPtr) ( ++addr ) ) ; \
		single_bit = 0x80 ; \
	} \
} \
outb( 0x3CF, single_bit ) ; \
*( (EgaMemoryPtr) addr ) = single_bit
/* END MACRO */

/* Draw First Dot */
tmp = *( (EgaMemoryPtr) ( addr = EGABASE + BYTE_OFFSET( ptx, pty ) ) ) ;
outb( 0x3CF,  single_bit = ( 0x80 >> BIT_OFFSET( ptx ) ) ) ;
*( (EgaMemoryPtr) addr ) = single_bit ;

if ( tmp = --len & 0xF ) /* Already Drew First Dot */
	len += 0xF ;
len >>= 0x4 ;

switch ( tmp ) {
	Loop_Top:
	/* */
	case 0x0 : SINGLE_STEP ;
	case 0xF : SINGLE_STEP ;
	case 0xE : SINGLE_STEP ;
	case 0xD : SINGLE_STEP ;
	case 0xC : SINGLE_STEP ;
	case 0xB : SINGLE_STEP ;
	case 0xA : SINGLE_STEP ;
	case 0x9 : SINGLE_STEP ;
	case 0x8 : SINGLE_STEP ;
	case 0x7 : SINGLE_STEP ;
	case 0x6 : SINGLE_STEP ;
	case 0x5 : SINGLE_STEP ;
	case 0x4 : SINGLE_STEP ;
	case 0x3 : SINGLE_STEP ;
	case 0x2 : SINGLE_STEP ;
	case 0x1 : SINGLE_STEP ;
	/* */
	if ( --len )
		goto Loop_Top ;
	else
		return ;
}
#undef SINGLE_STEP

/*NOTREACHED*/
}

/* X decreasing */
static void fast_y_line_left( et, e1, e2, len, y_direction, ptx, pty )
register int et ;
register const int e1 ;
register const int e2 ;
register unsigned int len ;
const int y_direction ;
const int ptx ;
const int pty ;
{
register volatile tmp ;
register unsigned single_bit ;
register const unsigned stop_bit = 0x100 ;
register unsigned volatile char *addr ;
register unsigned const long int y_increment =
		y_direction > 0 ? BYTES_PER_ROW : - BYTES_PER_ROW ;

#define SINGLE_STEP \
addr += y_increment ; \
if ( et < 0 ) { \
	tmp = *( (EgaMemoryPtr) addr ) ; \
	et += e1 ; \
} \
else { \
	et += e2 ; \
	if ( ( single_bit <<= 1 ) != stop_bit ) { \
		tmp = *( (EgaMemoryPtr) addr ) ; \
	} \
	else { \
		tmp = *( (EgaMemoryPtr) ( --addr ) ) ; \
		single_bit = 0x01 ; \
	} \
} \
outb( 0x3CF, single_bit ) ; \
*( (EgaMemoryPtr) addr ) = single_bit
/* END MACRO */

/* Draw First Dot */
tmp = *( (EgaMemoryPtr) ( addr = EGABASE + BYTE_OFFSET( ptx, pty ) ) ) ;
outb( 0x3CF,  single_bit = ( 0x01 << ( 7 - BIT_OFFSET( ptx ) ) ) ) ;
*( (EgaMemoryPtr) addr ) = single_bit;

if ( tmp = --len & 0xF ) /* Already Drew First Dot */
	len += 0xF ;
len >>= 0x4 ;

switch ( tmp ) {
	Loop_Top:
	/* */
	case 0x0 : SINGLE_STEP ;
	case 0xF : SINGLE_STEP ;
	case 0xE : SINGLE_STEP ;
	case 0xD : SINGLE_STEP ;
	case 0xC : SINGLE_STEP ;
	case 0xB : SINGLE_STEP ;
	case 0xA : SINGLE_STEP ;
	case 0x9 : SINGLE_STEP ;
	case 0x8 : SINGLE_STEP ;
	case 0x7 : SINGLE_STEP ;
	case 0x6 : SINGLE_STEP ;
	case 0x5 : SINGLE_STEP ;
	case 0x4 : SINGLE_STEP ;
	case 0x3 : SINGLE_STEP ;
	case 0x2 : SINGLE_STEP ;
	case 0x1 : SINGLE_STEP ;
	/* */
	if ( --len )
		goto Loop_Top ;
	else
		return ;
}
#undef SINGLE_STEP

/*NOTREACHED*/
}

#define X_AXIS 0
#define Y_AXIS 1

void egaBresLine( color, alu, planes, signdx, signdy,
		  axis, x, y, et, e1, e2, len )
unsigned long int color ;
int alu ;
unsigned long int planes ;
register int signdx, signdy ;
int axis, x, y ;
register int et, e1, e2, len ;
{
unsigned int invert_existing_data = 0 ;
unsigned int data_rotate_value = 0 ;
int cursor_saved ;

extern int egaCursorChecking ;

if ( len <= 0 )
	return ;
else if ( len == 1 ) {
	egaFillSolid( color, alu, planes, x, y, 1, 1 ) ;
	return ;
}

switch ( alu ) {
	case GXclear:		/* 0x0 Zero 0 */
		color = 0 ;
		break ;
	case GXnor:		/* 0x8 NOT src AND NOT dst */
		invert_existing_data = 1 ;
	case GXandInverted:	/* 0x4 NOT src AND dst */
		color = ~color ;
	case GXand:		/* 0x1 src AND dst */
		data_rotate_value = 0x08 ;
	case GXcopy:		/* 0x3 src */
		break ;
	case GXnoop:		/* 0x5 dst */
		return ;
	case GXequiv:		/* 0x9 NOT src XOR dst */
		color = ~color ;
	case GXxor:		/* 0x6 src XOR dst */
		data_rotate_value = 0x18 ;
		break ;
	case GXandReverse:	/* 0x2 src AND NOT dst */
		invert_existing_data = 1 ;
		data_rotate_value = 0x08 ;
		break ;
	case GXorReverse:	/* 0xb src OR NOT dst */
		invert_existing_data = 1 ;
		data_rotate_value = 0x10 ;
		break ;
	case GXnand:		/* 0xe NOT src OR NOT dst */
		invert_existing_data = 1 ;
	case GXorInverted:	/* 0xd NOT src OR dst */
		color = ~color ;
	case GXor:		/* 0x7 src OR dst */
		data_rotate_value = 0x10 ;
		break ;
	case GXcopyInverted:	/* 0xc NOT src */
		color = ~color ;
		break ;
	case GXinvert:		/* 0xa NOT dst */
		data_rotate_value = 0x18 ;
	case GXset:		/* 0xf 1 */
		color = 0xF ;
	default:
		break ;

}
/* Remove Cursor If In The Way */
cursor_saved = !egaCursorChecking
	    && egaCheckCursor( ( ( signdx == -1 ) ? ( x - len ) : x ),
			       ( ( signdy == -1 ) ? ( y - len ) : y ),
			       len, len ) ;
++egaCursorChecking ;

if ( invert_existing_data ) /* Recurse */
	egaBresLine( ALLPLANES, GXxor, planes, signdx, signdy,
		     axis, x, y, et, e1, e2, len ) ;

/* Set-Up ega registers */
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
outb( 0x3CF, color ) ; /* Set/Reset Register */
/*
 * Set The Planes in The Enable-Set/Reset Register
 */
outb( 0x3CE, 1 ) ;
outb( 0x3CF, planes ) ; /* Set/Reset Register */
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
outb( 0x3CF, 0xFF ) ;

/* Do It Here */
/* ********** */
( ( signdx > 0 ) ? ( axis ? fast_y_line_right : fast_x_line_right )
		 : ( axis ? fast_y_line_left  : fast_x_line_left ) )
			( et, e1, e2, len, signdy, x, y ) ;

/* ********** */
/* Disable Set/Reset Register */
outb( 0x3CE, 1 ) ;
outb( 0x3CF, 0 ) ;
if ( !--egaCursorChecking && cursor_saved )
	egaReplaceCursor() ;

return ;
}
