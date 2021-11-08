	/* moreBS.c : more backing store stuff
	 */

#include "X.h"
#include "Xmd.h"
#include "Xproto.h"
#include "gcstruct.h"
#include "windowstr.h"

#include "mibstore.h"


    /*
     * If this GC has ever been used with a window with backing-store enabled,
     * we must call miVaidateBackingStore to keep the backing-store module
     * up-to-date, should this GC be used with that drawable again. In
     * addition, if the current drawable is a window and has backing-store
     * enabled, we also call miValidateBackingStore to give it a chance to
     * get its hooks in.
     */

void hpValidateBS(pGC, pQ, changes, pDrawable, MIBS_mask)
register GC *pGC;
GCInterestPtr *pQ;
Mask changes;
DrawablePtr pDrawable;
int MIBS_mask;	/* the MIBS whoever called this cares about */
{
	/* bail if don't care about backing store */
  if (pGC->devBackingStore!=NULL ||
        (pDrawable->type==DRAWABLE_WINDOW &&
	 ((WindowPtr)pDrawable)->backingStore!=NotUseful))
    miValidateBackingStore(pDrawable, pGC, MIBS_mask);
}



#include    "scrnintstr.h"
#include    "regionstr.h"
#include    "cfb.h"

#define PIXER(Drawable)  ((cfbPrivPixmapPtr)((PixmapPtr)Drawable)->devPrivate)

/*-
 *-----------------------------------------------------------------------
 * cfbSaveAreas --
 *	Function called by miSaveAreas to actually fetch the areas to be
 *	saved into the backing pixmap. The region to save is
 *	already destination-relative and we're given the offset to the
 *	window origin, so we have only to create an array of points of the
 *	u.l. corners of the boxes in the region translated to the screen
 *	coordinate system and fetch the screen pixmap out of its devPrivate
 *	field....
 * screen -> pixmap
 *-----------------------------------------------------------------------
 */
void hpSaveAreas(pPixmap, prgnSave, xorg,yorg)
  PixmapPtr	pPixmap;  	/* Backing pixmap */
  RegionPtr	prgnSave; 	/* Region to save (pixmap-relative) */
  int	    	xorg, yorg;    	/* X,Y origin of region */
{
  BoxRec sbox, dbox;
  register int a,b;
  GC gc;

  ((cfbPrivScreenPtr)pPixmap->drawable.pScreen->devPrivate)->CursorOff
	(pPixmap->drawable.pScreen);

  sbox.x1 = sbox.y1 = 0; sbox.x2 = sbox.y2 = 10000;
  gc.alu = GXcopy;
  gc.planemask = ~0;

  if (pPixmap->devKind==PIXMAP_FRAME_BUFFER)
  {
    a = ((hpChunk *)PIXER(pPixmap)->pChunk)->x;
    b = ((hpChunk *)PIXER(pPixmap)->pChunk)->y;

    ScreenToScreen(&pPixmap->drawable, &gc,
	xorg,yorg, sbox.x2,sbox.y2,
	0,0,
	&sbox,1, prgnSave->rects,prgnSave->numRects,
	0,0, a,b);
  }
  else
  {
    MemToMem(
	((cfbPrivScreenPtr)pPixmap->drawable.pScreen->devPrivate)->pDrawable,
	&pPixmap->drawable,
	&gc,
	xorg,yorg, sbox.x2,sbox.y2,
	0,0,
	&sbox,1, prgnSave->rects, prgnSave->numRects);
  }
}

/*-
 *-----------------------------------------------------------------------
 * cfbRestoreAreas --
 *	Function called by miRestoreAreas to actually fetch the areas to be
 *	restored from the backing pixmap. The region to restore is
 *	already destination-relative and we're given the offset to the
 *	window origin, so we have only to create an array of points of the
 *	u.l. corners of the boxes in the region translated to the pixmap
 *	coordinate system and fetch the screen pixmap out of its devPrivate
 *	field....
 * pixmap -> screen
 *-----------------------------------------------------------------------
 */
void hpRestoreAreas(pPixmap, prgnRestore, xorg,yorg)
  PixmapPtr  	pPixmap;	/* Backing pixmap */
  RegionPtr  	prgnRestore; 	/* Region to restore (screen-relative)*/
  int	   	xorg, yorg;	/* origin of window */
{
  BoxRec sbox;
  register int a,b;
  GC gc;

  ((cfbPrivScreenPtr)pPixmap->drawable.pScreen->devPrivate)->CursorOff
	(pPixmap->drawable.pScreen);

  sbox.x1 = sbox.y1 = 0;
  sbox.x2 = pPixmap->width; sbox.y2 = pPixmap->height;
  gc.alu = GXcopy;
  gc.planemask = ~0;

  if (pPixmap->devKind==PIXMAP_FRAME_BUFFER)
  {
    a = ((hpChunk *)PIXER(pPixmap)->pChunk)->x;
    b = ((hpChunk *)PIXER(pPixmap)->pChunk)->y;

    ScreenToScreen(&pPixmap->drawable, &gc,
	0,0, sbox.x2,sbox.y2, xorg,yorg,
	&sbox,1, prgnRestore->rects,prgnRestore->numRects,
	a,b, 0,0);
  }
  else
  {
    gc.pScreen = pPixmap->drawable.pScreen;
    MemToScreen(
	&pPixmap->drawable, 
	((cfbPrivScreenPtr)pPixmap->drawable.pScreen->devPrivate)->pDrawable,
	&gc,
	0,0, sbox.x2,sbox.y2, xorg,yorg,
	prgnRestore->numRects,prgnRestore->rects,
	0,0);
  }
}
