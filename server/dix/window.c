/***********************************************************
Copyright 1987 by Digital Equipment Corporation, Maynard, Massachusetts,
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

/* $XConsortium: window.c,v 1.221 88/11/11 09:52:30 rws Exp $ */

#include "X.h"
#include "Xproto.h"
#include "misc.h"
#include "scrnintstr.h"
#include "os.h"
#include "regionstr.h"
#include "windowstr.h"
#include "input.h"
#include "resource.h"
#include "colormapst.h"
#include "cursorstr.h"
#include "dixstruct.h"
#include "gcstruct.h"
#include "servermd.h"

/******
 * Window stuff for server 
 *
 *    CreateWindow, ChangeWindowAttributes,
 *    GetWindowAttributes, DeleteWindow, DestroySubWindows,
 *    HandleSaveSet, ReparentWindow, MapWindow, MapSubWindows,
 *    UnmapWindow, UnmapSubWindows, ConfigureWindow, CirculateWindow,
 *
 ******/

static unsigned char _back_lsb[4] = {0x88, 0x22, 0x44, 0x11};
static unsigned char _back_msb[4] = {0x11, 0x44, 0x22, 0x88};

#define SCREEN_IS_BLANKED   0
#define SCREEN_IS_TILED     1
#define SCREEN_ISNT_SAVED   2

extern WindowRec WindowTable[];
extern void (* ReplySwapVector[256]) ();

static void ResizeChildrenWinSize();
extern void CheckCursorConfinement();
extern void DeleteWindowFromAnySelections();
extern void DeleteWindowFromAnyEvents();
extern Mask EventMaskForClient();
extern void WindowHasNewCursor();
extern void RecalculateDeliverableEvents();
extern long random();
static Bool MarkSiblingsBelowMe();

#define INPUTONLY_LEGAL_MASK (CWWinGravity | CWEventMask | \
			      CWDontPropagate | CWOverrideRedirect | CWCursor )

#define BOXES_OVERLAP(b1, b2) \
      (!( ((b1)->x2 <= (b2)->x1)  || \
        ( ((b1)->x1 >= (b2)->x2)) || \
        ( ((b1)->y2 <= (b2)->y1)) || \
        ( ((b1)->y1 >= (b2)->y2)) ) )

/*
 * For SaveUnders using backing-store. The idea is that when a window is mapped
 * with saveUnder set TRUE, any windows it obscures will have its backing
 * store turned on by or'ing in SAVE_UNDER_BIT, thus making it != NotUseful but
 * not Always. The backing-store code must be written to allow for this
 * (i.e. it should not depend on backingStore being one of the three defined
 * constants, but should treat backingStore as a boolean. In the case of Always
 * when a window is being unmapped, it should only examine the lower three
 * bits).
 *
 * SAVE_UNDER_CHANGE_BIT is used when backing-store no longer needs to be on
 * to implement save-unders. It is used because backing-store may be thrown
 * away if turned off before the window is exposed. SAVE_UNDER_CHANGE_BIT marks
 * windows whose backing-store should be switched back to the way they were
 * before when it is safe to do so (i.e. when the window has been exposed).
 */

/*
 * this is the configuration parameter "NO_BACK_SAVE"
 * it means that any existant backing store should not 
 * be used to implement save unders.
 */

#ifndef NO_BACK_SAVE
#define DO_SAVE_UNDERS(pWin)	((pWin)->drawable.pScreen->saveUnderSupport ==\
				 SAVE_UNDER_BIT)
#endif
#ifdef DO_SAVE_UNDERS

#define SAVE_UNDER_BIT	    	0x40
#define SAVE_UNDER_CHANGE_BIT	0x20



/*-
 *-----------------------------------------------------------------------
 * CheckSubSaveUnder --
 *	Check all the inferiors of a window for coverage by saveUnder
 *	windows. Called from ChangeSaveUnder and CheckSaveUnder.
 *
 * Results:
 *	None.
 *
 * Side Effects:
 *	Windows may have backing-store turned on or off.
 *
 *-----------------------------------------------------------------------
 */
void
CheckSubSaveUnder(pParent, pRegion)
    WindowPtr	  	pParent;    	/* Parent to check */
    RegionPtr	  	pRegion;    	/* Initial area obscured by saveUnder */
{
    register WindowPtr	pChild;	    	/* Current child */
    ScreenPtr	  	pScreen;    	/* Screen to use */
    RegionPtr	  	pSubRegion; 	/* Area of children obscured */

    pScreen = pParent->drawable.pScreen;
    if (pParent->firstChild)
    {
	pSubRegion = (* pScreen->RegionCreate) (NullBox, 1);
	
	for (pChild = pParent->firstChild;
	     pChild != NullWindow;
	     pChild = pChild->nextSib)
	{
	    /*
	     * Copy the region (so it may be modified in the recursion) and
	     * check this child. Note we don't bother finding the intersection
	     * since it is probably faster for RectIn to skip over the extra
	     * rectangles than for us to find the intersection
	     */
	    if (pChild->viewable)
	    {
		(* pScreen->RegionCopy) (pSubRegion, pRegion);
		CheckSubSaveUnder(pChild, pSubRegion);

		/*
		 * If the child is a save-under window, we want it to obscure
		 * the parent as well as its siblings, so we add its extents
		 * to the obscured region.
		 */
		if (pChild->saveUnder)
		{
		    (* pScreen->Union) (pRegion, pRegion, pChild->borderSize);
		}
	    }
	}

	(* pScreen->RegionDestroy) (pSubRegion);
    }

    switch ((*pScreen->RectIn) (pRegion,
				(*pScreen->RegionExtents)(pParent->borderSize)))
    {
	case rgnOUT:
	    if (pParent->backingStore & SAVE_UNDER_BIT)
	    {
		/*
		 * Want to turn off backing store, but not until the window
		 * has had a chance to be refreshed from the backing-store,
		 * so set the change bit in backingStore and we'll actually
		 * do the change when DoChangeSaveUnder is called
		 */
		pParent->backingStore ^= (SAVE_UNDER_BIT|SAVE_UNDER_CHANGE_BIT);
	    }
	    break;
	default:
	    if (!(pParent->backingStore & SAVE_UNDER_BIT))
	    {
		pParent->backingStore |= SAVE_UNDER_BIT;
		(* pScreen->ChangeWindowAttributes) (pParent, CWBackingStore);
	    }
	    break;
    }
}

/*-
 *-----------------------------------------------------------------------
 * CheckSaveUnder --
 *	See if a window's backing-store state should be changed because
 *	it is or is not obscured by a sibling or child window with saveUnder.
 *
 * Results:
 *	None.
 *
 * Side Effects:
 *	If the window's state should be changed, it is.
 *
 *-----------------------------------------------------------------------
 */
void
CheckSaveUnder (pWin)
    register WindowPtr	pWin;	    	/* Window to check */
{
    register RegionPtr	pRegion;    	/* Extent of siblings with saveUnder */
    register WindowPtr	pSib;
    ScreenPtr	  	pScreen;

    
    pScreen = pWin->drawable.pScreen;

    pRegion = (* pScreen->RegionCreate) (NullBox, 1);

    /*
     * First form a region of all the siblings above this one that have
     * saveUnder set TRUE.
     * XXX: Should this be done all the way up the tree?
     */
    for (pSib = pWin->parent->firstChild; pSib != pWin; pSib = pSib->nextSib)
    {
	if (pSib->saveUnder && pSib->viewable)
	{
	    (* pScreen->Union) (pRegion, pRegion, pSib->borderSize);
	}
    }

    /*
     * Now find the piece of that area that overlaps this window and check
     * to make sure the window isn't obscured by children as well. Note that
     * this also takes care of any newly-obscured or -exposed inferiors
     */
    (* pScreen->Intersect) (pRegion, pRegion, pWin->borderSize);
    CheckSubSaveUnder(pWin, pRegion);
    (* pScreen->RegionDestroy) (pRegion);
}


/*-
 *-----------------------------------------------------------------------
 * ChangeSaveUnder --
 *	Change the save-under state of a tree of windows. Called when
 *	a window with saveUnder TRUE is mapped/unmapped/reconfigured.
 *	
 * Results:
 *	None.
 *
 * Side Effects:
 *	Windows may have backing-store turned on or off.
 *
 *-----------------------------------------------------------------------
 */
void
ChangeSaveUnder(pWin, first)
    WindowPtr	  	pWin;
    WindowPtr  	  	first; 	    	/* First window to check.
					 * Used when pWin was restacked */
{
    register WindowPtr	pSib;	    	/* Sibling being examined */
    register RegionPtr	saveUnder;  	/* Area obscured by saveUnder windows */
    ScreenPtr	  	pScreen;
    RegionPtr     	subSaveUnder;
    

    pScreen = pWin->drawable.pScreen;

    if (first == NullWindow)
    {
	/*
	 * If on the bottom of the heap, don't need to do anything
	 */
	return;
    }

    saveUnder = (* pScreen->RegionCreate) (NullBox, 1);

    /*
     * First form the region of save-under windows above the first window
     * to check.
     */
    for (pSib = pWin->parent->firstChild; pSib != first; pSib = pSib->nextSib)
    {
	if (pSib->saveUnder && pSib->viewable)
	{
	    (* pScreen->Union) (saveUnder, saveUnder, pSib->borderSize);
	}
    }

    subSaveUnder = (* pScreen->RegionCreate) (NullBox, 1);

    /*
     * Now check the trees of all siblings of this window, building up the
     * saveUnder area as we go along.
     */
    while (pSib != NullWindow)
    {
	if (pSib->viewable)
	{
	    (* pScreen->Intersect) (subSaveUnder, saveUnder, pSib->borderSize);
	    CheckSubSaveUnder(pSib, subSaveUnder);
	    
	    if (pSib->saveUnder)
	    {
		(* pScreen->Union) (saveUnder, saveUnder, pSib->borderSize);
	    }
	}
	pSib = pSib->nextSib;
    }

    (* pScreen->RegionDestroy) (subSaveUnder);
    (* pScreen->RegionDestroy) (saveUnder);
}
	    
/*-
 *-----------------------------------------------------------------------
 * DoChangeSaveUnder --
 *	Actually turn backing-store off for those windows that no longer
 *	need to have it on.
 *
 * Results:
 *	None.
 *
 * Side Effects:
 *	Backing-store and SAVE_UNDER_CHANGE_BIT are turned off for those
 *	windows affected.
 *
 *-----------------------------------------------------------------------
 */
void
DoChangeSaveUnder(pWin)
    WindowPtr	  	pWin;
{
    register WindowPtr	pSib;
    
    for (pSib = pWin; pSib != NullWindow; pSib = pSib->nextSib)
    {
	if (pSib->backingStore & SAVE_UNDER_CHANGE_BIT)
	{
	    pSib->backingStore &= ~SAVE_UNDER_CHANGE_BIT;
	    (*pSib->drawable.pScreen->ChangeWindowAttributes)(pSib,
							      CWBackingStore);
	}
	DoChangeSaveUnder(pSib->firstChild);
    }
}
#endif /* DO_SAVE_UNDER */

#ifdef notdef
/******
 * PrintWindowTree
 *    For debugging only
 ******/

int
PrintChildren(p1, indent)
    WindowPtr p1;
    int indent;
{
    WindowPtr p2;
    int i;
 
    while (p1) 
    {
        p2 = p1->firstChild;
        for (i=0; i<indent; i++) ErrorF( " ");
	ErrorF( "%x\n", p1->wid);
        miprintRects(p1->clipList); 
	PrintChildren(p2, indent+4);
	p1 = p1->nextSib;
    }
}

PrintWindowTree()          
{
    int i;
    WindowPtr pWin, p1;

    for (i=0; i<screenInfo.numScreens; i++)
    {
	ErrorF( "WINDOW %d\n", i);
	pWin = &WindowTable[i];
        miprintRects(pWin->clipList); 
	p1 = pWin->firstChild;
	PrintChildren(p1, 4);
    }
}
#endif

int
TraverseTree(pWin, func, data)
    WindowPtr pWin;
    int (*func)();
    pointer data;
{
    int result;
    WindowPtr pChild;

    if (pWin == (WindowPtr)NULL)
       return(WT_NOMATCH);
    result = (* func)(pWin, data);

    if (result == WT_STOPWALKING) 
        return(WT_STOPWALKING);

    if (result == WT_WALKCHILDREN) 
        for (pChild = pWin->firstChild; pChild; pChild = pChild->nextSib)
            if (TraverseTree(pChild, func,data) ==  WT_STOPWALKING) 
                return(WT_STOPWALKING);

    return(WT_NOMATCH);
}

/*****
 * WalkTree
 *   Walk the window tree, for SCREEN, preforming FUNC(pWin, data) on
 *   each window.  If FUNC returns WT_WALKCHILDREN, traverse the children,
 *   if it returns WT_DONTWALKCHILDREN, dont.  If it returns WT_STOPWALKING
 *   exit WalkTree.  Does depth-first traverse.
 *****/

int
WalkTree(pScreen, func, data)
    ScreenPtr pScreen;
    int (* func)();
    pointer data;
{
    WindowPtr pWin;
    
    pWin = &WindowTable[pScreen->myNum];
    return(TraverseTree(pWin, func, data));
}

/*****
 *  DoObscures(pWin)
 *    
 *****/

static void
DoObscures(pWin)
    WindowPtr pWin;
{
    WindowPtr pSib;

    if ((pWin->backStorage != (BackingStorePtr)NULL) &&
	(pWin->backingStore != NotUseful) &&
	(* pWin->drawable.pScreen->RegionNotEmpty)(pWin->backStorage->obscured))
    {
        (*pWin->backStorage->SaveDoomedAreas)( pWin );
        (* pWin->drawable.pScreen->RegionEmpty)(pWin->backStorage->obscured);
    }
    pSib = pWin->firstChild;
    while (pSib)
    {
        DoObscures(pSib);
	pSib = pSib->nextSib;
    }
}

/*****
 *  HandleExposures(pWin)
 *    starting at pWin, draw background in any windows that have exposure
 *    regions, translate the regions, restore any backing store,
 *    and then send any regions stille xposed to the client
 *****/

/* NOTE
   the order of painting and restoration needs to be different,
to avoid an extra repaint of the background. --rgd
*/

void
HandleExposures(pWin)
    WindowPtr pWin;
{
    WindowPtr pSib;

    if ((* pWin->drawable.pScreen->RegionNotEmpty)(pWin->borderExposed))
    {
	(*pWin->PaintWindowBorder)(pWin, pWin->borderExposed, PW_BORDER);
	(* pWin->drawable.pScreen->RegionEmpty)(pWin->borderExposed);
    }
    (* pWin->drawable.pScreen->WindowExposures)(pWin);
    pSib = pWin->firstChild;
    while (pSib)
    {
        HandleExposures(pSib);
	pSib = pSib->nextSib;
    }
}
static void InitProcedures(WindowPtr  pWin) {}
int	defaultBackingStore = NotUseful; // hack for forcing backing store on all windows
Bool disableBackingStore = FALSE; // hack to force no backing store
Bool disableSaveUnders = FALSE; // hack to force no save unders

static void SetWindowToDefaults(WindowPtr pWin, ScreenPtr pScreen) {
    pWin->prevSib = NullWindow;
    pWin->firstChild = NullWindow;
    pWin->lastChild = NullWindow;
    pWin->userProps = (PropertyPtr)NULL;
    pWin->backingStore = NotUseful;
    pWin->backStorage = (BackingStorePtr) NULL;
    pWin->devBackingStore = (pointer) NULL;
    pWin->mapped = FALSE;           /* off */
    pWin->realized = FALSE;     /* off */
    pWin->viewable = FALSE;
    pWin->visibility = VisibilityNotViewable;
    pWin->overrideRedirect = FALSE;
    pWin->saveUnder = FALSE;
    pWin->bitGravity = ForgetGravity; 
    pWin->winGravity = NorthWestGravity;
    pWin->backingBitPlanes = ~0L;
    pWin->backingPixel = 0;
    pWin->eventMask = 0;
    pWin->dontPropagateMask = 0;
    pWin->allEventMasks = 0;
    pWin->deliverableEvents = 0;
    pWin->otherClients = (pointer)NULL;
    pWin->passiveGrabs = (pointer)NULL;
    pWin->exposed = (* pScreen->RegionCreate)(NULL, 1);
    pWin->borderExposed = (* pScreen->RegionCreate)(NULL, 1);
}

static void MakeRootCursor(WindowPtr pWin) {
    unsigned char *srcbits, *mskbits;
    int i;
    if (rootCursor) {
        pWin->cursor = rootCursor;
        rootCursor->refcnt++;
    } else {
        CursorMetricRec cm;
        cm.width=32;
        cm.height=16;
        cm.xhot=8;
        cm.yhot=8;
        srcbits = (unsigned char *)xalloc( PixmapBytePad(32, 1)*16); 
        mskbits = (unsigned char *)xalloc( PixmapBytePad(32, 1)*16); 
        for (i=0; i<PixmapBytePad(32, 1)*16; i++) { srcbits[i] = mskbits[i] = 0xff; }
        pWin->cursor = AllocCursor( srcbits, mskbits,	&cm, 0xFFFF, 0xFFFF, 0xFFFF, 0, 0, 0);
    }
}
static void MakeRootTile(WindowPtr pWin) {
    ScreenPtr pScreen = pWin->drawable.pScreen;
    GCPtr pGC;
    unsigned char back[128];
    int len = PixmapBytePad(4, 1);
    register unsigned char *from, *to;
    register int i, j;
    pWin->backgroundTile = (*pScreen->CreatePixmap)(pScreen, 4, 4, pScreen->rootDepth);
    pGC = GetScratchGC(pScreen->rootDepth, pScreen);
    {
        CARD32 attributes[2];
        attributes[0] = pScreen->whitePixel;
        attributes[1] = pScreen->blackPixel;
        ChangeGC(pGC, GCForeground | GCBackground, attributes);
    }
    ValidateGC((DrawablePtr)pWin->backgroundTile, pGC);
    from = (screenInfo.bitmapBitOrder == LSBFirst) ? _back_lsb : _back_msb;
    to = back;
    for (i = 4; i > 0; i--, from++)
    for (j = len; j > 0; j--)
    *to++ = *from;
    (*pGC->PutImage)(pWin->backgroundTile, pGC, 1, 0, 0, 4, 4, 0, XYBitmap, back);
    FreeScratchGC(pGC);
}
/* Set the region to the intersection of the rectangle and the
 * window's winSize.  The window is typically the parent of the
 * window from which the region came.
 */

ClippedRegionFromBox(pWin, Rgn, x, y, w, h)
    register WindowPtr pWin;
    RegionPtr Rgn;
    int x, y, w, h;
{
    register ScreenPtr pScreen = pWin->drawable.pScreen;
    BoxRec box;

    box = *((* pScreen->RegionExtents)(pWin->winSize));
    /* we do these calculations to avoid overflows */
    if (x > box.x1)
	box.x1 = x;
    if (y > box.y1)
	box.y1 = y;
    x += w;
    if (x < box.x2)
	box.x2 = x;
    y += h;
    if (y < box.y2)
	box.y2 = y;
    if (box.x1 > box.x2)
	box.x2 = box.x1;
    if (box.y1 > box.y2)
	box.y2 = box.y1;
    (* pScreen->RegionReset)(Rgn, &box);
    (* pScreen->Intersect)(Rgn, Rgn, pWin->winSize);
}

WindowPtr RealChildHead(register WindowPtr pWin) {
	return ((WindowPtr)NULL);
}

// CreateWindow - Makes a window in response to client request 
WindowPtr CreateWindow(Window wid, WindowPtr pParent, short x, short y, unsigned short w, unsigned short h,
    unsigned short bw, unsigned short class, Mask vmask, XID *vlist, int depth, ClientPtr client, VisualID visual,
    int *error) {
    WindowPtr pWin, pHead;
    ScreenPtr pScreen;
    xEvent event;
    int idepth, ivisual;
    Bool fOK;
    DepthPtr pDepth;
    if (class == CopyFromParent) { class = pParent->class; }
    if ((class != InputOutput) && (class != InputOnly)) {
	*error = BadValue;
	client->errorValue = class;
	return (WindowPtr)NULL;
    }

    if ((class != InputOnly) && (pParent->class == InputOnly))
    {
        *error = BadMatch;
	return (WindowPtr)NULL;
    }

    if ((class == InputOnly) && ((bw != 0) || (depth != 0)))
    {
        *error = BadMatch;
	return (WindowPtr)NULL;
    }

    pScreen = pParent->drawable.pScreen;
    /* Find out if the depth and visual are acceptable for this Screen */
    fOK = FALSE;
    if ((class == InputOutput) && (depth == 0))
        depth = pParent->drawable.depth;

    if (visual == CopyFromParent)
        visual = pParent->visual;

    for(idepth = 0; idepth < pScreen->numDepths; idepth++)
    {
	pDepth = (DepthPtr) &pScreen->allowedDepths[idepth];
	if ((depth == pDepth->depth) || (depth == 0))
	{
	    for (ivisual = 0; ivisual < pDepth->numVids; ivisual++)
	    {
		if (visual == pDepth->vids[ivisual])
		{
		    fOK = TRUE;
		    break;
		}
	    }
	}
    }
    if (fOK == FALSE)
    {
	*error = BadMatch;
	return (WindowPtr)NULL;
    }

    if (((vmask & (CWBorderPixmap | CWBorderPixel)) == 0) &&
	(class != InputOnly) &&
	(depth != pParent->drawable.depth))
    {
        *error = BadMatch;
        return (WindowPtr)NULL;
    }

    if (((vmask & CWColormap) == 0) &&
	(class != InputOnly) &&
	((visual != pParent->visual) || (pParent->colormap == None)))
    {
	*error = BadMatch;
        return (WindowPtr)NULL;
    }

    pWin = (WindowPtr) xalloc( sizeof(WindowRec) );
    InitProcedures(pWin);
    pWin->drawable = pParent->drawable;
    pWin->drawable.depth = depth;
    if (class == InputOnly)
        pWin->drawable.type = (short) UNDRAWABLE_WINDOW;
    pWin->drawable.serialNumber = NEXT_SERIAL_NUMBER;

    pWin->wid = wid;
    pWin->client = client;
    pWin->visual = visual;
    pWin->class = class;

    SetWindowToDefaults(pWin, pScreen);

    if ((class == InputOnly) || (visual != pParent->visual))
	pWin->colormap = None;
    else
	pWin->colormap = pParent->colormap;

    pWin->cursor = (CursorPtr)None;

    pWin->borderWidth = (int) bw;
    pWin->backgroundTile = (PixmapPtr)None;

    if ((vmask & (CWBorderPixmap | CWBorderPixel)) != 0)
		/* it will just get fixed in ChangeWindowAttributes */
        pWin->borderTile = (PixmapPtr)USE_BORDER_PIXEL;
    else
    {
        pWin->borderTile = pParent->borderTile;   
        if (IS_VALID_PIXMAP(pParent->borderTile))
            pParent->borderTile->refcnt++;
    }
    pWin->borderPixel = pParent->borderPixel;
		
    pWin->clientWinSize.x = x + (int)bw;
    pWin->clientWinSize.y = y + (int)bw;
    pWin->clientWinSize.height = h;
    pWin->clientWinSize.width = w;
    pWin->absCorner.x = pWin->oldAbsCorner.x = pParent->absCorner.x + x + (int)bw;
    pWin->absCorner.y = pWin->oldAbsCorner.y = pParent->absCorner.y + y + (int)bw;

        /* set up clip list correctly for unobscured WindowPtr */
    pWin->clipList = (* pScreen->RegionCreate)(NULL, 1);
    pWin->borderClip = (* pScreen->RegionCreate)(NULL, 1);
    pWin->winSize = (* pScreen->RegionCreate)(NULL, 1);
    ClippedRegionFromBox(pParent, pWin->winSize,
			 pWin->absCorner.x, pWin->absCorner.y, (int)w, (int)h);
    pWin->borderSize = (* pScreen->RegionCreate)(NULL, 1);
    if (bw)
	ClippedRegionFromBox(pParent, pWin->borderSize,
		pWin->absCorner.x - (int)bw, pWin->absCorner.y - (int)bw,
		(int)(w + (bw<<1)), (int)(h + (bw<<1)));
    else
	(* pScreen->RegionCopy)(pWin->borderSize, pWin->winSize);

    pWin->parent = pParent;    
    pHead = RealChildHead(pParent);
    if (pHead)
    {
	pWin->nextSib = pHead->nextSib;
        if (pHead->nextSib)
    	    pHead->nextSib->prevSib = pWin;
	else
	    pParent->lastChild = pWin;
        pHead->nextSib = pWin;
	pWin->prevSib = pHead;
    }
    else
    {
        pWin->nextSib = pParent->firstChild;
        if (pParent->firstChild) 
	    pParent->firstChild->prevSib = pWin;
        else
            pParent->lastChild = pWin;
	pParent->firstChild = pWin;
    }

    /* We SHOULD check for an error value here XXX */
    (*pScreen->CreateWindow)(pWin);
    /* We SHOULD check for an error value here XXX */
    (*pScreen->PositionWindow)(pWin, pWin->absCorner.x, pWin->absCorner.y);
    if ((vmask & CWEventMask) == 0)
        (void)EventSelectForWindow(pWin, client, (Mask)0); /* can't fail */

    if (vmask)
        *error = ChangeWindowAttributes(pWin, vmask, vlist, pWin->client);
    else
	*error = Success;

    if (*error != Success)
    {
        (void)EventSelectForWindow(pWin, client, (Mask)0); /* can't fail */
	DeleteWindow(pWin, wid);
	return (WindowPtr)NULL;
    }
    if (!(vmask & CWBackingStore) && (defaultBackingStore != NotUseful))
    {
        XID value = defaultBackingStore;
	(void)ChangeWindowAttributes(pWin, CWBackingStore, &value, pWin->client);
    }

    WindowHasNewCursor(pWin);

    event.u.u.type = CreateNotify;
    event.u.createNotify.window = wid;
    event.u.createNotify.parent = pParent->wid;
    event.u.createNotify.x = x;
    event.u.createNotify.y = y;
    event.u.createNotify.width = w;
    event.u.createNotify.height = h;
    event.u.createNotify.borderWidth = bw;
    event.u.createNotify.override = pWin->overrideRedirect;
    DeliverEvents(pParent, &event, 1, NullWindow);		

    return pWin;
}

static void
FreeWindowResources(pWin)
    WindowPtr pWin;
{
    ScreenPtr pScreen;
    void (* proc)();

    pScreen = pWin->drawable.pScreen;

    DeleteWindowFromAnySaveSet(pWin);
    DeleteWindowFromAnySelections(pWin);
    DeleteWindowFromAnyEvents(pWin, TRUE);
    proc = pScreen->RegionDestroy;
    (* proc)(pWin->clipList);
    (* proc)(pWin->winSize);
    (* proc)(pWin->borderClip);
    (* proc)(pWin->borderSize);
    (* proc)(pWin->exposed);
    (* proc)(pWin->borderExposed);
    if (pWin->backStorage)
    {
        (* proc)(pWin->backStorage->obscured);
	xfree(pWin->backStorage);
    }
    (* pScreen->DestroyPixmap)(pWin->borderTile);
    (* pScreen->DestroyPixmap)(pWin->backgroundTile);

    if (pWin->cursor != (CursorPtr)None)
        FreeCursor( pWin->cursor, (Cursor)0);

    DeleteAllWindowProperties(pWin);
    /* We SHOULD check for an error value here XXX */
    (* pScreen->DestroyWindow)(pWin);
}

static void
CrushTree(pWin)
    WindowPtr pWin;
{
    WindowPtr pSib;
    xEvent event;

    if (pWin == (WindowPtr) NULL) 
        return;
    while (pWin) 
    {
	CrushTree(pWin->firstChild);

	event.u.u.type = DestroyNotify;
       	event.u.destroyNotify.window = pWin->wid;
	DeliverEvents(pWin, &event, 1, NullWindow);		

	FreeResource(pWin->wid, RC_CORE);
	pSib = pWin->nextSib;
	pWin->realized = FALSE;
	pWin->viewable = FALSE;
	(* pWin->drawable.pScreen->UnrealizeWindow)(pWin);
	FreeWindowResources(pWin);
	xfree(pWin);
	pWin = pSib;
    }
}
	
/*****
 *  DeleteWindow
 *       Deletes child of window then window itself
 *****/

/*ARGSUSED*/
DeleteWindow(pWin, wid)
    WindowPtr pWin;
    Window wid;
{
    WindowPtr pParent;
    xEvent event;

    UnmapWindow(pWin, HANDLE_EXPOSURES, SEND_NOTIFICATION, FALSE);

    CrushTree(pWin->firstChild);

    event.u.u.type = DestroyNotify;
    event.u.destroyNotify.window = pWin->wid;
    DeliverEvents(pWin, &event, 1, NullWindow);		

    pParent = pWin->parent;
    FreeWindowResources(pWin);
    if (pParent)
    {
	if (pParent->firstChild == pWin)
            pParent->firstChild = pWin->nextSib;
	if (pParent->lastChild == pWin)
            pParent->lastChild = pWin->prevSib;
        if (pWin->nextSib) 
            pWin->nextSib->prevSib = pWin->prevSib;
        if (pWin->prevSib) 
            pWin->prevSib->nextSib = pWin->nextSib;
	xfree(pWin);
    }
}

/*ARGSUSED*/
DestroySubwindows(pWin, client)
    WindowPtr pWin;
    ClientPtr client;
{
    register WindowPtr pChild, pSib, pHead;

    pHead = RealChildHead(pWin);
    for (pChild = pWin->lastChild; pChild != pHead; pChild = pSib)
    {
	pSib = pChild->prevSib;
	/* a little lazy evaluation, don't send exposes until all deleted */
	UnmapWindow(pChild, DONT_HANDLE_EXPOSURES, SEND_NOTIFICATION, FALSE);
	FreeResource(pChild->wid, RC_NONE);
    }
    HandleExposures(pWin);
}

/*****
 *  ChangeWindowAttributes
 *   
 *  The value-mask specifies which attributes are to be changed; the
 *  value-list contains one value for each one bit in the mask, from least
 *  to most significant bit in the mask.  
 *****/
 
int 
ChangeWindowAttributes(pWin, vmask, vlist, client)
    WindowPtr pWin;
    Mask vmask;
    XID *vlist;
    ClientPtr client;
{
    Mask index;
    XID *pVlist;
    PixmapPtr pPixmap;
    Pixmap pixID;
    CursorPtr pCursor;
    Cursor cursorID;
    int result;
    ScreenPtr pScreen;
    Mask vmaskCopy = 0;
    Mask tmask;
    unsigned int val;
    int error;

    if ((pWin->class == InputOnly) && (vmask & (~INPUTONLY_LEGAL_MASK)))
        return BadMatch;

    error = Success;
    pScreen = pWin->drawable.pScreen;
    pVlist = vlist;
    tmask = vmask;
    while (tmask) 
    {
	index = (Mask) lowbit (tmask);
	tmask &= ~index;
	switch (index) 
        {
	  case CWBackPixmap: 
	    pixID = (Pixmap )*pVlist;
	    pVlist++;
	    if (pixID == None)
	    {
		(* pScreen->DestroyPixmap)(pWin->backgroundTile);
		if (pWin->parent == (WindowPtr) NULL)
                    MakeRootTile(pWin);
                else
                    pWin->backgroundTile = (PixmapPtr)None;
	    }
	    else if (pixID == ParentRelative)
	    {
		(* pScreen->DestroyPixmap)(pWin->backgroundTile);
		if (pWin->parent == (WindowPtr) NULL)
		    MakeRootTile(pWin);
		else
	            pWin->backgroundTile = (PixmapPtr)ParentRelative;
		/* Note that the parent's backgroundTile's refcnt is NOT
		 * incremented. */
	    }
            else
	    {	
                pPixmap = (PixmapPtr)LookupID(pixID, RT_PIXMAP, RC_CORE);
                if (pPixmap != (PixmapPtr) NULL)
		{
                    if  ((pPixmap->drawable.depth != pWin->drawable.depth) ||
			 (pPixmap->drawable.pScreen != pScreen))
		    {
                        error = BadMatch;
			goto PatchUp;
		    }
		    (* pScreen->DestroyPixmap)(pWin->backgroundTile); 
		    pWin->backgroundTile = pPixmap;
		    pPixmap->refcnt++;
		}
	        else 
		{
		    error = BadPixmap;
		    client->errorValue = pixID;
		    goto PatchUp;
		}
	    }
	    break;
	  case CWBackPixel: 
	    pWin->backgroundPixel = (CARD32 ) *pVlist;
	    (* pScreen->DestroyPixmap)(pWin->backgroundTile);
	    pWin->backgroundTile = (PixmapPtr)USE_BACKGROUND_PIXEL;
	           /* background pixel overrides background pixmap,
		      so don't let the ddx layer see both bits */
            vmaskCopy &= ~CWBackPixmap;
	    pVlist++;
	    break;
	  case CWBorderPixmap:
	    pixID = (Pixmap ) *pVlist;
	    pVlist++;
	    if (pixID == CopyFromParent)
	    {
		PixmapPtr parentPixmap;
		if ((pWin->parent == (WindowPtr) NULL) ||
		    (pWin->drawable.depth != pWin->parent->drawable.depth))
		{
		    error = BadMatch;
		    goto PatchUp;
		}
		(* pScreen->DestroyPixmap)(pWin->borderTile);
		parentPixmap = pWin->parent->borderTile;
		pWin->borderTile = parentPixmap;
		if (parentPixmap == (PixmapPtr)USE_BORDER_PIXEL)
		{
		    pWin->borderPixel = pWin->parent->borderPixel;
		    index = CWBorderPixel;
		}
                else
		{
		    parentPixmap->refcnt++;
		}
	    }
	    else
	    {	
		pPixmap = (PixmapPtr)LookupID(pixID, RT_PIXMAP, RC_CORE);
		if (pPixmap) 
		{
                    if  ((pPixmap->drawable.depth != pWin->drawable.depth) ||
			 (pPixmap->drawable.pScreen != pScreen))
		    {
			error = BadMatch;
			goto PatchUp;
		    }
		    (* pScreen->DestroyPixmap)(pWin->borderTile);
		    pWin->borderTile = pPixmap;
		    pPixmap->refcnt++;
		}
    	        else
		{
		    error = BadPixmap;
		    client->errorValue = pixID;
		    goto PatchUp;
		}
	    }
	    break;
	  case CWBorderPixel: 
            pWin->borderPixel = (CARD32) *pVlist;
	    (* pScreen->DestroyPixmap)(pWin->borderTile);	    
	    pWin->borderTile = (PixmapPtr)USE_BORDER_PIXEL;
		    /* border pixel overrides border pixmap,
		       so don't let the ddx layer see both bits */
	    vmaskCopy &= ~CWBorderPixmap;
	    pVlist++;
            break;
	  case CWBitGravity: 
	    val = (CARD8 )*pVlist;
	    pVlist++;
	    if (val > StaticGravity)
	    {
		error = BadValue;
		client->errorValue = val;
		goto PatchUp;
	    }
	    pWin->bitGravity = val;
	    break;
	  case CWWinGravity: 
	    val = (CARD8 )*pVlist;
	    pVlist++;
	    if (val > StaticGravity)
	    {
		error = BadValue;
		client->errorValue = val;
		goto PatchUp;
	    }
	    pWin->winGravity = val;
	    break;
	  case CWBackingStore: 
	    val = (CARD8 )*pVlist;
	    pVlist++;
	    if ((val != NotUseful) && (val != WhenMapped) && (val != Always))
	    {
		error = BadValue;
		client->errorValue = val;
		goto PatchUp;
	    }
#ifdef DO_SAVE_UNDERS
	    /*
	     * Maintain the saveUnder bit when backing-store changed
	     */
	    pWin->backingStore = val | (pWin->backingStore & SAVE_UNDER_BIT);
#else
	    pWin->backingStore = val;
#endif /* DO_SAVE_UNDERS */
	    break;
	  case CWBackingPlanes: 
	    pWin->backingBitPlanes = (CARD32) *pVlist;
	    pVlist++;
	    break;
	  case CWBackingPixel: 
            pWin->backingPixel = (CARD32)*pVlist;
	    pVlist++;
	    break;
	  case CWSaveUnder:
	    val = (BOOL) *pVlist;
	    pVlist++;
	    if ((val != xTrue) && (val != xFalse))
	    {
		error = BadValue;
		client->errorValue = val;
		goto PatchUp;
	    }
#ifdef DO_SAVE_UNDERS
	    if ((pWin->saveUnder != val) && (pWin->viewable) &&
		DO_SAVE_UNDERS(pWin))
	    {
		/*
		 * Re-check all siblings and inferiors for obscurity or
		 * exposition (hee hee).
		 */
		pWin->saveUnder = val;
		ChangeSaveUnder(pWin, pWin->nextSib);
		DoChangeSaveUnder(pWin->nextSib);
	    }
	    else
	    {
		pWin->saveUnder = val;
	    }
#else 
	    pWin->saveUnder = val;
#endif /* DO_SAVE_UNDERS */
	    break;
	  case CWEventMask:
	    result = EventSelectForWindow(pWin, client, (Mask )*pVlist);
	    if (result)
	    {
		error = result;
		goto PatchUp;
	    }
	    pVlist++;
	    break;
	  case CWDontPropagate:
	    result =  EventSuppressForWindow(pWin, client, (Mask )*pVlist);
	    if (result)
	    {
		error = result;
		goto PatchUp;
	    }
	    pVlist++;
	    break;
	  case CWOverrideRedirect:
	    val = (BOOL ) *pVlist;
	    pVlist++;
	    if ((val != xTrue) && (val != xFalse))
	    {
		error = BadValue;
		client->errorValue = val;
		goto PatchUp;
	    }
	    pWin->overrideRedirect = val;
	    break;
	  case CWColormap:
	    {
            Colormap	cmap;
	    ColormapPtr	pCmap;
	    xEvent	xE;

	    cmap = (Colormap ) *pVlist;
	    if (cmap == CopyFromParent)
	    {
		if (pWin->parent && (pWin->visual == pWin->parent->visual))
		    cmap = pWin->parent->colormap;
		else
		    cmap = None;
	    }
	    if (cmap == None)
	    {
		error = BadMatch;
		goto PatchUp;
	    }
	    pCmap = (ColormapPtr)LookupID(cmap, RT_COLORMAP, RC_CORE);
	    if (pCmap)
	    {
	        if (pCmap->pVisual->vid != pWin->visual)
		{
		    error = BadMatch;
		    goto PatchUp;
		}
		else if (pWin->colormap != cmap)
	        { 
		    pWin->colormap = cmap;
		    xE.u.u.type = ColormapNotify;
		    xE.u.colormap.window = pWin->wid;
		    xE.u.colormap.colormap = cmap;
	            xE.u.colormap.new = xTrue;
	            xE.u.colormap.state = IsMapInstalled(cmap, pWin);
		    DeliverEvents(pWin, &xE, 1, (WindowPtr) NULL);
		}
	    }
            else
	    {
		error = BadColor;
		client->errorValue = cmap;
		goto PatchUp;
	    }
	    pVlist++;
	    break;
	    }
	  case CWCursor:
	    cursorID = (Cursor ) *pVlist;
	    pVlist++;
	    /*
	     * install the new
	     */
	    if ( cursorID == None)
	    {
	        if (pWin->cursor != (CursorPtr)None)
		    FreeCursor(pWin->cursor, None);
                if (pWin == &WindowTable[pWin->drawable.pScreen->myNum])
		   MakeRootCursor( pWin);
                else            
                    pWin->cursor = (CursorPtr)None;
	    }
            else
	    {
	        pCursor = (CursorPtr)LookupID(cursorID, RT_CURSOR, RC_CORE);
                if (pCursor) 
		{
    	            if (pWin->cursor != (CursorPtr)None)
			FreeCursor(pWin->cursor, None);
                    pWin->cursor = pCursor;
		    pCursor->refcnt++;
		}
	        else
		{
		    error = BadCursor;
		    client->errorValue = cursorID;
		    goto PatchUp;
		}
	    }
	    WindowHasNewCursor( pWin);
	    break;
	 default:
	    error = BadValue;
	    client->errorValue = vmask;
	    goto PatchUp;
      }
      vmaskCopy |= index;
    }
PatchUp:
    	/* We SHOULD check for an error value here XXX */
    (*pScreen->ChangeWindowAttributes)(pWin, vmaskCopy);

    /* 
        If the border pixel changed, redraw the border. 
	Note that this has to be done AFTER pScreen->ChangeWindowAttributes
        for the tile to be rotated, and the correct function selected.
    */
    if ((vmaskCopy & (CWBorderPixel | CWBorderPixmap)) 
	&& pWin->viewable && pWin->borderWidth)
    {
        (* pScreen->Subtract)(pWin->borderExposed, pWin->borderClip, 
			      pWin->winSize);
	(*pWin->PaintWindowBorder)(pWin, pWin->borderExposed, PW_BORDER);
        (* pScreen->RegionEmpty)(pWin->borderExposed);
    }
    return error;
}


/*****
 * GetWindowAttributes
 *    Notice that this is different than ChangeWindowAttributes
 *****/

GetWindowAttributes(pWin, client)
    WindowPtr pWin;
    ClientPtr client;
{
    xGetWindowAttributesReply wa;

    wa.type = X_Reply;
    wa.bitGravity = pWin->bitGravity;
    wa.winGravity = pWin->winGravity;
#ifdef DO_SAVE_UNDERS
    wa.backingStore = pWin->backingStore & ~(SAVE_UNDER_BIT);
#else
    wa.backingStore  = pWin->backingStore;
#endif /* DO_SAVE_UNDERS */
    wa.length = (sizeof(xGetWindowAttributesReply) - 
		 sizeof(xGenericReply)) >> 2;
    wa.sequenceNumber = client->sequence;
    wa.backingBitPlanes =  pWin->backingBitPlanes;
    wa.backingPixel =  pWin->backingPixel;
    wa.saveUnder = (BOOL)pWin->saveUnder;
    wa.override = pWin->overrideRedirect;
    if (!pWin->mapped)
        wa.mapState = IsUnmapped;
    else if (pWin->realized)
        wa.mapState = IsViewable;
    else
        wa.mapState = IsUnviewable;

    wa.colormap =  pWin->colormap;
    wa.mapInstalled = (wa.colormap == None) ? xFalse
					    : IsMapInstalled(wa.colormap, pWin);

    wa.yourEventMask = EventMaskForClient(pWin, client, &wa.allEventMasks);
    wa.doNotPropagateMask = pWin->dontPropagateMask ;
    wa.class = pWin->class;
    wa.visualID = pWin->visual;

    WriteReplyToClient(client, sizeof(xGetWindowAttributesReply), &wa);
}


static WindowPtr
MoveWindowInStack(pWin, pNextSib)
    WindowPtr pWin, pNextSib;
{
    WindowPtr pParent = pWin->parent;
    WindowPtr pFirstChange = pWin; /* highest window where list changes */

    if (pWin->nextSib != pNextSib)
    {
        if (!pNextSib)        /* move to bottom */
	{
            if (pParent->firstChild == pWin)
                pParent->firstChild = pWin->nextSib;
	    /* if (pWin->nextSib) */	 /* is always True: pNextSib == NULL
				          * and pWin->nextSib != pNextSib
					  * therefore pWin->nextSib != NULL */
	        pFirstChange = pWin->nextSib;
		pWin->nextSib->prevSib = pWin->prevSib;
	    /* else pFirstChange = pWin; */	
	    if (pWin->prevSib) 
                pWin->prevSib->nextSib = pWin->nextSib;
            pParent->lastChild->nextSib = pWin;
            pWin->prevSib = pParent->lastChild;
            pWin->nextSib = (WindowPtr )NULL;
            pParent->lastChild = pWin;
	}
        else if (pParent->firstChild == pNextSib) /* move to top */
        {        
	    pFirstChange = pWin;
	    if (pParent->lastChild == pWin)
    	       pParent->lastChild = pWin->prevSib;
	    if (pWin->nextSib) 
		pWin->nextSib->prevSib = pWin->prevSib;
	    if (pWin->prevSib) 
                pWin->prevSib->nextSib = pWin->nextSib;
	    pWin->nextSib = pParent->firstChild;
	    pWin->prevSib = (WindowPtr ) NULL;
	    pNextSib->prevSib = pWin;
	    pParent->firstChild = pWin;
	}
        else			/* move in middle of list */
        {
	    WindowPtr pOldNext = pWin->nextSib;

	    pFirstChange = (WindowPtr )NULL;
            if (pParent->firstChild == pWin)
                pFirstChange = pParent->firstChild = pWin->nextSib;
	    if (pParent->lastChild == pWin) {
	       pFirstChange = pWin;
    	       pParent->lastChild = pWin->prevSib;
	    }
	    if (pWin->nextSib) 
		pWin->nextSib->prevSib = pWin->prevSib;
	    if (pWin->prevSib) 
                pWin->prevSib->nextSib = pWin->nextSib;
            pWin->nextSib = pNextSib;
            pWin->prevSib = pNextSib->prevSib;
	    if (pNextSib->prevSib)
                pNextSib->prevSib->nextSib = pWin;
            pNextSib->prevSib = pWin;
	    if (!pFirstChange) {		     /* do we know it yet? */
	        pFirstChange = pParent->firstChild;  /* no, search from top */
	        while ((pFirstChange != pWin) && (pFirstChange != pOldNext))
		     pFirstChange = pFirstChange->nextSib;
	    }
	}
    }

    return( pFirstChange );
}

static void
MoveWindow(pWin, x, y, pNextSib)
    WindowPtr pWin;
    short x,y;
    WindowPtr pNextSib;
{
    WindowPtr pParent;
    Bool WasViewable = (Bool)(pWin->viewable);
    short oldx, oldy, bw;
    RegionPtr oldRegion;
    DDXPointRec oldpt;
    Bool anyMarked;
    register ScreenPtr pScreen;
    BoxPtr pBox;
    WindowPtr windowToValidate = pWin;

    /* if this is a root window, can't be moved */
    if (!(pParent = pWin->parent)) 
       return ;
    pScreen = pWin->drawable.pScreen;
    bw = pWin->borderWidth;

    oldx = pWin->absCorner.x;
    oldy = pWin->absCorner.y;
    oldpt.x = oldx;
    oldpt.y = oldy;
    if (WasViewable)
    {
        oldRegion = (* pScreen->RegionCreate)(NULL, 1);
        (* pScreen->RegionCopy)(oldRegion, pWin->borderClip);
        pBox = (* pScreen->RegionExtents)(pWin->borderSize);
	anyMarked = MarkSiblingsBelowMe(pWin, pBox);
    }
    pWin->clientWinSize.x = x + (int)bw;
    pWin->clientWinSize.y = y + (int)bw;
    pWin->oldAbsCorner.x = oldx;
    pWin->oldAbsCorner.y = oldy;
    pWin->absCorner.x = pParent->absCorner.x + x + (int)bw;
    pWin->absCorner.y = pParent->absCorner.y + y + (int)bw;

    ClippedRegionFromBox(pParent, pWin->winSize,
			 pWin->absCorner.x, pWin->absCorner.y,
			 (int)pWin->clientWinSize.width,
			 (int)pWin->clientWinSize.height);

    if (bw)
	ClippedRegionFromBox(pParent, pWin->borderSize,
			     pWin->absCorner.x - bw, pWin->absCorner.y - bw,
			     (int)pWin->clientWinSize.width + (bw<<1),
			     (int)pWin->clientWinSize.height + (bw<<1));
    else
        (* pScreen->RegionCopy)(pWin->borderSize, pWin->winSize);

    (* pScreen->PositionWindow)(pWin,pWin->absCorner.x, pWin->absCorner.y);

    windowToValidate = MoveWindowInStack(pWin, pNextSib);

    ResizeChildrenWinSize(pWin, 0, 0, 0, 0);
    if (WasViewable)
    {

        anyMarked = MarkSiblingsBelowMe(windowToValidate, pBox) || anyMarked;
#ifdef DO_SAVE_UNDERS
	if (DO_SAVE_UNDERS(pWin))
	{
	    if (pWin->saveUnder)
	    {
		ChangeSaveUnder(pWin, windowToValidate);
	    }
	    else
	    {
		CheckSaveUnder(pWin);
	    }
	}
#endif /* DO_SAVE_UNDERS */

        (* pScreen->ValidateTree)(pParent, (WindowPtr)NULL, TRUE, anyMarked);
	
	DoObscures(pParent); 
	(* pWin->CopyWindow)(pWin, oldpt, oldRegion);
	(* pScreen->RegionDestroy)(oldRegion);
	/* XXX need to retile border if ParentRelative origin */
	HandleExposures(pParent); 
#ifdef DO_SAVE_UNDERS
	if (DO_SAVE_UNDERS(pWin))
	{
	    DoChangeSaveUnder(windowToValidate);
	}
#endif /* DO_SAVE_UNDERS */
    } 
    if (pWin->realized)
	WindowsRestructured ();
}

static void
gravityTranslate (x, y, oldx, oldy, dw, dh, gravity, destx, desty)
int	x, y;		/* new window position */
int	oldx, oldy;	/* old window position */
int	gravity;
int	*destx, *desty;	/* position relative to gravity */
{
    switch (gravity) {
    case NorthWestGravity: 
	*destx = x;
	*desty = y;
	break;
    case NorthGravity:  
	*destx = x + dw/2;
	*desty = y;
	break;
    case NorthEastGravity:    
	*destx = x + dw;	     
	*desty = y;
	break;
    case WestGravity:         
	*destx = x;
	*desty = y + dh/2;
	break;
    case CenterGravity:    
	*destx = x + dw/2;
	*desty = y + dh/2;
	break;
    case EastGravity:         
	*destx = x + dw;
	*desty = y + dh/2;
	break;
    case SouthWestGravity:    
	*destx = x;
	*desty = y + dh;
	break;
    case SouthGravity:        
	*destx = x + dw/2;
	*desty = y + dh;
	break;
    case SouthEastGravity:    
	*destx = x + dw;
	*desty = y + dh;
	break;
    case StaticGravity:
	*destx = oldx;
	*desty = oldy;
	break;
    default:
	*destx = x;
	*desty = y;
	break;
    }
}

/* XXX need to retile border on each window with ParentRelative origin */
static void
ResizeChildrenWinSize(pWin, dx, dy, dw, dh)
    WindowPtr pWin;
    int dx, dy, dw, dh;
{
    register WindowPtr pSib;
    short x, y;
    int	cwsx, cwsy;
    Bool unmap = FALSE;
    register ScreenPtr pScreen;
    xEvent event;

    pScreen = pWin->drawable.pScreen;
    pSib = pWin->firstChild;
    x = pWin->absCorner.x;
    y = pWin->absCorner.y;

    while (pSib) 
    {
	cwsx = pSib->clientWinSize.x;
	cwsy = pSib->clientWinSize.y;
        if (dw || dh)
        {
	    if (pSib->winGravity == UnmapGravity)
	        unmap = TRUE;
	    gravityTranslate (cwsx, cwsy, cwsx - dx, cwsx - dy, dw, dh,
			pSib->winGravity, &cwsx, &cwsy);
	    if (cwsx != pSib->clientWinSize.x || cwsy != pSib->clientWinSize.y)
	    {
		event.u.u.type = GravityNotify;
		event.u.gravity.window = pSib->wid;
		event.u.gravity.x = cwsx - pSib->borderWidth;
		event.u.gravity.y = cwsy - pSib->borderWidth;
		DeliverEvents (pSib, &event, 1, NullWindow);
		pSib->clientWinSize.x = cwsx;
		pSib->clientWinSize.y = cwsy;
	    }
	}

	pSib->oldAbsCorner.x = pSib->absCorner.x;
	pSib->oldAbsCorner.y = pSib->absCorner.y;
	pSib->absCorner.x = x + cwsx;
	pSib->absCorner.y = y + cwsy;

	ClippedRegionFromBox(pWin, pSib->winSize,
			     pSib->absCorner.x, pSib->absCorner.y,
			     (int)pSib->clientWinSize.width,
			     (int)pSib->clientWinSize.height);

	if (pSib->borderWidth)
	    ClippedRegionFromBox(pWin, pSib->borderSize,
		pSib->absCorner.x - pSib->borderWidth,
		pSib->absCorner.y - pSib->borderWidth,
		(int)pSib->clientWinSize.width + (pSib->borderWidth<<1),
		(int)pSib->clientWinSize.height + (pSib->borderWidth<<1));
	else
	    (* pScreen->RegionCopy)(pSib->borderSize, pSib->winSize);
	(* pScreen->PositionWindow)(pSib, pSib->absCorner.x, pSib->absCorner.y);
	pSib->marked = 1;
	if (pSib->firstChild) 
            ResizeChildrenWinSize(pSib, 0, 0, 0, 0);
        if (unmap)
	{
            UnmapWindow(pSib, DONT_HANDLE_EXPOSURES, SEND_NOTIFICATION,	TRUE);
	    unmap = FALSE;
	}
        pSib = pSib->nextSib;
    }
}

static int
ExposeAll(pWin, pScreen)
    WindowPtr pWin;
    ScreenPtr pScreen;
{
    if (!pWin)
        return(WT_NOMATCH);
    if (pWin->mapped)
    {
        (* pScreen->RegionCopy)(pWin->exposed, pWin->clipList);
        return (WT_WALKCHILDREN);
    }
    else
        return(WT_NOMATCH);
}

/*
 * pValid is a region of the screen which has been
 * successfully copied -- recomputed exposed regions for affected windows
 */

static int
RecomputeExposures (pWin, pValid)
    WindowPtr	pWin;
    RegionPtr	pValid;
{
    ScreenPtr	pScreen;

    if (pWin->viewable)
    {
	pScreen = pWin->drawable.pScreen;
	/*
	 * compute exposed regions of this window
	 */
	(*pScreen->Subtract) (pWin->exposed, pWin->clipList, pValid);
	/*
	 * compute exposed regions of the border
	 */
	(*pScreen->Subtract) (pWin->borderExposed,
 				pWin->borderClip, pWin->winSize);
	(*pScreen->Subtract) (pWin->borderExposed,
 				pWin->borderExposed, pValid);
	return WT_WALKCHILDREN;
    }
    return WT_NOMATCH;
}


static void
SlideAndSizeWindow(pWin, x, y, w, h, pSib)
    WindowPtr pWin;
    short x,y;
    unsigned short w, h;
    WindowPtr pSib;
{
    WindowPtr pParent;
    Bool WasViewable = (Bool)(pWin->viewable);
    unsigned short width = pWin->clientWinSize.width,
                   height = pWin->clientWinSize.height;    
    short oldx = pWin->absCorner.x,
          oldy = pWin->absCorner.y;
    int bw = pWin->borderWidth;
    short dw, dh;
    DDXPointRec oldpt;
    RegionPtr oldRegion;
    Bool anyMarked;
    register ScreenPtr pScreen;
    BoxPtr pBox;
    WindowPtr pFirstChange;
    WindowPtr pChild;
    RegionPtr	gravitate[StaticGravity + 1];
    int		g;
    int		nx, ny;		/* destination x,y */
    RegionPtr	pRegion;
    RegionPtr	destClip;	/* portions of destination already written */
    RegionPtr	oldWinClip;	/* old clip list for window */

    /* if this is a root window, can't be resized */
    if (!(pParent = pWin->parent)) 
        return ;

    pScreen = pWin->drawable.pScreen;
    if (WasViewable)
    {
	/*
	 * save the visible region of the window
	 */
	oldRegion = (*pScreen->RegionCreate) (NULL, 1);
	(*pScreen->RegionCopy) (oldRegion, pWin->winSize);

        pBox = (* pScreen->RegionExtents)(pWin->borderSize);
	anyMarked = MarkSiblingsBelowMe(pWin, pBox);

	/*
	 * catagorize child windows into regions to be moved
	 */
	for (g = 0; g <= StaticGravity; g++)
	    gravitate[g] = (RegionPtr) NULL;
	for (pChild = pWin->firstChild; pChild; pChild = pChild->nextSib)
	{
	    g = pChild->winGravity;
	    if (g != UnmapGravity)
	    {
		if (!gravitate[g])
		    gravitate[g] = (*pScreen->RegionCreate) (NULL, 1);
		(*pScreen->Union) (gravitate[g], gravitate[g], pChild->borderClip);
	    }
	}
	oldWinClip = NULL;
	if (pWin->bitGravity != ForgetGravity)
	{
	    oldWinClip = (*pScreen->RegionCreate) (NULL, 1);
	    (*pScreen->RegionCopy) (oldWinClip, pWin->clipList);
	}
    }
    pWin->clientWinSize.x = x + bw;
    pWin->clientWinSize.y = y + bw;
    pWin->clientWinSize.height = h;
    pWin->clientWinSize.width = w;
    pWin->oldAbsCorner.x = oldx;
    pWin->oldAbsCorner.y = oldy;
    oldpt.x = oldx;
    oldpt.y = oldy;

    x = pWin->absCorner.x = pParent->absCorner.x + x + bw;
    y = pWin->absCorner.y = pParent->absCorner.y + y + bw;

    ClippedRegionFromBox(pParent, pWin->winSize,
			 x, y, (int)w, (int)h);

    if (pWin->borderWidth)
	ClippedRegionFromBox(pParent, pWin->borderSize,
			     x - bw, y - bw, (int)w + (bw<<1), (int)h + (bw<<1));
    else
        (* pScreen->RegionCopy)(pWin->borderSize, pWin->winSize);

    dw = (int)w - (int)width;
    dh = (int)h - (int)height;
    ResizeChildrenWinSize(pWin, x - oldx, y - oldy, dw, dh);

    /* let the hardware adjust background and border pixmaps, if any */
    (* pScreen->PositionWindow)(pWin, pWin->absCorner.x, pWin->absCorner.y);

    pFirstChange = MoveWindowInStack(pWin, pSib);

    if (WasViewable)
    {
	pRegion = (*pScreen->RegionCreate) (NULL, 1);
	if (pWin->backStorage && (pWin->backingStore != NotUseful))
	    (*pScreen->RegionCopy) (pRegion, pWin->clipList);

	anyMarked = MarkSiblingsBelowMe(pFirstChange, pBox) || anyMarked;
#ifdef DO_SAVE_UNDERS
	if (DO_SAVE_UNDERS(pWin))
	{
	    if (pWin->saveUnder)
	    {
		ChangeSaveUnder(pWin, pFirstChange);
	    }
	    else
	    {
		CheckSaveUnder(pWin);
	    }
	}
#endif /* DO_SAVE_UNDERS */

	(* pScreen->ValidateTree)(pParent, pFirstChange, TRUE, anyMarked);

	DoObscures(pParent); 

	/*
	 * always redraw the border as CopyWindow will mash it
	 */

	(* pScreen->Subtract)(pWin->borderExposed, 
				pWin->borderClip, pWin->winSize);

	(* pScreen->RegionCopy) (pWin->exposed, pWin->clipList);
    }
 
    if (pWin->backStorage &&
	((pWin->backingStore == Always) ||
	 (WasViewable && (pWin->backingStore != NotUseful))))
    {
	if (!WasViewable)
	    pRegion = pWin->clipList; /* a convenient empty region */
	if (pWin->bitGravity == ForgetGravity)
	    (* pWin->backStorage->TranslateBackingStore) (pWin, 0, 0,
						      (RegionPtr)NULL);
	else
	    (* pWin->backStorage->TranslateBackingStore) (pWin, 
							  x - oldx,
							  y - oldy,
							  pRegion);
    }

    if (WasViewable)
    {
	/*
	 * add screen bits to the appropriate bucket
	 */

	if (oldWinClip)
	{
	    /*
	     * clip to new clipList
	     */
	    gravityTranslate (x, y, oldx, oldy, dw, dh, pWin->bitGravity, &nx, &ny);
	    (*pScreen->RegionCopy) (pRegion, oldWinClip);
	    (*pScreen->TranslateRegion) (pRegion, nx - oldx, ny - oldy);
	    (*pScreen->Intersect) (oldWinClip, pRegion, pWin->clipList);
	    /*
	     * don't step on any gravity bits which will be copied after this
	     * region.  Note -- this assumes that the regions will be copied
	     * in gravity order.
	     */
	    for (g = pWin->bitGravity + 1; g <= StaticGravity; g++)
	    {
		if (gravitate[g])
		    (*pScreen->Subtract) (oldWinClip, oldWinClip, gravitate[g]);
	    }
	    (*pScreen->TranslateRegion) (oldWinClip, oldx - nx, oldy - ny);
	    g = pWin->bitGravity;
	    if (!gravitate[g])
		gravitate[g] = oldWinClip;
	    else
	    {
		(*pScreen->Union) (gravitate[g], gravitate[g], oldWinClip);
		(*pScreen->RegionDestroy) (oldWinClip);
	    }
	}

	/*
	 * move the bits on the screen
	 */

	destClip = NULL;

	for (g = 0; g <= StaticGravity; g++)
	{
	    if (!gravitate[g])
	    	continue;

	    gravityTranslate (x, y, oldx, oldy, dw, dh, g, &nx, &ny);

            oldpt.x = oldx + (x - nx);
	    oldpt.y = oldy + (y - ny);

	    /* Note that gravitate[g] is *translated* by CopyWindow */

	    /* only copy the remaining useful bits */

	    (*pScreen->Intersect) (gravitate[g], gravitate[g], oldRegion);
	    
	    /* clip to not overwrite already copied areas */

	    if (destClip) {
		(*pScreen->TranslateRegion) (destClip, oldpt.x - x, oldpt.y - y);
		(*pScreen->Subtract) (gravitate[g], gravitate[g], destClip);
		(*pScreen->TranslateRegion) (destClip, x - oldpt.x, y - oldpt.y);
	    }

	    /* and move those bits */
	    
	    if (oldpt.x != x || oldpt.y != y)
		(*pWin->CopyWindow)(pWin, oldpt, gravitate[g]);

	    /* remove any overwritten bits from the remaining useful bits */

	    (*pScreen->Subtract) (oldRegion, oldRegion, gravitate[g]);

	    /*
	     * recompute exposed regions of child windows
	     */
	
	    for (pChild = pWin->firstChild; pChild; pChild = pChild->nextSib)
	    {
		if (pChild->winGravity != g)
		    continue;
		(*pScreen->Intersect) (pRegion, pChild->borderClip, gravitate[g]);
		TraverseTree (pChild, RecomputeExposures, pRegion);
	    }

	    /*
	     * remove the successfully copied regions of the
	     * window from its exposed region
	     */

	    if (g == pWin->bitGravity)
		(*pScreen->Subtract) (pWin->exposed, pWin->exposed, gravitate[g]);
	    if (!destClip)
	    	destClip = gravitate[g];
	    else
	    {
		(*pScreen->Union) (destClip, destClip, gravitate[g]);
		(*pScreen->RegionDestroy) (gravitate[g]);
	    }
	}

	(*pScreen->RegionDestroy) (oldRegion);
	(*pScreen->RegionDestroy) (pRegion);
	if (destClip)
	    (*pScreen->RegionDestroy) (destClip);

	HandleExposures(pParent);
#ifdef DO_SAVE_UNDERS
	if (DO_SAVE_UNDERS(pWin))
	{
	    DoChangeSaveUnder(pFirstChange);
	}
#endif /* DO_SAVE_UNDERS */
    }
    if (pWin->realized)
	WindowsRestructured ();
}

/* Keeps the same inside(!) origin */

static void
ChangeBorderWidth(pWin, width)
    WindowPtr pWin;
    unsigned short width;
{
    WindowPtr pParent;
    BoxPtr pBox;
    int oldwidth;
    Bool anyMarked;
    register ScreenPtr pScreen;
    Bool WasViewable = (Bool)(pWin->viewable);

    oldwidth = pWin->borderWidth;
    if (oldwidth == width) 
        return ;
    pScreen = pWin->drawable.pScreen;
    pParent = pWin->parent;
    pWin->borderWidth = width;

    if (width)
	ClippedRegionFromBox(pParent, pWin->borderSize,
	    pWin->absCorner.x - (int)width, pWin->absCorner.y - (int)width,
	    (int)(pWin->clientWinSize.width + (width<<1)),
	    (int)(pWin->clientWinSize.height + (width<<1)));
    else
        (* pScreen->RegionCopy)(pWin->borderSize, pWin->winSize);

    if (WasViewable)
    {
        if (width < oldwidth)
            pBox = (* pScreen->RegionExtents)(pWin->borderClip);
        else        
            pBox = (* pScreen->RegionExtents)(pWin->borderSize);
        anyMarked = MarkSiblingsBelowMe(pWin, pBox);
#ifdef DO_SAVE_UNDERS
	if (pWin->saveUnder && DO_SAVE_UNDERS(pWin))
	{
	    ChangeSaveUnder(pWin, pWin->nextSib);
	}
#endif /* DO_SAVE_UNDERS */

        (* pScreen->ValidateTree)(pParent,(anyMarked ? pWin : (WindowPtr)NULL),
					     TRUE, anyMarked );  

        if (width > oldwidth)
	{
            DoObscures(pParent);
	    (* pScreen->Subtract)(pWin->borderExposed,
				  pWin->borderClip, pWin->winSize);
	    (* pWin->PaintWindowBorder)(pWin, pWin->borderExposed, PW_BORDER);
	    (* pScreen->RegionEmpty)(pWin->borderExposed);
	}
	else
            HandleExposures(pParent);
#ifdef DO_SAVE_UNDERS
	if (DO_SAVE_UNDERS(pWin))
	{
	    DoChangeSaveUnder(pWin->nextSib);
	}
#endif /* DO_SAVE_UNDERS */
    }
    if (pWin->realized)
	WindowsRestructured ();
}


#define GET_INT16(m, f) \
  	if (m & mask) \
          { \
             f = (INT16) *pVlist;\
 	    pVlist++; \
         }
#define GET_CARD16(m, f) \
 	if (m & mask) \
         { \
            f = (CARD16) *pVlist;\
 	    pVlist++;\
         }

#define GET_CARD8(m, f) \
 	if (m & mask) \
         { \
            f = (CARD8) *pVlist;\
 	    pVlist++;\
         }

#define ChangeMask ((Mask)(CWX | CWY | CWWidth | CWHeight))

#define IllegalInputOnlyConfigureMask (CWBorderWidth)

/*
 * IsSiblingAboveMe
 *     returns Above if pSib above pMe in stack or Below otherwise 
 */

static int
IsSiblingAboveMe(pMe, pSib)
    WindowPtr pMe, pSib;
{
    WindowPtr pWin;

    pWin = pMe->parent->firstChild;
    while (pWin)
    {
        if (pWin == pSib)
            return(Above);
        else if (pWin == pMe)
            return(Below);
        pWin = pWin->nextSib;
    }
    return(Below);
}

static BoxPtr
WindowExtents(pWin, pBox)
	WindowPtr pWin;
	BoxPtr pBox;
{
	pBox->x1 = pWin->clientWinSize.x - pWin->borderWidth;
	pBox->y1 = pWin->clientWinSize.y - pWin->borderWidth;
	pBox->x2 = pWin->clientWinSize.x + (int)pWin->clientWinSize.width
		   + pWin->borderWidth;
	pBox->y2 = pWin->clientWinSize.y + (int)pWin->clientWinSize.height
		   + pWin->borderWidth;
	return(pBox);
}


static Bool
AnyWindowOverlapsMe(pWin, pHead, box)
    WindowPtr pWin, pHead;
    register BoxPtr box;
{
    WindowPtr pSib;
    BoxRec sboxrec;
    register BoxPtr sbox;

    for (pSib = pWin->prevSib; pSib != pHead; pSib = pSib->prevSib)
    {
	if (pSib->mapped)
	{
	    sbox = WindowExtents(pSib, &sboxrec);
	    if BOXES_OVERLAP(sbox, box)
		return(TRUE);
	}
    }
    return(FALSE);
}

static Bool
IOverlapAnyWindow(pWin, box)
    WindowPtr pWin;
    register BoxPtr box;
{
    WindowPtr pSib;
    BoxRec sboxrec;
    register BoxPtr sbox;

    for (pSib = pWin->nextSib; pSib; pSib = pSib->nextSib)
    {
	if (pSib->mapped)
	{
	    sbox = WindowExtents(pSib, &sboxrec);
	    if BOXES_OVERLAP(sbox, box)
		return(TRUE);
	}
    }
    return(FALSE);
}

/*
 *   WhereDoIGoInTheStack() 
 *        Given pWin and pSib and the relationshipe smode, return
 *        the window that pWin should go ABOVE.
 *        If a pSib is specified:
 *            Above:  pWin is placed just above pSib
 *            Below:  pWin is placed just below pSib
 *            TopIf:  if pSib occludes pWin, then pWin is placed
 *                    at the top of the stack
 *            BottomIf:  if pWin occludes pSib, then pWin is 
 *                       placed at the bottom of the stack
 *            Opposite: if pSib occludes pWin, then pWin is placed at the
 *                      top of the stack, else if pWin occludes pSib, then
 *                      pWin is placed at the bottom of the stack
 *
 *        If pSib is NULL:
 *            Above:  pWin is placed at the top of the stack
 *            Below:  pWin is placed at the bottom of the stack
 *            TopIf:  if any sibling occludes pWin, then pWin is placed at
 *                    the top of the stack
 *            BottomIf: if pWin occludes any sibline, then pWin is placed at
 *                      the bottom of the stack
 *            Opposite: if any sibling occludes pWin, then pWin is placed at
 *                      the top of the stack, else if pWin occludes any
 *                      sibling, then pWin is placed at the bottom of the stack
 *
 */

static WindowPtr 
WhereDoIGoInTheStack(pWin, pSib, x, y, w, h, smode)
    WindowPtr pWin, pSib;
    short x, y;
    unsigned short w, h;
    int smode;
{
    BoxRec box;
    register ScreenPtr pScreen;
    WindowPtr pHead, pFirst;

    if ((pWin == pWin->parent->firstChild) && 
	(pWin == pWin->parent->lastChild))
        return((WindowPtr ) NULL);
    pHead = RealChildHead(pWin->parent);
    pFirst = pHead ? pHead->nextSib : pWin->parent->firstChild;
    pScreen = pWin->drawable.pScreen;
    box.x1 = x;
    box.y1 = y;
    box.x2 = x + (int)w;
    box.y2 = y + (int)h;
    switch (smode)
    {
      case Above:
        if (pSib)
           return(pSib);
        else if (pWin == pFirst)
            return(pWin->nextSib);
        else
            return(pFirst);
      case Below:
        if (pSib)
	    if (pSib->nextSib != pWin)
	        return(pSib->nextSib);
	    else
	        return(pWin->nextSib);
        else
            return((WindowPtr )NULL);
      case TopIf:
        if (pSib)
	{
            if ((IsSiblingAboveMe(pWin, pSib) == Above) &&
                ((* pScreen->RectIn)(pSib->borderSize, &box) != rgnOUT))
                return(pFirst);
            else
                return(pWin->nextSib);
	}
        else if (AnyWindowOverlapsMe(pWin, pHead, &box))
            return(pFirst);
        else
            return(pWin->nextSib);
      case BottomIf:
        if (pSib)
	{
            if ((IsSiblingAboveMe(pWin, pSib) == Below) &&
                ((* pScreen->RectIn)(pSib->borderSize, &box) != rgnOUT))
                return(WindowPtr)NULL;
            else
                return(pWin->nextSib);
	}
        else if (IOverlapAnyWindow(pWin, &box))
            return((WindowPtr)NULL);
        else
            return(pWin->nextSib);
      case Opposite:
        if (pSib)
	{
	    if ((* pScreen->RectIn)(pSib->borderSize, &box) != rgnOUT)
            {
                if (IsSiblingAboveMe(pWin, pSib) == Above)
                    return(pFirst);
                else 
                    return((WindowPtr)NULL);
            }
            else
                return(pWin->nextSib);
	}
        else if (AnyWindowOverlapsMe(pWin, pHead, &box))
	{
	    /* If I'm occluded, I can't possibly be the first child
             * if (pWin == pWin->parent->firstChild)
             *    return pWin->nextSib;
	     */
            return(pFirst);
	}
        else if (IOverlapAnyWindow(pWin, &box))
            return((WindowPtr)NULL);
        else
            return pWin->nextSib;
      default:
      {
        ErrorF("Internal error in ConfigureWindow, smode == %d\n",smode );
        return((WindowPtr)pWin->nextSib);
      }
    }
}

static void
ReflectStackChange(pWin, pSib)
    WindowPtr pWin, pSib;
{
/* Note that pSib might be NULL */

    Bool doValidation = (Bool)pWin->viewable;
    WindowPtr pParent;
    Bool anyMarked;
    BoxPtr box;
    WindowPtr pFirstChange;

    /* if this is a root window, can't be restacked */
    if (!(pParent = pWin->parent))
        return ;

    pFirstChange = MoveWindowInStack(pWin, pSib);

    if (doValidation)
    {
        box = (* pWin->drawable.pScreen->RegionExtents)(pWin->borderSize);
        anyMarked = MarkSiblingsBelowMe(pFirstChange, box);
#ifdef DO_SAVE_UNDERS
	if (DO_SAVE_UNDERS(pWin))
	{
	    if (pWin->saveUnder)
	    {
		ChangeSaveUnder(pWin, pFirstChange);
	    }
	    else
	    {
		CheckSaveUnder(pWin);
	    }
	}
#endif /* DO_SAVE_UNDERS */
        (* pWin->drawable.pScreen->ValidateTree)(pParent, pFirstChange,
					 TRUE, anyMarked);
	DoObscures(pParent);
	HandleExposures(pParent);
#ifdef DO_SAVE_UNDERS
	if (DO_SAVE_UNDERS(pWin))
	{
	    DoChangeSaveUnder(pFirstChange);
	}
#endif /* DO_SAVE_UNDERS */
    }
    if (pWin->realized)
	WindowsRestructured ();
}

/*****
 * ConfigureWindow
 *****/


int 
ConfigureWindow(pWin, mask, vlist, client)
    WindowPtr pWin;
    Mask mask;
    XID *vlist;
    ClientPtr client;
{
#define RESTACK_WIN    0
#define MOVE_WIN       1
#define RESIZE_WIN     2
#define REBORDER_WIN   3
    WindowPtr pSib = (WindowPtr )NULL;
    Window sibwid;
    Mask index, tmask;
    XID *pVlist;
    short x,   y, beforeX, beforeY;
    unsigned short w = pWin->clientWinSize.width,
                   h = pWin->clientWinSize.height,
	           bw = pWin->borderWidth;
    int action, 
        smode = Above;
    xEvent event;

    if ((pWin->class == InputOnly) && (mask & IllegalInputOnlyConfigureMask))
        return(BadMatch);

    if ((mask & CWSibling) && !(mask & CWStackMode))
        return(BadMatch);

    pVlist = vlist;

    if (pWin->parent)
    {
        x = pWin->absCorner.x - pWin->parent->absCorner.x - (int)bw;
        y = pWin->absCorner.y - pWin->parent->absCorner.y - (int)bw;
    }
    else
    {
        x = pWin->absCorner.x;
        y = pWin->absCorner.y;
    }
    beforeX = x;
    beforeY = y;
    action = RESTACK_WIN;	
    if ((mask & (CWX | CWY)) && (!(mask & (CWHeight | CWWidth))))
    {
	GET_INT16(CWX, x);
 	GET_INT16(CWY, y);
	action = MOVE_WIN;
    }
	/* or should be resized */
    else if (mask & (CWX |  CWY | CWWidth | CWHeight))
    {
	GET_INT16(CWX, x);
	GET_INT16(CWY, y);
	GET_CARD16(CWWidth, w);
	GET_CARD16 (CWHeight, h);
	if (!w || !h)
	{
	    client->errorValue = 0;
            return BadValue;
	}
        action = RESIZE_WIN;
    }
    tmask = mask & ~ChangeMask;
    while (tmask) 
    {
	index = (Mask)lowbit (tmask);
	tmask &= ~index;
	switch (index) 
        {
          case CWBorderWidth:   
	    GET_CARD16(CWBorderWidth, bw);
	    break;
          case CWSibling: 
	    sibwid = (Window ) *pVlist;
	    pVlist++;
            pSib = (WindowPtr )LookupID(sibwid, RT_WINDOW, RC_CORE);
            if (!pSib)
	    {
		client->errorValue = sibwid;
                return(BadWindow);
	    }
            if (pSib->parent != pWin->parent)
		return(BadMatch);
	    if (pSib == pWin)
	        return(BadMatch);
	    break;
          case CWStackMode:
	    GET_CARD8(CWStackMode, smode);
	    if ((smode != TopIf) && (smode != BottomIf) &&
 		(smode != Opposite) && (smode != Above) && (smode != Below))
                   return(BadMatch);
	    break;
	  default: 
	    client->errorValue = mask;
	    return(BadValue);
	}
    }
	/* root really can't be reconfigured, so just return */
    if (!pWin->parent)    
	return Success;

        /* Figure out if the window should be moved.  Doesnt
           make the changes to the window if event sent */

    if (mask & CWStackMode)
        pSib = WhereDoIGoInTheStack(pWin, pSib, x, y,
				    w + (bw << 1), h + (bw << 1), smode);
    else
        pSib = pWin->nextSib;

    if ((!pWin->overrideRedirect) && 
        (pWin->parent->allEventMasks & SubstructureRedirectMask))
    {
	event.u.u.type = ConfigureRequest;
	event.u.configureRequest.window = pWin->wid;
	event.u.configureRequest.parent = pWin->parent->wid;
        if (mask & CWSibling)
	   event.u.configureRequest.sibling = sibwid;
        else
       	    event.u.configureRequest.sibling = None;
        if (mask & CWStackMode)
	   event.u.u.detail = smode;
        else
       	    event.u.u.detail = Above;
	event.u.configureRequest.x = x;
	event.u.configureRequest.y = y;
	event.u.configureRequest.width = w;
	event.u.configureRequest.height = h;
	event.u.configureRequest.borderWidth = bw; 
	event.u.configureRequest.valueMask = mask;
	if (MaybeDeliverEventsToClient(pWin->parent, &event, 1, 
	        SubstructureRedirectMask, client) == 1)
    	    return(Success);            
    }
    if (action == RESIZE_WIN)
    {
        Bool size_change = (w != pWin->clientWinSize.width)
                        || (h != pWin->clientWinSize.height);
	if (size_change && (pWin->allEventMasks & ResizeRedirectMask))
	{
	    xEvent eventT;
    	    eventT.u.u.type = ResizeRequest;
    	    eventT.u.resizeRequest.window = pWin->wid;
	    eventT.u.resizeRequest.width = w;
	    eventT.u.resizeRequest.height = h;
	    if (MaybeDeliverEventsToClient(pWin, &eventT, 1, 
				       ResizeRedirectMask, client) == 1)
	    {
                /* if event is delivered, leave the actual size alone. */
	        w = pWin->clientWinSize.width;
	        h = pWin->clientWinSize.height;
                size_change = FALSE;
	    }
	}
        if (!size_change)
	{
	    if (mask & (CWX | CWY))
    	        action = MOVE_WIN;
	    else if (mask & (CWStackMode | CWBorderWidth))
	        action = RESTACK_WIN;
            else   /* really nothing to do */
                return(Success) ;        
	}
    }

    if (action == RESIZE_WIN)
            /* we've already checked whether there's really a size change */
            goto ActuallyDoSomething;
    if ((mask & CWX) && (x != beforeX))
            goto ActuallyDoSomething;
    if ((mask & CWY) && (y != beforeY))
            goto ActuallyDoSomething;
    if ((mask & CWBorderWidth) && (bw != pWin->borderWidth))
            goto ActuallyDoSomething;
    if (mask & CWStackMode) 
    {
        if (pWin->nextSib != pSib)
            goto ActuallyDoSomething;
    }
    return(Success);

ActuallyDoSomething:
    event.u.u.type = ConfigureNotify;
    event.u.configureNotify.window = pWin->wid;
    if (pSib)
        event.u.configureNotify.aboveSibling = pSib->wid;
    else
        event.u.configureNotify.aboveSibling = None;
    event.u.configureNotify.x = x;
    event.u.configureNotify.y = y;
    event.u.configureNotify.width = w;
    event.u.configureNotify.height = h;
    event.u.configureNotify.borderWidth = bw;
    event.u.configureNotify.override = pWin->overrideRedirect;
    DeliverEvents(pWin, &event, 1, NullWindow);

    if (mask & CWBorderWidth) 
    {
	if (action == RESTACK_WIN)
	{
	    action = MOVE_WIN;
	    pWin->borderWidth = bw;
	}
	else if ((action == MOVE_WIN) &&
		 ((int)bw == (x - beforeX)) && ((int)bw == (y - beforeY)))
	{
	    action = REBORDER_WIN;
            ChangeBorderWidth(pWin, bw);
	}
        else
	    pWin->borderWidth = bw;
    }
    if (action == MOVE_WIN)
        MoveWindow(pWin, x, y, pSib);
    else if (action == RESIZE_WIN)
        SlideAndSizeWindow(pWin, x, y, w, h, pSib);
    else if (mask & CWStackMode)
        ReflectStackChange(pWin, pSib);

    if (action != RESTACK_WIN)
	CheckCursorConfinement(pWin);

    return(Success);
#undef RESTACK_WIN    
#undef MOVE_WIN   
#undef RESIZE_WIN  
#undef REBORDER_WIN
}


/******
 *
 * CirculateWindow
 *    For RaiseLowest, raises the lowest mapped child (if any) that is
 *    obscured by another child to the top of the stack.  For LowerHighest,
 *    lowers the highest mapped child (if any) that is obscuring another
 *    child to the bottom of the stack.  Exposure processing is performed 
 *
 ******/

int
CirculateWindow(pParent, direction, client)
    WindowPtr pParent;
    int direction;
    ClientPtr client;
{
    register WindowPtr pWin, pHead, pFirst;
    xEvent event;
    BoxRec box;

    pHead = RealChildHead(pParent);
    pFirst = pHead ? pHead->nextSib : pParent->firstChild;
    if (direction == RaiseLowest)
    {
	for (pWin = pParent->lastChild;
	     (pWin != pHead) &&
	     !(pWin->mapped &&
	       AnyWindowOverlapsMe(pWin, pHead, WindowExtents(pWin, &box)));
	     pWin = pWin->prevSib) ;
	if (pWin == pHead)
	    return Success;
    }
    else
    {
	for (pWin = pFirst;
	     pWin &&
	     !(pWin->mapped &&
	       IOverlapAnyWindow(pWin, WindowExtents(pWin, &box)));
	     pWin = pWin->nextSib) ;
	if (!pWin)
	    return Success;
    }

    event.u.circulate.window = pWin->wid;
    event.u.circulate.parent = pParent->wid;
    event.u.circulate.event = pParent->wid;
    if (direction == RaiseLowest)
	event.u.circulate.place = PlaceOnTop;
    else
        event.u.circulate.place = PlaceOnBottom;

    if (pParent->allEventMasks & SubstructureRedirectMask)
    {
	event.u.u.type = CirculateRequest;
	if (MaybeDeliverEventsToClient(pParent, &event, 1, 
	        SubstructureRedirectMask, client) == 1)
    	    return(Success);            
    }

    event.u.u.type = CirculateNotify;
    DeliverEvents(pWin, &event, 1, NullWindow);
    ReflectStackChange(pWin, (direction == RaiseLowest) ? pFirst
						        : (WindowPtr)NULL);

    return(Success);
}

static int
CompareWIDs(pWin, wid)
    WindowPtr pWin;
    int *wid;
{
    if (pWin->wid == *wid) 
       return(WT_STOPWALKING);
    else
       return(WT_WALKCHILDREN);
}

/*****
 *  ReparentWindow
 *****/

int 
ReparentWindow(pWin, pParent, x, y, client)
    WindowPtr pWin, pParent;
    short x,y;
    ClientPtr client;
{
    WindowPtr pPrev;
    Bool WasMapped = (Bool)(pWin->mapped);
    xEvent event;
    short oldx, oldy;
    int bw = pWin->borderWidth;
    register ScreenPtr pScreen;
    
    pScreen = pWin->drawable.pScreen;
    if (pScreen != pParent->drawable.pScreen)
        return(BadMatch);
    if (TraverseTree(pWin, CompareWIDs, (pointer)&pParent->wid) == WT_STOPWALKING)
        return(BadMatch);		

    oldx = pWin->absCorner.x;
    oldy = pWin->absCorner.y;
    if (WasMapped) 
       UnmapWindow(pWin, HANDLE_EXPOSURES, SEND_NOTIFICATION, FALSE);

    event.u.u.type = ReparentNotify;
    event.u.reparent.window = pWin->wid;
    event.u.reparent.parent = pParent->wid;
    event.u.reparent.x = x;
    event.u.reparent.y = y;
    event.u.reparent.override = pWin->overrideRedirect;
    DeliverEvents(pWin, &event, 1, pParent);

    /* take out of sibling chain */

    pPrev = pWin->parent;
    if (pPrev->firstChild == pWin)
        pPrev->firstChild = pWin->nextSib;
    if (pPrev->lastChild == pWin)
        pPrev->lastChild = pWin->prevSib;

    if (pWin->nextSib) 
        pWin->nextSib->prevSib = pWin->prevSib;
    if (pWin->prevSib) 
        pWin->prevSib->nextSib = pWin->nextSib;

    /* insert at begining of pParent */
    pWin->parent = pParent;
    pPrev = RealChildHead(pParent);
    if (pPrev)
    {
	pWin->nextSib = pPrev->nextSib;
        if (pPrev->nextSib)
    	    pPrev->nextSib->prevSib = pWin;
	else
	    pParent->lastChild = pWin;
        pPrev->nextSib = pWin;
	pWin->prevSib = pPrev;
    }
    else
    {
        pWin->nextSib = pParent->firstChild;
	pWin->prevSib = (WindowPtr)NULL;
        if (pParent->firstChild) 
	    pParent->firstChild->prevSib = pWin;
        else
            pParent->lastChild = pWin;
	pParent->firstChild = pWin;
    }

    pWin->clientWinSize.x = x + bw;
    pWin->clientWinSize.y = y + bw;
    pWin->oldAbsCorner.x = oldx;
    pWin->oldAbsCorner.y = oldy;
    pWin->absCorner.x = x + bw + pParent->absCorner.x;
    pWin->absCorner.y = y + bw + pParent->absCorner.y;

    /* clip to parent */
    ClippedRegionFromBox(pParent, pWin->winSize,
			 pWin->absCorner.x, pWin->absCorner.y,
			 (int)pWin->clientWinSize.width,
			 (int)pWin->clientWinSize.height);

    if (bw)
	ClippedRegionFromBox(pParent, pWin->borderSize,
			     pWin->absCorner.x - bw,
			     pWin->absCorner.y - bw,
			     (int)pWin->clientWinSize.width + (bw<<1),
			     (int)pWin->clientWinSize.height + (bw<<1));
    else
        (* pScreen->RegionCopy)(pWin->borderSize, pWin->winSize);

    (* pScreen->PositionWindow)(pWin, pWin->absCorner.x, pWin->absCorner.y);
    ResizeChildrenWinSize(pWin, 0, 0, 0, 0);

    if (WasMapped)    
    {
        MapWindow(pWin, HANDLE_EXPOSURES, BITS_DISCARDED, SEND_NOTIFICATION,
	                                    client);
    }
    RecalculateDeliverableEvents(pParent);
    return(Success);
}

static Bool
MarkSiblingsBelowMe(pWin, box)
    WindowPtr pWin;
    BoxPtr box;
{
    WindowPtr pSib;
    int anyMarked = 0;
    register ScreenPtr pScreen;

    pScreen = pWin->drawable.pScreen;

    pSib = pWin;
    while (pSib) 
    {
        if (pSib->mapped && ((* pScreen->RectIn)(pSib->borderSize, box)))
        {
	    pSib->marked = 1;
	    anyMarked++;
            if (pSib->firstChild)
    	        anyMarked += MarkSiblingsBelowMe(pSib->firstChild, box);
	}
	pSib = pSib->nextSib;
    }
    return(anyMarked != 0);
}    

static void
RealizeChildren(pWin, client)
    WindowPtr pWin;
    ClientPtr client;
{
    WindowPtr pSib;
    Bool (* Realize)();

    pSib = pWin;
    if (pWin)
       Realize = pSib->drawable.pScreen->RealizeWindow;
    while (pSib)
    {
        if (pSib->mapped)
	{
	    pSib->realized = TRUE;
            pSib->viewable = pSib->class == InputOutput;
            (* Realize)(pSib);
            if (pSib->firstChild) 
                RealizeChildren(pSib->firstChild, client);
	}
        pSib = pSib->nextSib;
    }
}

/*****
 * MapWindow
 *    If some other client has selected SubStructureReDirect on the parent
 *    and override-redirect is xFalse, then a MapRequest event is generated,
 *    but the window remains unmapped.  Otherwise, the window is mapped and a
 *    MapNotify event is generated.
 *****/

int
MapWindow(pWin, SendExposures, BitsAvailable, SendNotification, client)

    WindowPtr pWin;
    Bool SendExposures;
    Bool BitsAvailable;
    ClientPtr client;
{
    register ScreenPtr pScreen;

    WindowPtr pParent;

    if (pWin->mapped) 
        return(Success);
    pScreen = pWin->drawable.pScreen;
    if (pParent = pWin->parent)
    {
        xEvent event;
        BoxPtr box;

        if (SendNotification && (!pWin->overrideRedirect) && 
	    (pParent->allEventMasks & SubstructureRedirectMask))
	{
	    event.u.u.type = MapRequest;
	    event.u.mapRequest.window = pWin->wid;
	    event.u.mapRequest.parent = pParent->wid;

	    if (MaybeDeliverEventsToClient(pParent, &event, 1, 
	        SubstructureRedirectMask, client) == 1)
    	        return(Success);            
	}

	pWin->mapped = TRUE;          
        if (SendNotification)
	{
	    event.u.u.type = MapNotify;
	    event.u.mapNotify.window = pWin->wid;
	    event.u.mapNotify.override = pWin->overrideRedirect;
	    DeliverEvents(pWin, &event, 1, NullWindow);
	}

	pWin->marked = 0;         /* so siblings get mapped correctly */
        if (!pParent->realized)
            return(Success);
        pWin->realized = TRUE;
        pWin->viewable = pWin->class == InputOutput;
    	/* We SHOULD check for an error value here XXX */
        (* pScreen->RealizeWindow)(pWin);
        if (pWin->firstChild)
            RealizeChildren(pWin->firstChild, client);    
        box = (* pScreen->RegionExtents)(pWin->borderSize);
        (void)MarkSiblingsBelowMe(pWin, box);

#ifdef notdef
	/* kludge to force full region reset */
        if (!pParent->parent && 
	    (box->x1 == pParent->winSize->extents.x1) &&
	    (box->y1 == pParent->winSize->extents.y1) &&
	    (box->x2 == pParent->winSize->extents.x2) &&
	    (box->y2 == pParent->winSize->extents.y2) &&
	    (pParent->firstChild == pWin)) {
	  (*pWin->drawable.pScreen->RegionCopy)(pParent->clipList,
						pParent->winSize);
	}
	/* end of kludge */
#endif

#ifdef DO_SAVE_UNDERS
	if (DO_SAVE_UNDERS(pWin))
	{
	    if (pWin->saveUnder)
	    {
		ChangeSaveUnder(pWin, pWin->nextSib);
	    }
	    else
	    {
		CheckSaveUnder(pWin);
	    }
	}
#endif /* DO_SAVE_UNDERS */

	/* anyMarked must be TRUE to force visibility events on all windows */
	(* pScreen->ValidateTree)(pParent, pWin, TRUE, TRUE);
        if (SendExposures) 
        {
	    if (!BitsAvailable)
	    {
    	        (* pScreen->RegionCopy)(pWin->exposed, pWin->clipList);  
		DoObscures(pParent);
		HandleExposures(pWin);
	    }
	    else
	    {
		/*
		 * Children shouldn't be in the parent's exposed region...
                (* pScreen->Subtract)(pParent->exposed, pParent->exposed, 
				      pWin->borderSize);
		 */
                (* pScreen->RegionEmpty)(pWin->exposed);
		DoObscures(pParent);
		HandleExposures(pWin);
	    }
	}
#ifdef DO_SAVE_UNDERS
	if (DO_SAVE_UNDERS(pWin))
	{
	    DoChangeSaveUnder(pWin->nextSib);
	}
#endif /* DO_SAVE_UNDERS */
	WindowsRestructured ();
    }
    else
    {
	pWin->mapped = TRUE;
        pWin->realized = TRUE;     /* for roots */
        pWin->viewable = pWin->class == InputOutput;
    	/* We SHOULD check for an error value here XXX */
        (* pScreen->RealizeWindow)(pWin);
    }
    
    return(Success);
}

 
/*****
 * MapSubwindows
 *    Performs a MapWindow all unmapped children of the window, in top
 *    to bottom stacking order.
 *****/

MapSubwindows(pWin, SendExposures, client)
    WindowPtr pWin;
    Bool SendExposures;
    ClientPtr client;
{
    WindowPtr pChild;

    pChild = pWin->firstChild;
    while (pChild) 
    {
	if (!pChild->mapped) 
	    /* What about backing store? */
            MapWindow(pChild, SendExposures, BITS_DISCARDED, 
		      SEND_NOTIFICATION, client);
        pChild = pChild->nextSib;
    }
}

static void
UnrealizeChildren(pWin)
    WindowPtr pWin;
{
    WindowPtr pSib;
    void (*RegionEmpty)();
    Bool (*Unrealize)();
    int (*Union)();
    
    pSib = pWin;
    if (pWin)
    {
        RegionEmpty = pWin->drawable.pScreen->RegionEmpty;
        Unrealize = pWin->drawable.pScreen->UnrealizeWindow;
	Union = pWin->drawable.pScreen->Union;
    }
    while (pSib)
    {
	Bool wasRealized = (Bool)pSib->realized;
	Bool wasViewable = (Bool)pSib->viewable;

	pSib->realized = pSib->viewable = FALSE;
	pSib->visibility = VisibilityNotViewable;
	if (wasRealized)
	{
	    (* Unrealize)(pSib);
	    DeleteWindowFromAnyEvents(pSib, FALSE);
	    if (pSib->firstChild) 
		UnrealizeChildren(pSib->firstChild);
	}
	if (wasViewable)
	{
	    if (pSib->backStorage && (pSib->backingStore != NotUseful))
	    {
		/*
		 * Allow backing-store to grab stuff off the screen before
		 * the bits go away
		 */
		(* Union) (pSib->backStorage->obscured,
			   pSib->backStorage->obscured,
			   pSib->clipList);
	    }
		    /* to force exposures later */
	    (* RegionEmpty)(pSib->clipList);    
	    (* RegionEmpty)(pSib->borderClip);
	    (* RegionEmpty)(pSib->borderExposed);
	    (* RegionEmpty)(pSib->exposed);
	    pSib->drawable.serialNumber = NEXT_SERIAL_NUMBER;
	}
        pSib = pSib->nextSib;
    }
}

/*****
 * UnmapWindow
 *    If the window is already unmapped, this request has no effect.
 *    Otherwise, the window is unmapped and an UnMapNotify event is
 *    generated.  Cannot unmap a root window.
 *****/
 
UnmapWindow(pWin, SendExposures, SendNotification, fromConfigure)
    WindowPtr pWin;
    Bool SendExposures, fromConfigure;
{
    WindowPtr pParent;
    xEvent event;
    Bool anyMarked;
    Bool wasRealized = (Bool)pWin->realized;
    Bool wasViewable = (Bool)pWin->viewable;
    BoxPtr box;

    if ((!pWin->mapped) || (!(pParent = pWin->parent))) 
        return(Success);
    if (SendNotification)
    {
	event.u.u.type = UnmapNotify;
	event.u.unmapNotify.window = pWin->wid;
	event.u.unmapNotify.fromConfigure = fromConfigure;
	DeliverEvents(pWin, &event, 1, NullWindow);
    }
    if (wasViewable)
    {
        box = (* pWin->drawable.pScreen->RegionExtents)(pWin->borderSize);
        anyMarked = MarkSiblingsBelowMe(pWin, box);
    }
    pWin->mapped = FALSE;
    pWin->realized = pWin->viewable = FALSE;
    pWin->visibility = VisibilityNotViewable;
    if (wasRealized)
    {
    	/* We SHOULD check for an error value here XXX */
        (* pWin->drawable.pScreen->UnrealizeWindow)(pWin);
        DeleteWindowFromAnyEvents(pWin, FALSE);
        if (pWin->firstChild)
            UnrealizeChildren(pWin->firstChild);
    }
    if (wasViewable)
    {
        (* pWin->drawable.pScreen->ValidateTree)(pParent, pWin, 
						 TRUE, anyMarked);
	/*
	 * For backingStore == Always, we must allow the backing-store
	 * implementation to grab the bits off the screen before the window
	 * is unmapped.
	 */
	DoObscures(pParent);
	
        if (SendExposures)
        {
	    HandleExposures(pParent);
	}
#ifdef DO_SAVE_UNDERS
	if (pWin->saveUnder && DO_SAVE_UNDERS(pWin))
	{
	    ChangeSaveUnder(pWin, pWin->nextSib);
	    DoChangeSaveUnder(pWin->nextSib);
	}
	pWin->backingStore &= ~SAVE_UNDER_BIT;
#endif /* DO_SAVE_UNDERS */
    }        
    if (wasRealized)
	WindowsRestructured ();
    return(Success);
}

/*****
 * UnmapSubwindows
 *    Performs an UnMapWindow request with the specified mode on all mapped
 *    children of the window, in bottom to top stacking order.
 *****/

UnmapSubwindows(pWin, sendExposures)
    WindowPtr pWin;
    Bool sendExposures;
{
    register WindowPtr pChild, pHead;
    xEvent event;
    Bool (*UnrealizeWindow)();
    int (*Union)();
    Bool wasRealized = (Bool)pWin->realized;
    Bool wasViewable = (Bool)pWin->viewable;
    Bool anyMarked;
    BoxPtr box;

    if (!pWin->firstChild)
	return;
    if (wasViewable)
    {
	box = (* pWin->drawable.pScreen->RegionExtents)(pWin->winSize);
	anyMarked = MarkSiblingsBelowMe(pWin->firstChild, box);
    }

    UnrealizeWindow = pWin->drawable.pScreen->UnrealizeWindow;
    Union = pWin->drawable.pScreen->Union;
    pHead = RealChildHead(pWin);
    for (pChild = pWin->lastChild; pChild != pHead; pChild = pChild->prevSib)
    {
	if (pChild->mapped) 
        {
	    event.u.u.type = UnmapNotify;
	    event.u.unmapNotify.window = pChild->wid;
	    event.u.unmapNotify.fromConfigure = xFalse;
	    DeliverEvents(pChild, &event, 1, NullWindow);
	    pChild->mapped = FALSE;
            if (wasRealized)
	    {
    	        pChild->realized = pChild->viewable = FALSE;
		pChild->visibility = VisibilityNotViewable;
    		/* We SHOULD check for an error value here XXX */
                (* UnrealizeWindow)(pChild);
                DeleteWindowFromAnyEvents(pChild, FALSE);
	        if (pChild->firstChild)
                    UnrealizeChildren(pChild->firstChild);
	    }
	    if (wasViewable)
	    {
#ifdef DO_SAVE_UNDERS
		pChild->backingStore &= ~SAVE_UNDER_BIT;
		pChild->backingStore |= SAVE_UNDER_CHANGE_BIT;
#endif /* DO_SAVE_UNDERS */
		if (pChild->backStorage && (pChild->backingStore != NotUseful))
		{
		    /*
		     * Need to allow backing-store a chance to snag bits from
		     * the screen if backingStore == Always
		     */
		    (* Union) (pChild->backStorage->obscured,
			       pChild->backStorage->obscured,
			       pChild->clipList);
		}
	    }
	}
    }
    if (wasViewable)
    {
	(* pWin->drawable.pScreen->ValidateTree)(pWin, pHead,
						 TRUE, anyMarked);
	if (sendExposures)
	{
	    DoObscures(pWin);
	    HandleExposures(pWin);
	}
#ifdef DO_SAVE_UNDERS
	if (DO_SAVE_UNDERS(pWin))
	{
	    DoChangeSaveUnder(pWin->firstChild);
	}
#endif /* DO_SAVE_UNDERS */
    }
    if (wasRealized)
	WindowsRestructured ();
}


void
HandleSaveSet(client)
    ClientPtr client;
{
    WindowPtr pParent, pWin;
    int j;

    for (j=0; j<client->numSaved; j++)
    {
        pWin = (WindowPtr)client->saveSet[j]; 
        pParent = pWin->parent;
        while (pParent && (pParent->client == client))
            pParent = pParent->parent;
        if (pParent)
	{
            ReparentWindow(pWin, pParent, pWin->absCorner.x - pWin->borderWidth, 
			   pWin->absCorner.y - pWin->borderWidth, client);
	    if(!pWin->realized && pWin->mapped)
		pWin->mapped = FALSE;
            MapWindow(pWin, HANDLE_EXPOSURES, BITS_DISCARDED,
	                    SEND_NOTIFICATION, client);
	}
    }
    xfree(client->saveSet);
    client->numSaved = 0;
    client->saveSet = (pointer *)NULL;
}
    
Bool
VisibleBoundingBoxFromPoint(pWin, x, y, box)
    WindowPtr pWin;
    int x, y;   /* in root */
    BoxPtr box;   /* "return" value */
{
    if (!pWin->realized)
	return (FALSE);
    if ((* pWin->drawable.pScreen->PointInRegion)(pWin->clipList, x, y, box))
        return(TRUE);
    return(FALSE);
}

Bool
PointInWindowIsVisible(pWin, x, y)
    WindowPtr pWin;
    int x, y;	/* in root */
{
    BoxRec box;

    if (!pWin->realized)
	return (FALSE);
    if ((* pWin->drawable.pScreen->PointInRegion)(pWin->borderClip, x, y, &box))
        return(TRUE);
    return(FALSE);
}


RegionPtr 
NotClippedByChildren(pWin)
    WindowPtr pWin;
{
    register ScreenPtr pScreen;
    RegionPtr pReg;

    pScreen = pWin->drawable.pScreen;
    pReg = (* pScreen->RegionCreate)(NULL, 1);
    (* pScreen->Intersect) (pReg, pWin->borderClip, pWin->winSize);
    return(pReg);
}


void
SendVisibilityNotify(pWin)
    WindowPtr pWin;
{
    xEvent event;
    event.u.u.type = VisibilityNotify;
    event.u.visibility.window = pWin->wid;
    event.u.visibility.state = pWin->visibility;
    DeliverEvents(pWin, &event, 1, NullWindow);
}

#define RANDOM_WIDTH 32

#ifndef NOLOGOHACK
DrawLogo(pWin)
    WindowPtr pWin;
{
    DrawablePtr pDraw;
    ScreenPtr pScreen;
    int x, y;
    unsigned int width, height, size;
    GC *pGC;
    int d11, d21, d31;
    xRectangle rect;
    xPoint poly[4];
    XID fore[2], back[2];
    xrgb rgb[2];
    BITS32 fmask, bmask;
    ColormapPtr cmap;

    pDraw = (DrawablePtr)pWin;
    pScreen = pDraw->pScreen;
    x = -pWin->clientWinSize.x;
    y = -pWin->clientWinSize.y;
    width = pScreen->width;
    height = pScreen->height;
    pGC = GetScratchGC(pScreen->rootDepth, pScreen);

    if ((random() % 100) == 17) /* make the probability for white fairly low */
	fore[0] = pScreen->whitePixel;
    else
	fore[0] = pScreen->blackPixel;
    if ((pWin->backgroundTile == (PixmapPtr)USE_BACKGROUND_PIXEL) &&
	(cmap = (ColormapPtr)LookupID(pWin->colormap, RT_COLORMAP, RC_CORE))) {
	fore[1] = pWin->backgroundPixel;
	QueryColors(cmap, 2, fore, rgb);
	if ((rgb[0].red == rgb[1].red) &&
	    (rgb[0].green == rgb[1].green) &&
	    (rgb[0].blue == rgb[1].blue)) {
	    if (fore[0] == pScreen->blackPixel)
		fore[0] = pScreen->whitePixel;
	    else
		fore[0] = pScreen->blackPixel;
	}
    }
    fore[1] = FillSolid;
    fmask = GCForeground|GCFillStyle;
    if (pWin->backgroundTile == (PixmapPtr)USE_BACKGROUND_PIXEL) {
	back[0] = pWin->backgroundPixel;
	back[1] = FillSolid;
	bmask = GCForeground|GCFillStyle;
    } else {
	back[0] = 0;
	back[1] = 0;
	DoChangeGC(pGC, GCTileStipXOrigin|GCTileStipYOrigin, back, 0);
	back[0] = FillTiled;
	back[1] = (XID)pWin->backgroundTile;
	bmask = GCFillStyle|GCTile;
    }

    size = width;
    if (height < width)
	 size = height;
    size = RANDOM_WIDTH + random() % (size - RANDOM_WIDTH);
    size &= ~1;
    x += random() % (width - size);
    y += random() % (height - size);

/*    
 *           ----- 
 *          /    /
 *         /    /
 *        /    /
 *       /    /
 *      /____/
 */

    d11 = (size / 11);
    if (d11 < 1) d11 = 1;
    d21 = (d11+3) / 4;
    d31 = d11 + d11 + d21;
    poly[0].x = x + size;              poly[0].y = y;
    poly[1].x = x + size-d31;          poly[1].y = y;
    poly[2].x = x + 0;                 poly[2].y = y + size;
    poly[3].x = x + d31;               poly[3].y = y + size;
    DoChangeGC(pGC, fmask, fore, 1);
    ValidateGC(pDraw, pGC);
    (*pGC->FillPolygon)(pDraw, pGC, Convex, CoordModeOrigin, 4, poly);

/*    
 *           ------ 
 *          /     /
 *         /  __ /
 *        /  /  /
 *       /  /  /
 *      /__/__/
 */

    poly[0].x = x + d31/2;                       poly[0].y = y + size;
    poly[1].x = x + size / 2;                    poly[1].y = y + size/2;
    poly[2].x = x + (size/2)+(d31-(d31/2));      poly[2].y = y + size/2;
    poly[3].x = x + d31;                         poly[3].y = y + size;
    DoChangeGC(pGC, bmask, back, 1);
    ValidateGC(pDraw, pGC);
    (*pGC->FillPolygon)(pDraw, pGC, Convex, CoordModeOrigin, 4, poly);

/*    
 *           ------ 
 *          /  /  /
 *         /--/  /
 *        /     /
 *       /     /
 *      /_____/
 */

    poly[0].x = x + size - d31/2;                poly[0].y = y;
    poly[1].x = x + size / 2;                    poly[1].y = y + size/2;
    poly[2].x = x + (size/2)-(d31-(d31/2));      poly[2].y = y + size/2;
    poly[3].x = x + size - d31;                  poly[3].y = y;
    ValidateGC(pDraw, pGC);
    (*pGC->FillPolygon)(pDraw, pGC, Convex, CoordModeOrigin, 4, poly);

/*
 * -----
 * \    \
 *  \    \
 *   \    \
 *    \    \
 *     \____\
 */

    poly[0].x = x;                     poly[0].y = y;
    poly[1].x = x + size/4;            poly[1].y = y;
    poly[2].x = x + size;              poly[2].y = y + size;
    poly[3].x = x + size - size/4;     poly[3].y = y + size;
    DoChangeGC(pGC, fmask, fore, 1);    
    ValidateGC(pDraw, pGC);
    (*pGC->FillPolygon)(pDraw, pGC, Convex, CoordModeOrigin, 4, poly);

/*    
 *          /
 *         /
 *        /
 *       /
 *      /
 */

    poly[0].x = x + size- d11;        poly[0].y = y;
    poly[1].x = x + size-( d11+d21);  poly[1].y = y;
    poly[2].x = x + d11;              poly[2].y = y + size;
    poly[3].x = x + d11 + d21;        poly[3].y = y + size;
    DoChangeGC(pGC, bmask, back, 1);    
    ValidateGC(pDraw, pGC);
    (*pGC->FillPolygon)(pDraw, pGC, Convex, CoordModeOrigin, 4, poly);

    FreeScratchGC(pGC);
}
#endif
