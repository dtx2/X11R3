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
/* $Header: /andrew/X11/R3src/Xbeta/server/ddx/ibm/ega/RCS/egaSolid.c,v 9.0 88/10/18 12:52:51 erik Exp $ */
/* $Source: /andrew/X11/R3src/Xbeta/server/ddx/ibm/ega/RCS/egaSolid.c,v $ */

#ifndef lint
static char *rcsid = "$Header: /andrew/X11/R3src/Xbeta/server/ddx/ibm/ega/RCS/egaSolid.c,v 9.0 88/10/18 12:52:51 erik Exp $";
static char sccsid[] = "@(#)egaSolid.c	3.1 88/09/22 09:32:53";
#endif

#include "X.h"
#include "egaVideo.h"

#include "ibmIOArch.h"

#undef TRUE
#undef FALSE
#define TRUE 1
#define FALSE 0

/* Declared in "egacurs.c" */
extern int egaCursorChecking ;
extern int egaCheckCursor() ;
extern void egaReplaceCursor() ;

static void fastFill( destination, bytewidth, height )
register volatile char *destination ;
register const unsigned int bytewidth ;	/* MUST BE > 0 !! */
register unsigned int height ;		/* MUST BE > 0 !! */
{
register volatile char *stop_address = destination + bytewidth ;
register int row_jump = BYTES_PER_ROW - bytewidth ;
register const int notZero = ~0x0 ;

#define SINGLE_STORE \
    ( *( (EgaMemoryPtr) ( destination++ ) ) = notZero )

/* TOP OF FIRST LOOP */
BranchPoint:

switch ( bytewidth & 0xF ) {
	LoopTop :
	case 0x0 : SINGLE_STORE ;
	case 0xF : SINGLE_STORE ;
	case 0xE : SINGLE_STORE ;
	case 0xD : SINGLE_STORE ;
	case 0xC : SINGLE_STORE ;
	case 0xB : SINGLE_STORE ;
	case 0xA : SINGLE_STORE ;
	case 0x9 : SINGLE_STORE ;
	case 0x8 : SINGLE_STORE ;
	case 0x7 : SINGLE_STORE ;
	case 0x6 : SINGLE_STORE ;
	case 0x5 : SINGLE_STORE ;
	case 0x4 : SINGLE_STORE ;
	case 0x3 : SINGLE_STORE ;
	case 0x2 : SINGLE_STORE ;
	case 0x1 : SINGLE_STORE ;
/* FIRST LOOP */
		if ( destination != stop_address )
			goto LoopTop ;
/* SECOND LOOP */
		if ( --height ) {
			destination += row_jump ;
			stop_address += BYTES_PER_ROW ;
			goto BranchPoint ;
		}
		else
			return ;
#undef SINGLE_STORE
}
/*NOTREACHED*/
}

/* For Read-Modify-Write Case */
static void fastFillRMW( destination, bytewidth, height )
register volatile char *destination ;
register const unsigned int bytewidth ;	/* MUST BE > 0 !! */
register unsigned int height ;		/* MUST BE > 0 !! */
{
register volatile char *stop_address = destination + bytewidth ;
register int row_jump = BYTES_PER_ROW - bytewidth ;
#ifndef OLDHC
register const int notZero = ~0x0 ;
#else
#define notZero ( ~0 )
#endif
register volatile int tmp ;

#define SINGLE_STORE \
    tmp = *( (EgaMemoryPtr) destination ) ; \
    ( *( (EgaMemoryPtr) ( destination++ ) ) = notZero )

/* TOP OF FIRST LOOP */
BranchPoint:

switch ( bytewidth & 0xF ) {
	LoopTop :
	case 0x0 : SINGLE_STORE ;
	case 0xF : SINGLE_STORE ;
	case 0xE : SINGLE_STORE ;
	case 0xD : SINGLE_STORE ;
	case 0xC : SINGLE_STORE ;
	case 0xB : SINGLE_STORE ;
	case 0xA : SINGLE_STORE ;
	case 0x9 : SINGLE_STORE ;
	case 0x8 : SINGLE_STORE ;
	case 0x7 : SINGLE_STORE ;
	case 0x6 : SINGLE_STORE ;
	case 0x5 : SINGLE_STORE ;
	case 0x4 : SINGLE_STORE ;
	case 0x3 : SINGLE_STORE ;
	case 0x2 : SINGLE_STORE ;
	case 0x1 : SINGLE_STORE ;
/* FIRST LOOP */
		if ( destination != stop_address )
			goto LoopTop ;
/* SECOND LOOP */
		if ( --height ) {
			destination += row_jump ;
			stop_address += BYTES_PER_ROW ;
			goto BranchPoint ;
		}
		else
			return ;
}
#undef SINGLE_STORE
/*NOTREACHED*/
}

void egaFillSolid( color, alu, planes, x0, y0, lx, ly )
unsigned long int color ;
const int alu ;
const unsigned long int planes ;
register int x0 ;
register const int y0 ;
register int lx ;
register const int ly ;		/* MUST BE > 0 !! */
{
register volatile unsigned char *dst ;
register tmp ;
register tmp2 ;
register tmp3 ;
unsigned int data_rotate_value = 0 ;
unsigned int read_write_modify = FALSE ;
unsigned int invert_existing_data = FALSE ;
int cursor_saved ;

switch ( alu ) {
	case GXclear:		/* 0x0 Zero 0 */
		color = 0 ;
		break ;
	case GXnor:		/* 0x8 NOT src AND NOT dst */
		invert_existing_data = TRUE ;
	case GXandInverted:	/* 0x4 NOT src AND dst */
		color = ~color ;
	case GXand:		/* 0x1 src AND dst */
		data_rotate_value = 0x08 ;
		read_write_modify = TRUE ;
	case GXcopy:		/* 0x3 src */
		break ;
	case GXnoop:		/* 0x5 dst */
		return ;
	case GXequiv:		/* 0x9 NOT src XOR dst */
		color = ~color ;
	case GXxor:		/* 0x6 src XOR dst */
		data_rotate_value = 0x18 ;
		read_write_modify = TRUE ;
		break ;
	case GXandReverse:	/* 0x2 src AND NOT dst */
		invert_existing_data = TRUE ;
		data_rotate_value = 0x08 ;
		read_write_modify = TRUE ;
		break ;
	case GXorReverse:	/* 0xb src OR NOT dst */
		invert_existing_data = TRUE ;
		data_rotate_value = 0x10 ;
		read_write_modify = TRUE ;
		break ;
	case GXnand:		/* 0xe NOT src OR NOT dst */
		invert_existing_data = TRUE ;
	case GXorInverted:	/* 0xd NOT src OR dst */
		color = ~color ;
	case GXor:		/* 0x7 src OR dst */
		data_rotate_value = 0x10 ;
		read_write_modify = TRUE ;
		break ;
	case GXcopyInverted:	/* 0xc NOT src */
		color = ~color ;
		break ;
	case GXinvert:		/* 0xa NOT dst */
		data_rotate_value = 0x18 ;
		read_write_modify = TRUE ;
	case GXset:		/* 0xf 1 */
		color = 0xF ;
	default:
		break ;
}

/* Remove Cursor If In The Way */
cursor_saved = !egaCursorChecking && egaCheckCursor( x0, y0, lx, ly ) ;

/*
 * Set The Color in The Set/Reset Register
 */
outb( 0x3CE, 0 ) ;
outb( 0x3CF, color ) ; /* Set/Reset Register */
/*
 * Set The Plane-Enable
 */
outb( 0x3C4, 2 ) ;
outb( 0x3C5, planes ) ; /* Map Mask Register */
outb( 0x3CE, 1 ) ;
outb( 0x3CF, planes ) ; /* Enable Set/Reset Register */
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
outb( 0x3CF, 0x0 ) ;
/*
 * Set The Function-Select In The Data Rotate Register
 */
outb( 0x3CE, 3 ) ;
outb( 0x3CF, data_rotate_value ) ;

outb( 0x3CE, 8 ) ; /* Point At The Bit Mask Reg */
/* Do Left Edge */
if ( tmp = x0 & 07 ) {
	tmp2 = ( (unsigned) 0xFF ) >> tmp ;
	/* Catch The Cases Where The Entire Region Is Within One Byte */
	if ( ( lx -= 8 - tmp ) < 0 ) {
		tmp2 &= 0xFF << -lx ;
		lx = 0 ;
	}
	/* Set The Bit Mask Reg */
	outb( 0x3CF, tmp2 ) ;
	if ( invert_existing_data == TRUE ) {
		outb( 0x3CE, 3 ) ; /* Set XOR */
		outb( 0x3CF, 0x18 ) ;
		for ( tmp = ly, dst = EGABASE + BYTE_OFFSET( x0, y0 ) ;
		      tmp-- ;
		      dst += BYTES_PER_ROW ) {
			tmp3 = *( (EgaMemoryPtr) dst ) ;
			*( (EgaMemoryPtr) dst ) = tmp2 ;
		}
		outb( 0x3CF, data_rotate_value ) ; /* Un-Set XOR */
		outb( 0x3CE, 8 ) ; /* Point At The Bit Mask Reg */
	}
	for ( tmp = ly, dst = EGABASE + BYTE_OFFSET( x0, y0 ) ;
	      tmp-- ;
	      dst += BYTES_PER_ROW ) {
		tmp3 = *( (EgaMemoryPtr) dst ) ;
		*( (EgaMemoryPtr) dst ) = tmp2 ;
	}
	if ( !lx ) { /* All Handled In This Byte */
		if ( cursor_saved )
			egaReplaceCursor() ;
		return ;
	}
	x0 = ( x0 + 8 ) & ~07 ;
}

/* Fill The Center Of The Box */
if ( ROW_OFFSET( lx ) ) {
	outb( 0x3CF, 0xFF ) ; /* Set The Bit Mask Reg */
	if ( invert_existing_data == TRUE ) {
		outb( 0x3CE, 3 ) ; /* Set XOR */
		outb( 0x3CF, 0x18 ) ;
		fastFillRMW( EGABASE + BYTE_OFFSET( x0, y0 ),
			     ROW_OFFSET( lx ), ly ) ;
		outb( 0x3CF, data_rotate_value ) ; /* Un-Set XOR */
		outb( 0x3CE, 8 ) ; /* Point At The Bit Mask Reg */
	}
	( ( read_write_modify == FALSE ) ? fastFill : fastFillRMW )
		( EGABASE + BYTE_OFFSET( x0, y0 ), ROW_OFFSET( lx ), ly ) ;
}

/* Do Right Edge */
if ( tmp = BIT_OFFSET( lx ) ) { /* x0 Now Is Byte Aligned */
	outb( 0x3CF, tmp2 = ( 0xFF << ( 8 - tmp ) ) ) ; /* Set The Bit Mask */
	if ( invert_existing_data == TRUE ) {
		outb( 0x3CE, 3 ) ; /* Set XOR */
		outb( 0x3CF, 0x18 ) ;
		for ( tmp = ly, dst = EGABASE + BYTE_OFFSET( ( x0 + lx ), y0 ) ;
		      tmp-- ;
		      dst += BYTES_PER_ROW ) {
			tmp3 = *( (EgaMemoryPtr) dst ) ;
			*( (EgaMemoryPtr) dst ) = tmp2 ;
		}
		outb( 0x3CF, data_rotate_value ) ;
	}
	for ( tmp = ly, dst = EGABASE + BYTE_OFFSET( ( x0 + lx ), y0 ) ;
	      tmp-- ;
	      dst += BYTES_PER_ROW ) {
		tmp3 = *( (EgaMemoryPtr) dst ) ;
		*( (EgaMemoryPtr) dst ) = tmp2 ;
	}
}
/* Disable Set/Reset Register */
outb( 0x3CE, 1 ) ;
outb( 0x3CF, 0 ) ;

if ( cursor_saved )
	egaReplaceCursor() ;

return ;
}
