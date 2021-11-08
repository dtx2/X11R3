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
/* $Header: /andrew/X11/R3src/Xbeta/server/ddx/ibm/ega/RCS/egaBitBlt.c,v 9.0 88/10/18 12:51:34 erik Exp $ */
/* $Source: /andrew/X11/R3src/Xbeta/server/ddx/ibm/ega/RCS/egaBitBlt.c,v $ */

#ifndef lint
static char *rcsid = "$Header: /andrew/X11/R3src/Xbeta/server/ddx/ibm/ega/RCS/egaBitBlt.c,v 9.0 88/10/18 12:51:34 erik Exp $";
static char sccsid[] = "@(#)egaBitBlt.c	3.1 88/09/22 09:32:41";
#endif

/*
 * REGISTER USAGE
 * 0x3C4 -- GENERAL REGISTER INDEX
 * 0x3C5 -- MAP MASK
 * 0x3CE -- GRAPHICS REGISTER INDEX
 * 0x3CF -- DATA ROTATE
 * 0x3CF -- BIT MASK
 */

#include "X.h"
#include "egaVideo.h"

#include "ibmIOArch.h"

#undef TRUE
#undef FALSE
#define TRUE 1
#define FALSE 0

#ifdef LEFT_SHIFT
#undef LEFT_SHIFT
#endif
#ifdef NO_SHIFT
#undef NO_SHIFT
#endif
#define RIGHT_SHIFT
static /* fast_blt_Right() */
#include "egaAddr.c"
#undef RIGHT_SHIFT
#define LEFT_SHIFT
static /* fast_blt_Left() */
#include "egaAddr.c"
#undef LEFT_SHIFT
#define NO_SHIFT
#define MOVE_RIGHT
static /* fast_blt_Aligned_Right() */
#include "egaAddr.c"
#undef MOVE_RIGHT
#define MOVE_LEFT
static /* fast_blt_Aligned_Left() */
#include "egaAddr.c"
#undef MOVE_LEFT
#undef NO_SHIFT

static void fix_video_byte( source, destination, byte_offset, alu )
register volatile char *source ;
register volatile char *destination ;
register const int byte_offset ;
register const unsigned int alu ;
{
register unsigned long int tmp1, tmp2 ;

	if ( byte_offset )
		tmp1 =
	  ( *( (EgaMemoryPtr) ( source ) ) >> byte_offset )
	| ( *( (EgaMemoryPtr) ( source - 1 ) ) << ( 8 - byte_offset ) ) ;
	else
		tmp1 = *( (EgaMemoryPtr) source ) ;
	tmp2 = *( (EgaMemoryPtr) destination ) ;
	switch ( alu ) {
	case GXnor:
		tmp1 = ~( tmp1 | tmp2 ) ;
		break ;
	case GXandInverted:
		tmp1 = ~tmp1 & tmp2 ;
		break ;
	case GXand:
		tmp1 &= tmp2 ;
		break ;
	case GXequiv:
		tmp1 = ~tmp1 ^ tmp2 ;
		break ;
	case GXxor:
		tmp1 ^= tmp2 ;
		break ;
	case GXandReverse:
		tmp1 &= ~tmp2 ;
		break ;
	case GXorReverse:
		tmp1 |= ~tmp2 ;
		break ;
	case GXnand:
		tmp1 = ~( tmp1 & tmp2 ) ;
		break ;
	case GXorInverted:
		tmp1 = ~tmp1 | tmp2 ;
		break ;
	case GXor:
		tmp1 |= tmp2 ;
		break ;
	case GXcopyInverted:
		tmp1 = ~tmp1 ;
	case GXcopy:
	default:
		break ;
	}
	*( (EgaMemoryPtr) destination ) = tmp1 ;
return ;
}

extern int egaFillSolid() ;
/* Declared in "egacurs.c" */
extern int egaCursorChecking ;
extern int egaCheckCursor() ;
extern void egaReplaceCursor() ;

#define CLEANUP \
if ( cursor_saved ) \
	egaReplaceCursor() ;

void egaBitBlt( alu, readplanes, writeplanes, x0, y0, x1, y1, w, h )
const int alu, readplanes, writeplanes ;
register int x0 ;
int y0 ;
register int x1 ;
int y1 ;
register int w, h ;
{
register volatile char *s1ptr /* , *s2ptr */ ;
register unsigned int j ;
register volatile char *d1ptr /* , *d2ptr */ ;
register center_width ;
register int x_interval ;
register int y_interval ;
register unsigned int i ;
register volatile char *src ;
register volatile char *dst ;
unsigned int currplane ;
int byte_offset ;
int left_ragged ;
int right_ragged ;
int cursor_saved ;

switch ( alu ) {
	case GXclear:		/* 0x0 Zero 0 */
	case GXinvert:		/* 0xa NOT dst */
	case GXset:		/* 0xf 1 */
		egaFillSolid( 0xF, alu, writeplanes, x0, y0, w, h ) ;
	case GXnoop:		/* 0x5 dst */
		return ;
	default:
		break ;
}

left_ragged  = BIT_OFFSET( x1 ) ;
right_ragged = 7 - BIT_OFFSET( x1 + w - 1 ) ;
center_width = ROW_OFFSET( x1 + w ) - ROW_OFFSET( ( x1 + 0x7 ) & ~0x7 ) ;

src = (char *) EGABASE + ( BYTES_PER_ROW * y0 ) ;
dst = (char *) EGABASE + ( BYTES_PER_ROW * y1 ) ;
if ( y1 > y0 ) {
	y_interval = - BYTES_PER_ROW ;
	src += BYTES_PER_ROW * ( h - 1 ) ;
	dst += BYTES_PER_ROW * ( h - 1 ) ;
}
else {
	y_interval = BYTES_PER_ROW ;
}

if ( x1 < x0 ) {
	x_interval = 1 ;
	src += ROW_OFFSET( x0 ) ;
	dst += ROW_OFFSET( x1 ) ;
	byte_offset = left_ragged - BIT_OFFSET( x0 ) ;
}
else {
	x_interval = -1 ;
	src += ROW_OFFSET( x0 + w - 1 ) ;
	dst += ROW_OFFSET( x1 + w - 1 ) ;
	byte_offset = 7 - BIT_OFFSET( x0 + w - 1 ) - right_ragged ;
}
/*
 * byte_offset <==> Number Of Bits To Move To The Right
 * EXAMPLES:
 * if ( byte_offset == 2 )  THEN Shift "src" 2 Bits To To The Right
 *			    AND Shift In Byte From The Left
 * or
 * if ( byte_offset == -2 ) THEN Shift "src" 2 Bits To To The Left
 *			    AND Shift In Byte From The Right
 */
if ( byte_offset < 0 ) {
	src += 1 ;
	byte_offset += 8 ;
}
/* If Cursor Is In The Way Remove It */
cursor_saved = !egaCursorChecking
	    && ( egaCheckCursor( x0, y0, w, h )
	      || egaCheckCursor( x1, y1, w, h ) ) ;

/* Disable SET/RESET Function */
outb( 0x3CE, 1 ) ;
outb( 0x3CF, 0 ) ;
/* Set Write Mode To 0 -- Read Mode To 0 */
outb( 0x3CE, 5 ) ;
outb( 0x3CF, 0 ) ;

/* Test For Special Case -- Try To Do Fast Blt */
if ( alu == GXcopy ) {
	if ( !byte_offset ) { /* Test For Special Case -- VERY Fast Blt */
		unsigned int do_edge = FALSE ;

		outb( 0x3CE, 8 ) ; /* Prepare To Set Bit Mask */
		if ( x_interval > 0 ) {
			if ( left_ragged ) { /* Move Left Edge */
				if ( center_width >= 0 )
					outb( 0x3CF, ( 0xFF >> left_ragged ) ) ;
				else {
					outb( 0x3CF,
			( ( 0xFF >> left_ragged ) & ( 0xFF << right_ragged ) ) ) ;
				}
				do_edge = TRUE ;
			}
		}
		else if ( right_ragged ) {
			if ( center_width >= 0 )
				outb( 0x3CF, ( 0xFF << right_ragged ) ) ;
			else {
				outb( 0x3CF,
			( ( 0xFF >> left_ragged ) & ( 0xFF << right_ragged ) ) ) ;
			}
			do_edge = TRUE ;
		}
		else {
			outb( 0x3CF, 0xFF ) ;
		}
		s1ptr = src ;
		d1ptr = dst ;
		if ( do_edge ) {
			/* Set Data Rotate Function To Direct Write */
			outb( 0x3CE, 3 ) ;
			outb( 0x3CF, 0 ) ;
			for ( currplane = 0 ; currplane <= MAXPLANES ; currplane++  ) {
				if ( i = writeplanes & ( 1 << currplane ) ) {
					outb( 0x3C4, 2 ) ;
					outb( 0x3C5, i ) ; /* Set Map Mask */
					/* Set Map Read Select */
					outb( 0x3CE, 4 ) ;
					outb( 0x3CF, currplane ) ;
					/* Move First Edge */
					for ( d1ptr = dst, s1ptr = src, j = h ;
					      j-- ;
					      d1ptr += y_interval,
					      s1ptr += y_interval ) {
						register tmp1, tmp2 ;

						tmp1 =
						  *( (EgaMemoryPtr) s1ptr ) ;
						tmp2 =
						  *( (EgaMemoryPtr) d1ptr ) ;
						*( (EgaMemoryPtr) d1ptr )
						  = tmp1 ;
					}
				}
			}
			if ( center_width < 0 ) {
				CLEANUP ;
				return ;
			}
			src += x_interval ;
			dst += x_interval ;
			do_edge = FALSE ;
			/* ReSet Bit Mask */
			outb( 0x3CE, 8 ) ;
			outb( 0x3CF, 0xFF ) ;
		}
		/* Set Data Rotate Function To Data "AND" */
		outb( 0x3CE, 3 ) ;
		outb( 0x3CF, 0x8 ) ;
		/* Set Map Mask */
		outb( 0x3C4, 2 ) ;
		outb( 0x3C5, writeplanes ) ;
		/* Move Center Of Box */
		if ( center_width ) {
			( ( x_interval > 0 )
			   ? fast_blt_Aligned_Right : fast_blt_Aligned_Left )
				( src, dst, center_width, h, y_interval ) ;
		}
		/* ReSet Bit Mask */
		outb( 0x3CE, 8 ) ;
		/* Move Second Edge */
		if ( x_interval > 0 ) {
			if ( right_ragged ) { /* Move Right Edge */
				outb( 0x3CF, 0xFF << right_ragged ) ;
				/* Adjust Offsets */
				src += center_width ;
				dst += center_width ;
			}
			else {
				CLEANUP ;
				return ;
			}
		}
		else if ( left_ragged ) { /* Move Left Edge */
			outb( 0x3CF, 0xFF >> left_ragged ) ;
			/* Adjust Offsets */
			src -= center_width ;
			dst -= center_width ;
		}
		else {
			CLEANUP ;
			return ;
		}
		/* Do Last Edge */
		/* Set Data Rotate Function To Direct Write */
		outb( 0x3CE, 3 ) ;
		outb( 0x3CF, 0 ) ;
		for ( currplane = 0 ; currplane <= MAXPLANES ; currplane++  ) {
			if ( i = writeplanes & ( 1 << currplane ) ) {
				outb( 0x3C4, 2 ) ;
				outb( 0x3C5, i ) ; /* Set Map Mask */
				outb( 0x3CE, 4 ) ;
				/* Set Map Read Select */
				outb( 0x3CF, currplane ) ;
				for ( s1ptr = src, d1ptr = dst, j = h ;
			 	      j-- ;
				      d1ptr += y_interval, s1ptr += y_interval ) {
					register tmp1, tmp2 ;

					tmp1 = *( (EgaMemoryPtr) s1ptr ) ;
					tmp2 = *( (EgaMemoryPtr) d1ptr ) ;
					*( (EgaMemoryPtr) d1ptr ) = tmp1 ;
				}
			}
		}
	} /* End Of Very Fast BitBlt */
	else {	/* Slow GXcopy BitBlt Here -- Bits in Bytes NOT Aligned */
		/* Set Data Rotate Function To Direct Write */
		outb( 0x3CE, 3 ) ;
		outb( 0x3CF, 0 ) ;
		for ( currplane = 0 ; currplane <= MAXPLANES ; currplane++  ) {
			if ( i = writeplanes & ( 1 << currplane ) ) {
				/* Logical Operation Depending On Src & Dst */
				unsigned int do_edge = FALSE ;

				outb( 0x3C4, 2 ) ;
				outb( 0x3C5, i ) ; /* Set Map Mask */
				outb( 0x3CE, 4 ) ;
				outb( 0x3CF, currplane ) ; /* Set Map Read Select */
				outb( 0x3CE, 8 ) ;
				outb( 0x3CF, 0xFF ) ; /* Set Bit Mask */
				/* Move First Edge */
				s1ptr = src ;
				d1ptr = dst ;
				if ( x_interval > 0 ) {
					if ( left_ragged ) { /* Move Left Edge */
						if ( center_width >= 0 )
							outb( 0x3CF, ( 0xFF >> left_ragged ) ) ;
						else {
							outb( 0x3CF,
				( ( 0xFF >> left_ragged ) & ( 0xFF << right_ragged ) ) ) ;
						}
						do_edge = TRUE ;
					}
				}
				else if ( right_ragged ) {
					if ( center_width >= 0 )
						outb( 0x3CF, ( 0xFF << right_ragged ) ) ;
					else {
						outb( 0x3CF,
				( ( 0xFF >> left_ragged ) & ( 0xFF << right_ragged ) ) ) ;
					}
					do_edge = TRUE ;
				}
				if ( do_edge ) {
					for ( d1ptr = dst, s1ptr = src, j = h ;
				 	      j-- ;
					      d1ptr += y_interval, s1ptr += y_interval )
						fix_video_byte( s1ptr, d1ptr, byte_offset, GXcopy ) ;
					if ( center_width < 0 ) /* All In One Byte */
						continue ; /* Next Plane */
					outb( 0x3CF, 0xFF ) ;
					s1ptr = src + x_interval ;
					d1ptr = dst + x_interval ;
					do_edge = FALSE ;
				}
				/* Move Center Of Box */
				if ( center_width ) {
					( ( x_interval >= 0 ) ? fast_blt_Right : fast_blt_Left )
					( ( ( x_interval >= 0 ) ? s1ptr : s1ptr - 1 ),
					 d1ptr, byte_offset,
					 center_width, h, y_interval ) ;
/* CHANGE */
	/*				if ( x_interval > 0 ) { */
/* CHANGE */
					if ( x_interval >= 0 ) {
						s1ptr += center_width ;
						d1ptr += center_width ;
					}
					else {
						s1ptr -= center_width ;
						d1ptr -= center_width ;
					}
				}
				/* Move Second Edge */
				if ( x_interval > 0 ) {
					if ( right_ragged ) { /* Move Right Edge */
						outb( 0x3CF, 0xFF << right_ragged ) ;
						do_edge = TRUE ;
					}
				}
				else if ( left_ragged ) { /* Move Left Edge */
					outb( 0x3CF, 0xFF >> left_ragged ) ;
					do_edge = TRUE ;
				}
				if ( do_edge ) {
					for ( j = h ;
				 	      j-- ;
					      d1ptr += y_interval, s1ptr += y_interval )
						fix_video_byte( s1ptr, d1ptr, byte_offset, GXcopy ) ;
					outb( 0x3CF, 0xFF ) ;
					do_edge = FALSE ;
				}
			}
		}
		/* Re-Enable All Planes In Map Mask */
		outb( 0x3C4, 2 ) ;
		outb( 0x3C5, ALLPLANES ) ;
	}

	CLEANUP ;
	return ;
} /* End Of GXcopy BitBlt */

/* Slow BitBlt Here */
/* Set Data Rotate Function To Direct Write */
outb( 0x3CE, 3 ) ;
outb( 0x3CF, 0 ) ;
for ( currplane = 0 ; currplane <= MAXPLANES ; currplane++  ) {
	if ( i = writeplanes & ( 1 << currplane ) ) {
		register volatile char *s2ptr ;
		register volatile char *d2ptr ;
		/* Logical Operation Depending On Src & Dst */
		unsigned int do_edge = FALSE ;

		outb( 0x3C4, 2 ) ;
		outb( 0x3C5, i ) ; /* Set Map Mask */
		outb( 0x3CE, 4 ) ;
		outb( 0x3CF, currplane ) ; /* Set Map Read Select */
		outb( 0x3CE, 8 ) ;
		outb( 0x3CF, 0xFF ) ; /* Set Bit Mask */
		/* Move First Edge */
		s1ptr = src ;
		d1ptr = dst ;
		if ( x_interval > 0 ) {
			if ( left_ragged ) { /* Move Left Edge */
				if ( center_width >= 0 )
					outb( 0x3CF, ( 0xFF >> left_ragged ) ) ;
				else {
					outb( 0x3CF,
		( ( 0xFF >> left_ragged ) & ( 0xFF << right_ragged ) ) ) ;
				}
				do_edge = TRUE ;
			}
		}
		else if ( right_ragged ) {
			if ( center_width >= 0 )
				outb( 0x3CF, ( 0xFF << right_ragged ) ) ;
			else {
				outb( 0x3CF,
		( ( 0xFF >> left_ragged ) & ( 0xFF << right_ragged ) ) ) ;
			}
			do_edge = TRUE ;
		}
		if ( do_edge ) {
			for ( d1ptr = dst, s1ptr = src, j = h ;
		 	      j-- ;
			      d1ptr += y_interval, s1ptr += y_interval )
				fix_video_byte( s1ptr, d1ptr, byte_offset, alu ) ;
			if ( center_width < 0 ) /* All In One Byte */
				continue ; /* Next Plane */
			outb( 0x3CF, 0xFF ) ;
			s1ptr = src + x_interval ;
			d1ptr = dst + x_interval ;
			do_edge = FALSE ;
		}
		/* Move Center Of Box */
		for ( i = h ;
		      i-- ;
		      s1ptr += y_interval, d1ptr += y_interval )
			for ( j = center_width, s2ptr = s1ptr,
			      d2ptr = d1ptr ;
			      j-- ;
			      s2ptr += x_interval, d2ptr += x_interval ) {
				register unsigned long int tmp1 ;

				if ( byte_offset ) {
					tmp1 =
				  ( *( (EgaMemoryPtr) ( s2ptr ) ) >> byte_offset )
				| ( *( (EgaMemoryPtr) ( s2ptr - 1 ) ) << ( 8 - byte_offset ) ) ;
				}
				else
					tmp1 = *( (EgaMemoryPtr) s2ptr ) ;
				switch ( alu ) {
				case GXnor:
					tmp1 = ~( tmp1
					 | *( (EgaMemoryPtr) d2ptr ) ) ;
					break ;
				case GXandInverted:
					tmp1 =
				~tmp1 & *( (EgaMemoryPtr) d2ptr ) ;
					break ;
				case GXand:
					tmp1 &= *( (EgaMemoryPtr) d2ptr ) ;
					break ;
				case GXequiv:
					tmp1 = ~tmp1 ^ *( (EgaMemoryPtr) d2ptr ) ;
					break ;
				case GXxor:
					tmp1 ^= *( (EgaMemoryPtr) d2ptr ) ;
					break ;
				case GXandReverse:
					tmp1 &= ~ *( (EgaMemoryPtr) d2ptr ) ;
					break ;
				case GXorReverse:
					tmp1 |= ~*( (EgaMemoryPtr) d2ptr ) ;
					break ;
				case GXnand:
					tmp1 = ~( tmp1 & *( (EgaMemoryPtr) d2ptr ) ) ;
					break ;
				case GXorInverted:
					tmp1 = ~tmp1 | *( (EgaMemoryPtr) d2ptr ) ;
					break ;
				case GXor:
					tmp1 |= *( (EgaMemoryPtr) d2ptr ) ;
					break ;
				case GXcopyInverted:
					tmp1 = ~*( (EgaMemoryPtr) d2ptr ) ;
				default:
					break ;
				}
				*( (EgaMemoryPtr) d2ptr ) = tmp1 ;
		}
		/* Adjust Offsets */
		j = ( h * y_interval ) - ( center_width * x_interval ) ;
		s1ptr -= j ;
		d1ptr -= j ;
		/* Move Second Edge */
		if ( x_interval > 0 ) {
			if ( right_ragged ) { /* Move Right Edge */
				outb( 0x3CF, 0xFF << right_ragged ) ;
				do_edge = TRUE ;
			}
		}
		else if ( left_ragged ) { /* Move Left Edge */
			outb( 0x3CF, 0xFF >> left_ragged ) ;
			do_edge = TRUE ;
		}
		if ( do_edge ) {
			for ( j = h ;
		 	      j-- ;
			      d1ptr += y_interval, s1ptr += y_interval )
				fix_video_byte( s1ptr, d1ptr, byte_offset, alu ) ;
			outb( 0x3CF, 0xFF ) ;
			do_edge = FALSE ;
		}
	}
}
/* Re-Enable All Planes In Map Mask */
outb( 0x3C4, 2 ) ;
outb( 0x3C5, ALLPLANES ) ;

CLEANUP ;
return ;
}
