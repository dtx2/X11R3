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
	 * IBM Confidential Restricted
	 * Copyright (c) IBM Corporation 1987,1988
	 * All Rights Reserved.
	 *
	 * Owner: Tom Paquin, 8-465-4423
	 * Do not redistribute without permission 
	 */

/* $Header: /andrew/X11/R3src/Xbeta/server/ddx/ibm/ppc/RCS/ppc.h,v 9.0 88/10/17 17:34:40 erik Exp $ */
/* $Source: /andrew/X11/R3src/Xbeta/server/ddx/ibm/ppc/RCS/ppc.h,v $ */
/* "@(#)ppc.h	3.1 88/09/22 09:35:08" */

#if !defined(lint) && !defined(LOCORE) && defined(RCS_HDRS)
static char *rcsidppc = "$Header: /andrew/X11/R3src/Xbeta/server/ddx/ibm/ppc/RCS/ppc.h,v 9.0 88/10/17 17:34:40 erik Exp $";
#endif

#include "gcstruct.h"

typedef struct {
    unsigned long	planemask ;
    unsigned long	fgPixel ;
    unsigned long	bgPixel ;
    int			alu ;
    int			fillStyle ;
    } ppcReducedRrop ;

/* private field of GC */
typedef struct {
/* The Next eleven (11) fields MUST CORRESPOND to
 * the fields of a "mfbPrivGC" struct
 * ----- BEGINNING OF "DO-NOT-CHANGE" REGION ----- 
 */
    unsigned char	rop;		/* reduction of rasterop to 1 of 3 */
    unsigned char	ropOpStip;	/* rop for opaque stipple */
    unsigned char	ropFillArea;	/*  == alu, rop, or ropOpStip */
    short	fExpose;		/* callexposure handling ? */
    short	freeCompClip;
    PixmapPtr	pRotatedTile;		/* tile/stipple  rotated to align */
    PixmapPtr	pRotatedStipple;	/* with window and using offsets */
    RegionPtr	pAbsClientRegion;	/* client region in screen coords */
    RegionPtr	pCompositeClip;		/* free this based on freeCompClip
					   flag rather than NULLness */
    void 	(* FillArea)();		/* fills regions; look at the code */
    PixmapPtr   *ppPixmap;		/* points to the pixmapPtr to
					   use for tiles and stipples */
/* ----- END OF "DO-NOT-CHANGE" REGION ----- */
    ppcReducedRrop	colorRrop ;
    short lastDrawableType;	/* was last drawable a window or a pixmap? */
    short lastDrawableDepth;	/* was last drawable 1 or 8 planes? */
    pointer devPriv;		/* Private area for device specific stuff */
    } ppcPrivGC;
typedef ppcPrivGC	*ppcPrivGCPtr;

/* ************************************************************************ */

typedef struct {
    PixmapRec	pixmap;		/*
				 * this is here for mfb, it expects
				 * devPrivate to point to a pixmap.
				 */
    ColormapPtr	InstalledColormap;
    void	(* blit)();
    void	(* solidFill)();
    void	(* tileFill)();
    void	(* stipFill)();
    void	(* opstipFill)();
    void	(* imageFill)();
    void	(* imageRead)();
    void	(* lineBres)();
    void	(* lineHorz)();
    void	(* lineVert)();
    void	(* setColor)();
    void	(* RecolorCursor)();
    void	(* monoFill)();		/* 32 bit padded */
    void	(* glyphFill)();	/* GLYPHPADBYTES*8 bit padded */
    unsigned long *((* planeRead)());	/* for XYformat getImage() */
    void	(* replicateArea)();	/* Accelerator for Tile & Stipple */
    void	(* DestroyGCPriv)();	/* destroy devPriv in ppcPrivGC */
	/* High Level Software Cursor Support !! */
    int		* CursorSemaphore;
    int		(* CheckCursor)();
    void	(* ReplaceCursor)();
    } ppcScrnPriv, *ppcScrnPrivPtr;
