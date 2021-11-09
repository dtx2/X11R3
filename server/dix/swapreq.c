/************************************************************
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

********************************************************/

/* $XConsortium: swapreq.c,v 1.28 88/09/06 15:41:29 jim Exp $ */

#include "X.h"
#include "Xproto.h"
#include "Xprotostr.h"
#include "misc.h"
#include "dixstruct.h"

extern int (* ProcVector[256]) ();
extern void (* EventSwapVector[128]) ();  /* for SendEvent */

/* Thanks to Jack Palevich for testing and subsequently rewriting all this */

#define LengthRestB(stuff) \
    (((unsigned long)stuff->length << 2) - sizeof(*stuff))

#define LengthRestS(stuff) \
    (((unsigned long)stuff->length << 1) - (sizeof(*stuff) >> 1))

#define LengthRestL(stuff) \
    ((unsigned long)stuff->length - (sizeof(*stuff) >> 2))

#define SwapRestS(stuff) \
    SwapShorts((short *)(stuff + 1), LengthRestS(stuff))

#define SwapRestL(stuff) \
    SwapLongs((long *)(stuff + 1), LengthRestL(stuff))

/* Byte swap a list of longs */

void
SwapLongs (list, count)
	register long *list;
	register unsigned long count;
{
	register int n;

	while (count >= 8) {
	    swapl(list+0, n);
	    swapl(list+1, n);
	    swapl(list+2, n);
	    swapl(list+3, n);
	    swapl(list+4, n);
	    swapl(list+5, n);
	    swapl(list+6, n);
	    swapl(list+7, n);
	    list += 8;
	    count -= 8;
	}
	if (count != 0) {
	    do {
		swapl(list, n);
		list++;
	    } while (--count != 0);
	}
}

/* Byte swap a list of shorts */

void
SwapShorts (list, count)
	register short *list;
	register unsigned long count;
{
	register int n;

	while (count >= 16) {
	    swaps(list+0, n);
	    swaps(list+1, n);
	    swaps(list+2, n);
	    swaps(list+3, n);
	    swaps(list+4, n);
	    swaps(list+5, n);
	    swaps(list+6, n);
	    swaps(list+7, n);
	    swaps(list+8, n);
	    swaps(list+9, n);
	    swaps(list+10, n);
	    swaps(list+11, n);
	    swaps(list+12, n);
	    swaps(list+13, n);
	    swaps(list+14, n);
	    swaps(list+15, n);
	    list += 16;
	    count -= 16;
	}
	if (count != 0) {
	    do {
		swaps(list, n);
		list++;
	    } while (--count != 0);
	}
}

/* The following is used for all requests that have
   no fields to be swapped (except "length") */
int
SProcSimpleReq(client)
	register ClientPtr client;
{
    register char n;

    REQUEST(xReq);
    swaps(&stuff->length, n);
    return(*ProcVector[stuff->reqType])(client);
}

/* The following is used for all requests that have
   only a single 32-bit field to be swapped, coming
   right after the "length" field */
int
SProcResourceReq(client)
	register ClientPtr client;
{
    register char n;

    REQUEST(xResourceReq);
    swaps(&stuff->length, n);
    swapl(&stuff->id, n);
    return(*ProcVector[stuff->reqType])(client);
}

int
SProcCreateWindow(client)
    register ClientPtr client;
{
    register char n;

    REQUEST(xCreateWindowReq);
    swaps(&stuff->length, n);
    swapl(&stuff->wid, n);
    swapl(&stuff->parent, n);
    swaps(&stuff->x, n);
    swaps(&stuff->y, n);
    swaps(&stuff->width, n);
    swaps(&stuff->height, n);
    swaps(&stuff->borderWidth, n);
    swaps(&stuff->class, n);
    swapl(&stuff->visual, n);
    swapl(&stuff->mask, n);
    SwapRestL(stuff);
    return((* ProcVector[X_CreateWindow])(client));
}

int
SProcReparentWindow(client)
    register ClientPtr client;
{
    register char n;
    REQUEST(xReparentWindowReq);
    swaps(&stuff->length, n);
    swapl(&stuff->window, n);
    swapl(&stuff->parent, n);
    swaps(&stuff->x, n);
    swaps(&stuff->y, n);
    return((* ProcVector[X_ReparentWindow])(client));
}

int
SProcConfigureWindow(client)
    register ClientPtr client;
{
    register char n;
    REQUEST(xConfigureWindowReq);
    swaps(&stuff->length, n);
    swapl(&stuff->window, n);
    swaps(&stuff->mask, n);
    SwapRestL(stuff);
    return((* ProcVector[X_ConfigureWindow])(client));

}


int
SProcInternAtom(client)
    register ClientPtr client;
{
    register char n;
    REQUEST(xInternAtomReq);
    swaps(&stuff->length, n);
    swaps(&stuff->nbytes, n);
    return((* ProcVector[X_InternAtom])(client));
}

int
SProcChangeProperty(client)
    register ClientPtr client;
{
    register char n;
    REQUEST(xChangePropertyReq);
    swaps(&stuff->length, n);
    swapl(&stuff->window, n);
    swapl(&stuff->property, n);
    swapl(&stuff->type, n);
    swapl(&stuff->nUnits, n);
    switch ( stuff->format ) {
        case 8 : break;
        case 16:
            SwapShorts((short *)(stuff + 1), stuff->nUnits);
	    break;
	case 32:
	    SwapLongs((long *)(stuff + 1), stuff->nUnits);
	    break;
	}
    return((* ProcVector[X_ChangeProperty])(client));
}

int 
SProcDeleteProperty(client)
    register ClientPtr client;
{
    register char n;
    REQUEST(xDeletePropertyReq);
    swaps(&stuff->length, n);
    swapl(&stuff->window, n);
    swapl(&stuff->property, n);
    return((* ProcVector[X_DeleteProperty])(client));
              
}

int 
SProcGetProperty(client)
    register ClientPtr client;
{
    register char n;
    REQUEST(xGetPropertyReq);
    swaps(&stuff->length, n);
    swapl(&stuff->window, n);
    swapl(&stuff->property, n);
    swapl(&stuff->type, n);
    swapl(&stuff->longOffset, n);
    swapl(&stuff->longLength, n);
    return((* ProcVector[X_GetProperty])(client));
}

int
SProcSetSelectionOwner(client)
    register ClientPtr client;
{
    register char n;
    REQUEST(xSetSelectionOwnerReq);
    swaps(&stuff->length, n);
    swapl(&stuff->window, n);
    swapl(&stuff->selection, n);
    swapl(&stuff->time, n);
    return((* ProcVector[X_SetSelectionOwner])(client));
}

int
SProcConvertSelection(client)
    register ClientPtr client;
{
    register char n;
    REQUEST(xConvertSelectionReq);
    swaps(&stuff->length, n);
    swapl(&stuff->requestor, n);
    swapl(&stuff->selection, n);
    swapl(&stuff->target, n);
    swapl(&stuff->property, n);
    swapl(&stuff->time, n);
    return((* ProcVector[X_ConvertSelection])(client));
}

int
SProcSendEvent(client)
    register ClientPtr client;
{
    register char n;
    xEvent eventT;
    void (*proc)();
    REQUEST(xSendEventReq);
    swaps(&stuff->length, n);
    swapl(&stuff->destination, n);
    swapl(&stuff->eventMask, n);

    /* Swap event */
    proc = EventSwapVector[stuff->event.u.u.type & 0177];
    if (!proc)   /* no swapping proc; invalid event type? */
       return (BadValue);
    (*proc)(&stuff->event, &eventT);
    stuff->event = eventT;

    return((* ProcVector[X_SendEvent])(client));
}

int
SProcGrabPointer(client)
    register ClientPtr client;
{
    register char n;
    REQUEST(xGrabPointerReq);
    swaps(&stuff->length, n);
    swapl(&stuff->grabWindow, n);
    swaps(&stuff->eventMask, n);
    swapl(&stuff->confineTo, n);
    swapl(&stuff->cursor, n);
    swapl(&stuff->time, n);
    return((* ProcVector[X_GrabPointer])(client));
}

int
SProcGrabButton(client)
    register ClientPtr client;
{
    register char n;
    REQUEST(xGrabButtonReq);
    swaps(&stuff->length, n);
    swapl(&stuff->grabWindow, n);
    swaps(&stuff->eventMask, n);
    swapl(&stuff->confineTo, n);
    swapl(&stuff->cursor, n);
    swaps(&stuff->modifiers, n);
    return((* ProcVector[X_GrabButton])(client));
}

int
SProcUngrabButton(client)
    register ClientPtr client;
{
    register char n;
    REQUEST(xUngrabButtonReq);
    swaps(&stuff->length, n);
    swapl(&stuff->grabWindow, n);
    swaps(&stuff->modifiers, n);
    return((* ProcVector[X_UngrabButton])(client));
}

int
SProcChangeActivePointerGrab(client)
    register ClientPtr client;
{
    register char n;
    REQUEST(xChangeActivePointerGrabReq);
    swaps(&stuff->length, n);
    swapl(&stuff->cursor, n);
    swapl(&stuff->time, n);
    swaps(&stuff->eventMask, n);
    return((* ProcVector[X_ChangeActivePointerGrab])(client));
}

int
SProcGrabKeyboard(client)
    register ClientPtr client;
{
    register char n;
    REQUEST(xGrabKeyboardReq);
    swaps(&stuff->length, n);
    swapl(&stuff->grabWindow, n);
    swapl(&stuff->time, n);
    return((* ProcVector[X_GrabKeyboard])(client));
}

int
SProcGrabKey(client)
    register ClientPtr client;
{
    register char n;
    REQUEST(xGrabKeyReq);
    swaps(&stuff->length, n);
    swapl(&stuff->grabWindow, n);
    swaps(&stuff->modifiers, n);
    return((* ProcVector[X_GrabKey])(client));
}

int
SProcUngrabKey(client)
    register ClientPtr client;
{
    register char n;
    REQUEST(xUngrabKeyReq);
    swaps(&stuff->length, n);
    swapl(&stuff->grabWindow, n);
    swaps(&stuff->modifiers, n);
    return((* ProcVector[X_UngrabKey])(client));
}

int
SProcGetMotionEvents(client)
    register ClientPtr client;
{
    register char n;
    REQUEST(xGetMotionEventsReq);
    swaps(&stuff->length, n);
    swapl(&stuff->window, n);
    swapl(&stuff->start, n);
    swapl(&stuff->stop, n);
    return((* ProcVector[X_GetMotionEvents])(client));
}

int
SProcTranslateCoords(client)
    register ClientPtr client;
{
    register char n;
    REQUEST(xTranslateCoordsReq);
    swaps(&stuff->length, n);
    swapl(&stuff->srcWid, n);
    swapl(&stuff->dstWid, n);
    swaps(&stuff->srcX, n);
    swaps(&stuff->srcY, n);
    return((* ProcVector[X_TranslateCoords])(client));
}

int
SProcWarpPointer(client)
    register ClientPtr client;
{
    register char n;
    REQUEST(xWarpPointerReq);
    swaps(&stuff->length, n);
    swapl(&stuff->srcWid, n);
    swapl(&stuff->dstWid, n);
    swaps(&stuff->srcX, n);
    swaps(&stuff->srcY, n);
    swaps(&stuff->srcWidth, n);
    swaps(&stuff->srcHeight, n);
    swaps(&stuff->dstX, n);
    swaps(&stuff->dstY, n);
    return((* ProcVector[X_WarpPointer])(client));
}

int
SProcSetInputFocus(client)
    register ClientPtr client;
{
    register char n;
    REQUEST(xSetInputFocusReq);
    swaps(&stuff->length, n);
    swapl(&stuff->focus, n);
    swapl(&stuff->time, n);
    return((* ProcVector[X_SetInputFocus])(client));
}


SProcOpenFont(client)
    register ClientPtr client;
{
    register char n;
    REQUEST(xOpenFontReq);
    swaps(&stuff->length, n);
    swapl(&stuff->fid, n);
    swaps(&stuff->nbytes, n);
    return((* ProcVector[X_OpenFont])(client));
}

int
SProcListFonts(client)
    register ClientPtr client;
{
    register char n;
    REQUEST(xListFontsReq);
    swaps(&stuff->length, n);
    swaps(&stuff->maxNames, n);
    swaps(&stuff->nbytes, n);
    return((* ProcVector[X_ListFonts])(client));
}

int
SProcListFontsWithInfo(client)
    register ClientPtr client;
{
    register char n;
    REQUEST(xListFontsWithInfoReq);
    swaps(&stuff->length, n);
    swaps(&stuff->maxNames, n);
    swaps(&stuff->nbytes, n);
    return((* ProcVector[X_ListFontsWithInfo])(client));
}

int
SProcSetFontPath(client)
    register ClientPtr client;
{
    register char n;
    REQUEST(xSetFontPathReq);
    swaps(&stuff->length, n);
    swaps(&stuff->nFonts, n);
    return((* ProcVector[X_SetFontPath])(client));
}

int
SProcCreatePixmap(client)
    register ClientPtr client;
{
    register char n;
    REQUEST(xCreatePixmapReq);

    swaps(&stuff->length, n);
    swapl(&stuff->pid, n);
    swapl(&stuff->drawable, n);
    swaps(&stuff->width, n);
    swaps(&stuff->height, n);
    return((* ProcVector[X_CreatePixmap])(client));
}

int
SProcCreateGC(client)
    register ClientPtr client;
{
    register char n;
    REQUEST(xCreateGCReq);
    swaps(&stuff->length, n);
    swapl(&stuff->gc, n);
    swapl(&stuff->drawable, n);
    swapl(&stuff->mask, n);
    SwapRestL(stuff);
    return((* ProcVector[X_CreateGC])(client));
}

int
SProcChangeGC(client)
    register ClientPtr client;
{
    register char n;
    REQUEST(xChangeGCReq);
    swaps(&stuff->length, n);
    swapl(&stuff->gc, n);
    swapl(&stuff->mask, n);
    SwapRestL(stuff);
    return((* ProcVector[X_ChangeGC])(client));
}

int
SProcCopyGC(client)
    register ClientPtr client;
{
    register char n;
    REQUEST(xCopyGCReq);
    swaps(&stuff->length, n);
    swapl(&stuff->srcGC, n);
    swapl(&stuff->dstGC, n);
    swapl(&stuff->mask, n);
    return((* ProcVector[X_CopyGC])(client));
}

int
SProcSetDashes(client)
    register ClientPtr client;
{
    register char n;
    REQUEST(xSetDashesReq);
    swaps(&stuff->length, n);
    swapl(&stuff->gc, n);
    swaps(&stuff->dashOffset, n);
    swaps(&stuff->nDashes, n);
    return((* ProcVector[X_SetDashes])(client));

}

int
SProcSetClipRectangles(client)
    register ClientPtr client;
{
    register char n;
    REQUEST(xSetClipRectanglesReq);
    swaps(&stuff->length, n);
    swapl(&stuff->gc, n);
    swaps(&stuff->xOrigin, n);
    swaps(&stuff->yOrigin, n);
    SwapRestS(stuff);
    return((* ProcVector[X_SetClipRectangles])(client));
}

int
SProcClearToBackground(client)
    register ClientPtr client;
{
    register char n;
    REQUEST(xClearAreaReq);
    swaps(&stuff->length, n);
    swapl(&stuff->window, n);
    swaps(&stuff->x, n);
    swaps(&stuff->y, n);
    swaps(&stuff->width, n);
    swaps(&stuff->height, n);
    return((* ProcVector[X_ClearArea])(client));
}

int
SProcCopyArea(client)
    register ClientPtr client;
{
    register char n;
    REQUEST(xCopyAreaReq);
    swaps(&stuff->length, n);
    swapl(&stuff->srcDrawable, n);
    swapl(&stuff->dstDrawable, n);
    swapl(&stuff->gc, n);
    swaps(&stuff->srcX, n);
    swaps(&stuff->srcY, n);
    swaps(&stuff->dstX, n);
    swaps(&stuff->dstY, n);
    swaps(&stuff->width, n);
    swaps(&stuff->height, n);
    return((* ProcVector[X_CopyArea])(client));
}

int
SProcCopyPlane(client)
    register ClientPtr client;
{
    register char n;
    REQUEST(xCopyPlaneReq);
    swaps(&stuff->length, n);
    swapl(&stuff->srcDrawable, n);
    swapl(&stuff->dstDrawable, n);
    swapl(&stuff->gc, n);
    swaps(&stuff->srcX, n);
    swaps(&stuff->srcY, n);
    swaps(&stuff->dstX, n);
    swaps(&stuff->dstY, n);
    swaps(&stuff->width, n);
    swaps(&stuff->height, n);
    swapl(&stuff->bitPlane, n);
    return((* ProcVector[X_CopyPlane])(client));
}

/* The following routine is used for all Poly drawing requests
   (except FillPoly, which uses a different request format) */
int
SProcPoly(client)
    register ClientPtr client;
{
    register char n;

    REQUEST(xPolyPointReq);
    swaps(&stuff->length, n);
    swapl(&stuff->drawable, n);
    swapl(&stuff->gc, n);
    SwapRestS(stuff);
    return((* ProcVector[stuff->reqType])(client));
}

/* cannot use SProcPoly for this one, because xFillPolyReq
   is longer than xPolyPointReq, and we don't want to swap
   the difference as shorts! */
int
SProcFillPoly(client)
    register ClientPtr client;
{
    register char n;

    REQUEST(xFillPolyReq);
    swaps(&stuff->length, n);
    swapl(&stuff->drawable, n);
    swapl(&stuff->gc, n);
    SwapRestS(stuff);
    return((* ProcVector[X_FillPoly])(client));
}

int
SProcPutImage(client)
    register ClientPtr client;
{
    register char n;
    REQUEST(xPutImageReq);
    swaps(&stuff->length, n);
    swapl(&stuff->drawable, n);
    swapl(&stuff->gc, n);
    swaps(&stuff->width, n);
    swaps(&stuff->height, n);
    swaps(&stuff->dstX, n);
    swaps(&stuff->dstY, n);
    /* Image should already be swapped */
    return((* ProcVector[X_PutImage])(client));

}

int
SProcGetImage(client)
    register ClientPtr	client;
{
    register char n;
    REQUEST(xGetImageReq);
    swaps(&stuff->length, n);
    swapl(&stuff->drawable, n);
    swaps(&stuff->x, n);
    swaps(&stuff->y, n);
    swaps(&stuff->width, n);
    swaps(&stuff->height, n);
    swapl(&stuff->planeMask, n);
    return((* ProcVector[X_GetImage])(client));
}

/* ProcPolyText used for both PolyText8 and PolyText16 */

int
SProcPolyText(client)
    register ClientPtr client;
{
    register char n;
    REQUEST(xPolyTextReq);
    swaps(&stuff->length, n);
    swapl(&stuff->drawable, n);
    swapl(&stuff->gc, n);
    swaps(&stuff->x, n);
    swaps(&stuff->y, n);
    return((* ProcVector[stuff->reqType])(client));
}

/* ProcImageText used for both ImageText8 and ImageText16 */

int
SProcImageText(client)
    register ClientPtr client;
{
    register char n;
    REQUEST(xImageTextReq);
    swaps(&stuff->length, n);
    swapl(&stuff->drawable, n);
    swapl(&stuff->gc, n);
    swaps(&stuff->x, n);
    swaps(&stuff->y, n);
    return((* ProcVector[stuff->reqType])(client));
}

int
SProcCreateColormap(client)
    register ClientPtr client;
{
    register char n;
    REQUEST(xCreateColormapReq);
    swaps(&stuff->length, n);
    swapl(&stuff->mid, n);
    swapl(&stuff->window, n);
    swapl(&stuff->visual, n);
    return((* ProcVector[X_CreateColormap])(client));
}


int
SProcCopyColormapAndFree(client)
    register ClientPtr client;
{
    register char n;
    REQUEST(xCopyColormapAndFreeReq);
    swaps(&stuff->length, n);
    swapl(&stuff->mid, n);
    swapl(&stuff->srcCmap, n);
    return((* ProcVector[X_CopyColormapAndFree])(client));

}

int
SProcAllocColor                (client)
    register ClientPtr client;
{
    register char n;
    REQUEST(xAllocColorReq);
    swaps(&stuff->length, n);
    swapl(&stuff->cmap, n);
    swaps(&stuff->red, n);
    swaps(&stuff->green, n);
    swaps(&stuff->blue, n);
    return((* ProcVector[X_AllocColor])(client));
}

int
SProcAllocNamedColor           (client)
    register ClientPtr client;
{
    register char n;

    REQUEST(xAllocNamedColorReq);
    swaps(&stuff->length, n);
    swapl(&stuff->cmap, n);
    swaps(&stuff->nbytes, n);
    return((* ProcVector[X_AllocNamedColor])(client));
}

int
SProcAllocColorCells           (client)
    register ClientPtr client;
{
    register char n;
    REQUEST(xAllocColorCellsReq);
    swaps(&stuff->length, n);
    swapl(&stuff->cmap, n);
    swaps(&stuff->colors, n);
    swaps(&stuff->planes, n);
    return((* ProcVector[X_AllocColorCells])(client));
}

int
SProcAllocColorPlanes(client)
    register ClientPtr client;
{
    register char n;
    REQUEST(xAllocColorPlanesReq);
    swaps(&stuff->length, n);
    swapl(&stuff->cmap, n);
    swaps(&stuff->colors, n);
    swaps(&stuff->red, n);
    swaps(&stuff->green, n);
    swaps(&stuff->blue, n);
    return((* ProcVector[X_AllocColorPlanes])(client));
}

int
SProcFreeColors          (client)
    register ClientPtr client;
{
    register char n;
    REQUEST(xFreeColorsReq);
    swaps(&stuff->length, n);
    swapl(&stuff->cmap, n);
    swapl(&stuff->planeMask, n);
    SwapRestL(stuff);
    return((* ProcVector[X_FreeColors])(client));

}

int
SProcStoreColors               (client)
    register ClientPtr client;
{
    register char n;
    unsigned long count;
    xColorItem 	*pItem;

    REQUEST(xStoreColorsReq);
    swaps(&stuff->length, n);
    swapl(&stuff->cmap, n);
    pItem = (xColorItem *) &stuff[1];
    for(count = LengthRestB(stuff)/sizeof(xColorItem); count != 0; count--)
	SwapColorItem(pItem++);
    return((* ProcVector[X_StoreColors])(client));
}

int
SProcStoreNamedColor           (client)
    register ClientPtr client;
{
    register char n;
    REQUEST(xStoreNamedColorReq);
    swaps(&stuff->length, n);
    swapl(&stuff->cmap, n);
    swapl(&stuff->pixel, n);
    swaps(&stuff->nbytes, n);
    return((* ProcVector[X_StoreNamedColor])(client));
}

int
SProcQueryColors(client)
    register ClientPtr client;
{
    register char n;
    REQUEST(xQueryColorsReq);
    swaps(&stuff->length, n);
    swapl(&stuff->cmap, n);
    SwapRestL(stuff);
    return((* ProcVector[X_QueryColors])(client));
} 

int
SProcLookupColor(client)
    register ClientPtr client;
{
    register char n;
    REQUEST(xLookupColorReq);
    swaps(&stuff->length, n);
    swapl(&stuff->cmap, n);
    swaps(&stuff->nbytes, n);
    return((* ProcVector[X_LookupColor])(client));
}

int
SProcCreateCursor( client)
    register ClientPtr client;
{
    register char n;
    REQUEST(xCreateCursorReq);
    swaps(&stuff->length, n);
    swapl(&stuff->cid, n);
    swapl(&stuff->source, n);
    swapl(&stuff->mask, n);
    swaps(&stuff->foreRed, n);
    swaps(&stuff->foreGreen, n);
    swaps(&stuff->foreBlue, n);
    swaps(&stuff->backRed, n);
    swaps(&stuff->backGreen, n);
    swaps(&stuff->backBlue, n);
    swaps(&stuff->x, n);
    swaps(&stuff->y, n);
    return((* ProcVector[X_CreateCursor])(client));
}

int
SProcCreateGlyphCursor( client)
    register ClientPtr client;
{
    register char n;
    REQUEST(xCreateGlyphCursorReq);
    swaps(&stuff->length, n);
    swapl(&stuff->cid, n);
    swapl(&stuff->source, n);
    swapl(&stuff->mask, n);
    swaps(&stuff->sourceChar, n);
    swaps(&stuff->maskChar, n);
    swaps(&stuff->foreRed, n);
    swaps(&stuff->foreGreen, n);
    swaps(&stuff->foreBlue, n);
    swaps(&stuff->backRed, n);
    swaps(&stuff->backGreen, n);
    swaps(&stuff->backBlue, n);
    return((* ProcVector[X_CreateGlyphCursor])(client));
}


int
SProcRecolorCursor(client)
    register ClientPtr client;
{
    register char n;
    REQUEST(xRecolorCursorReq);
    swaps(&stuff->length, n);
    swapl(&stuff->cursor, n);
    swaps(&stuff->foreRed, n);
    swaps(&stuff->foreGreen, n);
    swaps(&stuff->foreBlue, n);
    swaps(&stuff->backRed, n);
    swaps(&stuff->backGreen, n);
    swaps(&stuff->backBlue, n);
    return((* ProcVector[X_RecolorCursor])(client));
}

int
SProcQueryBestSize   (client)
    register ClientPtr client;
{
    register char n;
    REQUEST(xQueryBestSizeReq);
    swaps(&stuff->length, n);
    swapl(&stuff->drawable, n);
    swaps(&stuff->width, n);
    swaps(&stuff->height, n);
    return((* ProcVector[X_QueryBestSize])(client));

}

int
SProcQueryExtension   (client)
    register ClientPtr client;
{
    register char n;
    REQUEST(xQueryExtensionReq);
    swaps(&stuff->length, n);
    swaps(&stuff->nbytes, n);
    return((* ProcVector[X_QueryExtension])(client));
}

int
SProcChangeKeyboardMapping   (client)
    register ClientPtr client;
{
    register char n;
    register long *p;
    register int i, count;

    REQUEST(xChangeKeyboardMappingReq);
    swaps(&stuff->length, n);
    p = (long *)&stuff[1];
    count = stuff->keyCodes * stuff->keySymsPerKeyCode;
    for(i = 0; i < count; i++)
    {
        swapl(p, n);
	p++;
    }
    return((* ProcVector[X_ChangeKeyboardMapping])(client));
}


int
SProcChangeKeyboardControl   (client)
    register ClientPtr client;
{
    register char n;
    REQUEST(xChangeKeyboardControlReq);
    swaps(&stuff->length, n);
    swapl(&stuff->mask, n);
    SwapRestL(stuff);
    return((* ProcVector[X_ChangeKeyboardControl])(client));
}

int
SProcChangePointerControl   (client)
    register ClientPtr client;
{
    register char n;
    REQUEST(xChangePointerControlReq);
    swaps(&stuff->length, n);
    swaps(&stuff->accelNum, n);
    swaps(&stuff->accelDenum, n);
    swaps(&stuff->threshold, n);
    return((* ProcVector[X_ChangePointerControl])(client));
}

int
SProcChangeHosts(client)
    register ClientPtr client;
{
    register char n;

    REQUEST(xChangeHostsReq);
    swaps(&stuff->length, n);
    swaps(&stuff->hostLength, n);
    return((* ProcVector[X_ChangeHosts])(client));

}

int SProcRotateProperties(client)
    register ClientPtr client;
{
    register char n;
    REQUEST(xRotatePropertiesReq);
    swaps(&stuff->length, n);
    swapl(&stuff->window, n);
    swaps(&stuff->nAtoms, n);
    swaps(&stuff->nPositions, n);
    SwapRestL(stuff);
    return ((* ProcVector[X_RotateProperties])(client));
}

/*ARGSUSED*/
int
SProcNoOperation(client)
    ClientPtr client;
{
    /* noop -- don't do anything */
    return(Success);
}

SwapTimecoord(pCoord)
    xTimecoord *pCoord;
{
    register char n;

    swapl(&pCoord->time, n);
    swaps(&pCoord->x, n);
    swaps(&pCoord->y, n);
}


SwapRGB(prgb)
    xrgb	*prgb;
{
    register char n;

    swaps(&prgb->red, n);
    swaps(&prgb->green, n);
    swaps(&prgb->blue, n);
}

SwapColorItem(pItem)
    xColorItem	*pItem;
{
    register char n;

    swapl(&pItem->pixel, n);
    swaps(&pItem->red, n);
    swaps(&pItem->green, n);
    swaps(&pItem->blue, n);
}

SwapConnClientPrefix(pCCP)
    xConnClientPrefix	*pCCP;
{
    register char n;

    swaps(&pCCP->majorVersion, n);
    swaps(&pCCP->minorVersion, n);
    swaps(&pCCP->nbytesAuthProto, n);
    swaps(&pCCP->nbytesAuthString, n);
}

