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
/* $Header: /andrew/X11/R3src/Xbeta/server/ddx/ibm/ega/RCS/egaAddr.c,v 9.0 88/10/18 12:51:30 erik Exp $ */
/* $Source: /andrew/X11/R3src/Xbeta/server/ddx/ibm/ega/RCS/egaAddr.c,v $ */

#ifndef LEFT_SHIFT
#ifndef RIGHT_SHIFT
#ifndef NO_SHIFT
#include "egaVideo.h"
#endif
#endif
#endif

void
#ifdef LEFT_SHIFT
fast_blt_Left
#else
#ifdef RIGHT_SHIFT
fast_blt_Right
#else
#ifdef NO_SHIFT
#ifdef MOVE_RIGHT
fast_blt_Aligned_Right
#else
fast_blt_Aligned_Left
#endif
#endif
#endif
#endif
	( source, destination,
#ifdef LEFT_SHIFT
 backshift,
#else
#ifdef RIGHT_SHIFT
 shift,
#endif
#endif
 bytewidth, height, y_increment )
register volatile char *source ;
register volatile char *destination ;
#ifdef LEFT_SHIFT
register const unsigned int backshift ;
#else
#ifdef RIGHT_SHIFT
register const unsigned int shift ;
#endif
#endif
register const unsigned int bytewidth ;
#if HCVERSION>21000
register unsigned int height ;
register const int y_increment ;
#else
unsigned int height ;
int y_increment ;
#endif
{
register int tmp ;

#ifdef LEFT_SHIFT
register int savebits ;
#if HCVERSION>21000
register const int shift = 8 - backshift ;
#else
#define shift ( 8 - backshift )
#endif
register volatile char *stop_address = source - bytewidth ;
register const int row_jump = y_increment + bytewidth ;
#define SHIFT_OP <<
#define BACKSHIFT_OP >>
#define INCREMENT_OP --
#define SAVE_OFFSET ( 1 )

#else
#ifdef RIGHT_SHIFT
register int savebits ;
#if HCVERSION>21000
register const int backshift = 8 - shift ;
#else
#define backshift ( 8 - shift )
#endif
register volatile char *stop_address = source + bytewidth ;
register int row_jump = y_increment - bytewidth ;
#define SHIFT_OP >>
#define BACKSHIFT_OP <<
#define INCREMENT_OP ++
#define SAVE_OFFSET ( -1 )

#else
#ifdef NO_SHIFT
#if HCVERSION>21000
register const unsigned int notZero = ~0 ;
#else
#define notZero ( ~0 )
#endif
#ifdef MOVE_RIGHT
register volatile char *stop_address = source + bytewidth ;
register int row_jump = y_increment - bytewidth ;
#define INCREMENT_OP ++
#else
register volatile char *stop_address = source - bytewidth ;
register int row_jump = y_increment + bytewidth ;
#define INCREMENT_OP --

#endif
#endif
#endif
#endif

/* TOP OF FIRST LOOP */
BranchPoint:

#define SINGLE_LOAD \
	tmp = *( (EgaMemoryPtr) ( source INCREMENT_OP ) ) ;

#ifndef NO_SHIFT
#define SINGLE_STORE \
    ( ( *( (EgaMemoryPtr) ( destination INCREMENT_OP ) ) = \
	   ( tmp SHIFT_OP shift ) | savebits ) )
#define SINGLE_SAVE ( savebits = tmp BACKSHIFT_OP backshift )

savebits =
    *( (EgaMemoryPtr) ( source + SAVE_OFFSET ) )
	BACKSHIFT_OP backshift ;

#else
#define SINGLE_STORE \
    ( *( (EgaMemoryPtr) ( destination INCREMENT_OP ) ) = notZero )
#define SINGLE_SAVE
#endif

#define SINGLE_STEP SINGLE_LOAD ; SINGLE_STORE ; SINGLE_SAVE

switch ( bytewidth & 0xF ) {
	LoopTop :
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
	case 0x1 : SINGLE_LOAD ; SINGLE_STORE ;
/* FIRST LOOP */
		if ( source != stop_address ) {
			SINGLE_SAVE ;
			goto LoopTop ;
		}
/* SECOND LOOP */
		if ( --height ) {
			source += row_jump ;
			destination += row_jump ;
			stop_address += y_increment ;
			goto BranchPoint ;
		}
		else
			return ;
}
/*NOTREACHED*/
}

#undef INCREMENT_OP
#undef SAVE_OFFSET
#undef SINGLE_STORE
#undef SINGLE_SAVE
#undef SINGLE_STEP

#ifndef NO_SHIFT
#undef SHIFT_OP
#ifndef NEW_HC
#ifdef backshift
#undef backshift
#endif
#undef BACKSHIFT_OP
#endif
#endif
