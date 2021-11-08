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


/* $XConsortium: events.c,v 1.161 88/10/14 14:14:34 rws Exp $ */

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

extern WindowRec WindowTable[];

extern void (* EventSwapVector[128]) ();
extern void (* ReplySwapVector[256]) ();
extern void CopySwap32Write(), SwapTimeCoordWrite();

#define NoSuchEvent 0x80000000	/* so doesn't match NoEventMask */
#define StructureAndSubMask ( StructureNotifyMask | SubstructureNotifyMask )
#define AllButtonsMask ( \
	Button1Mask | Button2Mask | Button3Mask | Button4Mask | Button5Mask )
#define MotionMask ( \
	PointerMotionMask | Button1MotionMask | \
	Button2MotionMask | Button3MotionMask | Button4MotionMask | \
	Button5MotionMask | ButtonMotionMask )
#define PropagateMask ( \
	KeyPressMask | KeyReleaseMask | ButtonPressMask | ButtonReleaseMask | \
	MotionMask )
#define AllModifiersMask ( \
	ShiftMask | LockMask | ControlMask | Mod1Mask | Mod2Mask | \
	Mod3Mask | Mod4Mask | Mod5Mask )
/*
 * The following relies on the fact that the Button<n>MotionMasks are equal
 * to the corresponding Button<n>Masks from the current modifier/button state.
 */
#define Motion_Filter(state) (PointerMotionMask | \
		(AllButtonsMask & state) | buttonMotionMask)


#define WID(w) ((w) ? ((w)->wid) : 0)

#define IsOn(ptr, bit) \
	(((BYTE *) (ptr))[(bit)>>3] & (1 << ((bit) & 7)))

static debug_events = 0;
static debug_modifiers = 0;
static InputInfo inputInfo;

static int keyThatActivatedPassiveGrab; 	/* The key that activated the
						current passive grab must be
						recorded, so that the grab may
						be deactivated upon that key's
						release. PRH
						*/

static KeySymsRec curKeySyms;

static GrabRec keybdGrab;	/* used for active grabs */
static GrabRec ptrGrab;

#define MAX_QUEUED_EVENTS 5000
static struct {
    unsigned int	num;
    QdEventRec		pending, free;	/* only forw, back used */
    DeviceIntPtr	replayDev;	/* kludgy rock to put flag for */
    WindowPtr		replayWin;	/*   ComputeFreezes            */
    Bool		playingEvents;
} syncEvents;

/*
 * The window trace information is used to avoid having to compute all the
 * windows between the root and the current pointer window each time a button
 * or key goes down. The grabs on each of those windows must be checked.
 */
static WindowPtr *spriteTrace = (WindowPtr *)NULL;
#define ROOT spriteTrace[0]
static int spriteTraceSize = 0;
static int spriteTraceGood;

static WindowPtr *focusTrace = (WindowPtr *)NULL;
static int focusTraceSize = 0;
static int focusTraceGood;

static CARD16 keyButtonState = 0;
/*
 *	For each modifier,  we keep a count of the number of keys that
 *	are currently setting it.
 */
static int modifierKeyCount[8];
static int buttonsDown = 0;		/* number of buttons currently down */
static Mask buttonMotionMask = 0;

typedef struct {
    int		x, y;
} HotSpot;

static  struct {
    CursorPtr	current;
    BoxRec	hotLimits;	/* logical constraints */
    BoxRec	physLimits;	/* of hot spot due to hardware limits */
    WindowPtr	win;
    HotSpot	hot;
} sprite;			/* info about the cursor sprite */

static WindowPtr motionHintWindow;

extern void DoEnterLeaveEvents();	/* merely forward declarations */
static WindowPtr XYToWindow();
extern Bool CheckKeyboardGrabs();
static void NormalKeyboardEvent();
extern int DeliverDeviceEvents();
static void DoFocusEvents();
extern Mask EventMaskForClient();
static WindowPtr CheckMotion();
extern void WriteEventsToClient();
static Bool CheckDeviceGrabs();

extern GrabPtr CreateGrab();		/* Defined in grabs.c */
extern void  DeleteGrab();
extern Bool GrabMatchesSecond();
extern void DeletePassiveGrabFromList();
extern void AddPassiveGrabToWindowList();

static ScreenPtr currentScreen;

/*
 *	For each key,  we keep a bitmap showing the modifiers it sets
 *	This is a CARD16 bitmap 'cos it includes the mouse buttons to
 *	make grab processing simpler.
 */
CARD16 keyModifiersList[MAP_LENGTH];
static CARD8 maxKeysPerModifier;
/*
 *	We also keep a copy of the modifier map in the format
 *	used by the protocol.
 */
static KeyCode *modifierKeyMap;

static Mask lastEventMask;

#define CantBeFiltered NoEventMask
static Mask filters[128] =
{
	NoSuchEvent,		       /* 0 */
	NoSuchEvent,		       /* 1 */
	KeyPressMask,		       /* KeyPress */
	KeyReleaseMask,		       /* KeyRelease */
	ButtonPressMask,	       /* ButtonPress */
	ButtonReleaseMask,	       /* ButtonRelease */
	MotionMask,		       /* MotionNotify - special cased */
	EnterWindowMask,	       /* EnterNotify */
	LeaveWindowMask,	       /* LeaveNotify */
	FocusChangeMask,	       /* FocusIn */
	FocusChangeMask,	       /* FocusOut */
	KeymapStateMask,	       /* KeymapNotify */
	ExposureMask,		       /* Expose */
	CantBeFiltered,		       /* GraphicsExpose */
	CantBeFiltered,		       /* NoExpose */
	VisibilityChangeMask,	       /* VisibilityNotify */
	SubstructureNotifyMask,	       /* CreateNotify */
	StructureAndSubMask,	       /* DestroyNotify */
	StructureAndSubMask,	       /* UnmapNotify */
	StructureAndSubMask,	       /* MapNotify */
	SubstructureRedirectMask,      /* MapRequest */
	StructureAndSubMask,	       /* ReparentNotify */
	StructureAndSubMask,	       /* ConfigureNotify */
	SubstructureRedirectMask,      /* ConfigureRequest */
	StructureAndSubMask,	       /* GravityNotify */
	ResizeRedirectMask,	       /* ResizeRequest */
	StructureAndSubMask,	       /* CirculateNotify */
	SubstructureRedirectMask,      /* CirculateRequest */
	PropertyChangeMask,	       /* PropertyNotify */
	CantBeFiltered,		       /* SelectionClear */
	CantBeFiltered,		       /* SelectionRequest */
	CantBeFiltered,		       /* SelectionNotify */
	ColormapChangeMask,	       /* ColormapNotify */
	CantBeFiltered		       /* InterpretNotify */
};

Mask
GetNextEventMask()
{
    lastEventMask <<= 1;
    return lastEventMask;
}

void
SetMaskForEvent(mask, event)
    Mask mask;
    int event;
{
    if ((event < LASTEvent) || (event >= 128))
	FatalError("MaskForEvent: bogus event number");
    filters[event] = mask;
}

static void
SyntheticMotion(x, y)
    int x, y;
{
    xEvent xE;

    xE.u.keyButtonPointer.rootX = x;
    xE.u.keyButtonPointer.rootY = y;
    xE.u.keyButtonPointer.time = currentTime.milliseconds;
    xE.u.u.type = MotionNotify;
    ProcessPointerEvent(&xE, inputInfo.pointer);
}

static void
CheckPhysLimits(cursor, generateEvents)
    CursorPtr cursor;
    Bool generateEvents;
{
    HotSpot new;

    if (!cursor)
	return;
    new = sprite.hot;
    (*currentScreen->CursorLimits) (
	currentScreen, cursor, &sprite.hotLimits, &sprite.physLimits);
    if (new.x < sprite.physLimits.x1)
	new.x = sprite.physLimits.x1;
    else
	if (new.x >= sprite.physLimits.x2)
	    new.x = sprite.physLimits.x2 - 1;
    if (new.y < sprite.physLimits.y1)
	new.y = sprite.physLimits.y1;
    else
	if (new.y >= sprite.physLimits.y2)
	    new.y = sprite.physLimits.y2 - 1;
    if ((new.x != sprite.hot.x) || (new.y != sprite.hot.y))
    {
	(*currentScreen->SetCursorPosition) (
	    currentScreen, new.x, new.y, generateEvents);
	if (!generateEvents)
	    SyntheticMotion(new.x, new.y);
    }
}

static void
ConfineCursorToWindow(pWin, x, y, generateEvents)
    WindowPtr pWin;
    int x, y;
    Bool generateEvents;
{
    ScreenPtr pScreen = pWin->drawable.pScreen;

    sprite.hotLimits = *(* pScreen->RegionExtents)(pWin->borderSize);
    if (currentScreen != pScreen)
    {
	/* XXX changing the (logical) currentScreen and sprite.hot is wrong;
	 * the pointer might be frozen.
	 */
	currentScreen = pScreen;
	ROOT = &WindowTable[pScreen->myNum];
	if (x < sprite.hotLimits.x1)
	    x = sprite.hotLimits.x1;
	else if (x >= sprite.hotLimits.x2)
	    x = sprite.hotLimits.x2 - 1;
	if (y < sprite.hotLimits.y1)
	    y = sprite.hotLimits.y1;
	else if (y >= sprite.hotLimits.y2)
	    y = sprite.hotLimits.y2 - 1;
	sprite.hot.x = -1; /* cause new sprite win computation */
	sprite.hot.y = -1;
	(* pScreen->SetCursorPosition)(pScreen, x, y, generateEvents);
	if (!generateEvents)
	    SyntheticMotion(x, y);
    }
    CheckPhysLimits(sprite.current, generateEvents);
    (* pScreen->ConstrainCursor)(pScreen, &sprite.physLimits);
}

static void
ChangeToCursor(cursor)
    CursorPtr cursor;
{
    if (!cursor)
	FatalError("Somebody is setting NullCursor");
    if (cursor != sprite.current)
    {
	if ((sprite.current->xhot != cursor->xhot) ||
		(sprite.current->yhot != cursor->yhot))
	    CheckPhysLimits(cursor, FALSE);
	(*currentScreen->DisplayCursor) (currentScreen, cursor);
	sprite.current = cursor;
    }
}

/* returns true if b is a descendent of a */
static Bool
IsParent(a, b)
    register WindowPtr a, b;
{
    for (b = b->parent; b; b = b->parent)
	if (b == a) return TRUE;
    return FALSE;
}

static void
PostNewCursor()
{
    register    WindowPtr win;
    register    GrabPtr grab = inputInfo.pointer->grab;
    if (grab)
    {
	if (grab->cursor)
	{
	    ChangeToCursor(grab->cursor);
	    return;
	}
	if (IsParent(grab->window, sprite.win))
	    win = sprite.win;
	else
	    win = grab->window;
    }
    else
	win = sprite.win;
    for (; win; win = win->parent)
	if (win->cursor != NullCursor)
	{
	    ChangeToCursor(win->cursor);
	    return;
	}
}

/**************************************************************************
 *            The following procedures deal with synchronous events       *
 **************************************************************************/

static void
EnqueueEvent(device, event)
    xEvent		*event;
    DeviceIntPtr	device;
{
    register QdEventPtr tail = syncEvents.pending.back;
    register QdEventPtr new;
/*
 * Collapsing of mouse events does not bother to test if qdEvents.num == 0,
 * since there will never be MotionNotify in the type of the head event which
 * is what last points at when num == 0.
 */
    if ((event->u.u.type == MotionNotify) && 
	(tail->event.u.u.type == MotionNotify))
    {
	tail->event = *event;
	return;
    }
    syncEvents.num++;
    if (syncEvents.free.forw == &syncEvents.free)
	new = (QdEventPtr)xalloc(sizeof(QdEventRec));
    else
    {
	new = syncEvents.free.forw;
	remque(new);
    }
    new->device = device;
    new->event = *event;
    insque(new, tail);
    if (syncEvents.num > MAX_QUEUED_EVENTS)
    {
	/* XXX here we send all the pending events and break the locks */
	return;
    }
}

static void
PlayReleasedEvents()
{
    register QdEventPtr qe = syncEvents.pending.forw;
    QdEventPtr next;
    while (qe != &syncEvents.pending)
    {
	register DeviceIntPtr device = qe->device;
	if (!device->sync.frozen)
	{
	    next = qe->forw;;
	    remque(qe);
	    (*device->public.processInputProc)(&qe->event, device);
	    insque(qe, &syncEvents.free);
	    qe = next;
	}
	else
	    qe = qe->forw;
    } 
}

static void
ComputeFreezes(dev1, dev2)
    DeviceIntPtr dev1, dev2;
{
    register DeviceIntPtr replayDev = syncEvents.replayDev;
    int i;
    WindowPtr w;
    Bool isKbd ;
    register xEvent *xE ;

    dev1->sync.frozen =
	((dev1->sync.other != NullGrab) || (dev1->sync.state >= FROZEN));
    dev2->sync.frozen =
	((dev2->sync.other != NullGrab) || (dev2->sync.state >= FROZEN));
    if (syncEvents.playingEvents)
	return;
    syncEvents.playingEvents = TRUE;
    if (replayDev)
    {
	isKbd = (replayDev == inputInfo.keyboard);
	xE = &replayDev->sync.event;
	syncEvents.replayDev = (DeviceIntPtr)NULL;
	w = XYToWindow(
	    xE->u.keyButtonPointer.rootX, xE->u.keyButtonPointer.rootY);
	for (i = 0; i < spriteTraceGood; i++)
	    if (syncEvents.replayWin == spriteTrace[i])
	    {
		if (!CheckDeviceGrabs(replayDev, xE, i+1, isKbd))
		    if (isKbd)
			NormalKeyboardEvent(replayDev, xE, w);
		    else
			DeliverDeviceEvents(w, xE, NullGrab, NullWindow);
		goto playmore;
	    }
	/* must not still be in the same stack */
	if (isKbd)
	    NormalKeyboardEvent(replayDev, xE, w);
	else
	    DeliverDeviceEvents(w, xE, NullGrab, NullWindow);
    }
playmore:
    if (!dev1->sync.frozen || !dev2->sync.frozen)
	PlayReleasedEvents();
    syncEvents.playingEvents = FALSE;
}

CheckGrabForSyncs(grab, thisDev, thisMode, otherDev, otherMode)
    GrabPtr grab;
    DeviceIntPtr thisDev, otherDev;
    int thisMode, otherMode;
{
    if (thisMode == GrabModeSync)
	thisDev->sync.state = FROZEN_NO_EVENT;
    else
    {	/* free both if same client owns both */
	thisDev->sync.state = THAWED;
	if (thisDev->sync.other &&
	    (thisDev->sync.other->client == grab->client))
	    thisDev->sync.other = NullGrab;
    }
    if (otherMode == GrabModeSync)
	otherDev->sync.other = grab;
    else
    {	/* free both if same client owns both */
	if (otherDev->sync.other &&
	    (otherDev->sync.other->client == grab->client))
	    otherDev->sync.other = NullGrab;
    }
    ComputeFreezes(thisDev, otherDev);
}

static void
ActivatePointerGrab(mouse, grab, time, autoGrab)
    GrabPtr grab;
    register DeviceIntPtr mouse;
    TimeStamp time;
    Bool autoGrab;
{
    WindowPtr oldWin = (mouse->grab) ? mouse->grab->window
				     : sprite.win;

    if (grab->confineTo)
	ConfineCursorToWindow(grab->confineTo, 0, 0, FALSE);
    DoEnterLeaveEvents(oldWin, grab->window, NotifyGrab);
    motionHintWindow = NullWindow;
    mouse->grabTime = time;
    ptrGrab = *grab;
    if (grab->cursor)
	grab->cursor->refcnt++;
    mouse->grab = &ptrGrab;
    mouse->u.ptr.autoReleaseGrab = autoGrab;
    PostNewCursor();
    CheckGrabForSyncs(
	mouse->grab, mouse, grab->pointerMode,
	inputInfo.keyboard, grab->keyboardMode);
}

static void
DeactivatePointerGrab(mouse)
    DeviceIntPtr mouse;
{
    GrabPtr grab = mouse->grab;
    DeviceIntPtr keybd = inputInfo.keyboard;

    motionHintWindow = NullWindow;
    mouse->grab = NullGrab;
    mouse->sync.state = NOT_GRABBED;
    mouse->u.ptr.autoReleaseGrab = FALSE;
    if (keybd->sync.other == grab)
	keybd->sync.other = NullGrab;
    DoEnterLeaveEvents(grab->window, sprite.win, NotifyUngrab);
    if (grab->confineTo)
	ConfineCursorToWindow(ROOT, 0, 0, FALSE);
    PostNewCursor();
    if (grab->cursor)
	FreeCursor(grab->cursor, 0);
    ComputeFreezes(keybd, mouse);
}

static void
ActivateKeyboardGrab(keybd, grab, time, passive)
    GrabPtr grab;
    register DeviceIntPtr keybd;
    TimeStamp time;
    Bool passive;
{
    WindowPtr oldWin = (keybd->grab) ? keybd->grab->window
				     : keybd->u.keybd.focus.win;

    DoFocusEvents(oldWin, grab->window, NotifyGrab);
    keybd->grabTime = time;
    keybdGrab = *grab;
    keybd->grab = &keybdGrab;
    keybd->u.keybd.passiveGrab = passive;
    CheckGrabForSyncs(
	keybd->grab, keybd, grab->keyboardMode,
	inputInfo.pointer, grab->pointerMode);
}

static void
DeactivateKeyboardGrab(keybd)
    DeviceIntPtr keybd;
{
    DeviceIntPtr mouse = inputInfo.pointer;
    GrabPtr grab = keybd->grab;

    keybd->grab = NullGrab;
    keybd->sync.state = NOT_GRABBED;
    keybd->u.keybd.passiveGrab = FALSE;
    if (mouse->sync.other == grab)
	mouse->sync.other = NullGrab;
    DoFocusEvents(grab->window, keybd->u.keybd.focus.win, NotifyUngrab);
    ComputeFreezes(keybd, mouse);
}

static void
AllowSome(client, time, thisDev, otherDev, newState)
    ClientPtr		client;
    TimeStamp		time;
    DeviceIntPtr	thisDev, otherDev;
    int			newState;
{
    Bool	thisGrabbed, otherGrabbed;
    TimeStamp	grabTime;

    thisGrabbed = thisDev->grab && (thisDev->grab->client == client);
    otherGrabbed = otherDev->grab && (otherDev->grab->client == client);
    if (!((thisGrabbed && thisDev->sync.state >= FROZEN) ||
	  (otherGrabbed && thisDev->sync.other)))
	return;
    if (thisGrabbed &&
	(!otherGrabbed ||
	 (CompareTimeStamps(otherDev->grabTime, thisDev->grabTime) == EARLIER)))
	grabTime = thisDev->grabTime;
    else
	grabTime = otherDev->grabTime;
    if ((CompareTimeStamps(time, currentTime) == LATER) ||
	(CompareTimeStamps(time, grabTime) == EARLIER))
	return;
    switch (newState)
    {
	case THAWED:	 	       /* Async */
	    if (thisGrabbed)
		thisDev->sync.state = THAWED;
	    if (otherGrabbed)
		thisDev->sync.other = NullGrab;
	    ComputeFreezes(thisDev, otherDev);
	    break;
	case FREEZE_NEXT_EVENT:		/* Sync */
	    if (thisGrabbed)
	    {
		thisDev->sync.state = FREEZE_NEXT_EVENT;
		if (otherGrabbed)
		    thisDev->sync.other = NullGrab;
		ComputeFreezes(thisDev, otherDev);
	    }
	    break;
	case THAWED_BOTH:		/* AsyncBoth */
	    if ((otherGrabbed && otherDev->sync.state >= FROZEN) ||
		(thisGrabbed && otherDev->sync.other))
	    {
		if (thisGrabbed)
		{
		    thisDev->sync.state = THAWED;
		    otherDev->sync.other = NullGrab;
		}
		if (otherGrabbed)
		{
		    otherDev->sync.state = THAWED;
		    thisDev->sync.other = NullGrab;
		}
		ComputeFreezes(thisDev, otherDev);
	    }
	    break;
	case FREEZE_BOTH_NEXT_EVENT:	/* SyncBoth */
	    if ((otherGrabbed && otherDev->sync.state >= FROZEN) ||
		(thisGrabbed && otherDev->sync.other))
	    {
		if (thisGrabbed)
		{
		    thisDev->sync.state = FREEZE_BOTH_NEXT_EVENT;
		    otherDev->sync.other = NullGrab;
		}
		if (otherGrabbed)
		{
		    otherDev->sync.state = FREEZE_BOTH_NEXT_EVENT;
		    thisDev->sync.other = NullGrab;
		}
		ComputeFreezes(thisDev, otherDev);
	    }
	    break;
	case NOT_GRABBED:		/* Replay */
	    if (thisGrabbed && thisDev->sync.state == FROZEN_WITH_EVENT)
	    {
		syncEvents.replayDev = thisDev;
		syncEvents.replayWin = thisDev->grab->window;
		if (thisDev == inputInfo.pointer)
		    DeactivatePointerGrab(thisDev);
		else
 		    DeactivateKeyboardGrab(thisDev);
		syncEvents.replayDev = (DeviceIntPtr)NULL;
	    }
	    break;
    }
}

int
ProcAllowEvents(client)
    register ClientPtr client;
{
    TimeStamp		time;
    DeviceIntPtr	mouse = inputInfo.pointer;
    DeviceIntPtr	keybd = inputInfo.keyboard;
    REQUEST(xAllowEventsReq);

    REQUEST_SIZE_MATCH(xAllowEventsReq);
    time = ClientTimeToServerTime(stuff->time);
    switch (stuff->mode)
    {
	case ReplayPointer:
	    AllowSome(client, time, mouse, keybd, NOT_GRABBED);
	    break;
	case SyncPointer: 
	    AllowSome(client, time, mouse, keybd, FREEZE_NEXT_EVENT);
	    break;
	case AsyncPointer: 
	    AllowSome(client, time, mouse, keybd, THAWED);
	    break;
	case ReplayKeyboard: 
	    AllowSome(client, time, keybd, mouse, NOT_GRABBED);
	    break;
	case SyncKeyboard: 
	    AllowSome(client, time, keybd, mouse, FREEZE_NEXT_EVENT);
	    break;
	case AsyncKeyboard: 
	    AllowSome(client, time, keybd, mouse, THAWED);
	    break;
	case SyncBoth:
	    AllowSome(client, time, keybd, mouse, FREEZE_BOTH_NEXT_EVENT);
	    break;
	case AsyncBoth:
	    AllowSome(client, time, keybd, mouse, THAWED_BOTH);
	    break;
	default: 
	    client->errorValue = stuff->mode;
	    return BadValue;
    }
    return Success;
}

void
ReleaseActiveGrabs(client)
    ClientPtr client;
{
    int i;
    register DeviceIntPtr d;
    for (i = 0; i < inputInfo.numDevices; i++)
    {
	d = inputInfo.devices[i];
	if (d->grab && (d->grab->client == client))
	{
	    if (d == inputInfo.keyboard)
		DeactivateKeyboardGrab(d);
	    else if (d == inputInfo.pointer)
		DeactivatePointerGrab(d);
	    else
		d->grab = NullGrab;
	}
    }
}

/**************************************************************************
 *            The following procedures deal with delivering events        *
 **************************************************************************/

int
TryClientEvents (client, pEvents, count, mask, filter, grab)
    ClientPtr client;
    GrabPtr grab;
    xEvent *pEvents;
    int count;
    Mask mask, filter;
{
    int i;

    if (debug_events) ErrorF(
	"Event([%d, %d], mask=0x%x), client=%d",
	pEvents->u.u.type, pEvents->u.u.detail, mask, client->index);
    if ((client) && (client != serverClient) && (!client->clientGone) &&
	((filter == CantBeFiltered) || (mask & filter)))
    {
	if (grab && (client != grab->client))
	    return -1; /* don't send, but notify caller */
	if (pEvents->u.u.type == MotionNotify)
	{
	    if (mask & PointerMotionHintMask)
	    {
		if (WID(motionHintWindow) == pEvents->u.keyButtonPointer.event)
		{
		    if (debug_events) ErrorF("\n");
		    return 1; /* don't send, but pretend we did */
		}
		pEvents->u.u.detail = NotifyHint;
	    }
	    else
	    {
		pEvents->u.u.detail = NotifyNormal;
	    }
	}
	if ((pEvents->u.u.type & 0177) != KeymapNotify)
	{
	    for (i = 0; i < count; i++)
		pEvents[i].u.u.sequenceNumber = client->sequence;
	}

	if (filters[pEvents->u.u.type] &
	    (ButtonPressMask | ButtonReleaseMask
             | KeyPressMask | KeyReleaseMask))
		SetCriticalOutputPending();

	WriteEventsToClient(client, count, pEvents);
	if (debug_events) ErrorF(  " delivered\n");
	return 1;
    }
    else
    {
	if (debug_events) ErrorF("\n");
	return 0;
    }
}

static int
DeliverEventsToWindow(pWin, pEvents, count, filter, grab)
    WindowPtr pWin;
    GrabPtr grab;
    xEvent *pEvents;
    int count;
    Mask filter;
{
    int deliveries = 0, nondeliveries = 0;
    int attempt;
    OtherClients *other;
    ClientPtr client = NullClient;
    Mask deliveryMask; 	/* If a grab occurs due to a button press, then
		              this mask is the mask of the grab. */

/* if nobody ever wants to see this event, skip some work */
    if ((filter != CantBeFiltered) && !(pWin->allEventMasks & filter))
	return 0;
    if (attempt = TryClientEvents(
	pWin->client, pEvents, count, pWin->eventMask, filter, grab))
    {
	if (attempt > 0)
	{
	    deliveries++;
	    client = pWin->client;
	    deliveryMask = pWin->eventMask;
	} else
	    nondeliveries--;
    }
    if (filter != CantBeFiltered) /* CantBeFiltered means only window owner gets the event */
	for (other = OTHERCLIENTS(pWin); other; other = other->next)
	{
	    if (attempt = TryClientEvents(
		  other->client, pEvents, count, other->mask, filter, grab))
	    {
		if (attempt > 0)
		{
		    deliveries++;
		    client = other->client;
		    deliveryMask = other->mask;
		} else
		    nondeliveries--;
	    }
	}
    if ((pEvents->u.u.type == ButtonPress) && deliveries && (!grab))
    {
	GrabRec tempGrab;

	tempGrab.device = inputInfo.pointer;
	tempGrab.client = client;
	tempGrab.window = pWin;
	tempGrab.ownerEvents = (deliveryMask & OwnerGrabButtonMask) ? TRUE : FALSE;
	tempGrab.eventMask =  deliveryMask;
	tempGrab.keyboardMode = GrabModeAsync;
	tempGrab.pointerMode = GrabModeAsync;
	tempGrab.confineTo = NullWindow;
	tempGrab.cursor = NullCursor;
	ActivatePointerGrab(inputInfo.pointer, &tempGrab, currentTime, TRUE);
    }
    else if ((pEvents->u.u.type == MotionNotify) && deliveries)
	motionHintWindow = pWin;
    if (deliveries)
	return deliveries;
    return nondeliveries;
}

/* If the event goes to dontDeliverToMe, don't send it and return 0.  if
   send works,  return 1 or if send didn't work, return 2.
*/

int
MaybeDeliverEventsToClient(pWin, pEvents, count, filter, dontDeliverToMe)
    WindowPtr pWin;
    xEvent *pEvents;
    int count;
    Mask filter;
    ClientPtr dontDeliverToMe;
{
    OtherClients * other;

    if (pWin->eventMask & filter)
    {
        if (pWin->client == dontDeliverToMe)
		return 0;
	return TryClientEvents(
	    pWin->client, pEvents, count, pWin->eventMask, filter, NullGrab);
    }
    for (other = OTHERCLIENTS(pWin); other; other = other->next)
	if (other->mask & filter)
	{
            if (other->client == dontDeliverToMe)
		return 0;
	    return TryClientEvents(
		other->client, pEvents, count, other->mask, filter, NullGrab);
	}
    return 2;
}

static WindowPtr
RootForWindow(pWin)
    WindowPtr pWin;
{
    return &WindowTable[pWin->drawable.pScreen->myNum];
}

static void
FixUpEventFromWindow(xE, pWin, child, calcChild)
    xEvent *xE;
    WindowPtr pWin;
    Window child;
    Bool calcChild;
{
    if (calcChild)
    {
        WindowPtr w=spriteTrace[spriteTraceGood-1];

	/* If the search ends up past the root should the child field be 
	 	set to none or should the value in the argument be passed 
		through. It probably doesn't matter since everyone calls 
		this function with child == None anyway. */

        while (w) 
        {
            /* If the source window is same as event window, child should be
		none.  Don't bother going all all the way back to the root. */

 	    if (w == pWin)
	    { 
   		child = None;
 		break;
	    }
	    
	    if (w->parent == pWin)
	    {
		child = w->wid;
		break;
            }
 	    w = w->parent;
        } 	    
    }
    xE->u.keyButtonPointer.root = ROOT->wid;
    xE->u.keyButtonPointer.event = pWin->wid;
    if (currentScreen == pWin->drawable.pScreen)
    {
	xE->u.keyButtonPointer.sameScreen = xTrue;
	xE->u.keyButtonPointer.child = child;
	xE->u.keyButtonPointer.eventX =
	    xE->u.keyButtonPointer.rootX - pWin->absCorner.x;
	xE->u.keyButtonPointer.eventY =
	    xE->u.keyButtonPointer.rootY - pWin->absCorner.y;
    }
    else
    {
	xE->u.keyButtonPointer.sameScreen = xFalse;
	xE->u.keyButtonPointer.child = None;
	xE->u.keyButtonPointer.eventX = 0;
	xE->u.keyButtonPointer.eventY = 0;
    }
}


int
DeliverDeviceEvents(pWin, xE, grab, stopAt)
    register WindowPtr pWin, stopAt;
    register xEvent *xE;
    GrabPtr grab;
{
    Mask filter;
    int     deliveries;
    Window child = None;

    filter = filters[xE->u.u.type];
    if ((filter != CantBeFiltered) && !(filter & pWin->deliverableEvents))
	return 0;
    while (pWin)
    {
	FixUpEventFromWindow(xE, pWin, child, FALSE);
	deliveries = DeliverEventsToWindow(pWin, xE, 1, filter, grab);
	if (deliveries > 0)
	    return deliveries;
	if ((deliveries < 0) ||
	    (pWin == stopAt) ||
	    (filter & pWin->dontPropagateMask))
	    return 0;
	child = pWin->wid;
	pWin = pWin->parent;
    }
    return 0;
}

int
DeliverEvents(pWin, xE, count, otherParent)
/* not useful for events that propagate up the tree */
    register WindowPtr pWin, otherParent;
    register xEvent *xE;
    int count;
{
    Mask filter;
    int     deliveries;

    if (!count)
	return 0;
    filter = filters[xE->u.u.type];
    if ((filter & SubstructureNotifyMask) && (xE->u.u.type != CreateNotify))
	xE->u.destroyNotify.event = pWin->wid;
    if (filter != StructureAndSubMask)
	return DeliverEventsToWindow(pWin, xE, count, filter, NullGrab);
    deliveries = DeliverEventsToWindow(
	    pWin, xE, count, StructureNotifyMask, NullGrab);
    if (pWin->parent)
    {
	xE->u.destroyNotify.event = pWin->parent->wid;
	deliveries += DeliverEventsToWindow(
		pWin->parent, xE, count, SubstructureNotifyMask, NullGrab);
	if (xE->u.u.type == ReparentNotify)
	{
	    xE->u.destroyNotify.event = otherParent->wid;
	    deliveries += DeliverEventsToWindow(
		    otherParent, xE, count, SubstructureNotifyMask, NullGrab);
	}
    }
    return deliveries;
}

/* 
 * XYToWindow is only called by CheckMotion after it has determined that
 * the current cache is not accurate.
 */
static WindowPtr 
XYToWindow(x, y)
	int x, y;
{
    register WindowPtr  pWin;

    spriteTraceGood = 1;	/* root window still there */
    pWin = ROOT->firstChild;
    while (pWin)
    {
	if ((pWin->mapped) &&
		(x >= pWin->absCorner.x - pWin->borderWidth) &&
		(x < pWin->absCorner.x + (int)pWin->clientWinSize.width +
		    pWin->borderWidth) &&
		(y >= pWin->absCorner.y - pWin->borderWidth) &&
		(y < pWin->absCorner.y + (int)pWin->clientWinSize.height +
		    pWin->borderWidth))
	{
	    if (spriteTraceGood >= spriteTraceSize)
	    {
		spriteTraceSize += 10;
		spriteTrace = (WindowPtr *)xrealloc(
		    spriteTrace, spriteTraceSize*sizeof(WindowPtr));
	    }
	    spriteTrace[spriteTraceGood] = pWin;
	    pWin = spriteTrace[spriteTraceGood++]->firstChild;
	}
	else
	    pWin = pWin->nextSib;
    }
    return spriteTrace[spriteTraceGood-1];
}

static WindowPtr 
CheckMotion(x, y, ignoreCache)
    int x, y;
    Bool ignoreCache;
{
    WindowPtr prevSpriteWin = sprite.win;

    if ((x != sprite.hot.x) || (y != sprite.hot.y))
    {
	sprite.win = XYToWindow(x, y);
	sprite.hot.x = x;
	sprite.hot.y = y;
/* XXX Do PointerNonInterestBox here */
/*
	if (!(sprite.win->deliverableEvents & Motion_Filter(keyButtonState)))
        {
	    
	}
*/
    }
    else
    {
	if ((ignoreCache) || (!sprite.win))
	    sprite.win = XYToWindow(x, y);
    }
    if (sprite.win != prevSpriteWin)
    {
	if (prevSpriteWin != NullWindow)
	    DoEnterLeaveEvents(prevSpriteWin, sprite.win, NotifyNormal);
	PostNewCursor();
        return NullWindow;
    }
    return sprite.win;
}

WindowsRestructured()
{
    (void) CheckMotion(sprite.hot.x, sprite.hot.y, TRUE);
}

void
DefineInitialRootWindow(win)
    WindowPtr win;
{
    register CursorPtr c = win->cursor;

    sprite.hot.x = currentScreen->width / 2;
    sprite.hot.y = currentScreen->height / 2;
    sprite.win = win;
    sprite.current = c;
    spriteTraceGood = 1;
    ROOT = win;
    (*currentScreen->CursorLimits) (
	currentScreen, win->cursor, &sprite.hotLimits, &sprite.physLimits);
    (*currentScreen->ConstrainCursor) (
	currentScreen, &sprite.physLimits);
    (*currentScreen->SetCursorPosition) (
	currentScreen, sprite.hot.x, sprite.hot.y, FALSE);
    (*currentScreen->DisplayCursor) (currentScreen, c);
}

/*
 * This does not take any shortcuts, and even ignores its argument, since
 * it does not happen very often, and one has to walk up the tree since
 * this might be a newly instantiated cursor for an intermediate window
 * between the one the pointer is in and the one that the last cursor was
 * instantiated from.
 */
/*ARGSUSED*/
void
WindowHasNewCursor(pWin)
    WindowPtr pWin;
{
    PostNewCursor();
}

Bool
PointerConfinedToScreen()
{
    register GrabPtr grab = inputInfo.pointer->grab;

    return (grab && grab->confineTo);
}

void
NewCurrentScreen(newScreen, x, y)
    ScreenPtr newScreen;
    int x,y;
{
    /* XXX need to distinguish logical/physical screens when frozen */
    if (newScreen != currentScreen)
	ConfineCursorToWindow(&WindowTable[newScreen->myNum], x, y, TRUE);
}

int
ProcWarpPointer(client)
    ClientPtr client;
{
    WindowPtr	dest = NULL;
    int		x, y;
    ScreenPtr	newScreen;

    REQUEST(xWarpPointerReq);

    REQUEST_SIZE_MATCH(xWarpPointerReq);
    if (stuff->dstWid != None)
    {
	dest = LookupWindow(stuff->dstWid, client);
	if (!dest)
	    return BadWindow;
    }
    if (stuff->srcWid != None)
    {
	int     winX, winY;
        WindowPtr source = LookupWindow(stuff->srcWid, client);
	if (!source)
	    return BadWindow;
	winX = source->absCorner.x;
	winY = source->absCorner.y;
	if (
		(sprite.hot.x < (winX + stuff->srcX)) ||
		(sprite.hot.y < (winY + stuff->srcY)) ||
		((stuff->srcWidth != 0) &&
		    (winX + stuff->srcX + (int)stuff->srcWidth < sprite.hot.x)) ||
		((stuff->srcHeight != 0) &&
		    (winY + stuff->srcY + (int)stuff->srcHeight < sprite.hot.y)) ||
		(!PointInWindowIsVisible(source, sprite.hot.x, sprite.hot.y)))
	    return Success;
    }
    if (dest)
    {
	x = dest->absCorner.x + stuff->dstX;
	y = dest->absCorner.y + stuff->dstY;
	newScreen = dest->drawable.pScreen;
    } else {
	x = sprite.hot.x + stuff->dstX;
	y = sprite.hot.y + stuff->dstY;
	newScreen = currentScreen;
    }
    if (x < 0)
	x = 0;
    else if (x >= newScreen->width)
	x = newScreen->width - 1;
    if (y < 0)
	y = 0;
    else if (y >= newScreen->height)
	y = newScreen->height - 1;

    if (newScreen == currentScreen)
    {
	if (x < sprite.physLimits.x1)
	    x = sprite.physLimits.x1;
	else if (x >= sprite.physLimits.x2)
	    x = sprite.physLimits.x2 - 1;
	if (y < sprite.physLimits.y1)
	    y = sprite.physLimits.y1;
	else if (y >= sprite.physLimits.y2)
	    y = sprite.physLimits.y2 - 1;
	(*newScreen->SetCursorPosition)(newScreen, x, y, TRUE);
    }
    else if (!PointerConfinedToScreen())
    {
	NewCurrentScreen(newScreen, x, y);
    }
    return Success;
}

static void
NoticeTimeAndState(xE)
    register xEvent *xE;
{
    if (xE->u.keyButtonPointer.time < currentTime.milliseconds)
	currentTime.months++;
    currentTime.milliseconds = xE->u.keyButtonPointer.time;
    xE->u.keyButtonPointer.pad1 = 0;
    xE->u.keyButtonPointer.state = keyButtonState;
}

/* "CheckPassiveGrabsOnWindow" checks to see if the event passed in causes a
	passive grab set on the window to be activated. */

static Bool
CheckPassiveGrabsOnWindow(pWin, device, xE, isKeyboard)
    WindowPtr pWin;
    register DeviceIntPtr device;
    register xEvent *xE;
    int isKeyboard;
{
    GrabPtr grab;
    GrabRec temporaryGrab;

    temporaryGrab.window = pWin;
    temporaryGrab.device = device;
    temporaryGrab.detail.exact = xE->u.u.detail;
    temporaryGrab.detail.pMask = NULL;
    temporaryGrab.modifiersDetail.exact = xE->u.keyButtonPointer.state
					    & AllModifiersMask;
    temporaryGrab.modifiersDetail.pMask = NULL;

    for (grab = PASSIVEGRABS(pWin); grab; grab = grab->next)
    {
	if (GrabMatchesSecond(&temporaryGrab, grab))
	{
	    if (isKeyboard)
		ActivateKeyboardGrab(device, grab, currentTime, TRUE);
	    else
		ActivatePointerGrab(device, grab, currentTime, TRUE);
 
	    FixUpEventFromWindow(xE, grab->window, None, TRUE);

	    (void) TryClientEvents(grab->client, xE, 1, grab->eventMask, 
				   filters[xE->u.u.type],  grab);

	    if (device->sync.state == FROZEN_NO_EVENT)
	    {
	    	device->sync.event = *xE;
	    	device->sync.state = FROZEN_WITH_EVENT;
            }	

	    return TRUE;
	}
    }

    return FALSE;
}

/*
"CheckDeviceGrabs" handles both keyboard and pointer events that may cause
a passive grab to be activated.  If the event is a keyboard event, the
ancestors of the focus window are traced down and tried to see if they have
any passive grabs to be activated.  If the focus window itself is reached and
it's descendants contain they pointer, the ancestors of the window that the
pointer is in are then traced down starting at the focus window, otherwise no
grabs are activated.  If the event is a pointer event, the ancestors of the
window that the pointer is in are traced down starting at the root until
CheckPassiveGrabs causes a passive grab to activate or all the windows are
tried. PRH
*/

static Bool
CheckDeviceGrabs(device, xE, checkFirst, isKeyboard)
    register DeviceIntPtr device;
    register xEvent *xE;
    int checkFirst;
{
    int i;
    WindowPtr pWin;

    i = checkFirst;

    if (isKeyboard)
    {
	for (; i < focusTraceGood; i++)
	{
	    pWin = focusTrace[i];
	    if (CheckPassiveGrabsOnWindow(pWin, device, xE, isKeyboard))
		return TRUE;
	}
  
	if ((device->u.keybd.focus.win == NoneWin) ||
	    (i >= spriteTraceGood) ||
	    ((i > 0) && (pWin != spriteTrace[i-1])))
	    return FALSE;
    }
        
    for (; i < spriteTraceGood; i++)
    {
	pWin = spriteTrace[i];
	if (CheckPassiveGrabsOnWindow(pWin, device, xE, isKeyboard))
	    return TRUE;
    }

    return FALSE;
}

static void
NormalKeyboardEvent(keybd, xE, window)
    xEvent *xE;
    DeviceIntPtr keybd;
    WindowPtr window;
{
    WindowPtr focus = keybd->u.keybd.focus.win;
    if (focus == NullWindow)
	return;
    if (focus == PointerRootWin)
    {
	DeliverDeviceEvents(window, xE, NullGrab, NullWindow);
	return;
    }
    if ((focus == window) || IsParent(focus, window))
    {
	if (DeliverDeviceEvents(window, xE, NullGrab, focus))
	    return;
    }
 /* just deliver it to the focus window */
    FixUpEventFromWindow(xE, focus, None, FALSE);
    (void)DeliverEventsToWindow(focus, xE, 1, filters[xE->u.u.type], NullGrab);
}

static void
DeliverGrabbedEvent(xE, thisDev, otherDev, deactivateGrab, isKeyboard)
    register xEvent *xE;
    register DeviceIntPtr thisDev;
    DeviceIntPtr otherDev;
    Bool deactivateGrab;
    Bool isKeyboard;
{
    register GrabPtr grab = thisDev->grab;
    int deliveries = 0;

    if (grab->ownerEvents)
    {
	WindowPtr focus = isKeyboard ? thisDev->u.keybd.focus.win
				     : PointerRootWin;

	if (focus == PointerRootWin)
	    deliveries = DeliverDeviceEvents(sprite.win, xE, grab, NullWindow);
	else if (focus && (focus == sprite.win || IsParent(focus, sprite.win)))
	    deliveries = DeliverDeviceEvents(sprite.win, xE, grab, focus);
    }
    if (!deliveries)
    {
	FixUpEventFromWindow(xE, grab->window, None, TRUE);
	deliveries = TryClientEvents(grab->client, xE, 1, grab->eventMask,
				     filters[xE->u.u.type], grab);
	if (deliveries && (xE->u.u.type == MotionNotify))
	    motionHintWindow = grab->window;
    }
    if (deliveries && !deactivateGrab && (xE->u.u.type != MotionNotify))
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

void
ProcessKeyboardEvent (xE, keybd)
    register xEvent *xE;
    register DeviceIntPtr keybd;
{
    int             key, bit;
    register BYTE   *kptr;
    register int    i;
    register CARD16 modifiers;
    register CARD16 mask;

    GrabPtr         grab = keybd->grab;
    Bool            deactivateGrab = FALSE;

    if (keybd->sync.frozen)
    {
	EnqueueEvent(keybd, xE);
	return;
    }
    NoticeTimeAndState(xE);
    key = xE->u.u.detail;
    kptr = &keybd->down[key >> 3];
    bit = 1 << (key & 7);
    modifiers = keyModifiersList[key];
    switch (xE->u.u.type)
    {
	case KeyPress: 
	    if (*kptr & bit) /* allow ddx to generate multiple downs */
	    {   
		if (!modifiers)
		{
		    xE->u.u.type = KeyRelease;
		    ProcessKeyboardEvent(xE, keybd);
		    xE->u.u.type = KeyPress;
		    /* release can have side effects, don't fall through */
		    ProcessKeyboardEvent(xE, keybd);
		}
		return;
	    }
	    motionHintWindow = NullWindow;
	    *kptr |= bit;
	    for (i = 0, mask = 1; modifiers; i++, mask <<= 1)
	    {
		if (mask & modifiers) {
		    /* This key affects modifier "i" */
		    modifierKeyCount[i]++;
		    keyButtonState |= mask;
		    modifiers &= ~mask;
		}
	    }
	    if (!grab && CheckDeviceGrabs(keybd, xE, 0, TRUE))
	    {
		keyThatActivatedPassiveGrab = key;
		return;
	    }
	    break;
	case KeyRelease: 
	    if (!(*kptr & bit)) /* guard against duplicates */
		return;
	    motionHintWindow = NullWindow;
	    *kptr &= ~bit;
	    for (i = 0, mask = 1; modifiers; i++, mask <<= 1)
	    {
		if (mask & modifiers) {
		    /* This key affects modifier "i" */
		    if (--modifierKeyCount[i] <= 0) {
			keyButtonState &= ~mask;
			modifierKeyCount[i] = 0;
		    }
		    modifiers &= ~mask;
		}
	    }
	    if ((keybd->u.keybd.passiveGrab) &&
			(key == keyThatActivatedPassiveGrab))
		deactivateGrab = TRUE;
	    break;
	default: 
	    FatalError("Impossible keyboard event");
    }
    if (grab)
	DeliverGrabbedEvent(xE, keybd, inputInfo.pointer, deactivateGrab,
			    TRUE);
    else
	NormalKeyboardEvent(keybd, xE, sprite.win);
    if (deactivateGrab)
        DeactivateKeyboardGrab(keybd);
}

void
ProcessPointerEvent (xE, mouse)
    register xEvent 		*xE;
    register DeviceIntPtr 	mouse;
{
    register int    	key;
    register GrabPtr	grab = mouse->grab;
    Bool		moveIt = FALSE;
    Bool                deactivateGrab = FALSE;
    register BYTE	*kptr;
    int			bit;

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
    {
	/* XXX if playing a queued event, should we move phyiscally? */
	(*currentScreen->SetCursorPosition)(
	    currentScreen, xE->u.keyButtonPointer.rootX,
	    xE->u.keyButtonPointer.rootY, FALSE);
    }
    if (mouse->sync.frozen)
    {
	/* XXX need to queue physical screen with event */
	EnqueueEvent(mouse, xE);
	return;
    }
    NoticeTimeAndState(xE);
    key = xE->u.u.detail;
    kptr = &mouse->down[key >> 3];
    bit = 1 << (key & 7);
    switch (xE->u.u.type)
    {
	case ButtonPress: 
	    motionHintWindow = NullWindow;
	    buttonsDown++;
	    buttonMotionMask = ButtonMotionMask;
	    *kptr |= bit;
	    xE->u.u.detail = mouse->u.ptr.map[key];
	    if (xE->u.u.detail == 0)
		return;
	    if (xE->u.u.detail <= 5)
		keyButtonState |= keyModifiersList[xE->u.u.detail];
	    filters[MotionNotify] = Motion_Filter(keyButtonState);
	    if (!grab)
		if (CheckDeviceGrabs(mouse, xE, 0, FALSE))
		    return;
	    break;
	case ButtonRelease: 
	    motionHintWindow = NullWindow;
	    buttonsDown--;
	    if (!buttonsDown)
		buttonMotionMask = 0;
	    *kptr &= ~bit;
	    xE->u.u.detail = mouse->u.ptr.map[key];
	    if (xE->u.u.detail == 0)
		return;
	    if (xE->u.u.detail <= 5)
		keyButtonState &= ~keyModifiersList[xE->u.u.detail];
	    filters[MotionNotify] = Motion_Filter(keyButtonState);
	    if ((!(keyButtonState & AllButtonsMask)) &&
		(mouse->u.ptr.autoReleaseGrab))
		deactivateGrab = TRUE;
	    break;
	case MotionNotify: 
	    if (!CheckMotion(xE->u.keyButtonPointer.rootX,
			     xE->u.keyButtonPointer.rootY, 
			     FALSE))
                return;
	    break;
	default: 
	    FatalError("bogus pointer event from ddx");
    }
    if (grab)
	DeliverGrabbedEvent(xE, mouse, inputInfo.keyboard, deactivateGrab,
			    FALSE);
    else
	DeliverDeviceEvents(sprite.win, xE, NullGrab, NullWindow);
    if (deactivateGrab)
        DeactivatePointerGrab(mouse);
}

/*ARGSUSED*/
void
ProcessOtherEvent (xE, pDevice)
    xEvent *xE;
    DevicePtr pDevice;
{
/*	XXX What should be done here ?
    Bool propogate = filters[xE->type];
*/
}

#define AtMostOneClient \
	(SubstructureRedirectMask | ResizeRedirectMask | ButtonPressMask)

void
RecalculateDeliverableEvents(pWin)
    WindowPtr pWin;
{
    OtherClients * others;
    WindowPtr child;

    pWin->allEventMasks = pWin->eventMask;
    for (others = OTHERCLIENTS(pWin); others; others = others->next)
    {
	pWin->allEventMasks |= others->mask;
    }
    if (pWin->parent)
	pWin->deliverableEvents = pWin->allEventMasks |
	    (pWin->parent->deliverableEvents & ~pWin->dontPropagateMask &
	     PropagateMask);
    else
	pWin->deliverableEvents = pWin->allEventMasks;
    for (child = pWin->firstChild; child; child = child->nextSib)
	RecalculateDeliverableEvents(child);
}

static int
OtherClientGone(pWin, id)
    WindowPtr pWin;
    XID   id;
{
    register OtherClientsPtr *next;
    register OtherClientsPtr other;

    for (next = (OtherClientsPtr *)&(pWin->otherClients);
	 *next; next = &((*next)->next))
    {
	if ((other = *next)->resource == id)
	{
	    *next = other->next;
	    xfree(other);
	    RecalculateDeliverableEvents(pWin);
	    return(Success);
	}
    }
    FatalError("client not on event list");
    /*NOTREACHED*/
}

int
PassiveClientGone(pWin, id)
    WindowPtr pWin;
    XID   id;
{
    register GrabPtr *next;
    register GrabPtr grab;

    for (next = (GrabPtr *)&(pWin->passiveGrabs);
	 *next; next = &((*next)->next))
    {
	if ((grab = *next)->resource == id)
	{
	    *next = grab->next;
	    DeleteGrab(grab);
	    return(Success);
	}
    }
    FatalError("client not on passive grab list");
    /*NOTREACHED*/
}

int
EventSelectForWindow(pWin, client, mask)
	WindowPtr pWin;
	ClientPtr client;
	Mask mask;
{
    Mask check;
    OtherClients * others;

    check = (mask & AtMostOneClient);
    if (check & pWin->allEventMasks)
    {				       /* It is illegal for two different
				          clients to select on any of the
				          events for AtMostOneClient. However,
				          it is OK, for some client to
				          continue selecting on one of those
				          events.  */
	if ((pWin->client != client) && (check & pWin->eventMask))
	    return BadAccess;
	for (others = OTHERCLIENTS(pWin); others; others = others->next)
	{
	    if ((others->client != client) && (check & others->mask))
		return BadAccess;
	}
    }
    if (pWin->client == client)
    {
	check = pWin->eventMask;
	pWin->eventMask = mask;
    }
    else
    {
	for (others = OTHERCLIENTS(pWin); others; others = others->next)
	{
	    if (others->client == client)
	    {
		check = others->mask;
		if (mask == 0)
		{
		    FreeResource(others->resource, RC_NONE);
		    return Success;
		}
		else
		    others->mask = mask;
		goto maskSet;
	    }
	}
	check = 0;
	others = (OtherClients *) xalloc(sizeof(OtherClients));
	others->client = client;
	others->mask = mask;
	others->resource = FakeClientID(client->index);
	others->next = OTHERCLIENTS(pWin);
	pWin->otherClients = (pointer)others;
	AddResource(others->resource, RT_FAKE, (pointer)pWin,
		    OtherClientGone, RC_CORE);
    }
maskSet: 
    if ((motionHintWindow == pWin) &&
	(mask & PointerMotionHintMask) &&
	!(check & PointerMotionHintMask) &&
	!inputInfo.pointer->grab)
	motionHintWindow = NullWindow;
    RecalculateDeliverableEvents(pWin);
    return Success;
}

/*ARGSUSED*/
int
EventSuppressForWindow(pWin, client, mask)
	WindowPtr pWin;
	ClientPtr client;
	Mask mask;
{
    pWin->dontPropagateMask = mask;
    RecalculateDeliverableEvents(pWin);
    return Success;
}

static WindowPtr 
CommonAncestor(a, b)
    register WindowPtr a, b;
{
    for (b = b->parent; b; b = b->parent)
	if (IsParent(b, a)) return b;
    return NullWindow;
}

static void
EnterLeaveEvent(type, mode, detail, pWin)
    int type, mode, detail;
    WindowPtr pWin;
{
    xEvent		event;
    DeviceIntPtr	keybd = inputInfo.keyboard;
    WindowPtr		focus = keybd->u.keybd.focus.win;
    GrabPtr		grab = inputInfo.pointer->grab;

    if ((pWin == motionHintWindow) && (detail != NotifyInferior))
	motionHintWindow = NullWindow;
    if ((mode == NotifyNormal) &&
	grab && !grab->ownerEvents && (grab->window != pWin))
	return;
    event.u.u.type = type;
    event.u.u.detail = detail;
    event.u.enterLeave.time = currentTime.milliseconds;
    event.u.enterLeave.rootX = sprite.hot.x;
    event.u.enterLeave.rootY = sprite.hot.y;
 /* This call counts on same initial structure beween enter & button events */
    FixUpEventFromWindow(&event, pWin, None, TRUE);		
    event.u.enterLeave.flags = event.u.keyButtonPointer.sameScreen ?
					ELFlagSameScreen : 0;
    event.u.enterLeave.state = keyButtonState;
    event.u.enterLeave.mode = mode;
    if ((focus != NoneWin) &&
	((pWin == focus) || (focus == PointerRootWin) ||
	 IsParent(focus, pWin)))
	event.u.enterLeave.flags |= ELFlagFocus;
    (void)DeliverEventsToWindow(pWin, &event, 1, filters[type], grab);
    if (type == EnterNotify)
    {
	xKeymapEvent ke;
	ke.type = KeymapNotify;
	bcopy((char *)&keybd->down[1], (char *)&ke.map[0], 31);
	(void)DeliverEventsToWindow(pWin, (xEvent *)&ke, 1,
				    KeymapStateMask, grab);
    }
}

static void
EnterNotifies(ancestor, child, mode, detail)
    WindowPtr ancestor, child;
    int mode, detail;
{
    if (!child || (ancestor == child))
	return;
    EnterNotifies(ancestor, child->parent, mode, detail);
    EnterLeaveEvent(EnterNotify, mode, detail, child);
}

/* dies horribly if ancestor is not an ancestor of child */
static void
LeaveNotifies(child, ancestor, mode, detail, doAncestor)
    WindowPtr child, ancestor;
    int detail, mode;
{
    register WindowPtr  pWin;

    if (ancestor == child)
	return;
    for (pWin = child->parent; pWin != ancestor; pWin = pWin->parent)
	EnterLeaveEvent(LeaveNotify, mode, detail, pWin);
    if (doAncestor)
	EnterLeaveEvent(LeaveNotify, mode, detail, ancestor);
}

static void
DoEnterLeaveEvents(fromWin, toWin, mode)
    WindowPtr fromWin, toWin;
    int mode;
{
    if (fromWin == toWin)
	return;
    if (IsParent(fromWin, toWin))
    {
	EnterLeaveEvent(LeaveNotify, mode, NotifyInferior, fromWin);
	EnterNotifies(fromWin, toWin->parent, mode, NotifyVirtual);
	EnterLeaveEvent(EnterNotify, mode, NotifyAncestor, toWin);
    }
    else if (IsParent(toWin, fromWin))
    {
	EnterLeaveEvent(LeaveNotify, mode, NotifyAncestor, fromWin);
	LeaveNotifies(fromWin, toWin, mode, NotifyVirtual, FALSE);
	EnterLeaveEvent(EnterNotify, mode, NotifyInferior, toWin);
    }
    else
    { /* neither fromWin nor toWin is descendent of the other */
	WindowPtr common = CommonAncestor(toWin, fromWin);
	/* common == NullWindow ==> different screens */
	EnterLeaveEvent(LeaveNotify, mode, NotifyNonlinear, fromWin);
	if (common)
	{
	    LeaveNotifies(
		fromWin, common, mode, NotifyNonlinearVirtual, FALSE);
	    EnterNotifies(common, toWin->parent, mode, NotifyNonlinearVirtual);
	}
	else
	{
	    LeaveNotifies(
		fromWin, RootForWindow(fromWin), mode,
		NotifyNonlinearVirtual, TRUE);
	    EnterNotifies(
		RootForWindow(toWin), toWin->parent, mode, NotifyNonlinearVirtual);
	}
	EnterLeaveEvent(EnterNotify, mode, NotifyNonlinear, toWin);
    }
}

static void
FocusEvent(type, mode, detail, pWin)
    int type, mode, detail;
    WindowPtr pWin;
{
   xEvent	event;
   DeviceIntPtr	keybd = inputInfo.keyboard;

    event.u.focus.mode = mode;
    event.u.u.type = type;
    event.u.u.detail = detail;
    event.u.focus.window = pWin->wid;
    (void)DeliverEventsToWindow(pWin, &event, 1, filters[type], NullGrab);
    if (type == FocusIn)
    {
	xKeymapEvent ke;
	ke.type = KeymapNotify;
	bcopy((char *)keybd->down, (char *)&ke.map[0], 31);
	(void)DeliverEventsToWindow(pWin, (xEvent *)&ke, 1,
				    KeymapStateMask, NullGrab);
    }
}

 /*
  * recursive because it is easier
  * no-op if child not descended from ancestor
  */
static Bool
FocusInEvents(ancestor, child, skipChild, mode, detail, doAncestor)
    WindowPtr ancestor, child, skipChild;
    int mode, detail;
    Bool doAncestor;
{
    if (child == NullWindow)
	return ancestor == NullWindow;
    if (ancestor == child)
    {
	if (doAncestor)
	    FocusEvent(FocusIn, mode, detail, child);
	return TRUE;
    }
    if (FocusInEvents(
	ancestor, child->parent, skipChild, mode, detail, doAncestor))
    {
	if (child != skipChild)
	    FocusEvent(FocusIn, mode, detail, child);
	return TRUE;
    }
    return FALSE;
}

/* dies horribly if ancestor is not an ancestor of child */
static void
FocusOutEvents(child, ancestor, mode, detail, doAncestor)
    WindowPtr child, ancestor;
    int detail;
    Bool doAncestor;
{
    register WindowPtr  pWin;

    for (pWin = child; pWin != ancestor; pWin = pWin->parent)
	FocusEvent(FocusOut, mode, detail, pWin);
    if (doAncestor)
	FocusEvent(FocusOut, mode, detail, ancestor);
}

static void
DoFocusEvents(fromWin, toWin, mode)
    WindowPtr fromWin, toWin;
    int mode;
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
		FocusOutEvents(sprite.win, ROOT, mode, NotifyPointer, TRUE);
	    /* Notify all the roots */
	    for (i=0; i<screenInfo.numScreens; i++)
	        FocusEvent(FocusOut, mode, out, &WindowTable[i]);
	}
	else
	{
	    if (IsParent(fromWin, sprite.win))
	      FocusOutEvents(sprite.win, fromWin, mode, NotifyPointer, FALSE);
	    FocusEvent(FocusOut, mode, NotifyNonlinear, fromWin);
	    /* next call catches the root too, if the screen changed */
	    FocusOutEvents( fromWin->parent, NullWindow, mode,
			    NotifyNonlinearVirtual, FALSE);
	}
	/* Notify all the roots */
	for (i=0; i<screenInfo.numScreens; i++)
	    FocusEvent(FocusIn, mode, in, &WindowTable[i]);
	if (toWin == PointerRootWin)
	    (void)FocusInEvents(
		ROOT, sprite.win, NullWindow, mode, NotifyPointer, TRUE);
    }
    else
    {
	if ((fromWin == NullWindow) || (fromWin == PointerRootWin))
	{
	    if (fromWin == PointerRootWin)
		FocusOutEvents(sprite.win, ROOT, mode, NotifyPointer, TRUE);
	    for (i=0; i<screenInfo.numScreens; i++)
	      FocusEvent(FocusOut, mode, out, &WindowTable[i]);
	    if (toWin->parent != NullWindow)
	      (void)FocusInEvents(
		ROOT, toWin, toWin, mode, NotifyNonlinearVirtual, TRUE);
	    FocusEvent(FocusIn, mode, NotifyNonlinear, toWin);
	    if (IsParent(toWin, sprite.win))
    	       (void)FocusInEvents(
		 toWin, sprite.win, NullWindow, mode, NotifyPointer, FALSE);
	}
	else
	{
	    if (IsParent(toWin, fromWin))
	    {
		FocusEvent(FocusOut, mode, NotifyAncestor, fromWin);
		FocusOutEvents(
		    fromWin->parent, toWin, mode, NotifyVirtual, FALSE);
		FocusEvent(FocusIn, mode, NotifyInferior, toWin);
		if ((IsParent(toWin, sprite.win)) &&
			(sprite.win != fromWin) &&
			(!IsParent(fromWin, sprite.win)) &&
			(!IsParent(sprite.win, fromWin)))
		    (void)FocusInEvents(
			toWin, sprite.win, NullWindow, mode,
			NotifyPointer, FALSE);
	    }
	    else
		if (IsParent(fromWin, toWin))
		{
		    if ((IsParent(fromWin, sprite.win)) &&
			    (sprite.win != fromWin) &&
			    (!IsParent(toWin, sprite.win)) &&
			    (!IsParent(sprite.win, toWin)))
			FocusOutEvents(
			    sprite.win, fromWin, mode, NotifyPointer, FALSE);
		    FocusEvent(FocusOut, mode, NotifyInferior, fromWin);
		    (void)FocusInEvents(
			fromWin, toWin, toWin, mode, NotifyVirtual, FALSE);
		    FocusEvent(FocusIn, mode, NotifyAncestor, toWin);
		}
		else
		{
		/* neither fromWin or toWin is child of other */
		    WindowPtr common = CommonAncestor(toWin, fromWin);
		/* common == NullWindow ==> different screens */
		    if (IsParent(fromWin, sprite.win))
			FocusOutEvents(
			    sprite.win, fromWin, mode, NotifyPointer, FALSE);
		    FocusEvent(FocusOut, mode, NotifyNonlinear, fromWin);
		    if (fromWin->parent != NullWindow)
		      FocusOutEvents(
			fromWin->parent, common, mode, NotifyNonlinearVirtual,
			FALSE);
		    if (toWin->parent != NullWindow)
		      (void)FocusInEvents(
			common, toWin, toWin, mode, NotifyNonlinearVirtual,
			FALSE);
		    FocusEvent(FocusIn, mode, NotifyNonlinear, toWin);
		    if (IsParent(toWin, sprite.win))
			(void)FocusInEvents(
			    toWin, sprite.win, NullWindow, mode,
			    NotifyPointer, FALSE);
		}
	}
    }
}

int
ProcSetInputFocus(client)
    ClientPtr client;
{
    TimeStamp			time;
    WindowPtr			focusWin;
    int				mode;
    register DeviceIntPtr	kbd = inputInfo.keyboard;
    register FocusPtr		focus = &kbd->u.keybd.focus;
    REQUEST(xSetInputFocusReq);

    REQUEST_SIZE_MATCH(xSetInputFocusReq);
    if ((stuff->revertTo != RevertToParent) &&
	    (stuff->revertTo != RevertToPointerRoot) &&
	    (stuff->revertTo != RevertToNone))
    {
	client->errorValue = stuff->revertTo;
	return BadValue;
    }
    time = ClientTimeToServerTime(stuff->time);
    if ((stuff->focus == None) || (stuff->focus == PointerRoot))
	focusWin = (WindowPtr)(stuff->focus);
    else if (!(focusWin = LookupWindow(stuff->focus, client)))
	return BadWindow;
    else
    {
 	/* It is a match error to try to set the input focus to an 
	unviewable window. */

	if(!focusWin->realized)
	    return(BadMatch);
    }

    if ((CompareTimeStamps(time, currentTime) == LATER) ||
	    (CompareTimeStamps(time, focus->time) == EARLIER))
	return Success;

    mode = (kbd->grab) ? NotifyWhileGrabbed : NotifyNormal;

    DoFocusEvents(focus->win, focusWin, mode);
    focus->time = time;
    focus->revert = stuff->revertTo;
    focus->win = focusWin;
    if ((focusWin == NoneWin) || (focusWin == PointerRootWin))
        focusTraceGood = 0;
    else
    {
        int depth=0;
        WindowPtr pWin;
        for (pWin = focusWin; pWin; pWin = pWin->parent) depth++;
        if (depth > focusTraceSize)
        {
	    focusTraceSize = depth+1;
	    focusTrace = (WindowPtr *)xrealloc(
		    focusTrace, focusTraceSize*sizeof(WindowPtr));
	}

 	focusTraceGood = depth;

        for (pWin = focusWin; pWin; pWin = pWin->parent, depth--) 
	    focusTrace[depth-1] = pWin;
    }
    return Success;
}

int
ProcGetInputFocus(client)
    ClientPtr client;
{
    xGetInputFocusReply rep;
    REQUEST(xReq);
    FocusPtr focus = &(inputInfo.keyboard->u.keybd.focus);

    REQUEST_SIZE_MATCH(xReq);
    rep.type = X_Reply;
    rep.length = 0;
    rep.sequenceNumber = client->sequence;
    if (focus->win == NoneWin)
	rep.focus = None;
    else if (focus->win == PointerRootWin)
	rep.focus = PointerRoot;
    else rep.focus = focus->win->wid;
    rep.revertTo = focus->revert;
    WriteReplyToClient(client, sizeof(xGetInputFocusReply), &rep);
    return Success;
}

int
ProcGrabPointer(client)
    ClientPtr client;
{
    xGrabPointerReply rep;
    DeviceIntPtr device = inputInfo.pointer;
    GrabPtr grab = device->grab;
    WindowPtr pWin, confineTo;
    CursorPtr cursor;
    REQUEST(xGrabPointerReq);
    TimeStamp time;

    REQUEST_SIZE_MATCH(xGrabPointerReq);
    if ((stuff->pointerMode != GrabModeSync) &&
	(stuff->pointerMode != GrabModeAsync))
    {
	client->errorValue = stuff->pointerMode;
        return BadValue;
    }
    if ((stuff->keyboardMode != GrabModeSync) &&
	(stuff->keyboardMode != GrabModeAsync))
    {
	client->errorValue = stuff->keyboardMode;
        return BadValue;
    }

    pWin = LookupWindow(stuff->grabWindow, client);
    if (!pWin)
	return BadWindow;
    if (stuff->confineTo == None)
	confineTo = NullWindow;
    else
    {
	confineTo = LookupWindow(stuff->confineTo, client);
	if (!confineTo)
	    return BadWindow;
    }
    if (stuff->cursor == None)
	cursor = NullCursor;
    else
    {
	cursor = (CursorPtr)LookupID(stuff->cursor, RT_CURSOR, RC_CORE);
	if (!cursor)
	{
	    client->errorValue = stuff->cursor;
	    return BadCursor;
	}
    }
	/* at this point, some sort of reply is guaranteed. */
    time = ClientTimeToServerTime(stuff->time);
    rep.type = X_Reply;
    rep.sequenceNumber = client->sequence;
    rep.length = 0;
    if ((grab) && (grab->client != client))
	rep.status = AlreadyGrabbed;
    else if ((!pWin->realized) ||
	     (confineTo &&
		!(confineTo->realized &&
		  (* confineTo->drawable.pScreen->RegionNotEmpty)
			(confineTo->borderSize))))
	rep.status = GrabNotViewable;
    else if (device->sync.frozen &&
	     ((device->sync.other && (device->sync.other->client != client)) ||
	     ((device->sync.state >= FROZEN) &&
	      (device->grab->client != client))))
	rep.status = GrabFrozen;
    else if ((CompareTimeStamps(time, currentTime) == LATER) ||
	     (device->grab &&
	     (CompareTimeStamps(time, device->grabTime) == EARLIER)))
	rep.status = GrabInvalidTime;
    else
    {
	GrabRec tempGrab;

	if (grab && grab->confineTo && !confineTo)
	    ConfineCursorToWindow(ROOT, 0, 0, FALSE);
	tempGrab.cursor = cursor;
	tempGrab.client = client;
	tempGrab.ownerEvents = stuff->ownerEvents;
	tempGrab.eventMask = stuff->eventMask;
	tempGrab.confineTo = confineTo;
	tempGrab.window = pWin;
	tempGrab.keyboardMode = stuff->keyboardMode;
	tempGrab.pointerMode = stuff->pointerMode;
	tempGrab.device = inputInfo.pointer;
	ActivatePointerGrab(inputInfo.pointer, &tempGrab, time, FALSE);
	rep.status = GrabSuccess;
    }
    WriteReplyToClient(client, sizeof(xGrabPointerReply), &rep);
    return Success;
}

int
ProcChangeActivePointerGrab(client)
    ClientPtr client;
{
    DeviceIntPtr device = inputInfo.pointer;
    register GrabPtr grab = device->grab;
    CursorPtr newCursor;
    REQUEST(xChangeActivePointerGrabReq);
    TimeStamp time;

    REQUEST_SIZE_MATCH(xChangeActivePointerGrabReq);
    if (!grab)
	return Success;
    if (grab->client != client)
	return BadAccess;
    if (stuff->cursor == None)
	newCursor = NullCursor;
    else
    {
	newCursor = (CursorPtr)LookupID(stuff->cursor, RT_CURSOR, RC_CORE);
	if (!newCursor)
	{
	    client->errorValue = stuff->cursor;
	    return BadCursor;
	}
    }
    time = ClientTimeToServerTime(stuff->time);
    if ((CompareTimeStamps(time, currentTime) == LATER) ||
	     (CompareTimeStamps(time, device->grabTime) == EARLIER))
	return Success;
    if (grab->cursor)
	FreeCursor(grab->cursor, 0);
    grab->cursor = newCursor;
    if (newCursor)
	newCursor->refcnt++;
    PostNewCursor();
    grab->eventMask = stuff->eventMask;
    return Success;
}

int
ProcUngrabPointer(client)
    ClientPtr client;
{
    DeviceIntPtr device = inputInfo.pointer;
    GrabPtr grab = device->grab;
    TimeStamp time;
    REQUEST(xResourceReq);

    REQUEST_SIZE_MATCH(xResourceReq);
    time = ClientTimeToServerTime(stuff->id);
    if ((CompareTimeStamps(time, currentTime) != LATER) &&
	    (CompareTimeStamps(time, device->grabTime) != EARLIER) &&
	    (grab) && (grab->client == client))
	DeactivatePointerGrab(inputInfo.pointer);
    return Success;
}

int
ProcGrabKeyboard(client)
    ClientPtr client;
{
    xGrabKeyboardReply rep;
    DeviceIntPtr device = inputInfo.keyboard;
    GrabPtr grab = device->grab;
    WindowPtr pWin;
    TimeStamp time;
    REQUEST(xGrabKeyboardReq);

    REQUEST_SIZE_MATCH(xGrabKeyboardReq);
    if ((stuff->pointerMode != GrabModeSync) &&
	(stuff->pointerMode != GrabModeAsync))
    {
	client->errorValue = stuff->pointerMode;
        return BadValue;
    }
    if ((stuff->keyboardMode != GrabModeSync) &&
	(stuff->keyboardMode != GrabModeAsync))
    {
	client->errorValue = stuff->keyboardMode;
        return BadValue;
    }
    pWin = LookupWindow(stuff->grabWindow, client);
    if (!pWin)
	return BadWindow;
    time = ClientTimeToServerTime(stuff->time);
    rep.type = X_Reply;
    rep.sequenceNumber = client->sequence;
    rep.length = 0;
    if ((grab) && (grab->client != client))
	rep.status = AlreadyGrabbed;
    else if (!pWin->realized)
	rep.status = GrabNotViewable;
    else if ((CompareTimeStamps(time, currentTime) == LATER) ||
	     (device->grab &&
	     (CompareTimeStamps(time, device->grabTime) == EARLIER)))
	rep.status = GrabInvalidTime;
    else if (device->sync.frozen &&
	     ((device->sync.other && (device->sync.other->client != client)) ||
	     ((device->sync.state >= FROZEN) &&
	      (device->grab->client != client))))
	rep.status = GrabFrozen;
    else
    {
	GrabRec tempGrab;

	tempGrab.window = pWin;
	tempGrab.client = client;
	tempGrab.ownerEvents = stuff->ownerEvents;
	tempGrab.keyboardMode = stuff->keyboardMode;
	tempGrab.pointerMode = stuff->pointerMode;
	tempGrab.eventMask = KeyPressMask | KeyReleaseMask;
	tempGrab.device = inputInfo.keyboard;
	ActivateKeyboardGrab(
	    device, &tempGrab, ClientTimeToServerTime(stuff->time), FALSE);
	
	rep.status = GrabSuccess;
    }
    WriteReplyToClient(client, sizeof(xGrabKeyboardReply), &rep);
    return Success;
}

int
ProcUngrabKeyboard(client)
    ClientPtr client;
{
    DeviceIntPtr device = inputInfo.keyboard;
    GrabPtr grab = device->grab;
    TimeStamp time;
    REQUEST(xResourceReq);

    REQUEST_SIZE_MATCH(xResourceReq);
    time = ClientTimeToServerTime(stuff->id);
    if ((CompareTimeStamps(time, currentTime) != LATER) &&
	(CompareTimeStamps(time, device->grabTime) != EARLIER) &&
	(grab) && (grab->client == client))
	DeactivateKeyboardGrab(device);
    return Success;
}

static void
SetPointerStateMasks()
{
 /* all have to be defined since some button might be mapped here */
    keyModifiersList[1] = Button1Mask;
    keyModifiersList[2] = Button2Mask;
    keyModifiersList[3] = Button3Mask;
    keyModifiersList[4] = Button4Mask;
    keyModifiersList[5] = Button5Mask;
}

static void
SetKeyboardStateMasks(keybd)
    DeviceIntPtr keybd;
{
    int	i;

    /*
     *	For all valid keys (from 8 up) copy the bitmap of the modifiers
     *	it sets from the keyboard info into the array we use internally.
     *  No need to test for bad entries - these are detected when the
     *	array in the kbd struct is built.
     */
    for (i = 8; i < MAP_LENGTH; i++)
	keyModifiersList[i] = (CARD16) keybd->u.keybd.modifierMap[i];
}

DevicePtr
AddInputDevice(deviceProc, autoStart)
    DeviceProc deviceProc;
    Bool autoStart;
{
    DeviceIntPtr d;
    if (inputInfo.numDevices == inputInfo.arraySize)
    {
	inputInfo.arraySize += 5;
	inputInfo.devices = (DeviceIntPtr *)xrealloc(
				inputInfo.devices,
				inputInfo.arraySize * sizeof(DeviceIntPtr));
    }
    d = (DeviceIntPtr) xalloc(sizeof(DeviceIntRec));
    inputInfo.devices[inputInfo.numDevices++] = d;
    d->public.on = FALSE;
    d->public.processInputProc = NoopDDA;
    d->deviceProc = deviceProc;
    d->startup = autoStart;
    d->sync.frozen = FALSE;
    d->sync.other = NullGrab;
    d->sync.state = NOT_GRABBED;
    d->grab = NullGrab;
    bzero((char *)d->down, sizeof(d->down));
    return &d->public;
}

DevicesDescriptor
GetInputDevices()
{
    DevicesDescriptor devs;
    devs.count = inputInfo.numDevices;
    devs.devices = (DevicePtr *)inputInfo.devices;
    return devs;
}

void
InitEvents()
{
    curKeySyms.map = (KeySym *)NULL;
    curKeySyms.minKeyCode = 0;
    curKeySyms.maxKeyCode = 0;
    curKeySyms.mapWidth = 0;

    currentScreen = &screenInfo.screen[0];
    inputInfo.numDevices = 0;
    if (spriteTraceSize == 0)
    {
	spriteTraceSize = 20;
	spriteTrace = (WindowPtr *)xalloc(20*sizeof(WindowPtr));
    }
    spriteTraceGood = 0;
    if (focusTraceSize == 0)
    {
	focusTraceSize = 20;
	focusTrace = (WindowPtr *)xalloc(20*sizeof(WindowPtr));
    }
    focusTraceGood = 0;
    lastEventMask = OwnerGrabButtonMask;
    sprite.win = NullWindow;
    sprite.current = NullCursor;
    sprite.hotLimits.x1 = 0;
    sprite.hotLimits.y1 = 0;
    sprite.hotLimits.x2 = currentScreen->width;
    sprite.hotLimits.y2 = currentScreen->height;
    motionHintWindow = NullWindow;
    syncEvents.replayDev = (DeviceIntPtr)NULL;
    syncEvents.pending.forw = &syncEvents.pending;
    syncEvents.pending.back = &syncEvents.pending;
    syncEvents.free.forw = &syncEvents.free;
    syncEvents.free.back = &syncEvents.free;
    syncEvents.num = 0;
    syncEvents.playingEvents = FALSE;
    currentTime.months = 0;
    currentTime.milliseconds = GetTimeInMillis();
}

int
InitAndStartDevices(argc, argv)
    int argc;
    char *argv[];
{
    int     i;
    DeviceIntPtr d;

    for (i=0; i<8; i++)
        modifierKeyCount[i] = 0;

    keyButtonState = 0;
    buttonsDown = 0;
    buttonMotionMask = 0;
        
    for (i = 0; i < inputInfo.numDevices; i++)
    {
	d = inputInfo.devices[i];
	if ((*d->deviceProc) (d, DEVICE_INIT, argc, argv) == Success)
	    d->inited = TRUE;
	else
	    d->inited = FALSE;
    }
 /* do not turn any devices on until all have been inited */
    for (i = 0; i < inputInfo.numDevices; i++)
    {
	d = inputInfo.devices[i];
	if ((d->startup) && (d->inited))
	    (*d->deviceProc) (d, DEVICE_ON, argc, argv);
    }
    if (inputInfo.pointer && inputInfo.pointer->inited &&
	    inputInfo.keyboard && inputInfo.keyboard->inited)
	return Success;
    return BadImplementation;
}

void
CloseDownDevices(argc, argv)
    int argc;
    char *argv[];
{
    int     		i;
    DeviceIntPtr	d;

    xfree(curKeySyms.map);
    curKeySyms.map = (KeySym *)NULL;

    for (i = inputInfo.numDevices - 1; i >= 0; i--)
    {
	d = inputInfo.devices[i];
	if (d->inited)
	    (*d->deviceProc) (d, DEVICE_CLOSE, argc, argv);
	inputInfo.numDevices = i;
	xfree(d);
    }
   
    /* The array inputInfo.devices doesn't need to be freed here since it
	will be reused when AddInputDevice is called when the server 
	resets again.*/
}

int
NumMotionEvents()
{
    return inputInfo.numMotionEvents;
}

void
RegisterPointerDevice(device, numMotionEvents)
    DevicePtr device;
    int numMotionEvents;
{
    inputInfo.pointer = (DeviceIntPtr)device;
    inputInfo.numMotionEvents = numMotionEvents;
    device->processInputProc = ProcessPointerEvent;
}

void
RegisterKeyboardDevice(device)
    DevicePtr device;
{
    inputInfo.keyboard = (DeviceIntPtr)device;
    device->processInputProc = ProcessKeyboardEvent;
}

void
InitPointerDeviceStruct(device, map, mapLength, motionProc, controlProc)
    DevicePtr device;
    BYTE *map;
    int mapLength;
    void (*controlProc)();
    int (*motionProc)();
{
    int i;
    DeviceIntPtr mouse = (DeviceIntPtr)device;

    mouse->grab = NullGrab;
    mouse->public.on = FALSE;
    mouse->u.ptr.mapLength = mapLength;
    mouse->u.ptr.map[0] = 0;
    for (i = 1; i <= mapLength; i++)
	mouse->u.ptr.map[i] = map[i];
    mouse->u.ptr.ctrl = defaultPointerControl;
    mouse->u.ptr.GetMotionProc = motionProc;
    mouse->u.ptr.CtrlProc = controlProc;
    mouse->u.ptr.autoReleaseGrab = FALSE;
    if (mouse == inputInfo.pointer)
	SetPointerStateMasks();
    (*mouse->u.ptr.CtrlProc)(mouse, &mouse->u.ptr.ctrl);
}

void
QueryMinMaxKeyCodes(minCode, maxCode)
    KeyCode *minCode, *maxCode;
{
    *minCode = curKeySyms.minKeyCode;
    *maxCode = curKeySyms.maxKeyCode;
}

static void
SetKeySymsMap(pKeySyms)
    KeySymsPtr pKeySyms;
{
    int i, j;
    int rowDif = pKeySyms->minKeyCode - curKeySyms.minKeyCode;
           /* if keysym map size changes, grow map first */

    if (pKeySyms->mapWidth < curKeySyms.mapWidth)
    {
        for (i = pKeySyms->minKeyCode; i <= pKeySyms->maxKeyCode; i++)
	{
#define SI(r, c) (((r-pKeySyms->minKeyCode)*pKeySyms->mapWidth) + (c))
#define DI(r, c) (((r - curKeySyms.minKeyCode)*curKeySyms.mapWidth) + (c))
	    for (j = 0; j < pKeySyms->mapWidth; j++)
		curKeySyms.map[DI(i, j)] = pKeySyms->map[SI(i, j)];
	    for (j = pKeySyms->mapWidth; j < curKeySyms.mapWidth; j++)
		curKeySyms.map[DI(i, j)] = NoSymbol;
#undef SI
#undef DI
	}
	return;
    }
    else if (pKeySyms->mapWidth > curKeySyms.mapWidth)
    {
        KeySym *map;
	int bytes = sizeof(KeySym) * pKeySyms->mapWidth *
               (curKeySyms.maxKeyCode - curKeySyms.minKeyCode + 1);
        map = (KeySym *)xalloc(bytes);
	bzero((char *)map, bytes);
        if (curKeySyms.map)
	{
            for (i = 0; i <= curKeySyms.maxKeyCode-curKeySyms.minKeyCode; i++)
		bcopy(
		    (char *)&curKeySyms.map[i*curKeySyms.mapWidth],
		    (char *)&map[i*pKeySyms->mapWidth],
		    curKeySyms.mapWidth * sizeof(KeySym));
	    xfree(curKeySyms.map);
	}
	curKeySyms.mapWidth = pKeySyms->mapWidth;
        curKeySyms.map = map;
    }
    bcopy(
	(char *)pKeySyms->map,
	(char *)&curKeySyms.map[rowDif * curKeySyms.mapWidth],
	(pKeySyms->maxKeyCode - pKeySyms->minKeyCode + 1) *
	    curKeySyms.mapWidth * sizeof(KeySym));
}

static CARD8
WidthOfModifierTable(modifierMap)
    CARD8 modifierMap[];
{
    int         i;
    CARD8	keysPerModifier[8],maxKeysPerMod;

    maxKeysPerMod = 0;
    bzero((char *)keysPerModifier, sizeof keysPerModifier);

    for (i = 8; i < MAP_LENGTH; i++) {
	int         j;
	CARD8       mask;

	for (j = 0, mask = 1; j < 8; j++, mask <<= 1) {
	    if (mask & modifierMap[i]) {
		if (++keysPerModifier[j] > maxKeysPerMod) {
		    maxKeysPerMod = keysPerModifier[j];
		}
		if (debug_modifiers)
		    ErrorF("Key 0x%x modifier %d sequence %d\n",
			i, j, keysPerModifier[j]);
	    }
	}
    }
    if (debug_modifiers)
	ErrorF("Max Keys per Modifier = %d\n", maxKeysPerMod);
    if (modifierKeyMap)
	xfree(modifierKeyMap);
    modifierKeyMap = (KeyCode *)xalloc(8*maxKeysPerMod);
    bzero((char *)modifierKeyMap, 8*maxKeysPerMod);
    bzero((char *)keysPerModifier, sizeof keysPerModifier);

    for (i = 8; i < MAP_LENGTH; i++) {
	int         j;
	CARD8       mask;

	for (j = 0, mask = 1; j < 8; j++, mask <<= 1) {
	    if (mask & modifierMap[i]) {
		if (debug_modifiers)
		    ErrorF("Key 0x%x modifier %d index %d\n", i, j,
			   j*maxKeysPerMod+keysPerModifier[j]);
		modifierKeyMap[j*maxKeysPerMod+keysPerModifier[j]] = i;
		keysPerModifier[j]++;
	    }
	}
    }

    return (maxKeysPerMod);
}

void 
InitKeyboardDeviceStruct(device, pKeySyms, pModifiers,
			      bellProc, controlProc)
    DevicePtr device;
    KeySymsPtr pKeySyms;
    CARD8	pModifiers[];
    void (*bellProc)();
    void (*controlProc)();
{
    DeviceIntPtr keybd = (DeviceIntPtr)device;

    keybd->grab = NullGrab;
    keybd->public.on = FALSE;

    keybd->u.keybd.ctrl = defaultKeyboardControl;
    keybd->u.keybd.BellProc = bellProc;
    keybd->u.keybd.CtrlProc = controlProc;
    keybd->u.keybd.focus.win = PointerRootWin;
    keybd->u.keybd.focus.revert = None;
    keybd->u.keybd.focus.time = currentTime;
    keybd->u.keybd.passiveGrab = FALSE;
    curKeySyms.minKeyCode = pKeySyms->minKeyCode;
    curKeySyms.maxKeyCode = pKeySyms->maxKeyCode;
    /*
     *	Copy the modifier info into the kdb stuct.
     */
    {
	int i;

	for (i = 8; i < MAP_LENGTH; i++) {
	    keybd->u.keybd.modifierMap[i] = pModifiers[i];
	}
	maxKeysPerModifier = WidthOfModifierTable(pModifiers);
    }
    if (keybd == inputInfo.keyboard)
    {
	SetKeyboardStateMasks(keybd);
	SetKeySymsMap(pKeySyms);
    }
    (*keybd->u.keybd.CtrlProc)(keybd, &keybd->u.keybd.ctrl);  
}

void
InitOtherDeviceStruct(device, map, mapLength)
    DevicePtr device;
    BYTE *map;
    int mapLength;
{
    int i;
    DeviceIntPtr other = (DeviceIntPtr)device;

    other->grab = NullGrab;
    other->public.on = FALSE;
    other->u.other.mapLength = mapLength;
    other->u.other.map[0] =  0;
    for (i = 1; i <= mapLength; i++)
	other->u.other.map[i] = map[i];
    other->u.other.focus.win = NoneWin;
    other->u.other.focus.revert = None;
    other->u.other.focus.time = currentTime;
}

GrabPtr
SetDeviceGrab(device, grab)
    DevicePtr device;
    GrabPtr grab;
{
    register DeviceIntPtr dev = (DeviceIntPtr)device;
    GrabPtr oldGrab = dev->grab;
    dev->grab = grab; /* must not be deallocated */
    return oldGrab;
}

/*
 * Devices can't be resources since the bit patterns don't fit very well.
 * For one, where the client field would be, is random bits and the client
 * object might not be defined. For another, the "server" bit might be on.
 */

#ifdef INPUT_EXTENSION

DevicePtr
LookupInputDevice(deviceID)
    Device deviceID;
{
    int i;
    for (i = 0; i < inputInfo.numDevices; i++)
	if (inputInfo.devices[i]->public.deviceID == deviceID)
	    return &(inputInfo.devices[i]->public);
    return NullDevice;
}
#endif /* INTPUT_EXTENSION */

DevicePtr
LookupKeyboardDevice()
{
    return &inputInfo.keyboard->public;
}

DevicePtr
LookupPointerDevice()
{
    return &inputInfo.pointer->public;
}


static int
SendMappingNotify(request, firstKeyCode, count)
    CARD8 request, count;
    KeyCode firstKeyCode;
{
    int i;
    xEvent event;

    event.u.u.type = MappingNotify;
    event.u.mappingNotify.request = request;
    if (request == MappingKeyboard)
    {
        event.u.mappingNotify.firstKeyCode = firstKeyCode;
        event.u.mappingNotify.count = count;
    }
    /* 0 is the server client */
    for (i=1; i<currentMaxClients; i++)
        if (clients[i] && ! clients[i]->clientGone)
	{
	    event.u.u.sequenceNumber = clients[i]->sequence;
            WriteEventsToClient(clients[i], 1, &event);
	}
}

/*
 * n-sqared algorithm. n < 255 and don't want to copy the whole thing and
 * sort it to do the checking. How often is it called? Just being lazy?
 */
static Bool
BadDeviceMap(buff, length, low, high, errval)
    register BYTE *buff;
    int length;
    unsigned low, high;
    XID *errval;
{
    register int     i, j;

    for (i = 0; i < length; i++)
	if (buff[i])		       /* only check non-zero elements */
	{
	    if ((low > buff[i]) || (high < buff[i]))
	    {
		*errval = buff[i];
		return TRUE;
	    }
	    for (j = i + 1; j < length; j++)
		if (buff[i] == buff[j])
		{
		    *errval = buff[i];
		    return TRUE;
		}
	}
    return FALSE;
}

static Bool
AllModifierKeysAreUp(map1, per1, map2, per2)
    register CARD8 *map1, *map2;
    int per1, per2;
{
    register int i, j, k;

    for (i = 8; --i >= 0; map2 += per2)
    {
	for (j = per1; --j >= 0; map1++)
	{
	    if (*map1 && IsOn(inputInfo.keyboard->down, *map1))
	    {
		for (k = per2; (--k >= 0) && (*map1 != map2[k]);)
		  ;
		if (k < 0)
		    return FALSE;
	    }
	}
    }
    return TRUE;
}

int 
ProcSetModifierMapping(client)
    ClientPtr client;
{
    xSetModifierMappingReply rep;
    REQUEST(xSetModifierMappingReq);
    KeyCode *inputMap;
    int inputMapLen;
    register int i;
    
    REQUEST_AT_LEAST_SIZE(xSetModifierMappingReq);

    if (stuff->length != ((stuff->numKeyPerModifier<<1) +
			  (sizeof (xSetModifierMappingReq)>>2)))
	return BadLength;

    inputMapLen = 8*stuff->numKeyPerModifier;
    inputMap = (KeyCode *)&stuff[1];

    /*
     *	Now enforce the restriction that "all of the non-zero keycodes must be
     *	in the range specified by min-keycode and max-keycode in the
     *	connection setup (else a Value error)"
     */
    i = inputMapLen;
    while (i--) {
	if (inputMap[i]
	    && (inputMap[i] < curKeySyms.minKeyCode
		|| inputMap[i] > curKeySyms.maxKeyCode)) {
		client->errorValue = inputMap[i];
		return BadValue;
		}
    }
    rep.type = X_Reply;
    rep.length = 0;
    rep.sequenceNumber = client->sequence;
    rep.success = MappingSuccess;

    /*
     *	Now enforce the restriction that none of the old or new
     *	modifier keys may be down while we change the mapping,  and
     *	that the DDX layer likes the choice.
     */
    if (!AllModifierKeysAreUp(modifierKeyMap, (int)maxKeysPerModifier,
			      inputMap, (int)stuff->numKeyPerModifier)
	    ||
	!AllModifierKeysAreUp(inputMap, (int)stuff->numKeyPerModifier,
			      modifierKeyMap, (int)maxKeysPerModifier)) {
	if (debug_modifiers)
	    ErrorF("Busy\n");
	rep.success = MappingBusy;
    } else {
	for (i = 0; i < inputMapLen; i++) {
	    if (inputMap[i] && !LegalModifier(inputMap[i])) {
		if (debug_modifiers)
		    ErrorF("Key 0x%x refused\n", inputMap[i]);
		rep.success = MappingFailed;
		break;
	    }
	}
    }

    WriteReplyToClient(client, sizeof(xSetModifierMappingReply), &rep);

    if (rep.success == MappingSuccess)
    {
	/*
	 *	Now build the keyboard's modifier bitmap from the
	 *	list of keycodes.
	 */
	if (modifierKeyMap)
	    xfree(modifierKeyMap);
	modifierKeyMap = (KeyCode *)xalloc(inputMapLen);
	bcopy((char *)inputMap, (char *)modifierKeyMap, inputMapLen);

	maxKeysPerModifier = stuff->numKeyPerModifier;
	for (i = 0; i < MAP_LENGTH; i++)
	    inputInfo.keyboard->u.keybd.modifierMap[i] = 0;
	for (i = 0; i < inputMapLen; i++) if (inputMap[i]) {
	    inputInfo.keyboard->u.keybd.modifierMap[inputMap[i]]
	      |= (1<<(i/maxKeysPerModifier));
	    if (debug_modifiers)
		ErrorF("Key 0x%x mod %d\n", inputMap[i], i/maxKeysPerModifier);
	}
	SetKeyboardStateMasks(inputInfo.keyboard);
        SendMappingNotify(MappingModifier, 0, 0);
    }
    return(client->noClientException);
}

int
ProcGetModifierMapping(client)
    ClientPtr client;
{
    xGetModifierMappingReply rep;
    REQUEST(xReq);

    REQUEST_SIZE_MATCH(xReq);
    rep.type = X_Reply;
    rep.numKeyPerModifier = maxKeysPerModifier;
    rep.sequenceNumber = client->sequence;
    /* length counts 4 byte quantities - there are 8 modifiers 1 byte big */
    rep.length = 2*maxKeysPerModifier;

    WriteReplyToClient(client, sizeof(xGetModifierMappingReply), &rep);

    /* Reply with the (modified by DDX) map that SetModifierMapping passed in */
    (void)WriteToClient(client, 8*maxKeysPerModifier, (char *)modifierKeyMap);
    return client->noClientException;
}

int
ProcChangeKeyboardMapping(client)
    ClientPtr client;
{
    REQUEST(xChangeKeyboardMappingReq);
    unsigned len;
    KeySymsRec keysyms;

    REQUEST_AT_LEAST_SIZE(xChangeKeyboardMappingReq);

    len = stuff->length - (sizeof(xChangeKeyboardMappingReq) >> 2);  
    if (len != (stuff->keyCodes * stuff->keySymsPerKeyCode))
            return BadLength;
    if ((stuff->firstKeyCode < curKeySyms.minKeyCode) ||
	(stuff->firstKeyCode + stuff->keyCodes - 1 > curKeySyms.maxKeyCode))
    {
	    client->errorValue = stuff->firstKeyCode;
	    return BadValue;
    }
    if (stuff->keySymsPerKeyCode == 0)
    {
	    client->errorValue = 0;
            return BadValue;
    }
    keysyms.minKeyCode = stuff->firstKeyCode;
    keysyms.maxKeyCode = stuff->firstKeyCode + stuff->keyCodes - 1;
    keysyms.mapWidth = stuff->keySymsPerKeyCode;
    keysyms.map = (KeySym *)&stuff[1];
    SetKeySymsMap(&keysyms);
    SendMappingNotify(MappingKeyboard, stuff->firstKeyCode, stuff->keyCodes);
    return client->noClientException;

}

int
ProcSetPointerMapping(client)
    ClientPtr client;
{
    REQUEST(xSetPointerMappingReq);
    BYTE *map;
    xSetPointerMappingReply rep;
    register int i;

    REQUEST_AT_LEAST_SIZE(xSetPointerMappingReq);
    if (stuff->length != (sizeof(xSetPointerMappingReq) + stuff->nElts + 3)>>2)
	return BadLength;
    rep.type = X_Reply;
    rep.length = 0;
    rep.sequenceNumber = client->sequence;
    rep.success = MappingSuccess;
    map = (BYTE *)&stuff[1];
    if (stuff->nElts != inputInfo.pointer->u.ptr.mapLength)
    {
	client->errorValue = stuff->nElts;
	return BadValue;
    }
    if (BadDeviceMap(&map[0], (int)stuff->nElts, 1, 255, &client->errorValue))
	return BadValue;
    for (i=0; i < stuff->nElts; i++)
	if ((inputInfo.pointer->u.ptr.map[i + 1] != map[i]) &&
		IsOn(inputInfo.pointer->down, i + 1))
	{
    	    rep.success = MappingBusy;
	    WriteReplyToClient(client, sizeof(xSetPointerMappingReply), &rep);
            return Success;
	}
    for (i = 0; i < stuff->nElts; i++)
	inputInfo.pointer->u.ptr.map[i + 1] = map[i];
    WriteReplyToClient(client, sizeof(xSetPointerMappingReply), &rep);
    SendMappingNotify(MappingPointer, 0, 0);
    return Success;
}

int
ProcGetKeyboardMapping(client)
    ClientPtr client;
{
    xGetKeyboardMappingReply rep;
    REQUEST(xGetKeyboardMappingReq);

    REQUEST_SIZE_MATCH(xGetKeyboardMappingReq);

    if ((stuff->firstKeyCode < curKeySyms.minKeyCode) ||
        (stuff->firstKeyCode > curKeySyms.maxKeyCode))
    {
	client->errorValue = stuff->firstKeyCode;
	return BadValue;
    }
    if (stuff->firstKeyCode + stuff->count > curKeySyms.maxKeyCode + 1)
    {
	client->errorValue = stuff->count;
        return BadValue;
    }

    rep.type = X_Reply;
    rep.sequenceNumber = client->sequence;
    rep.keySymsPerKeyCode = curKeySyms.mapWidth;
/* length is a count of 4 byte quantities and KeySyms are 4 bytes */
    rep.length = (curKeySyms.mapWidth * stuff->count);
    WriteReplyToClient(client, sizeof(xGetKeyboardMappingReply), &rep);
    client->pSwapReplyFunc = CopySwap32Write;
    WriteSwappedDataToClient(
	client,
	curKeySyms.mapWidth * stuff->count * sizeof(KeySym),
	&curKeySyms.map[(stuff->firstKeyCode - curKeySyms.minKeyCode) *
			curKeySyms.mapWidth]);

    return client->noClientException;
}

int
ProcGetPointerMapping(client)
    ClientPtr client;
{
    xGetPointerMappingReply rep;
    REQUEST(xReq);

    REQUEST_SIZE_MATCH(xReq);
    rep.type = X_Reply;
    rep.sequenceNumber = client->sequence;
    rep.nElts = inputInfo.pointer->u.ptr.mapLength;
    rep.length = (rep.nElts + (4-1))/4;
    WriteReplyToClient(client, sizeof(xGetPointerMappingReply), &rep);
    (void)WriteToClient(client, rep.nElts,
			(char *)&inputInfo.pointer->u.ptr.map[1]);
    return Success;    
}

int
Ones(mask)                /* HACKMEM 169 */
    Mask mask;
{
    register Mask y;

    y = (mask >> 1) &033333333333;
    y = mask - y - ((y >>1) & 033333333333);
    return (((y + (y >> 3)) & 030707070707) % 077);
}

void
NoteLedState(keybd, led, on)
    DeviceIntPtr keybd;
    int		led;
    Bool	on;
{
    KeybdCtrl *ctrl = &keybd->u.keybd.ctrl;
    if (on)
	ctrl->leds |= ((Leds)1 << (led - 1));
    else
	ctrl->leds &= ~((Leds)1 << (led - 1));
}

int
ProcChangeKeyboardControl (client)
    ClientPtr client;
{
#define DO_ALL    (-1)
    KeybdCtrl ctrl;
    DeviceIntPtr keybd = inputInfo.keyboard;
    XID *vlist;
    int t;
    int led = DO_ALL;
    int key = DO_ALL;
    BITS32 vmask, index;
    int mask, i;
    REQUEST(xChangeKeyboardControlReq);

    REQUEST_AT_LEAST_SIZE(xChangeKeyboardControlReq);
    vmask = stuff->mask;
    if (stuff->length !=(sizeof(xChangeKeyboardControlReq)>>2) + Ones(vmask))
	return BadLength;
    vlist = (XID *)&stuff[1];		/* first word of values */
    ctrl = keybd->u.keybd.ctrl;
    while (vmask)
    {
	index = (BITS32) lowbit (vmask);
	vmask &= ~index;
	switch (index)
	{
	case KBKeyClickPercent: 
	    t = (INT8)*vlist;
	    vlist++;
	    if (t == -1)
		t = defaultKeyboardControl.click;
	    else if (t < 0 || t > 100)
	    {
		client->errorValue = t;
		return BadValue;
	    }
	    ctrl.click = t;
	    break;
	case KBBellPercent:
	    t = (INT8)*vlist;
	    vlist++;
	    if (t == -1)
		t = defaultKeyboardControl.bell;
	    else if (t < 0 || t > 100)
	    {
		client->errorValue = t;
		return BadValue;
	    }
	    ctrl.bell = t;
	    break;
	case KBBellPitch:
	    t = (INT16)*vlist;
	    vlist++;
	    if (t == -1)
		t = defaultKeyboardControl.bell_pitch;
	    else if (t < 0)
	    {
		client->errorValue = t;
		return BadValue;
	    }
	    ctrl.bell_pitch = t;
	    break;
	case KBBellDuration:
	    t = (INT16)*vlist;
	    vlist++;
	    if (t == -1)
		t = defaultKeyboardControl.bell_duration;
	    else if (t < 0)
	    {
		client->errorValue = t;
		return BadValue;
	    }
	    ctrl.bell_duration = t;
	    break;
	case KBLed:
	    led = (CARD8)*vlist;
	    vlist++;
	    if (led < 1 || led > 32)
	    {
		client->errorValue = led;
		return BadValue;
	    }
	    if (!(stuff->mask & KBLedMode))
		return BadMatch;
	    break;
	case KBLedMode:
	    t = (CARD8)*vlist;
	    vlist++;
	    if (t == LedModeOff)
	    {
		if (led == DO_ALL)
		    ctrl.leds = 0x0;
		else
		    ctrl.leds &= ~(((Leds)(1)) << (led - 1));
	    }
	    else if (t == LedModeOn)
	    {
		if (led == DO_ALL)
		    ctrl.leds = ~0L;
		else
		    ctrl.leds |= (((Leds)(1)) << (led - 1));
	    }
	    else
	    {
		client->errorValue = t;
		return BadValue;
	    }
	    break;
	case KBKey:
	    key = (KeyCode)*vlist;
	    vlist++;
	    if (key < 8 || key > 255)
	    {
		client->errorValue = key;
		return BadValue;
	    }
	    if (!(stuff->mask & KBAutoRepeatMode))
		return BadMatch;
	    break;
	case KBAutoRepeatMode:
	    i = (key >> 3);
	    mask = (1 << (key & 7));
	    t = (CARD8)*vlist;
	    vlist++;
	    if (t == AutoRepeatModeOff)
	    {
		if (key == DO_ALL)
		    ctrl.autoRepeat = FALSE;
		else
		    ctrl.autoRepeats[i] &= ~mask;
	    }
	    else if (t == AutoRepeatModeOn)
	    {
		if (key == DO_ALL)
		    ctrl.autoRepeat = TRUE;
		else
		    ctrl.autoRepeats[i] |= mask;
	    }
	    else if (t == AutoRepeatModeDefault)
	    {
		if (key == DO_ALL)
		    ctrl.autoRepeat = defaultKeyboardControl.autoRepeat;
		else
		    ctrl.autoRepeats[i] &= ~mask;
		    ctrl.autoRepeats[i] =
			    (ctrl.autoRepeats[i] & ~mask) |
			    (defaultKeyboardControl.autoRepeats[i] & mask);
	    }
	    else
	    {
		client->errorValue = t;
		return BadValue;
	    }
	    break;
	default:
	    client->errorValue = stuff->mask;
	    return BadValue;
	}
    }
    keybd->u.keybd.ctrl = ctrl;
    (*keybd->u.keybd.CtrlProc)(keybd, &keybd->u.keybd.ctrl);
    return Success;
#undef DO_ALL
} 

int
ProcGetKeyboardControl (client)
    ClientPtr client;
{
    int i;
    DeviceIntPtr keybd = inputInfo.keyboard;
    xGetKeyboardControlReply rep;
    REQUEST(xReq);

    REQUEST_SIZE_MATCH(xReq);
    rep.type = X_Reply;
    rep.length = 5;
    rep.sequenceNumber = client->sequence;
    rep.globalAutoRepeat = keybd->u.keybd.ctrl.autoRepeat;
    rep.keyClickPercent = keybd->u.keybd.ctrl.click;
    rep.bellPercent = keybd->u.keybd.ctrl.bell;
    rep.bellPitch = keybd->u.keybd.ctrl.bell_pitch;
    rep.bellDuration = keybd->u.keybd.ctrl.bell_duration;
    rep.ledMask = keybd->u.keybd.ctrl.leds;
    for (i = 0; i < 32; i++)
	rep.map[i] = keybd->u.keybd.ctrl.autoRepeats[i];
    WriteReplyToClient(client, sizeof(xGetKeyboardControlReply), &rep);
    return Success;
} 

int
ProcBell(client)
    ClientPtr client;
{
    register DeviceIntPtr keybd = inputInfo.keyboard;
    int base = keybd->u.keybd.ctrl.bell;
    int newpercent;
    REQUEST(xBellReq);
    REQUEST_SIZE_MATCH(xBellReq);
    if (stuff->percent < -100 || stuff->percent > 100)
    {
	client->errorValue = stuff->percent;
	return BadValue;
    }
    newpercent = (base * stuff->percent) / 100;
    if (stuff->percent < 0)
        newpercent = base + newpercent;
    else
    	newpercent = base - newpercent + stuff->percent;
    (*keybd->u.keybd.BellProc)(newpercent, keybd);
    return Success;
} 

int
ProcChangePointerControl(client)
    ClientPtr client;
{
    DeviceIntPtr mouse = inputInfo.pointer;
    PtrCtrl ctrl;		/* might get BadValue part way through */
    REQUEST(xChangePointerControlReq);

    REQUEST_SIZE_MATCH(xChangePointerControlReq);
    ctrl = mouse->u.ptr.ctrl;
    if (stuff->doAccel)
    {
	if (stuff->accelNum == -1)
	    ctrl.num = defaultPointerControl.num;
	else if (stuff->accelNum < 0)
	{
	    client->errorValue = stuff->accelNum;
	    return BadValue;
	}
	else ctrl.num = stuff->accelNum;
	if (stuff->accelDenum == -1)
	    ctrl.den = defaultPointerControl.den;
	else if (stuff->accelDenum <= 0)
	{
	    client->errorValue = stuff->accelDenum;
	    return BadValue;
	}
	else ctrl.den = stuff->accelDenum;
    }
    if (stuff->doThresh)
    {
	if (stuff->threshold == -1)
	    ctrl.threshold = defaultPointerControl.threshold;
	else if (stuff->threshold < 0)
	{
	    client->errorValue = stuff->threshold;
	    return BadValue;
	}
	else ctrl.threshold = stuff->threshold;
    }
    mouse->u.ptr.ctrl = ctrl;
    (*mouse->u.ptr.CtrlProc)(mouse, &mouse->u.ptr.ctrl);
    return Success;
} 

int
ProcGetPointerControl(client)
    ClientPtr client;
{
    register DeviceIntPtr mouse = inputInfo.pointer;
    REQUEST(xReq);
    xGetPointerControlReply rep;

    REQUEST_SIZE_MATCH(xReq);
    rep.type = X_Reply;
    rep.length = 0;
    rep.sequenceNumber = client->sequence;
    rep.threshold = mouse->u.ptr.ctrl.threshold;
    rep.accelNumerator = mouse->u.ptr.ctrl.num;
    rep.accelDenominator = mouse->u.ptr.ctrl.den;
    WriteReplyToClient(client, sizeof(xGenericReply), &rep);
    return Success;
}

static void
MaybeStopHint(client)
    ClientPtr client;
{
    GrabPtr grab = inputInfo.pointer->grab;
    Mask mask;

    if ((grab && (client == grab->client) &&
	 (grab->eventMask & PointerMotionHintMask)) ||
	(!grab && (EventMaskForClient(motionHintWindow, client, &mask) &
		   PointerMotionHintMask)))
	motionHintWindow = NullWindow;
}

int
ProcGetMotionEvents(client)
    ClientPtr client;
{
    WindowPtr pWin;
    xTimecoord * coords;
    xGetMotionEventsReply rep;
    int     i, count, xmin, xmax, ymin, ymax;
    unsigned long nEvents;
    DeviceIntPtr mouse = inputInfo.pointer;
    TimeStamp start, stop;
    REQUEST(xGetMotionEventsReq);

    REQUEST_SIZE_MATCH(xGetMotionEventsReq);
    pWin = LookupWindow(stuff->window, client);
    if (!pWin)
	return BadWindow;
    if (motionHintWindow)
	MaybeStopHint(client);
    rep.type = X_Reply;
    rep.sequenceNumber = client->sequence;
    rep.nEvents = 0;
    start = ClientTimeToServerTime(stuff->start);
    stop = ClientTimeToServerTime(stuff->stop);
    if (CompareTimeStamps(start, stop) == LATER)
        return Success;
    if (CompareTimeStamps(start, currentTime) == LATER)
        return Success;
    if (CompareTimeStamps(stop, currentTime) == LATER)
        stop = currentTime;
    if (inputInfo.numMotionEvents)
    {
	coords = (xTimecoord *) xalloc(
		inputInfo.numMotionEvents * sizeof(xTimecoord));
	/* XXX this needs a screen arg */
	count = (*mouse->u.ptr.GetMotionProc) (
		mouse, coords, start.milliseconds, stop.milliseconds);
	xmin = pWin->absCorner.x - pWin->borderWidth;
	xmax =
	    pWin->absCorner.x + (int)pWin->clientWinSize.width + pWin->borderWidth;
	ymin = pWin->absCorner.y - pWin->borderWidth;
	ymax =
	    pWin->absCorner.y + (int)pWin->clientWinSize.height + pWin->borderWidth;
	for (i = 0; i < count; i++)
	    if ((xmin <= coords[i].x) && (coords[i].x < xmax) &&
		    (ymin <= coords[i].y) && (coords[i].y < ymax))
	    {
		coords[rep.nEvents].x = coords[i].x - pWin->absCorner.x;
		coords[rep.nEvents].y = coords[i].y - pWin->absCorner.y;
		rep.nEvents++;
	    }
    }
    rep.length = rep.nEvents * (sizeof(xTimecoord) / 4);
    nEvents = rep.nEvents;
    WriteReplyToClient(client, sizeof(xGetMotionEventsReply), &rep);
    if (inputInfo.numMotionEvents)
    {
	client->pSwapReplyFunc = SwapTimeCoordWrite;
	WriteSwappedDataToClient(client, nEvents * sizeof(xTimecoord),
				 (char *)coords);
	xfree(coords);
    }
    return Success;
}

int
ProcQueryPointer(client)
    ClientPtr client;
{
    xQueryPointerReply rep;
    WindowPtr pWin, t;
    REQUEST(xResourceReq);

    REQUEST_SIZE_MATCH(xResourceReq);
    pWin = LookupWindow(stuff->id, client);
    if (!pWin)
	return BadWindow;
    if (motionHintWindow)
	MaybeStopHint(client);
    rep.type = X_Reply;
    rep.sequenceNumber = client->sequence;
    rep.mask = keyButtonState;
    rep.length = 0;
    rep.root = (ROOT)->wid;
    rep.rootX = sprite.hot.x;
    rep.rootY = sprite.hot.y;
    rep.child = None;
    if (currentScreen == pWin->drawable.pScreen)
    {
	rep.sameScreen = xTrue;
	rep.winX = sprite.hot.x - pWin->absCorner.x;
	rep.winY = sprite.hot.y - pWin->absCorner.y;
	for (t = sprite.win; t; t = t->parent)
	    if (t->parent == pWin)
	    {
		rep.child = t->wid;
		break;
	    }
    }
    else
    {
	rep.sameScreen = xFalse;
	rep.winX = 0;
	rep.winY = 0;
    }
    WriteReplyToClient(client, sizeof(xQueryPointerReply), &rep);

    return(Success);    
}

int
ProcQueryKeymap(client)
    ClientPtr client;
{
    xQueryKeymapReply rep;
    int i;

    rep.type = X_Reply;
    rep.sequenceNumber = client->sequence;
    rep.length = 2;
    for (i = 0; i<32; i++)
	rep.map[i] = inputInfo.keyboard->down[i];
    WriteReplyToClient(client, sizeof(xQueryKeymapReply), &rep);
    return Success;
}

/* This define is taken from extension.c and must be consistent with it.
	This is probably not the best programming practice. PRH */

#define EXTENSION_EVENT_BASE  64
int
ProcSendEvent(client)
    ClientPtr client;
{
    extern int lastEvent; 		/* Defined in extension.c */
    WindowPtr pWin;
    WindowPtr effectiveFocus = NullWindow; /* only set if dest==InputFocus */
    REQUEST(xSendEventReq);

    REQUEST_SIZE_MATCH(xSendEventReq);

    /* The client's event type must be a core event type or one defined by an
	extension. */

    if ( ! ((stuff->event.u.u.type < LASTEvent) || 
	((EXTENSION_EVENT_BASE  <= stuff->event.u.u.type) &&
	(stuff->event.u.u.type < lastEvent))) )
    {
	client->errorValue = stuff->event.u.u.type;
	return BadValue;
    }

    if (stuff->destination == PointerWindow)
	pWin = sprite.win;
    else if (stuff->destination == InputFocus)
    {
	WindowPtr inputFocus = inputInfo.keyboard->u.keybd.focus.win;

	if (inputFocus == NoneWin)
	    return Success;

	/* If the input focus is PointerRootWin, send the event to where
	the pointer is if possible, then perhaps propogate up to root. */
   	if (inputFocus == PointerRootWin)
	    inputFocus = ROOT;

	if (IsParent(inputFocus, sprite.win))
	{
	    effectiveFocus = inputFocus;
	    pWin = sprite.win;
	}
	else
	    effectiveFocus = pWin = inputFocus;
    }
    else
	pWin = LookupWindow(stuff->destination, client);
    if (!pWin)
	return BadWindow;
    stuff->event.u.u.type |= 0x80;
    if (stuff->propagate)
    {
	for (;pWin; pWin = pWin->parent)
	{
	    if (DeliverEventsToWindow(
			pWin, &stuff->event, 1, stuff->eventMask, NullGrab))
		return Success;
	    if (pWin == effectiveFocus)
		return Success;
	    stuff->eventMask &= ~pWin->dontPropagateMask;
	}
    }
    else
	(void)DeliverEventsToWindow(
	    pWin, &stuff->event, 1, stuff->eventMask, NullGrab);
    return Success;
}

int
ProcUngrabKey(client)
    ClientPtr client;
{
    REQUEST(xUngrabKeyReq);
    WindowPtr pWin;
    GrabRec temporaryGrab;

    REQUEST_SIZE_MATCH(xUngrabKeyReq);
    pWin = LookupWindow(stuff->grabWindow, client);
    if (!pWin)
	return BadWindow;

    temporaryGrab.client = client;
    temporaryGrab.device = inputInfo.keyboard;
    temporaryGrab.window = pWin;
    temporaryGrab.modifiersDetail.exact = stuff->modifiers;
    temporaryGrab.modifiersDetail.pMask = NULL;
    temporaryGrab.detail.exact = stuff->key;
    temporaryGrab.detail.pMask = NULL;

    DeletePassiveGrabFromList(&temporaryGrab);

    return(Success);
}

int
ProcGrabKey(client)
    ClientPtr client;
{
    WindowPtr pWin;
    REQUEST(xGrabKeyReq);
    GrabPtr grab;
    GrabPtr temporaryGrab;

    REQUEST_SIZE_MATCH(xGrabKeyReq);
    if (((stuff->key > curKeySyms.maxKeyCode) || (stuff->key < curKeySyms.minKeyCode))
	&& (stuff->key != AnyKey))
    {
	client->errorValue = stuff->key;
        return BadValue;
    }
    pWin = LookupWindow(stuff->grabWindow, client);
    if (!pWin)
	return BadWindow;

    temporaryGrab = CreateGrab(client, inputInfo.keyboard, pWin, 
	(Mask)(KeyPressMask | KeyReleaseMask), (Bool)stuff->ownerEvents,
	(Bool)stuff->keyboardMode, (Bool)stuff->pointerMode,
	stuff->modifiers, stuff->key, NullWindow, NullCursor);

    for (grab = PASSIVEGRABS(pWin); grab; grab = grab->next)
    {
	if (GrabMatchesSecond(temporaryGrab, grab))
	{
	    if (client != grab->client)
	    {
		DeleteGrab(temporaryGrab);
		return BadAccess;
	    }
	}
    }

    DeletePassiveGrabFromList(temporaryGrab);

    AddPassiveGrabToWindowList(temporaryGrab);

    return(Success);
}

int
ProcGrabButton(client)
    ClientPtr client;
{
    WindowPtr pWin, confineTo;
    REQUEST(xGrabButtonReq);
    GrabPtr grab;
    CursorPtr cursor;
    GrabPtr temporaryGrab;

    REQUEST_SIZE_MATCH(xGrabButtonReq);
    if ((stuff->pointerMode != GrabModeSync) &&
	(stuff->pointerMode != GrabModeAsync))
    {
	client->errorValue = stuff->pointerMode;
        return BadValue;
    }
    if ((stuff->keyboardMode != GrabModeSync) &&
	(stuff->keyboardMode != GrabModeAsync))
    {
	client->errorValue = stuff->keyboardMode;
        return BadValue;
    }

    pWin = LookupWindow(stuff->grabWindow, client);
    if (!pWin)
	return BadWindow;
    if (stuff->confineTo == None)
	confineTo = NullWindow;
    else
    {
	confineTo = LookupWindow(stuff->confineTo, client);
	if (!confineTo)
	    return BadWindow;
    }
    if (stuff->cursor == None)
	cursor = NullCursor;
    else
    {
	cursor = (CursorPtr)LookupID(stuff->cursor, RT_CURSOR, RC_CORE);
	if (!cursor)
	{
	    client->errorValue = stuff->cursor;
	    return BadCursor;
	}
    }


    temporaryGrab = CreateGrab(client, inputInfo.pointer, pWin, 
	(Mask)(stuff->eventMask | ButtonPressMask | ButtonReleaseMask),
	(Bool)stuff->ownerEvents, (Bool) stuff->keyboardMode,
	(Bool)stuff->pointerMode, stuff->modifiers, stuff->button,
	confineTo, cursor);

    for (grab = PASSIVEGRABS(pWin); grab; grab = grab->next)
    {
	if (GrabMatchesSecond(temporaryGrab, grab))
	{
	    if (client != grab->client)
	    {
		DeleteGrab(temporaryGrab);
		return BadAccess;
	    }
	}
    }

    DeletePassiveGrabFromList(temporaryGrab);

    AddPassiveGrabToWindowList(temporaryGrab);

    return(Success);
}

int
ProcUngrabButton(client)
    ClientPtr client;
{
    REQUEST(xUngrabButtonReq);
    WindowPtr pWin;
    GrabRec temporaryGrab;

    REQUEST_SIZE_MATCH(xUngrabButtonReq);
    pWin = LookupWindow(stuff->grabWindow, client);
    if (!pWin)
	return BadWindow;

    temporaryGrab.client = client;
    temporaryGrab.device = inputInfo.pointer;
    temporaryGrab.window = pWin;
    temporaryGrab.modifiersDetail.exact = stuff->modifiers;
    temporaryGrab.modifiersDetail.pMask = NULL;
    temporaryGrab.detail.exact = stuff->button;
    temporaryGrab.detail.pMask = NULL;

    DeletePassiveGrabFromList(&temporaryGrab);

    return(Success);
}

void
DeleteWindowFromAnyEvents(pWin, freeResources)
    WindowPtr		pWin;
    Bool		freeResources;
{
    WindowPtr		parent;
    FocusPtr		focus = &inputInfo.keyboard->u.keybd.focus;
    DeviceIntPtr	mouse = inputInfo.pointer;
    OtherClientsPtr	oc;
    GrabPtr		passive;


    /* Deactivate any grabs performed on this window, before making any
	input focus changes. */

    if ((mouse->grab) &&
	((mouse->grab->window == pWin) ||
	 (mouse->grab->confineTo == pWin)))
	DeactivatePointerGrab(mouse);

    /* Deactivating a keyboard grab should cause focus events. */

    if ((inputInfo.keyboard->grab) &&
	(inputInfo.keyboard->grab->window == pWin))
	DeactivateKeyboardGrab(inputInfo.keyboard);

    /* If the focus window is a root window (ie. has no parent) then don't 
	delete the focus from it. */
    
    if ((pWin == focus->win) && (pWin->parent != NullWindow))
    {
	int focusEventMode = NotifyNormal;

 	/* If a grab is in progress, then alter the mode of focus events. */

	if (inputInfo.keyboard->grab)
		focusEventMode = NotifyWhileGrabbed;

	switch (focus->revert)
	{
	    case RevertToNone:
		DoFocusEvents(pWin, NoneWin, focusEventMode);
		focus->win = NoneWin;
	        focusTraceGood = 0;
		break;
	    case RevertToParent:
		parent = pWin;
		do
		{
		    parent = parent->parent;
		    focusTraceGood--;
		} while (!parent->realized);
		DoFocusEvents(pWin, parent, focusEventMode);
		focus->win = parent;
		focus->revert = RevertToNone;
		break;
	    case RevertToPointerRoot:
		DoFocusEvents(pWin, PointerRootWin, focusEventMode);
		focus->win = PointerRootWin;
		focusTraceGood = 0;
		break;
	}
    }

    if (motionHintWindow == pWin)
	motionHintWindow = NullWindow;

    if (freeResources)
    {
	while (oc = OTHERCLIENTS(pWin))
	    FreeResource(oc->resource, RC_NONE);
	while (passive = PASSIVEGRABS(pWin))
	    FreeResource(passive->resource, RC_NONE);
     }
}

/* Call this whenever some window at or below pWin has changed geometry */

/*ARGSUSED*/
void
CheckCursorConfinement(pWin)
    WindowPtr pWin;
{
    GrabPtr grab = inputInfo.pointer->grab;
    WindowPtr confineTo;

    if (grab && (confineTo = grab->confineTo))
    {
	if (!(* confineTo->drawable.pScreen->RegionNotEmpty)
			(confineTo->borderSize))
	    DeactivatePointerGrab(inputInfo.pointer);
	/* We could traverse the tree rooted at pWin to see if it contained
	 * confineTo, and only then call ConfineCursorToWindow, but it hardly
	 * seems worth it.
	 */
	else if (pWin->firstChild || (pWin == confineTo))
	    ConfineCursorToWindow(confineTo, sprite.hot.x, sprite.hot.y, TRUE);
    }
}

Mask
EventMaskForClient(win, client, allMask)
    WindowPtr		win;
    ClientPtr		client;
    Mask		*allMask;
{
    OtherClientsPtr	other;
    Mask		him = 0;
    if (win->client == client)
	him = win->eventMask;
    *allMask = win->eventMask;
    for (other = OTHERCLIENTS(win); other; other = other->next)
    {
	if (other->client == client)
	    him = other->mask;
	*allMask |= other->mask;
    }
    return him;
}


int
ProcRecolorCursor(client)
    ClientPtr client;
{
    CursorPtr pCursor;
    int		nscr;
    ScreenPtr	pscr;
    REQUEST(xRecolorCursorReq);

    REQUEST_SIZE_MATCH(xRecolorCursorReq);
    pCursor = (CursorPtr)LookupID(stuff->cursor, RT_CURSOR, RC_CORE);
    if ( !pCursor) 
    {
	client->errorValue = stuff->cursor;
	return (BadCursor);
    }

    pCursor->foreRed = stuff->foreRed;
    pCursor->foreGreen = stuff->foreGreen;
    pCursor->foreBlue = stuff->foreBlue;

    pCursor->backRed = stuff->backRed;
    pCursor->backGreen = stuff->backGreen;
    pCursor->backBlue = stuff->backBlue;

    for ( nscr=0, pscr=screenInfo.screen;
	  nscr<screenInfo.numScreens;
	  nscr++, pscr++)
    {
	( *pscr->RecolorCursor)(pscr, pCursor,
		(pCursor == sprite.current) && (pscr == currentScreen));
    }
    return (Success);
}

void
WriteEventsToClient(pClient, count, events)
    ClientPtr	pClient;
    int		count;
    xEvent	*events;
{
    if(pClient->swapped)
    {
        int	i;
        xEvent	eventTo, *eventFrom;

	for(i = 0; i < count; i++)
	{
	    eventFrom = &events[i];
	    /* Remember to strip off the leading bit of type in case
	       this event was sent with "SendEvent." */
	    (*EventSwapVector[eventFrom->u.u.type & 0177])
		(eventFrom, &eventTo);
	    (void)WriteToClient(pClient, sizeof(xEvent), (char *)&eventTo);
	}
    }

    else
    {
	(void)WriteToClient(pClient, count * sizeof(xEvent), (char *) events);
    }
}

