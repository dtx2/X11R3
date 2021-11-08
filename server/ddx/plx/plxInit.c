/*
 *   Copyright (c) 1987, 88 by
 *   PARALLAX GRAPHICS, INCORPORATED, Santa Clara, California.
 *   All rights reserved
 *
 *   This software is furnished on an as-is basis, and may be used and copied
 *   only with the inclusion of the above copyright notice.
 *
 *   The information in this software is subject to change without notice.
 *   No committment is made as to the usability or reliability of this
 *   software.
 *
 *   Parallax Graphics, Inc.
 *   2500 Condensa Street
 *   Santa Clara, California  95051
 */

#ifndef lint
static char *sid_ = "@(#)plxInit.c	1.8 08/31/88 Parallax Graphics Inc";
#endif

#include	"Xplx.h"
#include	"input.h"

extern Bool plxScreenInit();
extern void plxMouseProc();
extern void plxKeybdProc();

/*
 * No motion buffer for now
 */
#define MOTION_BUFFER_SIZE 0

static PixmapFormatRec formats [] = {
	1, 1, BITMAP_SCANLINE_PAD,	/* required one-bit depth */
	8, 8, BITMAP_SCANLINE_PAD,	/* 1280-8 */
	16,16,BITMAP_SCANLINE_PAD,	/* 1280-16 */
	24,32,BITMAP_SCANLINE_PAD	/* 1280-24 */
};

#define NUMFORMATS ((sizeof formats)/(sizeof formats[0]))

InitOutput(screenInfo, argc, argv)
ScreenInfo *screenInfo;
int argc;
char *argv[];
{
	int i;

	screenInfo->imageByteOrder = IMAGE_BYTE_ORDER;
	screenInfo->bitmapScanlineUnit = BITMAP_SCANLINE_UNIT;
	screenInfo->bitmapScanlinePad = BITMAP_SCANLINE_PAD;
	screenInfo->bitmapBitOrder = BITMAP_BIT_ORDER;

	for (i=0;i<argc;i++) {
		if (strcmp(argv[i], "-image=lsb") == 0)
			screenInfo->imageByteOrder = LSBFirst;
		else
		if (strcmp(argv[i], "-image=msb") == 0)
			screenInfo->imageByteOrder = MSBFirst;
		else
		if (strcmp(argv[i], "-bitmap=lsb") == 0)
			screenInfo->bitmapBitOrder = LSBFirst;
		else
		if (strcmp(argv[i], "-bitmap=msb") == 0)
			screenInfo->bitmapBitOrder = MSBFirst;
		else
		if (strcmp(argv[i], "-debug") == 0)
			sscanf(argv[++i], "%x", &plx_debug);
	}

	ifdebug(5) printf("plx InitOutput()\n");

	screenInfo->numPixmapFormats = NUMFORMATS;
	for (i=0;i< NUMFORMATS;i++) {
		screenInfo->formats[i].depth = formats[i].depth;
		screenInfo->formats[i].bitsPerPixel = formats[i].bitsPerPixel;
		screenInfo->formats[i].scanlinePad = formats[i].scanlinePad;
	}
	AddScreen(plxScreenInit, argc, argv);
}

void
InitInput(argc, argv)
int argc;
char *argv[];
{
	DevicePtr p, k;

	ifdebug(5) printf("plx InitInput()\n");

	p = AddInputDevice(plxMouseProc, TRUE);

	k = AddInputDevice(plxKeybdProc, TRUE);

	RegisterPointerDevice(p, MOTION_BUFFER_SIZE);
	RegisterKeyboardDevice(k);
}

/*
 * DDX - specific abort routine.  Called by AbortServer().
 */
void
AbortDDX()
{
	p_coldstart();
}

/* Called by GiveUp(). */
void
ddxGiveUp()
{
}

int
ddxProcessArgument(argc, argv, i)
    int	argc;
    char *argv[];
    int	i;
{
    return 0;
}

void
ddxUseMsg()
{
}

#ifndef X11R2
void
InitExtensions()
{
#ifdef ZOID
	ZoidExtensionInit();
#endif ZOID
#ifdef BEZIER
	BezierExtensionInit();
#endif BEZIER
	/*
	 * always init video extensions
	 */
	PlxVideoExtensionInit();
}
#endif /* not X11R2 */
