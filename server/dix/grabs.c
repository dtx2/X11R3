/* $XConsortium: grabs.c,v 1.6 88/09/06 15:41:12 jim Exp $ */
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
WHETHER IN AN action OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.

********************************************************/

#include "X.h"
#include "misc.h"
#include "Xproto.h"
#include "windowstr.h"
#include "inputstr.h"
#include "cursorstr.h"

#define BITMASK(i) (((Mask)1) << ((i) & 31))
#define MASKIDX(i) ((i) >> 5)
#define MASKWORD(buf, i) buf[MASKIDX(i)]
#define BITSET(buf, i) MASKWORD(buf, i) |= BITMASK(i)
#define BITCLEAR(buf, i) MASKWORD(buf, i) &= ~BITMASK(i)
#define GETBIT(buf, i) (MASKWORD(buf, i) & BITMASK(i))

static Mask *
CreateDetailMask()
{
    Mask *pTempMask;
    int i;

    pTempMask = (Mask *)xalloc(sizeof(Mask) * MasksPerDetailMask);

    for ( i = 0; i < MasksPerDetailMask; i++)
	pTempMask[i]= ~0;

    return pTempMask;

}

static void
DeleteDetailFromMask(ppDetailMask, detail)
Mask **ppDetailMask;
unsigned short detail;
{
    if (*ppDetailMask == NULL)
	*ppDetailMask = CreateDetailMask();
 
    BITCLEAR((*ppDetailMask), detail);
}

static Mask *
CopyDetailMask(pOriginalDetailMask)
Mask *pOriginalDetailMask;
{
    Mask *pTempMask;
    int i;

    if (pOriginalDetailMask == NULL)
	return NULL;

    pTempMask = (Mask *)xalloc(sizeof(Mask) * MasksPerDetailMask);

    for ( i = 0; i < MasksPerDetailMask; i++)
	pTempMask[i]= pOriginalDetailMask[i];

    return pTempMask;

}

extern int PassiveClientGone();	/* This is defined in events.c */

void
AddPassiveGrabToWindowList(pGrab)
GrabPtr pGrab;
{
    pGrab->resource = FakeClientID(pGrab->client->index);
    pGrab->next = PASSIVEGRABS(pGrab->window);
    pGrab->window->passiveGrabs = (pointer)pGrab;
    AddResource(pGrab->resource, RT_FAKE, (pointer)pGrab->window,
		PassiveClientGone, RC_CORE);
}


GrabPtr
CreateGrab(client, device, window, eventMask, ownerEvents, keyboardMode,
	   pointerMode, modifiers, keybut, confineTo, cursor)
ClientPtr client;
DeviceIntPtr device;
WindowPtr window;
Mask eventMask;
Bool ownerEvents, keyboardMode, pointerMode;
unsigned short modifiers;
KeyCode keybut;	/* key or button */
WindowPtr confineTo;
CursorPtr cursor;
{
    GrabPtr grab;

    grab = (GrabPtr)xalloc(sizeof(GrabRec));
    grab->client = client;
    grab->device = device;
    grab->window = window;
    grab->eventMask = eventMask;
    grab->ownerEvents = ownerEvents;
    grab->keyboardMode = keyboardMode;
    grab->pointerMode = pointerMode;
    grab->modifiersDetail.exact = modifiers;
    grab->modifiersDetail.pMask = NULL;
    grab->detail.exact = keybut;
    grab->detail.pMask = NULL;
    grab->confineTo = confineTo;
    grab->cursor = cursor;
    if (cursor)
	cursor->refcnt++;
    return grab;

}



void
DeleteGrab(pGrab)
GrabPtr pGrab;
{
    if (pGrab->modifiersDetail.pMask != NULL)
	xfree(pGrab->modifiersDetail.pMask);

    if (pGrab->detail.pMask != NULL)
	xfree(pGrab->detail.pMask);

    if (pGrab->cursor)
	FreeCursor(pGrab->cursor, 0);

    xfree(pGrab);

}


static Bool
IsInGrabMask(firstDetail, secondDetail, exception)
DetailRec firstDetail, secondDetail;
unsigned short exception;
{
    if (firstDetail.exact == exception)
    {
	if (firstDetail.pMask == NULL)
	    return TRUE;
	
	/* (at present) never called with two non-null pMasks */
	if (secondDetail.exact == exception)
	    return FALSE;

 	if (GETBIT(firstDetail.pMask, secondDetail.exact))
	    return TRUE;
    }
    
    return FALSE;
}

static Bool 
IdenticalExactDetails(firstExact, secondExact, exception)
unsigned short firstExact, secondExact, exception;
{
    if ((firstExact == exception) || (secondExact == exception))
	return FALSE;
   
    if (firstExact == secondExact)
	return TRUE;

    return FALSE;
}

static Bool 
DetailSupersedesSecond(firstDetail, secondDetail, exception)
DetailRec firstDetail, secondDetail;
unsigned short exception;
{
    if (IsInGrabMask(firstDetail, secondDetail, exception))
	return TRUE;

    if (IdenticalExactDetails(firstDetail.exact, secondDetail.exact, exception))
	return TRUE;
  
    return FALSE;

}

Bool
GrabSupersedesSecond(pFirstGrab, pSecondGrab)
GrabPtr pFirstGrab, pSecondGrab;
{
    if (!DetailSupersedesSecond(pFirstGrab->modifiersDetail,
	pSecondGrab->modifiersDetail, 
	(unsigned short)AnyModifier))
	return FALSE;

    if (DetailSupersedesSecond(pFirstGrab->detail,
	pSecondGrab->detail, (unsigned short)AnyKey))
	return TRUE;
 
    return FALSE;
}

Bool
GrabMatchesSecond(pFirstGrab, pSecondGrab)
GrabPtr pFirstGrab, pSecondGrab;
{
    if (pFirstGrab->device != pSecondGrab->device)
	return FALSE;

    if (GrabSupersedesSecond(pFirstGrab, pSecondGrab))
	return TRUE;

    if (GrabSupersedesSecond(pSecondGrab, pFirstGrab))
	return TRUE;
 
    if (DetailSupersedesSecond(pSecondGrab->detail, 
	pFirstGrab->detail, (unsigned short)AnyKey) 
	&& 
	DetailSupersedesSecond(pFirstGrab->modifiersDetail, 
	pSecondGrab->modifiersDetail, (unsigned short)AnyModifier))
	return TRUE;

    if (DetailSupersedesSecond(pFirstGrab->detail, 
	pSecondGrab->detail, (unsigned short)AnyKey) 
	&& 
	DetailSupersedesSecond(pSecondGrab->modifiersDetail, 
	pFirstGrab->modifiersDetail, (unsigned short)AnyModifier))
	return TRUE;

    return FALSE;

}



void
DeletePassiveGrabFromList(pMinuendGrab)
GrabPtr pMinuendGrab;
{
    register GrabPtr *next;
    register GrabPtr grab;

    for (next = (GrabPtr *)&(pMinuendGrab->window->passiveGrabs); *next; )
    {
	grab = *next;

	if (GrabMatchesSecond(grab, pMinuendGrab) && 
	    (grab->client == pMinuendGrab->client))
        {
	    if (GrabSupersedesSecond(pMinuendGrab, grab))
	    {
 	        /* This is really sleazy and counts on FreeResource to update
		    *next ( notice the continue ). */

	    	FreeResource(grab->resource, RC_NONE);
	    	continue;
            }

            if ((grab->detail.exact == AnyKey)
		&& (grab->modifiersDetail.exact != AnyModifier))
	    {
	        DeleteDetailFromMask(&(grab->detail.pMask),
			pMinuendGrab->detail.exact);
	    }
	    else
	    {
		if ((grab->modifiersDetail.exact == AnyModifier) 
		    && (grab->detail.exact != AnyKey))
		{
		    DeleteDetailFromMask(&(grab->modifiersDetail.pMask),
			pMinuendGrab->modifiersDetail.exact); 
		}
		else
		{
		    if ((pMinuendGrab->detail.exact != AnyKey)
			&& (pMinuendGrab->modifiersDetail.exact != AnyModifier))
		    {
		    	GrabPtr pNewGrab;
		
			DeleteDetailFromMask(&(grab->detail.pMask),
			    pMinuendGrab->detail.exact); 	
			
			pNewGrab = CreateGrab(grab->client, grab->device, 
			    grab->window,
			    grab->eventMask, grab->ownerEvents,
			    grab->keyboardMode, grab->pointerMode, AnyModifier,
			    pMinuendGrab->detail.exact,
			    grab->confineTo, grab->cursor);

			pNewGrab->modifiersDetail.pMask = 
				CopyDetailMask(grab->modifiersDetail.pMask);

			DeleteDetailFromMask(&(pNewGrab->modifiersDetail.pMask),
			    pMinuendGrab->modifiersDetail.exact); 
		
			AddPassiveGrabToWindowList(pNewGrab);
		    }   
		    else
		    {
			if (pMinuendGrab->detail.exact == AnyKey)
			{
			    DeleteDetailFromMask(&(grab->modifiersDetail.pMask),
				pMinuendGrab->modifiersDetail.exact);   	
			}
			else
			{
			    DeleteDetailFromMask(
				&(grab->detail.pMask),
			    	pMinuendGrab->detail.exact); 	
			 }
		    }
	   	}
	
	    }
	}
  	next = &((*next)->next);
    }
}
