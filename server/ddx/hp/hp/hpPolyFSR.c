#include <sys/types.h>
#include "X.h"
#include "Xmd.h"
#include "servermd.h"
#include "gcstruct.h"
#include "window.h"
#include "pixmapstr.h"
#include "scrnintstr.h"
#include "regionstr.h"
#include "windowstr.h"
#include "Xproto.h"

#include "cfb.h"

extern u_char XHP_NewRule[16][6];

#define PIXER(Drawable)  ((cfbPrivPixmapPtr)((PixmapPtr)Drawable)->devPrivate)

/*
 * hpPaintBlockClipped
 *   This routine paints a solid-colored block clipped to the composite
 * clipping region described in the devPriv of the GC.
 * Basically it programs the replacement rule registers to place a 
 * rectangle "width" pixels wide by "height" pixels tall
 * with the appropriate color (fgPixel) and alu starting at x, y.
 *  ~jra
 *  ~hp
 *  ~cd
 *  ~sph
 */
void
hpPaintBlockClipped( dst,pGC, x,y, width,height)
DrawablePtr dst;
GC *pGC;
int x, y, width, height;
{ 
ScreenPtr pScreen = dst->pScreen;
RegionPtr pRegion = (RegionPtr)((cfbPrivGC *)pGC->devPriv)->pCompositeClip;
u_char pMask	 = getPlanesMask(pScreen);
int clippedWidth,clippedHeight, startx,starty,
	zmask = pMask & pGC->planemask,
	srcpix = pGC->fgPixel,
	alu = pGC->alu, nbox = pRegion->numRects,
	ctx = 0, cty = 0;	/* clip list translation */
register BoxPtr pBox = pRegion->rects;
register void (*bitMover)();

bitMover = ((cfbPrivScreenPtr)(pScreen->devPrivate))->MoveBits;

/* translate cliplist from pixmap coordinates to window coordinates */
 if (dst->type==DRAWABLE_PIXMAP)	/* offscreen memory pixmap */
	{
	ctx = ((hpChunk *)PIXER(dst)->pChunk)->x;
	cty = ((hpChunk *)PIXER(dst)->pChunk)->y;
	}

for (; nbox--; pBox++)
	{
	/*
	 * for each clipping rectangle, put out the portion of the
	 * block contained in the clipping rect
	 */

	clipper(x,y,width,height,
	  pBox->x1 +ctx, pBox->y1 +cty, pBox->x2 +ctx, pBox->y2 +cty,
	  &startx,&starty, &clippedWidth,&clippedHeight);

	/*
	 * put out the block part in this clip box
	 */
	if(0<clippedWidth && 0<clippedHeight)
		{
		(*bitMover)(pScreen,zmask & ~srcpix,XHP_NewRule[alu][0],0,0,
			startx,starty,clippedWidth,clippedHeight);
		(*bitMover)(pScreen,zmask & srcpix,XHP_NewRule[alu][3],0,0,
			startx,starty,clippedWidth,clippedHeight);
		}
	}
}

/*
 * hpPolyFillSolidRect -- A fast fill routine for filling rectangles
 *   in video ram with a solid color.
 * This is the pGC->PolyFillRect function when FillStyle == Solid
 */
void
hpPolyFillSolidRect(pDrawable, pGC, nrectFill, prect)
DrawablePtr	pDrawable;
GCPtr	pGC;
int nrectFill;	 	/* number of rectangles to fill */
register xRectangle *prect;  	/* Pointer to first rectangle to fill */
{
register int xorg, yorg;

/* convert rectangle to screen coordinates */
if (pDrawable->type==DRAWABLE_WINDOW)
	{
	xorg = ((WindowPtr)pDrawable)->absCorner.x;
	yorg = ((WindowPtr)pDrawable)->absCorner.y;
	}
else	/* offscreen memory pixmap */
	{
	xorg = ((hpChunk *)PIXER(pDrawable)->pChunk)->x;
	yorg = ((hpChunk *)PIXER(pDrawable)->pChunk)->y;
	}

for (; nrectFill--; prect++)
	hpPaintBlockClipped(pDrawable,pGC,
		prect->x + xorg,prect->y + yorg,prect->width,prect->height);
}

