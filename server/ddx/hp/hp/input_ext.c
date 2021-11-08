#ifdef HPINPUT
/* $XConsortium: input_ext.c,v 1.2 88/09/06 15:25:44 jim Exp $ */
/*

Copyright (c) 1986, 1987 by Hewlett-Packard Company
Copyright (c) 1986, 1987 by the Massachusetts Institute of Technology

Permission to use, copy, modify, and distribute this
software and its documentation for any purpose and without
fee is hereby granted, provided that the above copyright
notice appear in all copies and that both that copyright
notice and this permission notice appear in supporting
documentation, and that the name of M.I.T. not be used in
advertising or publicity pertaining to distribution of the
software without specific, written prior permission.

HEWLETT-PACKARD MAKES NO WARRANTY OF ANY KIND WITH REGARD
TO THIS SOFWARE, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR 
PURPOSE.  Hewlett-Packard shall not be liable for errors 
contained herein or direct, indirect, special, incidental or 
consequential damages in connection with the furnishing, 
performance, or use of this material.

This software is not subject to any license of the American
Telephone and Telegraph Company or of the Regents of the
University of California.

*/
#include <ndir.h>
#include "X.h"
#include "misc.h"
#include "resource.h"
#define NEED_EVENTS
#define NEED_REPLIES
#include "Xproto.h"
#include "windowstr.h"
#include "inputstr.h"
#include "scrnintstr.h"
#include "cursorstr.h"
#include "dixstruct.h"
#include "hildef.h"
#include "hpext.h"
#include "XInputExt.h"
#include "XHPproto.h"
#include "sun.h"

/***************************************************************************
 *
 * DIX defines and constants.  This extension should not modify these.
 *
 */

#define CantBeFiltered NoEventMask
#define AllButtonsMask ( \
	Button1Mask | Button2Mask | Button3Mask | Button4Mask | Button5Mask )

#define Motion_Filter(state) (PointerMotionMask | \
		(AllButtonsMask & state) | DevicebuttonMotionMask)

extern 		WindowRec 	WindowTable[];
extern 		WindowPtr 	*spriteTrace;
#define 	ROOT 		spriteTrace[0]
extern 		Mask		filters[];
extern 		InputInfo 	inputInfo;
extern 		WindowPtr 	motionHintWindow;
extern 		CARD16 		keyModifiersList[];
WindowPtr 	CommonAncestor();

typedef struct {
    int		x, y;
} HotSpot;

extern struct {
    CursorPtr	current;
    BoxRec	hotLimits;	/* logical constraints */
    BoxRec	physLimits;	/* of hot spot due to hardware limits */
    WindowPtr	win;
    HotSpot	hot;
} sprite;			/* info about the cursor sprite */

/***************************************************************************
 *
 * HP extension equivalents of dix structures and globals.
 *
 */

WindowPtr 	*devfocusTrace[MAX_LOGICAL_DEVS];
int 		devfocusTraceSize[MAX_LOGICAL_DEVS];
int 		devfocusTraceGood[MAX_LOGICAL_DEVS];
int 		DevicemodifierKeyCount[8];
Mask 		DevicebuttonMotionMask = 0;
int 		DevicebuttonsDown = 0;	/* number of buttons currently down */
CARD16 		DevicekeyButtonState;
int 		DevicekeyThatActivatedPassiveGrab;


/***************************************************************************
 *
 * HP extension stuff.
 *
 */

extern	Mask		DeviceKeymapStateMask;
extern	DevicePtr	hpOther[];
extern	int		DeviceFocusIn;
extern	int		DeviceFocusOut;
extern	int		DeviceKeymapNotify;
extern	int		DeviceMotionNotify;
extern	int		xEventExtension;
Bool	hpKeybdProc();

/***************************************************************************
 *
 * HPNormalKeyboardEvent. (Needed to avoid calling normal DeliverDeviceEvents.)
 *
 */

static void
HPNormalKeyboardEvent(keybd, xE, w, fix, focus, id, filter)
    xEvent 		*xE;
    DeviceIntPtr 	keybd;
    WindowPtr 		w;
    Bool 		fix;
    WindowPtr 		focus;
    int			id;
    Mask		filter;
    {
    if (focus == NullWindow)
	return;
    if (focus == PointerRootWin)
	{
	HPDeliverDeviceEvents(w, xE, NullGrab, NullWindow, fix, id, filter);
	return;
	}
    if ((focus == w) || IsParent(focus, w))
	{
	if (HPDeliverDeviceEvents(w, xE, NullGrab, focus, fix, id, filter))
	    return;
	}
    /* just deliver it to the focus window */
    if (fix)
        FixUpEventFromWindow(xE, focus, None, FALSE);
    DeliverDeviceEventsToWindow(focus, xE, 1, filter, NullGrab, id);
    }

/***************************************************************************
 *
 * HPDeliverGrabbedEvent. 
 *
 */

static void
HPDeliverGrabbedEvent(xE, thisDev, otherDev, deactivateGrab, fix, id, filter)
    register 	xEvent *xE;
    register 	DeviceIntPtr thisDev;
    DeviceIntPtr otherDev;
    Bool 	deactivateGrab;
    Bool 	fix;
    int		id;
    Mask	filter;
    {
    register GrabPtr grab = thisDev->grab;
    int deliveries;

    if ((!grab->ownerEvents) ||
	(!(deliveries = HPDeliverDeviceEvents(sprite.win, xE, grab, NullWindow, fix, id, filter))))
	{
	if (fix)
	    FixUpEventFromWindow(xE, grab->window, None, TRUE);
	deliveries = DeviceTryClientEvents(grab->client, xE, 1, grab->eventMask,
				     filter, grab);
        }
    if (deliveries && !deactivateGrab)
	switch (thisDev->sync.state)
	    {
	    case FREEZE_BOTH_NEXT_EVENT:
		otherDev->sync.frozen = TRUE;
		if ((otherDev->sync.state == FREEZE_BOTH_NEXT_EVENT) &&
		    (otherDev->grab->client == thisDev->grab->client))
		    otherDev->sync.state = FROZEN_NO_EVENT;
		else
		    otherDev->sync.other = thisDev->grab;
		/* fall through */
	    case FREEZE_NEXT_EVENT:
		thisDev->sync.state = FROZEN_WITH_EVENT;
		thisDev->sync.frozen = TRUE;
		thisDev->sync.event = *xE;
		break;
	    }
    }

/****************************************************************************
 *
 * This routine actually sends the focus event for nonstandard devices.
 *
 */

static void
DeviceFocusEvent (type, mode, detail, pWin, id)
    int 	type, mode, detail;
    WindowPtr 	pWin;
    int		id;
    {
    int	     		i;
    int	     		ndx;
    xHPdevicefocus	ev;
    DeviceIntPtr	dev = (DeviceIntPtr) hpOther[id];
    extern		HPInputDevice	l_devs[];

    ndx = find_device (id);

    ev.mode = mode;
    ev.type = type;
    ev.detail = detail;
    ev.window = pWin->wid;
    ev.deviceid = id;

    DeliverDeviceEventsToWindow(pWin, &ev, 1, filters[type], NullGrab,id);
    if (type == DeviceFocusIn && l_devs[ndx].x_type == KEYBOARD)
        {
	xHPDeviceKeymapEvent ke;

	ke.type = DeviceKeymapNotify;
	ke.deviceid = id;
	bcopy(dev->down, &ke.map[0], 30);
	DeliverDeviceEventsToWindow(pWin, &ke, 1, DeviceKeymapStateMask, NullGrab,id);
        }
    }

/****************************************************************************
 *
 * This routine sends focus in events for nonstandard devices.
 * recursive because it is easier
 * no-op if child not descended from ancestor
 *
 */

static Bool
DeviceFocusInEvents(ancestor, child, skipChild, mode, detail, doAncestor, id)
    WindowPtr 	ancestor;
    WindowPtr 	child;
    WindowPtr 	skipChild;
    int 	mode;
    int 	detail;
    Bool 	doAncestor;
    int 	id;
    {

    if (child == NullWindow)
	return FALSE;

    if (ancestor == child)
	{
	if (doAncestor)
	    DeviceFocusEvent (DeviceFocusIn, mode, detail, child, id);
	return TRUE;
	}

    if (DeviceFocusInEvents(
	ancestor, child->parent, skipChild, mode, detail, doAncestor, id))
	{
	if (child != skipChild)
	    DeviceFocusEvent (DeviceFocusIn, mode, detail, child, id);
	return TRUE;
	}
    return FALSE;
    }

/****************************************************************************
 *
 * This routine sends focus out events for nonstandard devices.
 * dies horribly if ancestor is not an ancestor of child.
 *
 */

static void
DeviceFocusOutEvents (child, ancestor, mode, detail, doAncestor, id)
    WindowPtr 	child;
    WindowPtr 	ancestor;
    int 	detail;
    Bool 	doAncestor;
    int		id;
    {
    register WindowPtr  pWin;

    for (pWin = child; pWin != ancestor; pWin = pWin->parent)
	DeviceFocusEvent (DeviceFocusOut, mode, detail, pWin, id);

    if (doAncestor)
	DeviceFocusEvent (DeviceFocusOut, mode, detail, ancestor, id);
    }

/****************************************************************************
 *
 * This routine sends focus in and out events for nonstandard devices.
 *
 */

DoDeviceFocusEvents(fromWin, toWin, mode, id)
    WindowPtr fromWin;
    WindowPtr toWin;
    int mode;
    int id;
    {
    int     out, in;		       /* for holding details for to/from
				          PointerRoot/None */
    int     i;

    if (fromWin == toWin)
	return;

    out = (fromWin == NoneWin) ? NotifyDetailNone : NotifyPointerRoot;
    in = (toWin == NoneWin) ? NotifyDetailNone : NotifyPointerRoot;
    /* wrong values if neither, but then not referenced */

    if ((toWin == NullWindow) || (toWin == PointerRootWin))
        {
	if ((fromWin == NullWindow) || (fromWin == PointerRootWin))
   	    {
	    if (fromWin == PointerRootWin)
		DeviceFocusOutEvents(sprite.win, ROOT, mode, NotifyPointer, TRUE, id);
	    /* Notify all the roots */
	    for (i=0; i<screenInfo.numScreens; i++)
	        DeviceFocusEvent (DeviceFocusOut, mode, out, &WindowTable[i], id);
	    }
	else
	    {
	    if (IsParent(fromWin, sprite.win))
	        DeviceFocusOutEvents(sprite.win, fromWin, mode, NotifyPointer, FALSE, id);
	    DeviceFocusEvent (DeviceFocusOut, mode, NotifyNonlinear, fromWin, id);
	    /* next call catches the root too, if the screen changed */
	    DeviceFocusOutEvents( fromWin->parent, NullWindow, mode,
			    NotifyNonlinearVirtual, FALSE, id);
	    }
	/* Notify all the roots */
	for (i=0; i<screenInfo.numScreens; i++)
	    DeviceFocusEvent (DeviceFocusIn, mode, in, &WindowTable[i], id);
	if (toWin == PointerRootWin)
	    DeviceFocusInEvents(
		ROOT, sprite.win, NullWindow, mode, NotifyPointer, TRUE, id);
        }
    else
        {
	if ((fromWin == NullWindow) || (fromWin == PointerRootWin))
	    {
	    if (fromWin == PointerRootWin)
		DeviceFocusOutEvents(sprite.win, ROOT, mode, NotifyPointer, TRUE, id);
	    for (i=0; i<screenInfo.numScreens; i++)
	      DeviceFocusEvent (DeviceFocusOut, mode, out, &WindowTable[i], id);
	    if (toWin->parent != NullWindow)
	      DeviceFocusInEvents(
		ROOT, toWin, toWin, mode, NotifyNonlinearVirtual, TRUE, id);
	    DeviceFocusEvent (DeviceFocusIn, mode, NotifyNonlinear, toWin, id);
	    if (IsParent(toWin, sprite.win))
    	       DeviceFocusInEvents(
		 toWin, sprite.win, NullWindow, mode, NotifyPointer, FALSE, id);
	    }
	else
	    {
	    if (IsParent(toWin, fromWin))
	        {
		DeviceFocusEvent (DeviceFocusOut, mode, NotifyAncestor, fromWin, id);
		DeviceFocusOutEvents(
		    fromWin->parent, toWin, mode, NotifyVirtual, FALSE, id);
		DeviceFocusEvent (DeviceFocusIn, mode, NotifyInferior, toWin, id);
		if ((IsParent(toWin, sprite.win)) &&
			(sprite.win != fromWin) &&
			(!IsParent(fromWin, sprite.win)) &&
			(!IsParent(sprite.win, fromWin)))
		    DeviceFocusInEvents(
			toWin, sprite.win, NullWindow, mode,
			NotifyPointer, FALSE, id);
	        }
	    else
		if (IsParent(fromWin, toWin))
		    {
		    if ((IsParent(fromWin, sprite.win)) &&
			    (sprite.win != fromWin) &&
			    (!IsParent(toWin, sprite.win)) &&
			    (!IsParent(sprite.win, toWin)))
			DeviceFocusOutEvents(
			    sprite.win, fromWin, mode, NotifyPointer, FALSE, id);
		    DeviceFocusEvent (DeviceFocusOut, mode, NotifyInferior, fromWin, id);
		    DeviceFocusInEvents(
			fromWin, toWin, toWin, mode, NotifyVirtual, FALSE, id);
		    DeviceFocusEvent (DeviceFocusIn, mode, NotifyAncestor, toWin, id);
		    }
		else
		    {
		/* neither fromWin or toWin is child of other */
		    WindowPtr common = CommonAncestor(toWin, fromWin);
		/* common == NullWindow ==> different screens */
		    if (IsParent(fromWin, sprite.win))
			DeviceFocusOutEvents(
			    sprite.win, fromWin, mode, NotifyPointer, FALSE, id);
		    DeviceFocusEvent (DeviceFocusOut, mode, NotifyNonlinear, fromWin, id);
		    if (fromWin->parent != NullWindow)
		      DeviceFocusOutEvents(
			fromWin->parent, common, mode, NotifyNonlinearVirtual,
			FALSE, id);
		    if (toWin->parent != NullWindow)
		      DeviceFocusInEvents(
			common, toWin, toWin, mode, NotifyNonlinearVirtual,
			FALSE, id);
		    DeviceFocusEvent (DeviceFocusIn, mode, NotifyNonlinear, toWin, id);
		    if (IsParent(toWin, sprite.win))
			DeviceFocusInEvents(
			    toWin, sprite.win, NullWindow, mode,
			    NotifyPointer, FALSE, id);
		    }
	    }
        }
    }


/**********************************************************************
 *
 * This should go in dix events.c
 *
 */

static int
HPDeliverDeviceEvents(pWin, xE, grab, stopAt, fix, id, filter)
    register 	WindowPtr pWin, stopAt;
    register 	xEvent *xE;
    GrabPtr 	grab;
    int		id;
    Mask 	filter;
    {
    int     	deliveries;
    Window 	child = None;
    DeviceMasksPtr priv = (DeviceMasksPtr) pWin->devPrivate;

    if ((filter != CantBeFiltered) && !(filter & priv->deliverableEvents))
	return 0;

    while (pWin)
        {
	if (fix)
	    FixUpEventFromWindow(xE, pWin, child, FALSE);
	deliveries = DeliverDeviceEventsToWindow(pWin, xE, 1, filter, grab,id);
	if ((deliveries > 0) || (filter & pWin->dontPropagateMask))
	    return deliveries;
	if (pWin == stopAt)
	    return 0;
	child = pWin->wid;
	pWin = pWin->parent;
        }
    return 0;
    }

/****************************************************************************
 *
 * DeliverDeviceEventsToWindow.
 * 
 */

static int
DeliverDeviceEventsToWindow(pWin, pEvents, count, filter, grab,id)
    WindowPtr pWin;
    GrabPtr grab;
    xEvent *pEvents;
    int count;
    Mask filter;
    int id;
    {
    extern	int	DeviceButtonPress;
    DeviceMasksPtr	priv = (DeviceMasksPtr) pWin->devPrivate;
    int     		deliveries = 0;
    ExtOtherClientsPtr 	other;
    ClientPtr 		client = NullClient;
    Mask 		deliveryMask; 	
			/* If a grab occurs due to a button press, then
		              this mask is the mask of the grab. */
    /* if nobody ever wants to see this event, skip some work */

    if (pWin == PointerRootWin)
	{
	priv = (DeviceMasksPtr) sprite.win->devPrivate;
	}

    if (priv->client == 0)	/* no client listening.*/
	return 0;

    if ((filter != CantBeFiltered) && !(priv->allEventMasks & filter))
	return 0;
    if (DeviceTryClientEvents(
	priv->client, pEvents, count, priv->eventMask[id], filter, grab))
        {
	deliveries++;
	client = (ClientPtr) priv->client;
	deliveryMask = priv->eventMask[id];
        }
    if (filter != CantBeFiltered) /* others besides window owner get event */
	for (other=priv->otherClients; other; other = other->next)
	    {
	    if (DeviceTryClientEvents(
		  other->client, pEvents, count, other->mask[id], filter, grab))
	        {
		deliveries++;
		client = other->client;
                deliveryMask = other->mask[id];
	        }
	    }
    return deliveries;
    }

/****************************************************************************
 *
 * Try sending events to clients.
 *
 */

#define WID(w) ((w) ? ((w)->wid) : 0)

static int
DeviceTryClientEvents (client, pEvents, count, mask, filter, grab)
    ClientPtr client;
    GrabPtr grab;
    xEvent *pEvents;
    int count;
    Mask mask, filter;
{
    int i;

    if ((client) && (client != serverClient) && (!client->clientGone) &&
	((filter == CantBeFiltered) || (mask & filter)) &&
	((!grab) || (client == grab->client)))
    {
	/*
	if (pEvents->u.u.type == DeviceMotionNotify)
	{
	    if (mask & PointerMotionHintMask)
	    {
		if (WID(motionHintWindow) == pEvents->u.keyButtonPointer.event)
		{
		    return 0;
		}
		pEvents->u.u.detail = NotifyHint;
	    }
	    else
	    {
		pEvents->u.u.detail = NotifyNormal;
	    }
	}
	*/
	if (pEvents->u.u.type != DeviceKeymapNotify)
	{
	    for (i = 0; i < count; i++)
		pEvents[i].u.u.sequenceNumber = client->sequence;
	}
	WriteEventsToClient(client, count, pEvents);
	return 1;
    }
return 0;
}

/****************************************************************************
 *
 * Deactivate a grab of a device.
 * Syncronization is currently not implemented.
 *
 */

void
DeactivateDeviceGrab (dev, id)
    DeviceIntPtr dev;
    int		 id;
    {
    GrabPtr 	grab = dev->grab;
    FocusPtr	focus;

    dev->grab = NullGrab;
    dev->sync.state = NOT_GRABBED;
    if (dev->deviceProc == hpKeybdProc)
	{
	dev->u.keybd.passiveGrab = FALSE;
	focus = &dev->u.keybd.focus;
	}
    else
	{
	dev->u.ptr.passiveGrab = FALSE;
	focus = &dev->u.ptr.focus;
	}
    DoDeviceFocusEvents(grab->window, focus->win, NotifyUngrab, id);
    }

/****************************************************************************
 *
 * Main entry point for dix routing of nonstandard events.
 *
 */

WindowPtr		focus;
Bool            	deactivateGrabk = FALSE;
Bool            	deactivateGrabp = FALSE;
#define			NEXT	2

void
HpProcessOtherEvent (xE, other, add_fields)
    xEvent *xE;
    register DeviceIntPtr other;
    Bool add_fields;
    {
    extern	int	DeviceKeyPress;
    extern	int	DeviceKeyRelease;
    extern	int	DeviceButtonPress;
    extern	int	DeviceButtonRelease;
    extern	int	DeviceMotionNotify;
    extern	int	ProximityIn;
    extern	int	ProximityOut;
    int             	key, bit;
    int             	id;
    register BYTE   	*kptr;
    register int    	i;
    register CARD16 	modifiers;
    register CARD16 	mask;
    GrabPtr         	grab = other->grab;
    DeviceIntPtr	dev;
    Bool		moveIt = FALSE;
    Bool            	fix = TRUE;
    Mask		filter;
    void		DeactivateDeviceGrab();

/*
    if (xE->u.keyButtonPointer.rootX < sprite.physLimits.x1)
        {
	xE->u.keyButtonPointer.rootX = sprite.physLimits.x1;
	moveIt = TRUE;
        }
    else if (xE->u.keyButtonPointer.rootX >= sprite.physLimits.x2)
        {
	xE->u.keyButtonPointer.rootX = sprite.physLimits.x2 - 1;
	moveIt = TRUE;
        }
    if (xE->u.keyButtonPointer.rootY < sprite.physLimits.y1)
        {
	xE->u.keyButtonPointer.rootY = sprite.physLimits.y1;
	moveIt = TRUE;
        }
    else if (xE->u.keyButtonPointer.rootY >= sprite.physLimits.y2)
        {
	xE->u.keyButtonPointer.rootY = sprite.physLimits.y2 - 1;
	moveIt = TRUE;
        }
    if (moveIt)
	(*currentScreen->SetCursorPosition)(
	    currentScreen, xE->u.keyButtonPointer.rootX,
	    xE->u.keyButtonPointer.rootY, FALSE);
*/

    if (other->sync.frozen)
        {
	EnqueueEvent(other, xE);
	return;
        }

    if (add_fields)
	{
	id = xE->u.keyButtonPointer.pad1;
        NoticeTimeAndState(xE);
	}
    filter = filters[xE->u.u.type];
    key = xE->u.u.detail;
    kptr = &other->down[key >> 3];
    bit = 1 << (key & 7);
    modifiers = keyModifiersList[key];
    motionHintWindow = NullWindow;

    if (xE->u.u.type == DeviceKeyPress)
	{
	dev = inputInfo.pointer; 
        focus = other->u.keybd.focus.win;
	if (*kptr & bit) /* allow ddx to generate multiple downs */
	    {   
	    if (!modifiers)
		{
		xE->u.u.type = DeviceKeyRelease;
		HpProcessOtherEvent(xE, other);
		xE->u.u.type = DeviceKeyPress;
		/* release can have side effects, don't fall through */
		HpProcessOtherEvent(xE, other);
		}
	    return;
	    }
	*kptr |= bit;
	for (i = 0, mask = 1; modifiers; i++, mask <<= 1)
	    {
	    if (mask & modifiers) 
		{
		/* This key affects modifier "i" */
		DevicemodifierKeyCount[i]++;
		DevicekeyButtonState |= mask;
		modifiers &= ~mask;
		}
	    }
	/*
	if (!grab && CheckDeviceGrabs(other, xE, 0, TRUE))
	    {
	    DevicekeyThatActivatedPassiveGrab = key;
	    return;
	    }
	    */
	}
    else if (xE->u.u.type == DeviceKeyRelease)
	{
	dev = inputInfo.pointer;
        focus = other->u.keybd.focus.win;
	if (!(*kptr & bit)) /* guard against duplicates */
	    return;
	*kptr &= ~bit;
	for (i = 0, mask = 1; modifiers; i++, mask <<= 1)
	    {
	    if (mask & modifiers) 
		{
		/* This key affects modifier "i" */
		if (--DevicemodifierKeyCount[i] <= 0) 
		    {
		    DevicekeyButtonState &= ~mask;
		    DevicemodifierKeyCount[i] = 0;
		    }
		modifiers &= ~mask;
		}
	    }
	if ((other->u.keybd.passiveGrab) &&
		(key == DevicekeyThatActivatedPassiveGrab))
		deactivateGrabk = NEXT;
	}
    else if (xE->u.u.type == DeviceButtonPress)
	{
        focus = other->u.ptr.focus.win;
	dev = inputInfo.keyboard;
	DevicebuttonsDown++;
	DevicebuttonMotionMask = ButtonMotionMask;
	xE->u.u.detail = other->u.ptr.map[key];
	if (xE->u.u.detail <= 5)
	    DevicekeyButtonState |= keyModifiersList[xE->u.u.detail];
/*
	filters[DeviceMotionNotify] = Motion_Filter(DevicekeyButtonState);
	if (!grab)
	    if (CheckDeviceGrabs(other, xE, 0, FALSE))
	        return;
*/
	}
    else if (xE->u.u.type == DeviceButtonRelease)
	{
	dev = inputInfo.keyboard;
        focus = other->u.ptr.focus.win;
	DevicebuttonsDown--;
	if (!DevicebuttonsDown)
	    DevicebuttonMotionMask = 0;
	xE->u.u.detail = other->u.ptr.map[key];
	if (xE->u.u.detail <= 5)
	    DevicekeyButtonState &= ~keyModifiersList[xE->u.u.detail];
/*
	filters[DeviceMotionNotify] = Motion_Filter(DevicekeyButtonState);
*/
	if ((!(DevicekeyButtonState & AllButtonsMask)) &&
	    (other->u.ptr.autoReleaseGrab))
	    deactivateGrabp = NEXT;

	}
    else if (xE->u.u.type == DeviceMotionNotify)
	{
        focus = other->u.ptr.focus.win;
	dev = inputInfo.keyboard;
	/*
	if (!CheckMotion(xE->u.keyButtonPointer.rootX,
	     xE->u.keyButtonPointer.rootY, FALSE))
            return;
	    */
	}
    else if (xE->u.u.type == ProximityIn ||
             xE->u.u.type == ProximityOut)
	{
        focus = other->u.ptr.focus.win;
	dev = inputInfo.keyboard;
	}
    else if (xE->u.u.type == xEventExtension)
	{
	xHPExtensionEvent *xext = (xHPExtensionEvent *) xE;
        filter = filters[xext->ext_type];
	id = xext->deviceid;
	fix = FALSE;
	if (deactivateGrabp == NEXT)
	    deactivateGrabp = TRUE;
	if (deactivateGrabk == NEXT)
	    deactivateGrabk = TRUE;
	}
    else 
	FatalError("Impossible other event");

    if (grab)
	HPDeliverGrabbedEvent(xE, other, dev, deactivateGrabp, fix, id, filter);
    else
	HPNormalKeyboardEvent(other, xE, sprite.win, fix, focus, id, filter);

    if (deactivateGrabk == TRUE)
	{
        DeactivateDeviceGrab(other, id);
        deactivateGrabk = FALSE;
	}
    if (deactivateGrabp == TRUE)
	{
        DeactivateDeviceGrab(other, id);
        deactivateGrabp = FALSE;
	}
    }
/****************************************************************************
 *
 * Register a nonstandard device.
 * 
 */

RegisterOtherDevice (pXDev)
    DevicePtr pXDev;
    {
    pXDev->processInputProc = HpProcessOtherEvent;
    }

/****************************************************************************
 *
 * This routine is the equivalent of the dix DeleteWindowFromAnyEvents routine.
 * It cleans up any grabs and takes care of focus reversion when a window
 * is destroyed or otherwise made nonviewable.
 * 
 */

void
DeleteWindowFromAnyExtEvents(pWin, freeResources)
    WindowPtr		pWin;
    Bool		freeResources;
    {
    XID			id;
    int			i;
    int			ndx;
    WindowPtr		parent;
    FocusPtr		focus;
    DeviceIntPtr	dev;
    DevicePtr		d;
    OtherClientsPtr	oc;
    GrabPtr		passive;
    register 		DeviceMasksPtr		priv = (DeviceMasksPtr) pWin->devPrivate;
    register 		ExtOtherClientsPtr 	other;
    PtrPrivPtr 		pPtrPriv;
    HPInputDevice 	*pHPDev;
extern PtrPrivRec	*other_p[];


    /* Deactivate any device grabs performed on this window, before making any
	input focus changes. */

    for (i=XOTHER; i<MAX_LOGICAL_DEVS; i++)
	{
	dev = (DeviceIntPtr) hpOther[i];
	d = hpOther[i];

	if (dev->deviceProc == hpKeybdProc)
	    {
	    focus = &dev->u.keybd.focus;
	    pHPDev   = (HPInputDevice *) (d->devicePrivate);
	    }
	else
	    {
	    pPtrPriv = (PtrPrivPtr) (d->devicePrivate);
	    if (pPtrPriv == 0)
		continue;
	    pHPDev   = (HPInputDevice *) (pPtrPriv->devPrivate);
	    focus = &dev->u.ptr.focus;
	    }
	if (pHPDev == 0)
	    continue;
	id = pHPDev->dev_id;
	ndx = find_device (id);
	if (ndx == -1)
	    continue;
	if (l_devs[ndx].open_cnt == 0)
	    continue;

        if ((dev->grab) && (dev->grab->window == pWin))
	    DeactivateDeviceGrab (dev, id);

    /* If the focus window is a root window (ie. has no parent) then don't 
	delete the focus from it. */
    
	if ((pWin == focus->win) && (pWin->parent != NullWindow))
	    {
	    int focusEventMode = NotifyNormal;

 	/* If a grab is in progress, then alter the mode of focus events. */

	    if (dev->grab)
		focusEventMode = NotifyWhileGrabbed;

	    switch (focus->revert)
		{
		case RevertToNone:
		    DoDeviceFocusEvents(pWin, NoneWin, focusEventMode, id);
		    focus->win = NoneWin;
	            devfocusTraceGood[ndx] = 0;
		    break;
	        case RevertToParent:
		    parent = pWin;
		    do
			{
			parent = parent->parent;
			devfocusTraceGood[ndx]--;
			} while (!parent->realized);
		    DoDeviceFocusEvents (pWin, parent, focusEventMode, id);
		    focus->win = parent;
		    focus->revert = RevertToNone;
		    break;
	        case RevertToPointerRoot:
		    DoDeviceFocusEvents(pWin, PointerRootWin, focusEventMode, id);
		    focus->win = PointerRootWin;
		    devfocusTraceGood[ndx] = 0;
		    break;
		}
	    }

        if (freeResources)
	    {
	    other = (ExtOtherClientsPtr) (priv->otherClients);
	    if (other != NULL)
 		{
	        FreeResource(other->resource, RC_NONE);
		}
	    }
	}
    }
#endif /* HPINPUT */
