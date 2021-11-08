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

/* $Header: vga_video.h,v 6.1 88/09/09 20:37:34 erik Exp $ */
/* $Source: /vice/X11/r3/server/ddx/ibm/vga/RCS/vga_video.h,v $ */

#if !defined(lint) && !defined(LOCORE) && defined(RCS_HDRS)
static char *rcsidvga_video = "$Header: vga_video.h,v 6.1 88/09/09 20:37:34 erik Exp $";
#endif

#ifdef lint
#if defined(volatile)
#undef volatile
#endif
#define volatile /**/
#if defined(const)
#undef const
#endif
#define const /**/
#if defined(signed)
#undef signed
#endif
#define signed /**/
#endif

/*
 * References to all pc ( i.e. '286 ) memory in the
 * regions used by the vga server ( the 128K windows )
 * MUST be long-word ( i.e. 32-bit ) reads or writes.
 * This definition will change for other memory architectures
 * ( e.g. AIX-Rt )
 */
#if defined(ATRIO)
typedef unsigned long int VideoAdapterObject ;
#else
#if defined(RTIO) || defined(PCIO)
typedef unsigned char VideoAdapterObject ;
#else
	******** ERROR ********
#endif
#endif
typedef volatile VideoAdapterObject *VgaMemoryPtr, *VideoMemoryPtr ;

#if !defined(BITMAP_BIT_ORDER)
#if defined(ibm032) || defined(i386) || defined(iAPX286) || defined(DOS)
#define BITMAP_BIT_ORDER MSBFirst
#else
	******** ERROR ********
#endif
#endif

#if !defined(IMAGE_BYTE_ORDER)
#if defined(ibm032)
#define IMAGE_BYTE_ORDER MSBFirst
#else
#if defined(i386) || defined(iAPX286) || defined(DOS)
#define IMAGE_BYTE_ORDER LSBFirst
#else
	******** ERROR ********
#endif
#endif
#endif

#if defined(i386) && defined(AIX386)
#define SCREEN_ADDR	0x020400000 /* "very" MAGIC NUMBER */
#else
#define SCREEN_ADDR	0x000a0000
#endif

#if defined(i386) || defined(iAPX286) || defined(DOS)
#define VGABASE ( (volatile unsigned char *) ( SCREEN_ADDR ) )
#else /* BSD43 */
#if defined(ATRIO)
#define VGABASE ( (volatile unsigned char *) \
	( 0xd00c0000 | ( SCREEN_ADDR & 0x0001ffff ) ) )
#else
#if defined(RTIO)
#define VGABASE ( (volatile unsigned char *) \
	( 0xd00c0000 | ( SCREEN_ADDR & 0x0001ffff ) ) )
#endif
#endif
#endif

#define VIDEO_MEMORY_BASE VGABASE

/* Bit Ordering Macros */
#if !defined(SCRLEFT8)
#if (BITMAP_BIT_ORDER == MSBFirst)	/* pc/rt */
#define SCRLEFT8(lw, n)	( (unsigned char) (((unsigned char) lw) << (n)) )
#else
#if (BITMAP_BIT_ORDER == LSBFirst)	/* intel */
#define SCRLEFT8(lw, n)	( (unsigned char) (((unsigned char) lw) >> (n)) )
#else
	******** ERROR ********
#endif
#endif
#endif
#if !defined(SCRRIGHT8)
#if BITMAP_BIT_ORDER == MSBFirst	/* pc/rt */
#define SCRRIGHT8(lw, n)	( (unsigned char) (((unsigned char)lw) >> (n)) )
#else
#if BITMAP_BIT_ORDER == LSBFirst
#define SCRRIGHT8(lw, n)	( (unsigned char) (((unsigned char)lw) << (n)) )
#else
	******** ERROR ********
#endif
#endif
#endif
/* These work ONLY on 8-bit wide Quantities !! */
#define LeftmostBit ( SCRLEFT8( 0xFF, 7 ) & 0xFF )
#define RightmostBit ( SCRRIGHT8( 0xFF, 7 ) & 0xFF )

/*
 * vga video screen defines & macros
 */
#define NO_TUBE 0
#define COLOR_TUBE 1
#define MONO_TUBE 2

#define VGA_BLACK_PIXEL 0
#define VGA_WHITE_PIXEL 1

#define VGA_MAXPLANES 4
#define VGA_ALLPLANES 0xF
#define PIX_PER_BYTE 8

#ifndef VGA720
#define BYTES_PER_ROW 80
#define MAX_ROW	479
#else
#define BYTES_PER_ROW 90
#define MAX_ROW	539
#endif

#define MAX_COLUMN ( ( BYTES_PER_ROW * PIX_PER_BYTE ) - 1 )
#define MAX_OFFSCREEN_ROW ( ( ( 64 * 1024 ) / BYTES_PER_ROW ) - 1 )
#define ROW_OFFSET( PIXEL_X_VALUE ) ( ( PIXEL_X_VALUE ) >> 3 )
#define BIT_OFFSET( PIXEL_X_VALUE ) ( ( PIXEL_X_VALUE ) & 0x7 )
#define BYTE_OFFSET( PIXEL_X_VALUE, PIXEL_Y_VALUE ) \
	( ( ( PIXEL_Y_VALUE ) * BYTES_PER_ROW ) + ( ( PIXEL_X_VALUE ) >> 3 ) )
#define SCREENADDRESS( PIXEL_X_VALUE, PIXEL_Y_VALUE ) \
	( VIDEO_MEMORY_BASE + BYTE_OFFSET( PIXEL_X_VALUE, PIXEL_Y_VALUE ) )
