/******************************************************************
Copyright 1987 by Apollo Computer Inc., Chelmsford, Massachusetts.

                        All Rights Reserved

Permission to use, duplicate, change, and distribute this software and
its documentation for any purpose and without fee is granted, provided
that the above copyright notice appear in such copy and that this
copyright notice appear in all supporting documentation, and that the
names of Apollo Computer Inc. or MIT not be used in advertising or publicity
pertaining to distribution of the software without written prior permission.
******************************************************************/

/*
 * Functions implementing Apollo-display-independent parts of the driver
 * having to do with the software cursor.
 */

#include "apollo.h"

/*
 * There are four window functions which bypass the usual GC validation
 * path (PaintWindow{Background,Border}, CopyWindow & ClearToBackground)
 * so we must go out of our way to protect the cursor from them.  This is
 * accomplished by intercepting the two screen calls which change the window
 * vectors so we can note when they do and substitute our own function which
 * figures out what's going to be nuked and makes sure the cursor isn't there.
 *
 * The structure for the window is tracked in a somewhat sneaky way:
 * we create a new resource class (not type) and use that to associate
 * the WinPrivRec with the window (using the window's id) in the resource
 * table.  This makes it easy to find and has the added benefit of freeing
 * the private data when the window is destroyed.
 */
typedef struct {
    void        (*PaintWindowBackground)();
    void        (*PaintWindowBorder)();
    void        (*CopyWindow)();
    void    	(*SaveAreas)();
    RegionPtr  	(*RestoreAreas)();
    void	(*DrawGuarantee)();
} WinPrivRec, *WinPrivPtr;

static int      wPrivClass;     /* Resource class for private window structure (WinPrivRec)
                                 * needed to protect the cursor from background/border paintings
                                 */

/*
 * When we call SetInputCheck with pointers to the two values in
 * alwaysCheckForInput (we never change them so they always differ),
 * ProcessInputEvents will be called, at every conceivable time.
 *
 * When the cursor is down, we must do this so that we will get a
 * chance at the end of ProcessInputEvents to put the cursor back
 * up again.  When the cursor goes up, we call SetInputCheck with
 * pointers to the GPR eventcount, and its last known value.  This
 * lets DIX not call ProcessInputEvents until there is actually
 * some unprocessed input, as long as the cursor stays up.
 */
static int      alwaysCheckForInput[2] = {0, 1};

/*
 * Boolean which tells us whether the cursor is now in some frame buffer.
 */
static Bool     cursorIsUp;


/*
 * apInitCursor -- Driver internal code
 *      Initialize the cursor handling:  first create the
 *      window-private resource class.  Then initialize
 *      other static variables.
 */
void
apInitCursor ()
{
    wPrivClass = CreateNewResourceClass();
    cursorIsUp = FALSE;
}

/*
 * apConstrainPointer -- Driver internal code
 *      Given the pointer device private data,
 *      force the pointer if necessary to stay within the constraintBox.
 */
void
apConstrainPointer (pPrivP)
    apPrivPointrPtr pPrivP;
{
    gpr_$position_t  newpos;
    status_$t        st;

    if (pPrivP->x < pPrivP->constraintBox.x1)
        newpos.x_coord = pPrivP->constraintBox.x1;
    else if (pPrivP->x > pPrivP->constraintBox.x2)
        newpos.x_coord = pPrivP->constraintBox.x2;
    else
        newpos.x_coord = pPrivP->x;

    if (pPrivP->y < pPrivP->constraintBox.y1)
        newpos.y_coord = pPrivP->constraintBox.y1;
    else if (pPrivP->y > pPrivP->constraintBox.y2)
        newpos.y_coord = pPrivP->constraintBox.y2;
    else
        newpos.y_coord = pPrivP->y;

    if (newpos.x_coord != pPrivP->x || newpos.x_coord != pPrivP->x) {
        pPrivP->x = newpos.x_coord;
        pPrivP->y = newpos.y_coord;
        gpr_$set_cursor_position( newpos, st );
    }
}


/*
 * apPointerNonInterestBox -- DDX interface (screen)
 *      Given a screen and a box assumed to be within its limits,
 *      try to make the pointer device not report any motion events
 *      while it remains within that box.
 *      We note the box in the pointer device private data, but ignore
 *      it otherwise.
 */
void
apPointerNonInterestBox (pScr, pBox)
    ScreenPtr   pScr;
    BoxPtr      pBox;
{
    apPrivPointrPtr pPrivP;

    pPrivP = (apPrivPointrPtr) apPointer->devicePrivate;

    if ((pScr->myNum) == (pPrivP->numCurScreen))
        pPrivP->nonInterestBox = *pBox;
/*
 *  At the moment, nothing is done with this data (it's optional).
 */
}

/*
 * apConstrainCursor -- DDX interface (screen)
 *      Given a screen and a box assumed to be within its limits,
 *      try to make the pointer device not wander outside that box.
 *      We note the box in the pointer device private data, and
 *      hopefully use apConstrainPointer everywhere in the driver we have
 *      to in order to accomplish this.
 *
 *      If the given screen is not the current screen, nothing happens.
 *      However, since DIX always calls this with currentScreen as first
 *      argument, this seems to be moot.
 */
void
apConstrainCursor (pScr, pBox)
    ScreenPtr   pScr;
    BoxPtr      pBox;
{
    apPrivPointrPtr pPrivP;

    pPrivP = (apPrivPointrPtr) apPointer->devicePrivate;

    if ((pScr->myNum) == (pPrivP->numCurScreen))
        pPrivP->constraintBox = *pBox;
}

/*
 * apSetCursorPosition -- DDX interface (screen)
 *      Yank the cursor to the given point on the given screen.
 *      Optionally, generate a motion event as a result.
 */
Bool
apSetCursorPosition (pScr, x, y, generateEvent)
    ScreenPtr   pScr;
    int         x;
    int         y;
    Bool        generateEvent;
{
    int                 newx;
    int                 newy;
    xEvent              motion;
    CursorPtr           pCur;
    apPrivPointrPtr     pPrivP;
    apDisplayDataPtr    pDisp;
    gpr_$position_t     newpos;
    status_$t           st;

    pPrivP = (apPrivPointrPtr) apPointer->devicePrivate;
    pCur = pPrivP->pCurCursor;
 
    if (pCur && cursorIsUp)
    {
        pDisp = &apDisplayData[pPrivP->numCurScreen];
        (pDisp->apCursorDown)(pPrivP->numCurScreen, pCur);
    }
    pPrivP->numCurScreen = pScr->myNum;

    /* now actually change the position */
    pPrivP->x = x;
    pPrivP->y = y;
    newpos.x_coord = x;
    newpos.y_coord = y;
    gpr_$set_cursor_position( newpos, st );

    if (pCur && cursorIsUp)
    {
        pDisp = &apDisplayData[pPrivP->numCurScreen];
        (pDisp->apCursorUp)(pPrivP->numCurScreen, pCur);
    }

    if (generateEvent)
    {
      if (*apECV != *apLastECV)
          ProcessInputEvents();
      motion.u.keyButtonPointer.rootX = x;
      motion.u.keyButtonPointer.rootY = y;
      motion.u.keyButtonPointer.time = lastEventTime;
      motion.u.u.type = MotionNotify;
      (*apPointer->processInputProc) (&motion, apPointer);
    }
    return TRUE;
}

/*
 * apRemoveCursor -- Driver internal code
 *      Take down the cursor, and leave it down until someone calls
 *      apRestoreCursor.  Also call SetInputCheck to make DIX call
 *      ProcessInputEvents sometime soon, since that is the routine
 *      which is supposed to call apRestoreCursor.
 */
void
apRemoveCursor ()
{
    if (cursorIsUp)
    {
        CursorPtr           pCur;
        apPrivPointrPtr     pPrivP;
        apDisplayDataPtr    pDisp;

        pPrivP = (apPrivPointrPtr) apPointer->devicePrivate;
        pCur = pPrivP->pCurCursor;

        if (pCur)
        {
            pDisp = &apDisplayData[pPrivP->numCurScreen];
            (pDisp->apCursorDown)(pPrivP->numCurScreen, pCur);
            SetInputCheck(&alwaysCheckForInput[0], &alwaysCheckForInput[1]);
        }
        cursorIsUp = FALSE;
    }
}

/*
 * apMoveCursor -- Driver internal code
 *      Take down the cursor, and put it up again.  Presumably the caller
 *      wants to do this because the apEventPosition has changed.
 *      (The cursor is actually rendered based on apEventPosition,
 *      not on the pointer device private data.)
 */
void
apMoveCursor()
{
    if (cursorIsUp)
    {
        CursorPtr           pCur;
        apPrivPointrPtr     pPrivP;
        apDisplayDataPtr    pDisp;

        pPrivP = (apPrivPointrPtr) apPointer->devicePrivate;
        pCur = pPrivP->pCurCursor;

        if (pCur)
        {
            pDisp = &apDisplayData[pPrivP->numCurScreen];
            (pDisp->apCursorDown)(pPrivP->numCurScreen, pCur);
            (pDisp->apCursorUp)(pPrivP->numCurScreen, pCur);
        }
    }
}

/*
 * apRestoreCursor -- Driver internal code
 *      If the cursor is up, do nothing.  If the cursor is down,
 *      first put it up.  Then, since SetInputCheck is in the wrong
 *      state, re-call SetInputCheck to look at the eventcount values.
 */
void
apRestoreCursor ()
{
    if (!cursorIsUp)
    {
        CursorPtr           pCur;
        apPrivPointrPtr     pPrivP;
        apDisplayDataPtr    pDisp;

        pPrivP = (apPrivPointrPtr) apPointer->devicePrivate;
        pCur = pPrivP->pCurCursor;

        if (pCur)
        {
            pDisp = &apDisplayData[pPrivP->numCurScreen];
            (pDisp->apCursorUp)(pPrivP->numCurScreen, pCur);
            SetInputCheck(apECV, apLastECV);
        }
        cursorIsUp = TRUE;
    }
}

/*
 * apDisplayCursor -- DDX interface (screen)
 *      Change the cursor to be the given cursor on the given screen.
 */
Bool
apDisplayCursor (pScr, pCurs)
    ScreenPtr   pScr;
    CursorPtr   pCurs;
{
    apPrivPointrPtr     pPrivP;
    CursorPtr           pOldCur;
    apDisplayDataPtr    pDisp;
    Bool                newScreen;

    pPrivP = (apPrivPointrPtr) apPointer->devicePrivate;
    pOldCur = pPrivP->pCurCursor;
    pDisp = &apDisplayData[pPrivP->numCurScreen];

    newScreen = (pScr->myNum) != (pPrivP->numCurScreen);
    if ((pCurs != pOldCur) || newScreen || (!cursorIsUp))
    {
        if (pOldCur && cursorIsUp)
            (pDisp->apCursorDown)(pPrivP->numCurScreen, pOldCur);
        pPrivP->pCurCursor = pCurs;
        pPrivP->hotX = pCurs->xhot;
        pPrivP->hotY = pCurs->yhot;
        if (newScreen)
        {
            BoxRec          newConstraint;

            pPrivP->numCurScreen = pScr->myNum;

            newConstraint.x1 = newConstraint.y1 = 0;
            newConstraint.x2 = pScr->width;
            newConstraint.x2 = pScr->height;
            apConstrainCursor(pScr, &newConstraint);
            apConstrainPointer (pPrivP);
        }

        (pDisp->apDisplayCurs)(pPrivP->numCurScreen, pCurs);
        if (!cursorIsUp)
            apRestoreCursor();
        else if (pCurs)
            (pDisp->apCursorUp)(pPrivP->numCurScreen, pCurs);
    }
    return TRUE;
}

/*
 * apCursorLimits -- DDX interface (screen)
 *      Return the box within which the cursor hotspot is actually allowed
 *      by our device to roam, given a particular screen, cursor, and box
 *      within which DIX would like to move it.
 *
 *      Our software cursor must be completely visible (we don't try to clip it
 *      to the screen).
 */
void
apCursorLimits (pScr, pCurs, pHotBox, pPhysBox)
    ScreenPtr   pScr;
    CursorPtr   pCurs;
    BoxPtr      pHotBox;
    BoxPtr      pPhysBox;
{
    apDisplayDataPtr    pDisp;

    pDisp = &apDisplayData[pScr->myNum];

    pPhysBox->x1 = max (pHotBox->x1, 0);
    pPhysBox->y1 = max (pHotBox->y1, 0);
    pPhysBox->x2 = min (pHotBox->x2,
                        pDisp->display_char.x_visible_size);
    pPhysBox->y2 = min (pHotBox->y2,
                        pDisp->display_char.y_visible_size);
}

/*
 * apCursorLoc -- Driver internal code
 *      If the current cursor is currently up in the given screen,
 *      fill in the given BoxRec with the extent of the cursor and
 *      return TRUE.  If the cursor is either on a different screen
 *      or not currently up, return FALSE.
 */
Bool
apCursorLoc (pScreen, pBox)
    ScreenPtr     pScreen;
    BoxRec        *pBox;
{
    if (cursorIsUp)
    {
        apPrivCursPtr   pPrivC;
        apPrivPointrPtr pPrivP;
        CursorPtr       pCur;

        pPrivP = (apPrivPointrPtr) apPointer->devicePrivate;
        if ((pScreen->myNum) == pPrivP->numCurScreen)
        {
            pCur = pPrivP->pCurCursor;
            if (pCur)
            {
                pPrivC = (apPrivCursPtr) pCur->devPriv[pPrivP->numCurScreen];
                *pBox = pPrivC->cursorBox;
                return TRUE;
            }
        }
    }
    return FALSE;
}

/*
 * apPaintWindowBorder -- DDX interface (window)
 *      Paint the window's border while preserving the cursor
 */
#define apPaintWindowBorder apPaintWindowBackground

/*
 * apPaintWindowBackground -- DDX interface (window)
 *      Paint the window's background while preserving the cursor
 */
void
apPaintWindowBackground (pWin, pRegion, what)
    WindowPtr   pWin;
    RegionPtr   pRegion;
    int         what;
{
    BoxRec      cursorBox;
    WinPrivPtr  pPrivW;
    ScreenPtr   pScreen;

    pScreen = pWin->drawable.pScreen;

    if (apCursorLoc (pScreen, &cursorBox)) {
        /*
         * If the cursor is on the same screen as the window, check the
         * region to paint for the cursor and remove it as necessary
         */
        if ((* pScreen->RectIn) (pRegion, &cursorBox) != rgnOUT) {
            apRemoveCursor();
        }
    }

    pPrivW = (WinPrivPtr) LookupID (pWin->wid, RT_WINDOW, wPrivClass);
    if (what == PW_BACKGROUND) {
        (* pPrivW->PaintWindowBackground) (pWin, pRegion, what);
    } else {
        (* pPrivW->PaintWindowBorder) (pWin, pRegion, what);
    }
}

/*
 * apCopyWindow -- DDX interface (window)
 *      Protect the cursor from window copies, by possibly taking it down.
 */
void
apCopyWindow (pWin, ptOldOrg, prgnSrc)
    WindowPtr     pWin;
    DDXPointRec   ptOldOrg;
    RegionPtr     prgnSrc;
{
    BoxRec      cursorBox;
    WinPrivPtr  pPrivW;
    ScreenPtr   pScreen;

    pScreen = pWin->drawable.pScreen;

    if (apCursorLoc (pScreen, &cursorBox)) {
        /*
         * If the cursor is on the same screen, compare the box for the
         * cursor against the original window clip region (prgnSrc) and
         * the current window clip region (pWin->borderClip) and if it
         * overlaps either one, remove the cursor. (Should it really be
         * borderClip?)
         */
        switch ((* pScreen->RectIn) (prgnSrc, &cursorBox)) {
            case rgnOUT:
                if ((* pScreen->RectIn) (pWin->borderClip, &cursorBox) ==
                    rgnOUT) {
                        break;
                }
            case rgnIN:
            case rgnPART:
                apRemoveCursor();
        }
    }

    pPrivW = (WinPrivPtr) LookupID (pWin->wid, RT_WINDOW, wPrivClass);
    (* pPrivW->CopyWindow) (pWin, ptOldOrg, prgnSrc);
}

/*-
 *-----------------------------------------------------------------------
 * apSaveAreas --
 *	Keep the cursor from getting in the way of any SaveAreas operation
 *	by backing-store.
 *
 * Results:
 *	None.
 *
 * Side Effects:
 *	The cursor may be removed.
 *
 *-----------------------------------------------------------------------
 */
void
apSaveAreas(pWin)
    WindowPtr	  pWin;
{
    BoxRec  	  cursorBox;
    WinPrivPtr	  pPriv;
    ScreenPtr	  pScreen;

    pScreen = pWin->drawable.pScreen;

    if (apCursorLoc(pScreen, &cursorBox)) {
	/*
	 * If the areas are obscured because the window moved, we need to
	 * translate the box to the correct relationship with the region,
	 * which is at the new window coordinates.
	 */
	int dx, dy;

	dx = pWin->absCorner.x - pWin->backStorage->oldAbsCorner.x;
	dy = pWin->absCorner.y - pWin->backStorage->oldAbsCorner.y;

	if (dx || dy) {
	    cursorBox.x1 += dx;
	    cursorBox.y1 += dy;
	    cursorBox.x2 += dx;
	    cursorBox.y2 += dy;
	}
	if ((* pScreen->RectIn) (pWin->backStorage->obscured, &cursorBox) != rgnOUT) {
	    apRemoveCursor();
	}
    }
    pPriv = (WinPrivPtr) LookupID (pWin->wid, RT_WINDOW, wPrivClass);
    (* pPriv->SaveAreas) (pWin);
}

/*-
 *-----------------------------------------------------------------------
 * apRestoreAreas --
 *	Keep the cursor from getting in the way of any RestoreAreas operation
 *	by backing-store.
 *
 * Results:
 *	None.
 *
 * Side Effects:
 *	The cursor may be removed.
 *
 *-----------------------------------------------------------------------
 */
RegionPtr
apRestoreAreas(pWin)
    WindowPtr	  pWin;
{
    BoxRec  	  cursorBox;
    WinPrivPtr	  pPriv;
    ScreenPtr	  pScreen;

    pScreen = pWin->drawable.pScreen;

    if (apCursorLoc(pScreen, &cursorBox)) {
	/*
	 * The exposed region is now window-relative, so we have to make the
	 * cursor box window-relative too.
	 */
	cursorBox.x1 -= pWin->absCorner.x;
	cursorBox.x2 -= pWin->absCorner.x;
	cursorBox.y1 -= pWin->absCorner.y;
	cursorBox.y2 -= pWin->absCorner.y;
	if ((* pScreen->RectIn) (pWin->exposed, &cursorBox) != rgnOUT) {
	    apRemoveCursor();
	}
    }
    pPriv = (WinPrivPtr) LookupID (pWin->wid, RT_WINDOW, wPrivClass);
    return (* pPriv->RestoreAreas) (pWin);
}

/*-
 *-----------------------------------------------------------------------
 * apDrawGuarantee --
 *	Makes any DrawGuarantee operation see the shadow GC
 *
 * Results:
 *	None.
 *
 * Side Effects:
 *	The cursor may be removed.
 *
 *-----------------------------------------------------------------------
 */

void
apDrawGuarantee(pWin, pGC, guarantee)
    WindowPtr	  pWin;
    GCPtr	  pGC;
    int		  guarantee;
{
    WinPrivPtr	  pPriv;

    pPriv = (WinPrivPtr) LookupID (pWin->wid, RT_WINDOW, wPrivClass);
    (* pPriv->DrawGuarantee) (pWin, (GCPtr) pGC->devPriv, guarantee);
}

/*
 * apCreateWindow -- DDX interface (window)
 *      First, allow the output library to do its thing, and then make
 *      sure we intercept calls to PaintWindow{Border,Background} and
 *      ClearToBackground.
 */
Bool
apCreateWindow(pWin)
    WindowPtr   pWin;
{
    WinPrivPtr  pPriv;

    (* apDisplayData[((DrawablePtr)pWin)->pScreen->myNum].CreateWindow) (pWin);

    pPriv = (WinPrivPtr) Xalloc (sizeof (WinPrivRec));

    pPriv->PaintWindowBackground = pWin->PaintWindowBackground;
    pPriv->PaintWindowBorder = pWin->PaintWindowBorder;
    pPriv->CopyWindow = pWin->CopyWindow;

    pWin->PaintWindowBackground = apPaintWindowBackground;
    pWin->PaintWindowBorder = apPaintWindowBorder;
    pWin->CopyWindow = apCopyWindow;

    if (pWin->backStorage) {
	pPriv->SaveAreas = pWin->backStorage->SaveDoomedAreas;
	pPriv->RestoreAreas = pWin->backStorage->RestoreAreas;
	pPriv->DrawGuarantee = pWin->backStorage->DrawGuarantee;

	pWin->backStorage->SaveDoomedAreas = apSaveAreas;
	pWin->backStorage->RestoreAreas = apRestoreAreas;
	pWin->backStorage->DrawGuarantee = apDrawGuarantee;
    }

    AddResource (pWin->wid, RT_WINDOW, (pointer)pPriv, Xfree, 
                 wPrivClass);

}

/*
 * apChangeWindowAttributes -- DDX interface (window)
 *      Catch the possible changing of the background/border functions.
 */
Bool
apChangeWindowAttributes(pWin, mask)
    WindowPtr   pWin;
    Mask        mask;
{
    WinPrivPtr  pPriv;

    (* apDisplayData[((DrawablePtr)pWin)->pScreen->myNum].ChangeWindowAttributes) (pWin, mask);

    pPriv = (WinPrivPtr) LookupID (pWin->wid, RT_WINDOW, wPrivClass);
    if (pPriv == (WinPrivPtr)0) {
        FatalError("apChangeWindowAttributes got null pPriv\n");
    }

    if ((char *)pWin->PaintWindowBackground !=(char *)apPaintWindowBackground) {
        pPriv->PaintWindowBackground = pWin->PaintWindowBackground;
        pWin->PaintWindowBackground = apPaintWindowBackground;
    }
    if ((char *)pWin->PaintWindowBorder != (char *)apPaintWindowBorder) {
        pPriv->PaintWindowBorder = pWin->PaintWindowBorder;
        pWin->PaintWindowBorder = apPaintWindowBorder;
    }
    if ((char *)pWin->CopyWindow != (char *)apCopyWindow) {
        pPriv->CopyWindow = pWin->CopyWindow;
        pWin->CopyWindow = apCopyWindow;
    }

    if (pWin->backStorage &&
	((void (*)())pWin->backStorage->SaveDoomedAreas != (void (*)())apSaveAreas)){
	    pPriv->SaveAreas = pWin->backStorage->SaveDoomedAreas;
	    pWin->backStorage->SaveDoomedAreas = apSaveAreas;
    }
    if (pWin->backStorage &&
	(pWin->backStorage->RestoreAreas != apRestoreAreas)){
	    pPriv->RestoreAreas = pWin->backStorage->RestoreAreas;
	    pWin->backStorage->RestoreAreas = apRestoreAreas;
    }
    if (pWin->backStorage &&
	((void (*) ())pWin->backStorage->DrawGuarantee != (void (*)())apDrawGuarantee)){
	    pPriv->DrawGuarantee = pWin->backStorage->DrawGuarantee;
	    pWin->backStorage->DrawGuarantee = apDrawGuarantee;
    }
    
    return (TRUE);
}

/*
 * apRealizeCursor -- DDX interface (screen)
 *      Given a screen and cursor, realize the given cursor for the
 *      given screen.  We allocate the cursor private structure, fill in
 *      the display-independent parts (actually there aren't any), and
 *      then invoke the display-dependent code to finish filling it in.
 */
Bool
apRealizeCursor (pScr, pCurs)
    ScreenPtr pScr;
    CursorPtr pCurs;
{
    apPrivCursPtr       pPrivC;
    apDisplayDataPtr    pDisp;

    pPrivC = (apPrivCursPtr) Xalloc (sizeof(apPrivCursRec));
    pCurs->devPriv[pScr->myNum] = (pointer) pPrivC;

    pDisp = &apDisplayData[pScr->myNum];
    return ( (pDisp->apRealizeCurs)(pDisp, pCurs, pPrivC) );
}

/*
 * apUnrealizeCursor -- DDX interface (screen)
 *      Given a screen and cursor, unrealize the given cursor for the
 *      given screen.  If this cursor is up on this screen, take it
 *      down first.
 *      After letting the display-dependent code clean up,
 *      we just deallocate the cursor private structure.
 */
Bool
apUnrealizeCursor (pScr, pCurs)
    ScreenPtr   pScr;
    CursorPtr   pCurs;
{
    apPrivCursPtr       pPrivC;
    apPrivPointrPtr     pPrivP;
    apDisplayDataPtr    pDisp;
    Bool                retval;

    pPrivC = (apPrivCursPtr) pCurs->devPriv[pScr->myNum];
    pPrivP = (apPrivPointrPtr) apPointer->devicePrivate;

    if ( ((pPrivP->numCurScreen) == (pScr->myNum)) &&
         ((pPrivP->pCurCursor) == pCurs) )
    {
        apRemoveCursor();
        pPrivP->pCurCursor = NullCursor;
    }

    pDisp = &apDisplayData[pScr->myNum];
    retval = (pDisp->apUnrealizeCurs)(pCurs, pPrivC);
    Xfree (pPrivC);
    return (retval);
}
