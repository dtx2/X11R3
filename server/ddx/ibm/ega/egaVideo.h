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
/* "@(#)ega_video.h	3.1 88/09/22 09:33:06" */
/*
 * PRPQ 5799-PFF (C) COPYRIGHT IBM CORPORATION 1987
 * LICENSED MATERIALS - PROPERTY OF IBM
 * REFER TO COPYRIGHT INSTRUCTIONS FORM NUMBER G120-2083
 */
/* $Header: /andrew/X11/R3src/Xbeta/server/ddx/ibm/ega/RCS/egaVideo.h,v 9.0 88/10/18 12:53:08 erik Exp $ */
/* $Source: /andrew/X11/R3src/Xbeta/server/ddx/ibm/ega/RCS/egaVideo.h,v $ */

#if !defined(lint) && !defined(LOCORE) && defined(RCS_HDRS)
static char *rcsidega_video = "$Header: /andrew/X11/R3src/Xbeta/server/ddx/ibm/ega/RCS/egaVideo.h,v 9.0 88/10/18 12:53:08 erik Exp $";
#endif

#ifdef lint
#define volatile /**/
#endif


#define EGA 1

/*
 * References to all pc ( i.e. '286 ) memory in the
 * regions used by the ega server ( the 128K windows )
 * MUST be long-word ( i.e. 32-bit ) reads or writes.
 * This definition will change for other memory architectures
 * ( e.g. AIX-Rt )
 */

typedef unsigned volatile char *EgaMemoryPtr;
/*
 * ega video screen defines & macros
 */
#define SCREEN_ADDR	( 0x000B8000 )
#define NUMSCREENS 1
#define MAXPLANES 4
#define ALLPLANES 0xF
#define BYTES_PER_ROW 80
#define PIX_PER_BYTE 8
#define MAX_ROW	350
#define MAX_OFFSCREEN_ROW   408
#define MAX_COLUMN ( ( BYTES_PER_ROW * PIX_PER_BYTE ) - 1 )

#define ROW_OFFSET( PIXEL_X_VALUE ) ( ( PIXEL_X_VALUE ) >> 3 )
#define BIT_OFFSET( PIXEL_X_VALUE ) ( ( PIXEL_X_VALUE ) & 0x7 )
#define BYTE_OFFSET( PIXEL_X_VALUE, PIXEL_Y_VALUE ) \
	( ( ( PIXEL_Y_VALUE ) * BYTES_PER_ROW ) + ( ( PIXEL_X_VALUE ) >> 3 ) )

#ifdef iAPX286
#define EGABASE ( (volatile unsigned char *) ( SCREEN_ADDR ) )
extern int GO_BIOS_VIDEO_MODE( ) ;
#else
extern int ega_screen_file ; /* Program GLOBAL File Descriptor for /dev/ega */
#ifdef ATRIO
#define EGABASE ( (volatile unsigned char *) \
	( 0xD00C0000 | ( SCREEN_ADDR & 0x0000ffff ) ) )
#else
#define EGABASE ( (volatile unsigned char *) \
	( 0xF4000000 | ( SCREEN_ADDR & 0x000fffff ) ) )
#endif

#define GO_BIOS_VIDEO_MODE( mode ) ioctl( ega_screen_file, BUFINITEGA, &mode )
#endif
