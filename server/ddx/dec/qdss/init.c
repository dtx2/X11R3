/***********************************************************
Copyright 1987, 1988 by Digital Equipment Corporation, Maynard, Massachusetts,
and the Massachusetts Institute of Technology, Cambridge, Massachusetts.

                        All Rights Reserved

Permission to use, copy, modify, and distribute this software and its 
documentation for any purpose and without fee is hereby granted, 
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in 
supporting documentation, and that the names of Digital or MIT not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.  

DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.

******************************************************************/

#include "X.h"
#include "Xproto.h"
#include "Xmd.h"
#include "servermd.h"

#include "scrnintstr.h"
#include "input.h"
#include "cursor.h"
#include "misc.h"

#include "qd.h"
#include "qdprocs.h"

extern Bool qdScreenInit();
extern Bool qdScreenClose();		
extern void qdMouseProc();
extern void qdKeybdProc();

#define MOTION_BUFFER_SIZE 0
#define NUMSCREENS 1
#define NUMFORMATS 2

static PixmapFormatRec formats[NUMFORMATS] = {
    {1, 1, BITMAP_SCANLINE_PAD},
    {NPLANES, NPLANES, 8}
    };
extern	int	Nplanes;

Bool (*screenInitProcs[NUMSCREENS])() = {
    {qdScreenInit}
    };
Bool (*screenCloseProcs[NUMSCREENS])() = {
    {qdScreenClose}
    };

InitOutput( screenInfo, argc, argv)
    ScreenInfo *screenInfo;
    int		argc;
    char *	argv[];
{
    int i;

    /* This MUST be called before the Nplanes   *
     * global variable is used.  Gross, eh?	*/
    if ( tlInit() < 0)
    {
	ErrorF( "qdScreenInit: tlInit failed\n");
	return FALSE;
    }

    screenInfo->imageByteOrder = IMAGE_BYTE_ORDER;
    screenInfo->bitmapScanlineUnit = BITMAP_SCANLINE_UNIT;
    screenInfo->bitmapScanlinePad = BITMAP_SCANLINE_PAD;
    screenInfo->bitmapBitOrder = BITMAP_BIT_ORDER;

    screenInfo->numPixmapFormats = NUMFORMATS;
    for (i=0; i< NUMFORMATS; i++)
    {
        if (Nplanes != 4) {
	    screenInfo->formats[i].depth = formats[i].depth;
        } else {
	    if (formats[i].depth == NPLANES)
		screenInfo->formats[i].depth = 4;	/* for screen format */
	    else
		screenInfo->formats[i].depth = formats[i].depth;  /* i.e.: 1 */
        }
	screenInfo->formats[i].bitsPerPixel = formats[i].bitsPerPixel;
	screenInfo->formats[i].scanlinePad = formats[i].scanlinePad;

    }
	
    AddScreen( qdScreenInit, argc, argv);
}

InitInput( argc, argv)
    int		argc;
    char *	argv[];
{
    DevicePtr	p, k;
    
    p = AddInputDevice( qdMouseProc, TRUE);
    k = AddInputDevice( qdKeybdProc, TRUE);

    RegisterPointerDevice( p, MOTION_BUFFER_SIZE);
    RegisterKeyboardDevice( k);
}
