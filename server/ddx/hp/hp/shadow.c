/* 
 * shadow.c : the "protect the cursor" layer between the GC and
 *   other routines that diddle the screen
 */

/*
Copyright (c) 1986, 1987 by Hewlett-Packard Company
Copyright (c) 1986, 1987 by the Massachusetts Institute of Technology

Permission to use, copy, modify, and distribute this
software and its documentation for any purpose and without
fee is hereby granted, provided that the above copyright
notice appear in all copies and that both that copyright
notice and this permission notice appear in supporting
documentation, and that the name of M.I.T. not be used in
advertising or publicity pertaining to distribution of the
software without specific, written prior permission.

HEWLETT-PACKARD MAKES NO WARRANTY OF ANY KIND WITH REGARD
TO THIS SOFWARE, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR 
PURPOSE.  Hewlett-Packard shall not be liable for errors 
contained herein or direct, indirect, special, incidental or 
consequential damages in connection with the furnishing, 
performance, or use of this material.

This software is not subject to any license of the American
Telephone and Telegraph Company or of the Regents of the
University of California.
*/

#include    "sun.h"

#include    "X.h"
#include    "Xmd.h"
#include    "extnsionst.h"
#include    "regionstr.h"
#include    "windowstr.h"
#include    "fontstruct.h"
#include    "dixfontstr.h"

#include "shadow.h"

/* ******************************************************************** */
/* ***************** the cfb shadow *********************************** */
/* ******************************************************************** */

void cfbsolidfs(pDrawable, pGC, nInit, pptInit, pwidthInit, fSorted)
    DrawablePtr pDrawable;
    GCPtr	pGC;
    int		nInit;			/* number of spans to fill */
    DDXPointPtr pptInit;		/* pointer to list of start points */
    int		*pwidthInit;		/* pointer to list of n widths */
    int 	fSorted;
{
  hpFillSpans(pDrawable, pGC, nInit, pptInit, pwidthInit, fSorted);
  cfbSolidFS(pDrawable, pGC, nInit, pptInit, pwidthInit, fSorted);
}

void cfbunnaturaltilefs(pDrawable, pGC, nInit, pptInit, pwidthInit, fSorted)
    DrawablePtr pDrawable;
    GCPtr	pGC;
    int		nInit;			/* number of spans to fill */
    DDXPointPtr pptInit;		/* pointer to list of start points */
    int		*pwidthInit;		/* pointer to list of n widths */
    int 	fSorted;
{
  hpFillSpans(pDrawable, pGC, nInit, pptInit, pwidthInit, fSorted);
  cfbUnnaturalTileFS(pDrawable, pGC, nInit, pptInit, pwidthInit, fSorted);
}

void cfbunnaturalstipplefs(pDrawable, pGC, nInit, pptInit, pwidthInit, fSorted)
    DrawablePtr pDrawable;
    GCPtr	pGC;
    int		nInit;			/* number of spans to fill */
    DDXPointPtr pptInit;		/* pointer to list of start points */
    int		*pwidthInit;		/* pointer to list of n widths */
    int 	fSorted;
{
  hpFillSpans(pDrawable, pGC, nInit, pptInit, pwidthInit, fSorted);
  cfbUnnaturalStippleFS(pDrawable, pGC, nInit, pptInit, pwidthInit, fSorted);
}

void cfbsetspans(pDrawable, pGC, psrc, ppt, pwidth, nspans, fSorted)
    DrawablePtr		pDrawable;
    GCPtr		pGC;
    int			*psrc, *pwidth, nspans, fSorted;
    register DDXPointPtr ppt;
{
  hpSetSpans(pDrawable, pGC, psrc, ppt, pwidth, nspans, fSorted);
  cfbSetSpans(pDrawable, pGC, psrc, ppt, pwidth, nspans, fSorted);
}

unsigned int *cfbGetSpans();

unsigned int *cfbgetspans(pDrawable, wMax, ppt, pwidth, nspans)
    DrawablePtr		pDrawable;	/* drawable from which to get bits */
    int			wMax;		/* largest value of all *pwidths */
    register DDXPointPtr ppt;		/* points to start copying from */
    int			*pwidth;	/* list of number of bits to copy */
    int			nspans;		/* number of scanlines to copy */
{
  hpGetSpans(pDrawable, wMax, ppt, pwidth, nspans);
  return cfbGetSpans(pDrawable, wMax, ppt, pwidth, nspans);
}

/* ******************************************************************** */
/* ***************** the MRM/MRC cfb shadow *************************** */
/* ******************************************************************** */

void mrcfbsolidfs(pDrawable, pGC, nInit, pptInit, pwidthInit, fSorted)
    DrawablePtr pDrawable;
    GCPtr	pGC;
    int		nInit;			/* number of spans to fill */
    DDXPointPtr pptInit;		/* pointer to list of start points */
    int		*pwidthInit;		/* pointer to list of n widths */
    int 	fSorted;
{
  hpFillSpans(pDrawable, pGC, nInit, pptInit, pwidthInit, fSorted);
  mrcfbSolidFS(pDrawable, pGC, nInit, pptInit, pwidthInit, fSorted);
}

void mrcfbunnaturaltilefs(pDrawable, pGC, nInit, pptInit, pwidthInit, fSorted)
    DrawablePtr pDrawable;
    GCPtr	pGC;
    int		nInit;			/* number of spans to fill */
    DDXPointPtr pptInit;		/* pointer to list of start points */
    int		*pwidthInit;		/* pointer to list of n widths */
    int 	fSorted;
{
  hpFillSpans(pDrawable, pGC, nInit, pptInit, pwidthInit, fSorted);
  mrcfbUnnaturalTileFS(pDrawable, pGC, nInit, pptInit, pwidthInit, fSorted);
}

void mrcfbunnaturalstipplefs(pDrawable, pGC, nInit, pptInit, pwidthInit, fSorted)
    DrawablePtr pDrawable;
    GCPtr	pGC;
    int		nInit;			/* number of spans to fill */
    DDXPointPtr pptInit;		/* pointer to list of start points */
    int		*pwidthInit;		/* pointer to list of n widths */
    int 	fSorted;
{
  hpFillSpans(pDrawable, pGC, nInit, pptInit, pwidthInit, fSorted);
  mrcfbUnnaturalStippleFS(pDrawable, pGC, nInit, pptInit, pwidthInit, fSorted);
}

void mrcfbsetspans(pDrawable, pGC, psrc, ppt, pwidth, nspans, fSorted)
    DrawablePtr		pDrawable;
    GCPtr		pGC;
    int			*psrc, *pwidth, nspans, fSorted;
    register DDXPointPtr ppt;
{
  hpSetSpans(pDrawable, pGC, psrc, ppt, pwidth, nspans, fSorted);
  mrcfbSetSpans(pDrawable, pGC, psrc, ppt, pwidth, nspans, fSorted);
}

unsigned int *mrcfbGetSpans();

unsigned int *mrcfbgetspans(pDrawable, wMax, ppt, pwidth, nspans)
    DrawablePtr		pDrawable;	/* drawable from which to get bits */
    int			wMax;		/* largest value of all *pwidths */
    register DDXPointPtr ppt;		/* points to start copying from */
    int			*pwidth;	/* list of number of bits to copy */
    int			nspans;		/* number of scanlines to copy */
{
  hpGetSpans(pDrawable, wMax, ppt, pwidth, nspans);
  return mrcfbGetSpans(pDrawable, wMax, ppt, pwidth, nspans);
}
/* ******************************************************************** */
/* ***************** the mi shadow ************************************ */
/* ******************************************************************** */

void miputimage(pDst, pGC, depth, x, y, w, h, leftPad, format, pBits)
    DrawablePtr	  pDst;
    GCPtr   	  pGC;
    int		  depth, x, y, w, h, format;
    char    	  *pBits;
{
  hpPutImage(pDst, pGC, depth, x, y, w, h, leftPad, format, pBits);
  miPutImage(pDst, pGC, depth, x, y, w, h, leftPad, format, pBits);
}

RegionPtr micopyarea(pSrc, pDst, pGC, srcx, srcy, w, h, dstx, dsty)
    DrawablePtr	  pSrc, pDst;
    GCPtr   	  pGC;
    int	    	  srcx, srcy, w, h, dstx, dsty;
{
  extern RegionPtr miCopyArea ();
  hpCopyArea(pSrc, pDst, pGC, srcx, srcy, w, h, dstx, dsty);
  return miCopyArea(pSrc, pDst, pGC, srcx, srcy, w, h, dstx, dsty); 
}

RegionPtr micopyplane(pSrc, pDst, pGC, srcx, srcy, w, h, dstx, dsty, plane)
    DrawablePtr	  pSrc, pDst;
    register GC   *pGC;
    int     	  srcx, srcy, w,h, dstx,dsty;
    unsigned int  plane;
{
  extern RegionPtr miCopyPlane ();
  hpCopyPlane(pSrc, pDst, pGC, srcx, srcy, w, h, dstx, dsty, plane);
  return miCopyPlane(pSrc, pDst, pGC, srcx, srcy, w, h, dstx, dsty, plane);
}

void mipolypoint(pDrawable, pGC, mode, npt, pptInit)
    DrawablePtr pDrawable;
    GCPtr	pGC;
    int		mode;		/* Origin or Previous */
    int		npt;
    xPoint 	*pptInit;
{
  hpPolyPoint(pDrawable, pGC, mode, npt, pptInit);
  miPolyPoint(pDrawable, pGC, mode, npt, pptInit);
}

void minotmiter(pDrawable, pGC, mode, npt, pptInit)
    DrawablePtr	  pDrawable;
    GCPtr   	  pGC;
    int	    	  mode, npt;
    DDXPointPtr	  pptInit;
{
  hpPolylines (pDrawable, pGC, mode, npt, pptInit);
  miNotMiter(pDrawable, pGC, mode, npt, pptInit);
}

void miwideline(pDrawable, pGC, mode, npt, pptInit)
    DrawablePtr	  pDrawable;
    GCPtr   	  pGC;
    int	    	  mode, npt;
    DDXPointPtr	  pptInit;
{
  hpPolylines (pDrawable, pGC, mode, npt, pptInit);
  miWideLine(pDrawable, pGC, mode, npt, pptInit);
}

void miwidedash(pDrawable, pGC, mode, npt, pptInit)
    DrawablePtr	  pDrawable;
    GCPtr   	  pGC;
    int	    	  mode, npt;
    DDXPointPtr	  pptInit;
{
  hpPolylines (pDrawable, pGC, mode, npt, pptInit);
  miWideDash(pDrawable, pGC, mode, npt, pptInit);
}

void mizeroline(pDrawable, pGC, mode, npt, pptInit)
    DrawablePtr	  pDrawable;
    GCPtr   	  pGC;
    int	    	  mode, npt;
    DDXPointPtr	  pptInit;
{
  hpPolylines (pDrawable, pGC, mode, npt, pptInit);
  miZeroLine(pDrawable, pGC, mode, npt, pptInit);
}

void mipolysegment(pDraw, pGC, nseg, pSegs)
    DrawablePtr pDraw;
    GCPtr 	pGC;
    int		nseg;
    xSegment	*pSegs;
{
  hpPolySegment(pDraw, pGC, nseg, pSegs);
  miPolySegment(pDraw, pGC, nseg, pSegs);
}

void mipolyrectangle(pDraw, pGC, nrects, pRects)
    DrawablePtr	pDraw;
    GCPtr	pGC;
    int		nrects;
    xRectangle	*pRects;
{
  hpPolyRectangle(pDraw, pGC, nrects, pRects);
  miPolyRectangle(pDraw, pGC, nrects, pRects);
}

void mipolyarc(pDraw, pGC, narcs, parcs)
    DrawablePtr	pDraw;
    GCPtr	pGC;
    int		narcs;
    xArc	*parcs;
{
  hpPolyArc(pDraw, pGC, narcs, parcs);
  miPolyArc(pDraw, pGC, narcs, parcs);
}

void mifillpolygon(pDraw, pGC, shape, mode, count, pPts)
    DrawablePtr		pDraw;
    register GCPtr	pGC;
    int			shape, mode;
    register int	count;
    DDXPointPtr		pPts;
{
  hpFillPolygon(pDraw, pGC, shape, mode, count, pPts);
  miFillPolygon(pDraw, pGC, shape, mode, count, pPts);
}

void mipolyfillrect(pDrawable, pGC, nrectFill, prectInit)
    DrawablePtr pDrawable;
    GCPtr	pGC;
    int		nrectFill; 	/* number of rectangles to fill */
    xRectangle	*prectInit;  	/* Pointer to first rectangle to fill */
{
  hpPolyFillRect(pDrawable, pGC, nrectFill, prectInit);
  miPolyFillRect(pDrawable, pGC, nrectFill, prectInit);
}

void mipolyfillarc(pDraw, pGC, narcs, parcs)
    DrawablePtr	pDraw;
    GCPtr	pGC;
    int		narcs;
    xArc	*parcs;
{
  hpPolyFillArc(pDraw, pGC, narcs, parcs);
  miPolyFillArc(pDraw, pGC, narcs, parcs);
}

int mipolytext8(pDraw, pGC, x, y, count, chars)
    DrawablePtr pDraw;
    GCPtr	pGC;
    int		x, y, count;
    char	*chars;
{
  hpPolyText8(pDraw, pGC, x, y, count, chars);
  return miPolyText8(pDraw, pGC, x, y, count, chars);
}

void miimagetext8(pDraw, pGC, x, y, count, chars)
    DrawablePtr pDraw;
    GCPtr	pGC;
    int		x, y, count;
    char	*chars;
{
  hpImageText8(pDraw, pGC, x, y, count, chars);
  miImageText8(pDraw, pGC, x, y, count, chars);
}

void miimagetext16(pDraw, pGC, x, y, count, chars)
    DrawablePtr pDraw;
    GCPtr	pGC;
    int		x, y, count;
    unsigned short *chars;
{
  hpImageText16(pDraw, pGC, x, y, count, chars);
  miImageText16(pDraw, pGC, x, y, count, chars);
}

int mipolytext16(pDraw, pGC, x, y, count, chars)
    DrawablePtr pDraw;
    GCPtr	pGC;
    int		x, y, count;
    unsigned short *chars;
{
  hpPolyText16(pDraw, pGC, x, y, count, chars);
  return miPolyText16(pDraw, pGC, x, y, count, chars);
}

void miimageglyphblt(pDrawable, pGC, x, y, nglyph, ppci, pglyphBase)
    DrawablePtr pDrawable;
    GC 		*pGC;
    int 	x, y;
    unsigned int nglyph;
    CharInfoPtr *ppci;		/* array of character info */
    pointer 	pglyphBase;	/* start of array of glyphs */
{
  hpImageGlyphBlt(pDrawable, pGC, x, y, nglyph, ppci, pglyphBase);
  miImageGlyphBlt(pDrawable, pGC, x, y, nglyph, ppci, pglyphBase);
}

void mipolyglyphblt(pDrawable, pGC, x, y, nglyph, ppci, pglyphBase)
    DrawablePtr pDrawable;
    GCPtr	pGC;
    int 	x, y;
    unsigned int nglyph;
    CharInfoPtr *ppci;		/* array of character info */
    char 	*pglyphBase;	/* start of array of glyphs */
{
  hpPolyGlyphBlt(pDrawable, pGC, x, y, nglyph, ppci, pglyphBase);
  miPolyGlyphBlt(pDrawable, pGC, x, y, nglyph, ppci, pglyphBase);
}


void mipushpixels(pGC, pBitMap, pDst, w, h, x, y)
    GCPtr	pGC;
    PixmapPtr	pBitMap;
    DrawablePtr pDst;
    int		w, h, x, y;
{
  hpPushPixels(pGC, pBitMap, pDst, w, h, x, y);
  miPushPixels(pGC, pBitMap, pDst, w, h, x, y);
}

/* ******************************************************************** */
/* ***************** the mfb shadow *********************************** */
/* ******************************************************************** */

void mfbputimage(pDst, pGC, depth, x, y, w, h, leftPad, format, pBits)
    DrawablePtr	  pDst;
    GCPtr   	  pGC;
    int		  depth, x, y, w, h, format;
    char    	  *pBits;
{
  hpPutImage(pDst, pGC, depth, x, y, w, h, leftPad, format, pBits);
  mfbPutImage(pDst, pGC, depth, x, y, w, h, leftPad, format, pBits);
}

RegionPtr mfbcopyarea (pSrc, pDst, pGC, srcx, srcy, w, h, dstx, dsty)
    DrawablePtr	  pSrc, pDst;
    GCPtr   	  pGC;
    int	    	  srcx, srcy, w, h, dstx, dsty;
{
  extern RegionPtr mfbCopyArea ();
  hpCopyArea (pSrc, pDst, pGC, srcx, srcy, w, h, dstx, dsty);
  return mfbCopyArea (pSrc, pDst, pGC, srcx, srcy, w, h, dstx, dsty); 
}

RegionPtr mfbcopyplane (pSrc, pDst, pGC, srcx, srcy, w, h, dstx, dsty, plane)
    DrawablePtr	  pSrc, pDst;
    register GC   *pGC;
    int     	  srcx, srcy, w,h, dstx,dsty;
    unsigned int  plane;
{
  extern RegionPtr mfbCopyPlane ();
  hpCopyPlane (pSrc, pDst, pGC, srcx, srcy, w, h, dstx, dsty, plane);
  return mfbCopyPlane (pSrc, pDst, pGC, srcx, srcy, w, h, dstx, dsty, plane);
}

void mfbpolypoint (pDrawable, pGC, mode, npt, pptInit)
    DrawablePtr pDrawable;
    GCPtr	pGC;
    int		mode;		/* Origin or Previous */
    int		npt;
    xPoint 	*pptInit;
{
  hpPolyPoint (pDrawable, pGC, mode, npt, pptInit);
  mfbPolyPoint (pDrawable, pGC, mode, npt, pptInit);
}

void mfbpolylines(pDrawable, pGC, mode, npt, pptInit)
    DrawablePtr	  pDrawable;
    GCPtr   	  pGC;
    int	    	  mode, npt;
    DDXPointPtr	  pptInit;
{
  hpPolylines(pDrawable, pGC, mode, npt, pptInit);
  mfbLineSS(pDrawable, pGC, mode, npt, pptInit);
}

void mfbpolyfillrect(pDrawable, pGC, nrectFill, prectInit)
    DrawablePtr pDrawable;
    GCPtr	pGC;
    int		nrectFill; 	/* number of rectangles to fill */
    xRectangle	*prectInit;  	/* Pointer to first rectangle to fill */
{
  hpPolyFillRect(pDrawable, pGC, nrectFill, prectInit);
  mfbPolyFillRect(pDrawable, pGC, nrectFill, prectInit);
}

void mfbimageglyphblt(pDrawable, pGC, x, y, nglyph, ppci, pglyphBase)
    DrawablePtr pDrawable;
    GC 		*pGC;
    int 	x, y;
    unsigned int nglyph;
    CharInfoPtr *ppci;		/* array of character info */
    pointer 	pglyphBase;	/* start of array of glyphs */
{
  hpImageGlyphBlt(pDrawable, pGC, x, y, nglyph, ppci, pglyphBase);
  mfbImageGlyphBltWhite(pDrawable, pGC, x, y, nglyph, ppci, pglyphBase);
}

void mfbpolyglyphblt(pDrawable, pGC, x, y, nglyph, ppci, pglyphBase)
    DrawablePtr pDrawable;
    GCPtr	pGC;
    int 	x, y;
    unsigned int nglyph;
    CharInfoPtr *ppci;		/* array of character info */
    char 	*pglyphBase;	/* start of array of glyphs */
{
  hpPolyGlyphBlt(pDrawable, pGC, x, y, nglyph, ppci, pglyphBase);
  mfbPolyGlyphBltInvert(pDrawable, pGC, x, y, nglyph, ppci, pglyphBase);
}


void mfbpushpixels(pGC, pBitMap, pDst, w, h, x, y)
    GCPtr	pGC;
    PixmapPtr	pBitMap;
    DrawablePtr pDst;
    int		w, h, x, y;
{
  hpPushPixels(pGC, pBitMap, pDst, w, h, x, y);
  mfbPushPixels(pGC, pBitMap, pDst, w, h, x, y);
}

void mfbfillspans(pDrawable, pGC, nInit, pptInit, pwidthInit, fSorted)
    DrawablePtr pDrawable;
    GCPtr	pGC;
    int		nInit;			/* number of spans to fill */
    DDXPointPtr pptInit;		/* pointer to list of start points */
    int		*pwidthInit;		/* pointer to list of n widths */
    int 	fSorted;
{
  hpFillSpans(pDrawable, pGC, nInit, pptInit, pwidthInit, fSorted);
  mfbWhiteSolidFS(pDrawable, pGC, nInit, pptInit, pwidthInit, fSorted);
}

void mfbsetspans(pDrawable, pGC, psrc, ppt, pwidth, nspans, fSorted)
    DrawablePtr		pDrawable;
    GCPtr		pGC;
    int			*psrc, *pwidth, nspans, fSorted;
    register DDXPointPtr ppt;
{
  hpSetSpans(pDrawable, pGC, psrc, ppt, pwidth, nspans, fSorted);
  mfbSetSpans(pDrawable, pGC, psrc, ppt, pwidth, nspans, fSorted);
}

extern unsigned int *mfbGetSpans();
unsigned int *mfbgetspans(pDrawable, wMax, ppt, pwidth, nspans)
    DrawablePtr		pDrawable;	/* drawable from which to get bits */
    int			wMax;		/* largest value of all *pwidths */
    register DDXPointPtr ppt;		/* points to start copying from */
    int			*pwidth;	/* list of number of bits to copy */
    int			nspans;		/* number of scanlines to copy */
{
  hpGetSpans(pDrawable, wMax, ppt, pwidth, nspans);
  return mfbGetSpans(pDrawable, wMax, ppt, pwidth, nspans);
}

/* ******************************************************************** */
/* ***************** the hpfb shadow ********************************** */
/* ******************************************************************** */

RegionPtr hpfbcopyarea(pSrc, pDst, pGC, srcx, srcy, w, h, dstx, dsty)
    DrawablePtr	  pSrc, pDst;
    GCPtr   	  pGC;
    int	    	  srcx, srcy, w, h, dstx, dsty;
{
  extern RegionPtr hpfbCopyArea ();
  hpCopyArea(pSrc, pDst, pGC, srcx, srcy, w, h, dstx, dsty);
  return hpfbCopyArea(pSrc, pDst, pGC, srcx, srcy, w, h, dstx, dsty); 
}

void hpzerodash(pDrawable, pGC, mode, npt, pptInit)
    DrawablePtr	  pDrawable;
    GCPtr   	  pGC;
    int	    	  mode, npt;
    DDXPointPtr	  pptInit;
{
  hpPolylines (pDrawable, pGC, mode, npt, pptInit);
  hpZeroDash(pDrawable, pGC, mode, npt, pptInit);
}

void hpfbpolyfillrect(pDrawable, pGC, nrectFill, prectInit)
    DrawablePtr pDrawable;
    GCPtr	pGC;
    int		nrectFill; 	/* number of rectangles to fill */
    xRectangle	*prectInit;  	/* Pointer to first rectangle to fill */
{
  hpPolyFillRect(pDrawable, pGC, nrectFill, prectInit);
  hpfbPolyFillRect(pDrawable, pGC, nrectFill, prectInit);
}

void mrhpfbpolyfillrect(pDrawable, pGC, nrectFill, prectInit)
    DrawablePtr pDrawable;
    GCPtr	pGC;
    int		nrectFill; 	/* number of rectangles to fill */
    xRectangle	*prectInit;  	/* Pointer to first rectangle to fill */
{
  hpPolyFillRect(pDrawable, pGC, nrectFill, prectInit);
  mrhpfbPolyFillRect(pDrawable, pGC, nrectFill, prectInit);
}

/* ******************************************************************** */
/* ***************** the hp shadow ************************************ */
/* ******************************************************************** */

void hpputbyteimage(pDst, pGC, depth, x, y, w, h, leftPad, format, pBits)
    DrawablePtr	  pDst;
    GCPtr   	  pGC;
    int		  depth, x, y, w, h, format;
    char    	  *pBits;
{
  hpPutImage(pDst, pGC, depth, x, y, w, h, leftPad, format, pBits);
  hpPutByteImage(pDst, pGC, depth, x, y, w, h, leftPad, format, pBits);
}

void hpgetbyteimage(pSrc, x, y, w, h, format, planeMask, pBits)
    DrawablePtr	  pSrc;
    int	    	  x, y, w, h, *pBits;
    unsigned int  format, planeMask;
{
  hpGetImage(pSrc, x, y, w, h, format, planeMask, pBits);
  hpGetByteImage(pSrc, x, y, w, h, format, planeMask, pBits);
}

RegionPtr hpccopyarea (pSrc, pDst, pGC, srcx, srcy, w, h, dstx, dsty)
    DrawablePtr	  pSrc, pDst;
    GCPtr   	  pGC;
    int	    	  srcx, srcy, w, h, dstx, dsty;
{
  extern RegionPtr hpcCopyArea ();
  hpCopyArea (pSrc, pDst, pGC, srcx, srcy, w, h, dstx, dsty);
  return hpcCopyArea(pSrc, pDst, pGC, srcx, srcy, w, h, dstx, dsty); 
}

#if hp9000s800
RegionPtr hpcsrxcopyarea (pSrc, pDst, pGC, srcx, srcy, w, h, dstx, dsty)
    DrawablePtr	  pSrc, pDst;
    GCPtr   	  pGC;
    int	    	  srcx, srcy, w, h, dstx, dsty;
{
  extern RegionPtr hpcCopyArea ();
  hpCopyArea (pSrc, pDst, pGC, srcx, srcy, w, h, dstx, dsty);
  return hpcCopyArea(pSrc, pDst, pGC, srcx, srcy, w, h, dstx, dsty); 
}
#endif

#if 0
void hpzeroline(pDrawable, pGC, mode, npt, pptInit)
    DrawablePtr	  pDrawable;
    GCPtr   	  pGC;
    int	    	  mode, npt;
    DDXPointPtr	  pptInit;
{
  hpPolylines (pDrawable, pGC, mode, npt, pptInit);
  hpZeroLine(pDrawable, pGC, mode, npt, pptInit);
}

void hppolyfillsolidrect(pDrawable, pGC, nrectFill, prectInit)
    DrawablePtr pDrawable;
    GCPtr	pGC;
    int		nrectFill; 	/* number of rectangles to fill */
    xRectangle	*prectInit;  	/* Pointer to first rectangle to fill */
{
  hpPolyFillRect(pDrawable, pGC, nrectFill, prectInit);
  hpPolyFillSolidRect(pDrawable, pGC, nrectFill, prectInit);
}
#endif

void hppaintareasolid(pWin, pRegion, what)
    WindowPtr	pWin;
    RegionPtr	pRegion;
    int		what;
{
  hpPaintWindowBackground(pWin, pRegion, what);
  hpPaintAreaSolid(pWin, pRegion, what);
}

/* ******************************************************************** */
/* ***************** device specific shadows ************************** */
/* ******************************************************************** */

/* note: this section should be real short - device specific shadows 
 *  should be in the device directories
 */

	/* this is here 'cause its shared be topcat and catseye */
void tcpolyfillsolidrect(pDrawable, pGC, nrectFill, prectInit)
    DrawablePtr pDrawable;
    GCPtr	pGC;
    int		nrectFill; 	/* number of rectangles to fill */
    xRectangle	*prectInit;  	/* Pointer to first rectangle to fill */
{
  hpPolyFillRect(pDrawable, pGC, nrectFill, prectInit);
  tcPolyFillSolidRect(pDrawable, pGC, nrectFill, prectInit);
}

