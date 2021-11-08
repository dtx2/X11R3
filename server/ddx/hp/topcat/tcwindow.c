/* 
 * tcwindow.c : window stuff
 */

#include <stdio.h>

#include "X.h"
#define  NEED_EVENTS
#include "Xproto.h"

#include "scrnintstr.h"
#include "pixmapstr.h"
#include "regionstr.h"
#include "mi.h"

#include "../cfb/cfb.h"
#include "topcat.h"
#include "windowstr.h"

#include "shadow.h"

extern unsigned char XHP_NewRule[16][6];
extern void hpSaveAreas(), hpRestoreAreas();

Bool tcCreateWindow(pWin)
WindowPtr pWin;
{
    cfbPrivWin *pPrivWin;

    pWin->ClearToBackground = miClearToBackground;
    pWin->PaintWindowBackground = cfbpaintareanone;
    pWin->PaintWindowBorder = cfbpaintareapr;
    pWin->CopyWindow = cfbcopywindow;
    pPrivWin = (cfbPrivWin *)Xalloc(sizeof(cfbPrivWin));
    memset ((char *) pPrivWin, 0, sizeof (cfbPrivWin));
    pWin->devPrivate = (pointer)pPrivWin;
    pPrivWin->pRotatedBorder = NullPixmap;
    pPrivWin->pRotatedBackground = NullPixmap;
    pPrivWin->fastBackground = 0;
    pPrivWin->fastBorder = 0;

    /* backing store stuff 
       is this ever called with backing store turned on ???
    */
    if ((pWin->backingStore == WhenMapped) ||
	(pWin->backingStore == Always))
    {
	miInitBackingStore(pWin, hpSaveAreas, hpRestoreAreas, (void (*)()) 0);
    }
    else
    {
	pWin->devBackingStore = (pointer)NULL;
	pWin->backStorage = (BackingStorePtr)NULL;
    }
    return TRUE;
}


 /*
  * This routine is a (very) mild modification of cfbChangeWindowAttributes.
  * The changes consist of adding checks for solid background and border
  * fills, and corresponding function vectoring.
  * If it's not a solid fill, this should behave just as the cfb routine.
  * ~hp
  */
Bool
tcChangeWindowAttributes(pWin, mask)
    WindowPtr pWin;
    int mask;
{
    register int index;
    register cfbPrivWin *pPrivWin;

    pPrivWin = (cfbPrivWin *)(pWin->devPrivate);
    while(mask)
    {
	index = lowbit (mask);
	mask &= ~index;
	switch(index)
	{
	  case CWBackingStore:
	      if (pWin->backingStore != NotUseful)
	      {
		  miInitBackingStore(pWin, hpSaveAreas, hpRestoreAreas, (void (*)()) 0);
	      }
	      else
	      {
		  miFreeBackingStore(pWin);
	      }
	      /*
	       * XXX: The changing of the backing-store status of a window
	       * is serious enough to warrant a validation, since otherwise
	       * the backing-store stuff won't work.
	       */
	      pWin->drawable.serialNumber = NEXT_SERIAL_NUMBER;
	    break;

	  case CWBackPixmap:
	      switch((int)pWin->backgroundTile)
	      {
		case None:
		  pWin->PaintWindowBackground = cfbpaintareanone; /* cfbPaintAreaNone; */
		  pPrivWin->fastBackground = 0;
		  break;
		case ParentRelative:
		  pWin->PaintWindowBackground = cfbpaintareapr;	/* cfbPaintAreaPR; */
		  pPrivWin->fastBackground = 0;
		  break;
		default:
		  if(cfbPadPixmap(pWin->backgroundTile))
		  {
		      pPrivWin->fastBackground = 1;
		      pPrivWin->oldRotate.x = pWin->absCorner.x;
		      pPrivWin->oldRotate.y = pWin->absCorner.y;
		      if (pPrivWin->pRotatedBackground)
			  cfbDestroyPixmap(pPrivWin->pRotatedBackground);
		      pPrivWin->pRotatedBackground =
			cfbCopyPixmap(pWin->backgroundTile);
		      cfbXRotatePixmap(pPrivWin->pRotatedBackground,
				    pWin->absCorner.x);
		      cfbYRotatePixmap(pPrivWin->pRotatedBackground,
				    pWin->absCorner.y);
		      pWin->PaintWindowBackground = cfbpaintarea32;
		  }
		  else
		  {
		      pPrivWin->fastBackground = 0;
		      pWin->PaintWindowBackground = mipaintwindow;	/* miPaintWindow; */
		  }
		  break;
	      }
	      break;

	  case CWBackPixel:
              pWin->PaintWindowBackground = tcpaintareasolid;  /* tcPaintAreaSolid; */
	      pPrivWin->fastBackground = 0;
	      break;

	  case CWBorderPixmap:
	      switch((int)pWin->borderTile)
	      {
		case None:
		  pWin->PaintWindowBorder = cfbpaintareanone;	/* cfbPaintAreaNone; */
		  pPrivWin->fastBorder = 0;
		  break;
		case ParentRelative:
		  pWin->PaintWindowBorder = cfbpaintareapr;	/* cfbPaintAreaPR; */
		  pPrivWin->fastBorder = 0;
		  break;
		default:
		  if(cfbPadPixmap(pWin->borderTile))
		  {
		      pPrivWin->fastBorder = 1;
		      pPrivWin->oldRotate.x = pWin->absCorner.x;
		      pPrivWin->oldRotate.y = pWin->absCorner.y;
		      if (pPrivWin->pRotatedBorder)
			  cfbDestroyPixmap(pPrivWin->pRotatedBorder);
		      pPrivWin->pRotatedBorder =
			cfbCopyPixmap(pWin->borderTile);
		      cfbXRotatePixmap(pPrivWin->pRotatedBorder,
				    pWin->absCorner.x);
		      cfbYRotatePixmap(pPrivWin->pRotatedBorder,
				    pWin->absCorner.y);
		      pWin->PaintWindowBorder = cfbpaintarea32;	/* cfbPaintArea32; */
		  }
		  else
		  {
		      pPrivWin->fastBorder = 0;
		      pWin->PaintWindowBorder = mipaintwindow;  /* miPaintWindow; */
		  }
		  break;
	      }
	      break;
	    case CWBorderPixel:
	      pWin->PaintWindowBorder = tcpaintareasolid;	/* tcPaintAreaSolid; */
	      pPrivWin->fastBorder = 0;
	      break;

	}
    }
}

/*
 * tcPaintAreaSolid - solid fill a rectangle from the WindowRec
 *  called through the function pointers in the windowrec
 *  The pointers are installed by tcChangeWindowAttributes.
 *  ~hp
 */
void
tcPaintAreaSolid(pWin, pRegion, what)
    WindowPtr pWin;
    RegionPtr pRegion;
    int what;		
{
    int nbox = pRegion->numRects;		/* number of boxes to fill */
    BoxPtr pbox = pRegion->rects;	/* pointer to list of boxes to fill */
    int srcpix;/* source pixel of the window */

    TOPCAT *hardware = getGpHardware(pWin->drawable.pScreen);
    int zmask;		/* plane mask - set to all planes */
    int XHP_bits = hardware->bits;	/* 1 if low-res topcat else 0 */

    /*
     * do I really *need* to do this check, and if so how do I get at PSZ? 
     *
    if ( pWin->drawable.depth != PSZ )
	FatalError( "cfbPaintAreaSolid: invalid depth\n" );
    */

    if (what == PW_BACKGROUND)
    {
        srcpix = pWin->backgroundPixel;
    } 
    else
    {
        srcpix = pWin->borderPixel;
    } 

    zmask = getPlanesMask((pWin->drawable.pScreen));

    while(nbox--) {
      if((pbox->x1 != pbox->x2) && (pbox->y1 != pbox->y2)) {
	waitbusy(zmask, hardware);
        hardware->write_enable = zmask & srcpix;
        hardware->window_move_replacement_rule = XHP_NewRule[GXcopy][3];
        hardware->write_enable = zmask & ~srcpix;
        hardware->window_move_replacement_rule = XHP_NewRule[GXcopy][0];
        hardware->write_enable = zmask;
        hardware->pixel_write_replacement_rule = GXcopy;

        hardware->source_x = pbox->x1 << XHP_bits;
        hardware->source_y = pbox->y1;
        hardware->dest_x = pbox->x1 << XHP_bits;
        hardware->dest_y = pbox->y1;
        hardware->window_width = (pbox->x2 - pbox->x1) << XHP_bits;
        hardware->window_height = pbox->y2 - pbox->y1;
        hardware->start_move = zmask;
      }

      pbox++;
    }
}

