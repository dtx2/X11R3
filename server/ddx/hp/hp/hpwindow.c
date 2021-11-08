/* 
 * hpwindow.c : window stuff
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
#include "../cfb/cfbmskbits.h"
#include "windowstr.h"

#include "shadow.h"

void hpPaintAreaSolid();

#ifdef hp9000s300
extern void miPaintWindow(), hpSaveAreas(), hpRestoreAreas();
#else hp9000s800
extern void miPaintWindow(), hpsrxSaveAreas(), hpsrxRestoreAreas();
#endif

Bool hpCreateWindow(pWin)
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

/* backing store stuff is this ever called with backing store turned on ??? */
if ((pWin->backingStore == WhenMapped) || (pWin->backingStore == Always))
	{
#ifdef hp9000s300
	miInitBackingStore(pWin, hpSaveAreas, hpRestoreAreas, (void (*)()) 0);
#else hp9000s800
	miInitBackingStore(pWin, hpsrxSaveAreas, hpsrxRestoreAreas, (void (*)()) 0);
#endif	
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
 * ~sph
 */
Bool
hpChangeWindowAttributes(pWin, mask)
WindowPtr pWin;
int mask;
{
register int index;
register cfbPrivWin *pPrivWin;

pPrivWin = (cfbPrivWin *)(pWin->devPrivate);
while(mask)
	{
	index = ffs(mask) -1;
	mask &= ~(index = 1 << index);
	switch(index)
		{
		case CWBackingStore:
	      if (pWin->backingStore != NotUseful)
	      {
#ifdef hp9000s300
                  miInitBackingStore(pWin, hpSaveAreas, hpRestoreAreas, (void (*)()) 0);
#else hp9000s800
		  miInitBackingStore(pWin, hpsrxSaveAreas, hpsrxRestoreAreas, (void (*)()) 0);
#endif
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
					pWin->PaintWindowBackground = cfbpaintareanone;
					pPrivWin->fastBackground = 0;
					break;
				case ParentRelative:
					pWin->PaintWindowBackground = cfbpaintareapr;
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
						pWin->PaintWindowBackground = mipaintwindow;
						}
					break;
				}
			break;

		case CWBackPixel:
 			pWin->PaintWindowBackground = hppaintareasolid;
			pPrivWin->fastBackground = 0;
			break;

		case CWBorderPixmap:
			switch((int)pWin->borderTile)
				{
				case None:
					pWin->PaintWindowBorder = cfbpaintareanone;
					pPrivWin->fastBorder = 0;
					break;
				case ParentRelative:
					pWin->PaintWindowBorder = cfbpaintareapr;
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
						pWin->PaintWindowBorder = cfbpaintarea32;
						}
					else
						{
						pPrivWin->fastBorder = 0;
						pWin->PaintWindowBorder = mipaintwindow;
						}
					break;
				}
			break;
		case CWBorderPixel:
			pWin->PaintWindowBorder = hppaintareasolid;
			pPrivWin->fastBorder = 0;
			break;
		}
	}
}

/*
 * hpPaintAreaSolid - solid fill a rectangle from the WindowRec
 *  called through the function pointers in the windowrec
 *  The pointers are installed by hpChangeWindowAttributes.
 *  ~hp
 *  ~sph
 */

void
hpPaintAreaSolid(pWin, pRegion, what)
WindowPtr pWin;
RegionPtr pRegion;
int what;		
{
int nbox;             /* number of boxes to fill */
register BoxPtr pbox; /* pointer to list of boxes to fill */
register int srcpix;  /* source pixel of the window */

ScreenPtr pScreen;
cfbPrivScreenPtr pPrivScreen;
int w;          /* width of current box */
register int h; /* height of current box */
register void (*bitMover)();

if ( pWin->drawable.depth != PSZ )
	FatalError( "hpPaintAreaSolid: invalid depth\n" );

if (what == PW_BACKGROUND)
	{
	srcpix = PFILL(pWin->backgroundPixel);
	} 
else
	{
	srcpix = PFILL(pWin->borderPixel);
	} 

pScreen = pWin->drawable.pScreen;
pPrivScreen = (cfbPrivScreenPtr)pScreen->devPrivate;
bitMover = pPrivScreen->MoveBits;
nbox = pRegion->numRects;
pbox = pRegion->rects;

while (nbox--)
	{
	w = pbox->x2 - pbox->x1;
	h = pbox->y2 - pbox->y1;
	(*bitMover)(pScreen,~0,GXclear,0,0,pbox->x1,pbox->y1,w,h);
	(*bitMover)(pScreen,srcpix,GXset,0,0,pbox->x1,pbox->y1,w,h);
	pbox++;
	}
}
