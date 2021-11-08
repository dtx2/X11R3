/* 
 * tcshadow.c : a sleezy cursor layer that checks for cursor conflicts.
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

RegionPtr tcCopyPlane();

RegionPtr tccopyplane (pSrc, pDst, pGC, srcx, srcy, w, h, dstx, dsty, plane)
    DrawablePtr	  pSrc, pDst;
    register GC   *pGC;
    int     	  srcx, srcy, w,h, dstx,dsty;
    unsigned int  plane;
{
  hpCopyPlane (pSrc, pDst, pGC, srcx, srcy, w, h, dstx, dsty, plane);
  return tcCopyPlane (pSrc, pDst, pGC, srcx, srcy, w, h, dstx, dsty, plane);
}
void topcatzeroline(pDrawable, pGC, mode, npt, pptInit)
    DrawablePtr	  pDrawable;
    GCPtr   	  pGC;
    int	    	  mode, npt;
    DDXPointPtr	  pptInit;
{
  hpPolylines (pDrawable, pGC, mode, npt, pptInit);
  topcatZeroLine(pDrawable, pGC, mode, npt, pptInit);
}
void topcatzerodash(pDrawable, pGC, mode, npt, pptInit)
    DrawablePtr	  pDrawable;
    GCPtr   	  pGC;
    int	    	  mode, npt;
    DDXPointPtr	  pptInit;
{
  hpPolylines (pDrawable, pGC, mode, npt, pptInit);
  topcatZeroDash(pDrawable, pGC, mode, npt, pptInit);
}

void tcimageglyphblt(pDrawable, pGC, x, y, nglyph, ppci, pglyphBase)
    DrawablePtr pDrawable;
    GC 		*pGC;
    int 	x, y;
    unsigned int nglyph;
    CharInfoPtr *ppci;		/* array of character info */
    pointer 	pglyphBase;	/* start of array of glyphs */
{
  hpImageGlyphBlt(pDrawable, pGC, x, y, nglyph, ppci, pglyphBase);
  tcImageGlyphBlt(pDrawable, pGC, x, y, nglyph, ppci, pglyphBase);
}

void tcpaintareasolid(pWin, pRegion, what)
    WindowPtr	pWin;
    RegionPtr	pRegion;
    int		what;
{
  hpPaintWindowBackground(pWin, pRegion, what);
  tcPaintAreaSolid(pWin, pRegion, what);
}

void tcputimage(pDst, pGC, depth, x,y,w,h, leftPad, format, pBits)
    DrawablePtr   pDst;
    GCPtr         pGC;
    int           depth, x, y, w, h, format;
    char          *pBits;
{
  hpPutImage(pDst, pGC, depth, x, y, w, h, leftPad, format, pBits);
  tcPutImage(pDst, pGC, depth, x, y, w, h, leftPad, format, pBits);
}

void tcsolidfs(pDrawable, pGC, nInit, pptInit, pwidthInit, fSorted)
    DrawablePtr pDrawable;
    GCPtr	pGC;
    int		nInit;			/* number of spans to fill */
    DDXPointPtr pptInit;		/* pointer to list of start points */
    int		*pwidthInit;		/* pointer to list of n widths */
    int 	fSorted;
{
  hpFillSpans(pDrawable, pGC, nInit, pptInit, pwidthInit, fSorted);
  tcSolidFS(pDrawable, pGC, nInit, pptInit, pwidthInit, fSorted);
}

void tcpolyfillstippledrect(pDrawable, pGC, nrectFill, prectInit)
    DrawablePtr pDrawable;
    GCPtr       pGC;
    int         nrectFill;      /* number of rectangles to fill */
    xRectangle  *prectInit;     /* Pointer to first rectangle to fill */
{
  hpPolyFillRect(pDrawable, pGC, nrectFill, prectInit);
  tcPolyFillStippledRect(pDrawable, pGC, nrectFill, prectInit);
}
