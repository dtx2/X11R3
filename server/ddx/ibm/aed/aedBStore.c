/***********************************************************

Copyright 1987 by the Regents of the University of California
and the Massachusetts Institute of Technology, Cambridge, Massachusetts.

                        All Rights Reserved

Permission to use, copy, modify, and distribute this software and its 
documentation for any purpose and without fee is hereby granted, 
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in 
supporting documentation, and that the names of Digital or MIT not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.  
******************************************************************/
/***********************************************************
		Copyright IBM Corporation 1987

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

/* $Header: */

#include    "mfb.h"
#include    "X.h"
#include    "mibstore.h"
#include    "regionstr.h"
#include    "scrnintstr.h"
#include    "pixmapstr.h"
#include    "windowstr.h"
#include    "servermd.h"

#include    "aedHdwr.h"
#include    "ibmTrace.h"


/***====================================================================***/

static void
aedRestoreArea( scrX, scrY, width, height, pData, dWidth )
	int	scrX,	scrY;
	int	width,	height;
	char	*pData;
	int	dWidth;
{

    TRACE(("aedRestoreArea( scrX, scrY, width, height, pData, dWidth )\n"));

#ifdef NOTDEF
    vforce();
    clear(2);
    vikint[ORMERGE] = mergexlate[alu];
    vikint[ORXPOSN] = scrX;
    vikint[ORYPOSN] = scrY;
    vikint[ORCLIPLX] = scrX;
    vikint[ORCLIPLY] = scrY; 
    vikint[ORCLIPHX] = scrX+width;
    vikint[ORCLIPHY] = scrY+height;
    pbox++;
    nbox++;
    vikint[vikoff++] = 9;	/* draw image order */
    vikint[vikoff++] = width;	/* image width */
    vikint[vikoff++] = height;	/* image height */

    imagewidth = (w + 15) / 16 ;
    while ( h-- )
    {
	MOVE( (char *) pData, &vikint[vikoff], imagewidth * 2 ) ;
	VIKSTORE( vikoff , vikoff + imagewidth ) ;
	pData += dWidth ;
    }
    vforce();
    clear(2);
#endif /* NOTDEF */
    return;
}
/*-
 *-----------------------------------------------------------------------
 * aedSaveAreas --
 *	Function called by miSaveAreas to actually fetch the areas to be
 *	saved into the backing pixmap. 
 *
 * Results:
 *	None.
 *
 * Side Effects:
 *	Data are copied from the screen into the pixmap.
 *
 *-----------------------------------------------------------------------
 */
void
aedSaveAreas(pPixmap, prgnSave, xorg, yorg)
    PixmapPtr	  	pPixmap;  	/* Backing pixmap */
    RegionPtr	  	prgnSave; 	/* Region to save (pixmap-relative) */
    int	    	  	xorg;	    	/* X origin of region */
    int	    	  	yorg;	    	/* Y origin of region */
{
    BoxPtr		pBox;
    int			nBox,nRow,depth;
    int			bxWidth,pmapWidth;
    char		*pDst;
    void		(*pFn)();

    TRACE(("aedSaveAreas(0x%x,0x%x,%d,%d)\n",pPixmap,prgnSave,xorg,yorg));
    return;
}

/*-
 *-----------------------------------------------------------------------
 * aedRestoreAreas --
 *	Function called by miRestoreAreas to actually fetch the areas to be
 *	restored from the backing pixmap. 
 *
 * Results:
 *	None.
 *
 * Side Effects:
 *	Data are copied from the pixmap into the screen.
 *
 *-----------------------------------------------------------------------
 */
void
aedRestoreAreas(pPixmap, prgnRestore, xorg, yorg)
    PixmapPtr	  	pPixmap;  	/* Backing pixmap */
    RegionPtr	  	prgnRestore; 	/* Region to restore (screen-relative)*/
    int	    	  	xorg;	    	/* X origin of window */
    int	    	  	yorg;	    	/* Y origin of window */
{
    BoxPtr		pBox;
    int			nBox,nRow,depth;
    int			bxWidth,pmapWidth;
    char		*pDst;

    TRACE(("aedRestoreAreas(0x%x,0x%x,%d,%d)\n",pPixmap,prgnRestore,xorg,yorg));
    return;
}
