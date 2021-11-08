#ifndef lint
static char Xrcsid[] = "$XConsortium: Event.c,v 1.90 88/10/21 08:43:10 swick Exp $";
/* $oHeader: Event.c,v 1.9 88/09/01 11:33:51 asente Exp $ */
#endif lint

/***********************************************************
Copyright 1987, 1988 by Digital Equipment Corporation, Maynard, Massachusetts,
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

#include "IntrinsicI.h"
#include "Shell.h"
#include "StringDefs.h"

static XtAsyncHandler asyncHandler = NULL;
static caddr_t asyncClosure = NULL;

static GrabList grabList;	/* %%% should this be in the AppContext? */
static GrabList focusList;	/* %%% should this be in the AppContext? */
static Boolean focusTraceGood;	/* %%% ditto */

extern void bzero();

CallbackList *_XtDestroyList;

EventMask XtBuildEventMask(widget)
    Widget widget;
{
    XtEventTable ev;
    EventMask	mask = 0L;

    for (ev = widget->core.event_table; ev != NULL; ev = ev->next)
	if (ev->select) mask |= ev->mask;
    if (widget->core.widget_class->core_class.expose != NULL)
	mask |= ExposureMask;
    if (widget->core.widget_class->core_class.visible_interest) 
	mask |= VisibilityChangeMask;

    return mask;
}

static void RemoveEventHandler(widget, eventMask, other, proc, closure, raw,
			       check_closure)
    Widget	widget;
    EventMask   eventMask;
    Boolean	other;
    XtEventHandler proc;
    caddr_t	closure;
    Boolean	raw;
    Boolean	check_closure;
{
    XtEventRec *p, **pp;
    EventMask oldMask = XtBuildEventMask(widget);

    pp = &widget->core.event_table;
    p = *pp;

    /* find it */
    while (p != NULL &&
	   (p->proc != proc || (check_closure && p->closure != closure))) {
	pp = &p->next;
	p = *pp;
    }
    if (p == NULL) return; /* couldn't find it */
    if (raw) p->raw = FALSE; else p->select = FALSE;
    if (p->raw || p->select) return;

    /* un-register it */
    p->mask &= ~eventMask;
    if (other) p->non_filter = FALSE;

    if (p->mask == 0 && !p->non_filter) {
	/* delete it entirely */
	*pp = p->next;
	XtFree((char *)p);
    }

    /* reset select mask if realized */
    if (XtIsRealized(widget)&& !widget->core.being_destroyed) {
	EventMask mask = XtBuildEventMask(widget);

	if (oldMask != mask) {
	    XSelectInput(XtDisplay(widget), XtWindow(widget), mask);
#ifdef notdef
	    if (asyncHandler != NULL) {
		XSelectAsyncInput(
		    XtDisplay(widget), XtWindow(widget), mask,
		    asyncHandler, (unsigned long) XtDisplay(widget));
	    }
#endif /*notdef*/
	}
    }
}

static void AddEventHandler(widget, eventMask, other, proc, closure, raw,
			    check_closure)
    Widget	    widget;
    EventMask   eventMask;
    Boolean         other;
    XtEventHandler  proc;
    caddr_t	closure;
    Boolean	raw;
    Boolean	check_closure;
{
   register XtEventRec *p,**pp;
   EventMask oldMask;

   if (eventMask == 0 && other == FALSE) return;

   if (XtIsRealized(widget) && ! raw) oldMask = XtBuildEventMask(widget);

   pp = & widget->core.event_table;
   p = *pp;
   while (p != NULL &&
	  (p->proc != proc || (check_closure && p->closure != closure))) {
         pp = &p->next;
         p = *pp;
   }

   if (p == NULL) {
	/* new proc to add to list */
	p = XtNew(XtEventRec);
	p->proc = proc;
	p->closure = closure;
	p->mask = eventMask;
	p->non_filter = other;
	p->select = ! raw;
	p->raw = raw;

	p->next = widget->core.event_table;
	widget->core.event_table = p;

    } else {
	/* update existing proc */
	p->mask |= eventMask;
	p->non_filter = p->non_filter || other;
	p->select |= ! raw;
	p->raw |= raw;
	if (!check_closure) p->closure = closure;
    }

    if (XtIsRealized(widget) && ! raw) {
	EventMask mask = XtBuildEventMask(widget);

	if (oldMask != mask) {
	    XSelectInput(XtDisplay(widget), XtWindow(widget), mask);
#ifdef notdef
	    if (asyncHandler != NULL) {
		XSelectAsyncInput(
		    XtDisplay(widget), XtWindow(widget), mask,
		    asyncHandler, (unsigned long)XtDisplay(widget));
	    }
#endif /*notdef*/
	}
    }

}


void XtRemoveEventHandler(widget, eventMask, other, proc, closure)
    Widget	widget;
    EventMask   eventMask;
    Boolean	other;
    XtEventHandler proc;
    caddr_t	closure;
{
    RemoveEventHandler(widget, eventMask, other, proc, closure, FALSE, TRUE);
}


void XtAddEventHandler(widget, eventMask, other, proc, closure)
    Widget	    widget;
    EventMask   eventMask;
    Boolean         other;
    XtEventHandler  proc;
    caddr_t	closure;
{
    AddEventHandler(widget, eventMask, other, proc, closure, FALSE, TRUE);
}


void XtRemoveRawEventHandler(widget, eventMask, other, proc, closure)
    Widget	widget;
    EventMask   eventMask;
    Boolean	other;
    XtEventHandler proc;
    caddr_t	closure;
{
    RemoveEventHandler(widget, eventMask, other, proc, closure, TRUE, TRUE);
}


void XtAddRawEventHandler(widget, eventMask, other, proc, closure)
    Widget	    widget;
    EventMask   eventMask;
    Boolean         other;
    XtEventHandler  proc;
    caddr_t	closure;
{
    AddEventHandler(widget, eventMask, other, proc, closure, TRUE, TRUE);
}


typedef struct _HashRec *HashPtr;

typedef struct _HashRec {
    Display	*display;
    Window	window;
    Widget	widget;
    HashPtr	next;
} HashRec;

typedef struct {
    unsigned int	size;
    unsigned int	count;
    HashPtr		entries[1];
} HashTableRec, *HashTable;

static HashTable table = NULL;

static void ExpandTable();

void _XtRegisterWindow(window, widget)
    Window window;
    Widget widget;
{
    register HashPtr hp, *hpp;

    if ((table->count + (table->count / 4)) >= table->size) ExpandTable();

    hpp = &table->entries[(unsigned int)window & (table->size-1)];
    hp = *hpp;

    while (hp != NULL) {
        if (hp->window == window && hp->display == XtDisplay(widget)) {
	    if (hp->widget != widget)
		XtAppWarningMsg(XtWidgetToApplicationContext(widget),
			"registerWindowError","xtRegisterWindow",
                         "XtToolkitError",
                        "Attempt to change already registered window.",
                          (String *)NULL, (Cardinal *)NULL);
	    return;
	}
        hpp = &hp->next;
	hp = *hpp;
    }

    hp = *hpp = XtNew(HashRec);
    hp->display = XtDisplay(widget);
    hp->window = window;
    hp->widget = widget;
    hp->next = NULL;
    table->count++;
}


void _XtUnregisterWindow(window, widget)
    Window window;
    Widget widget;
{
    HashPtr hp, *hpp;

    hpp = &table->entries[(unsigned int)window  & (table->size-1)];
    hp = *hpp;

    while (hp != NULL) {
        if (hp->window == window && hp->display == XtDisplay(widget)) {
	    if (hp->widget != widget) {
                XtAppWarningMsg(XtWidgetToApplicationContext(widget),
			"registerWindowError","xtUnregisterWindow",
                         "XtToolkitError",
                        "Attempt to unregister invalid window.",
                          (String *)NULL, (Cardinal *)NULL);

                return;
                }
             else /* found entry to delete */
                  (*hpp) = hp->next;
                  XtFree((char*)hp);
                  table->count--;
                  return;
	}
        hpp = &hp->next;
	hp = *hpp;
    }
    
}

static void ExpandTable()
{
    HashTable	oldTable = table;
    unsigned int i;

    i = oldTable->size * 2;
    table = (HashTable) XtCalloc(1,
	    (unsigned) sizeof(HashTableRec)+i*sizeof(HashPtr));

    table->size = i;
    table->count = 0;
    for (i = 0; i<oldTable->size; i++) {
	HashPtr hp;
	hp = oldTable->entries[i];
	while (hp != NULL) {
	    HashPtr temp = hp;
	    _XtRegisterWindow(hp->window, hp->widget);
	    hp = hp->next;
	    XtFree((char *) temp);
	}
    }
    XtFree((char *)oldTable);
}


/*ARGSUSED*/
Widget XtWindowToWidget(display, window)
    Display *display;
    Window window;
{
    register HashPtr hp;

    for (hp = table->entries[(unsigned int)window & (table->size-1)];
	    hp != NULL; hp = hp->next) {
	if (hp->window == window && hp->display == display) return hp->widget;
    }

    return NULL;
}

static void InitializeHash()
{
    int size = sizeof(HashTableRec)+1024*sizeof(HashPtr);

    table = (HashTable) XtMalloc((unsigned) size);
    bzero((char *) table, size);

    table->size = 1024;
    table->count = 0;
}

static Region nullRegion;

static void DispatchEvent(event, widget, mask)
    register XEvent    *event;
    Widget    widget;
    unsigned long mask;
{
    XtEventRec *p;   
    XtEventHandler proc[100];
    caddr_t closure[100];
    int numprocs, i;
    XEvent nextEvent;

    if (mask == ExposureMask) {
	XtExposeProc expose = widget->core.widget_class->core_class.expose;
	if (expose != NULL) {
	    if (!widget->core.widget_class->core_class.compress_exposure) {
		(*expose)(widget, event, (Region)NULL);
	    }
	    else {
		XtPerDisplay pd = _XtGetPerDisplay(event->xany.display);
		XtAddExposureToRegion(event, pd->region);
		if (event->xexpose.count == 0) {
		    /* Patch event to have the new bounding box.  Unless
		       someone's goofed, it can only be an Expose event */
		    XRectangle rect;
		    XClipBox(pd->region, &rect);
		    event->xexpose.x = rect.x;
		    event->xexpose.y = rect.y;
		    event->xexpose.width = rect.width;
		    event->xexpose.height = rect.height;
		    (*expose)(widget, event, pd->region);
		    (void) XIntersectRegion(
		        nullRegion, pd->region, pd->region);
		}
	    }
	}
    }

    if (mask == EnterWindowMask &&
	    widget->core.widget_class->core_class.compress_enterleave) {
	if (XPending(event->xcrossing.display)) {
	    XPeekEvent(event->xcrossing.display, &nextEvent);
	    if (nextEvent.type == LeaveNotify &&
		    event->xcrossing.window == nextEvent.xcrossing.window &&
		    event->xcrossing.subwindow == nextEvent.xcrossing.subwindow) {
		/* skip the enter/leave pair */
		XNextEvent(event->xcrossing.display, &nextEvent);
		return;
	    }
	}
    }

    if (event->type == MotionNotify &&
	    widget->core.widget_class->core_class.compress_motion) {
	while (XPending(event->xmotion.display)) {
	    XPeekEvent(event->xmotion.display, &nextEvent);
	    if (nextEvent.type == MotionNotify &&
		    event->xmotion.window == nextEvent.xmotion.window &&
		    event->xmotion.subwindow == nextEvent.xmotion.subwindow) {
		/* replace the current event with the next one */
		XNextEvent(event->xmotion.display, event);
	    } else break;
	}
    }

    if ((mask == VisibilityChangeMask) &&
        XtClass(widget)->core_class.visible_interest) {
	    /* our visibility just changed... */
	    switch (((XVisibilityEvent *)event)->state) {
		case VisibilityUnobscured:
		    widget->core.visible = TRUE;
		    break;

		case VisibilityPartiallyObscured:
		    /* what do we want to say here? */
		    /* well... some of us is visible */
		    widget->core.visible = TRUE;
		    break;

		case VisibilityFullyObscured:
		    widget->core.visible = FALSE;
		    /* do we want to mark our children obscured? */
		    break;
	    }
	}

    /* Have to copy the procs into an array, because calling one of them */
    /* might call XtRemoveEventHandler, which would break our linked list.*/

    numprocs = 0;

    for (p=widget->core.event_table; p != NULL; p = p->next) {
	if ((mask & p->mask) != 0 || (mask == 0 && p->non_filter)) {
	    proc[numprocs] = p->proc;
	    closure[numprocs++] = p->closure;
	}
    }

    for (i=0 ; i<numprocs ; i++) (*(proc[i]))(widget, closure[i], event);
}


typedef enum _GrabType {pass, ignore, remap} GrabType;

static void ConvertTypeToMask (eventType, mask, grabType)
    int		eventType;
    EventMask   *mask;
    GrabType    *grabType;
{

static struct {
    EventMask   mask;
    GrabType    grabType;
} masks[] = {
    {0,				pass},      /* shouldn't see 0  */
    {0,				pass},      /* shouldn't see 1  */
    {KeyPressMask,		remap},     /* KeyPress		*/
    {KeyReleaseMask,		remap},     /* KeyRelease       */
    {ButtonPressMask,		remap},     /* ButtonPress      */
    {ButtonReleaseMask,		remap},     /* ButtonRelease    */
    {PointerMotionMask
     | Button1MotionMask
     | Button2MotionMask
     | Button3MotionMask
     | Button4MotionMask
     | Button5MotionMask
     | ButtonMotionMask,	ignore},    /* MotionNotify	*/
    {EnterWindowMask,		ignore},    /* EnterNotify	*/
    {LeaveWindowMask,		ignore},    /* LeaveNotify	*/
    {FocusChangeMask,		pass},      /* FocusIn		*/
    {FocusChangeMask,		pass},      /* FocusOut		*/
    {KeymapStateMask,		pass},      /* KeymapNotify	*/
    {ExposureMask,		pass},      /* Expose		*/
    {0,				pass},      /* GraphicsExpose   */
    {0,				pass},      /* NoExpose		*/
    {VisibilityChangeMask,      pass},      /* VisibilityNotify */
    {SubstructureNotifyMask,    pass},      /* CreateNotify	*/
    {StructureNotifyMask
     | SubstructureNotifyMask,  pass},      /* DestroyNotify	*/
    {StructureNotifyMask
     | SubstructureNotifyMask,  pass},      /* UnmapNotify	*/
    {StructureNotifyMask
     | SubstructureNotifyMask,  pass},      /* MapNotify	*/
    {SubstructureRedirectMask,  pass},      /* MapRequest	*/
    {StructureNotifyMask
     | SubstructureNotifyMask,  pass},      /* ReparentNotify   */
    {StructureNotifyMask
     | SubstructureNotifyMask,  pass},      /* ConfigureNotify  */
    {SubstructureRedirectMask,  pass},      /* ConfigureRequest */
    {StructureNotifyMask
     | SubstructureNotifyMask,  pass},      /* GravityNotify	*/
    {ResizeRedirectMask,	pass},      /* ResizeRequest	*/
    {StructureNotifyMask
     | SubstructureNotifyMask,  pass},      /* CirculateNotify	*/
    {SubstructureRedirectMask,  pass},      /* CirculateRequest */
    {PropertyChangeMask,	pass},      /* PropertyNotify   */
    {0,				pass},      /* SelectionClear   */
    {0,				pass},      /* SelectionRequest */
    {0,				pass},      /* SelectionNotify  */
    {ColormapChangeMask,	pass},      /* ColormapNotify   */
    {0,				pass},      /* ClientMessage	*/
    {0,				pass},      /* MappingNotify ???*/
  };

    eventType &= 0x7f;	/* Events sent with XSendEvent have high bit set. */
    (*mask)      = masks[eventType].mask;
    (*grabType)  = masks[eventType].grabType;
};

static Boolean OnGrabList(widget)
    register Widget widget;
{
    register GrabRec* gl;
    for (; widget != NULL; widget = (Widget)widget->core.parent) {
	for (gl = grabList; gl != NULL; gl = gl->next) {
	    if (gl->widget == widget) return TRUE;
	    if (gl->exclusive) break;
	}
    }
    return FALSE;
}

static Widget FindFocusWidget(widget)
     Widget widget;
{
#define CACHESIZE 20
    register GrabRec* gl;
    GrabRec *start = focusList;
    static Widget *anc = NULL;
    static int ancSize;
    register Widget w;
    register int i;
    int src;
    Widget dst;
    static Widget lastQueryWidget, focusWidget;

    if (focusList == NULL) return widget;

    /* First time in, allocate the ancestor list */
    if (anc == NULL) {
	anc = (Widget *) XtMalloc(CACHESIZE * sizeof(Widget));
	ancSize = CACHESIZE;
    }

    if (!focusTraceGood || widget != lastQueryWidget) {
	lastQueryWidget = widget;
	focusTraceGood = True;
	/* First fill in the ancestor list */

	for (i = 0, w = widget; w != NULL; w = XtParent(w), i++) {
	    if (i == ancSize) {
		/* This should rarely happen, but if it does it'll probably
		   happen again, so grow the ancestor list */
		ancSize += CACHESIZE;
		anc = (Widget *) XtRealloc(anc, sizeof(Widget) * ancSize);
	    }
	    anc[i] = w;
	}
	src = i-1;
	dst = widget;

	/* For each ancestor, starting at the top, see if it's forwarded */

	while (src >= 0) {
	    for (gl = start; gl != NULL && gl->widget != anc[src]; gl = gl->next) ;
	    if (gl == NULL)	src--;	/* not in list, try next anc. */
	    else {
		dst = gl->keyboard_focus;
		start = gl->next;	/* Continue from the one we found */
		/* See if dst is an ancestor */
		for (i = src-1; i > 0 && anc[i] != dst; i--) {}
		/* If out of forwarding, send to dst if it's not an ancestor, and
		   the widget if it is (don't forward events to an ancestor) */

		if (start == NULL) {
		    if (i <= 0) return (focusWidget = dst);
		    else return (focusWidget = widget);
		}
		if (i <= 0) break;		/* We've moved to an uncle */
		else src = i;		/* Continue looking from dst */
	    }
	}

	/* If we haven't moved into some other branch, forwarding is either
	   to the widget or a descendent */

	for (gl = start; gl != NULL; gl = gl->next) {
	    if (gl->widget == dst) dst = gl->keyboard_focus;
	}
	focusWidget = dst;
    }

    return focusWidget;
#undef CACHESIZE
}

void DispatchToSpringLoaded(event, widget, mask)
    XEvent  *event;
    Widget widget;
    EventMask   mask;
{
    register    GrabList gl;

    for (gl = grabList; gl != NULL; gl = gl->next) {
	if (gl->spring_loaded) {
	    if (gl->widget != widget) {
		/* ||| Should this test for sensitive? */
		DispatchEvent(event, gl->widget, mask);
	    }
	    break;
	}
	if (gl->exclusive) break;
    }
}

static void DecideToDispatch(event)
    XEvent  *event;
{
    register    Widget widget;
    EventMask   mask;
    GrabType    grabType;
    GrabList    gl;
    Widget	dspWidget;

    widget = XtWindowToWidget (event->xany.display, event->xany.window);

    /* Lint complains about &grabType not matching the declaration.
       Don't bother trying to fix it, it won't work */
    ConvertTypeToMask(event->xany.type, &mask, &grabType);

    if (widget == NULL) {
	if (grabType != remap) return;
	/* event occurred in a non-widget window, but we've promised also
	   to dispatch it to the nearest accessible spring_loaded widget */
	DispatchToSpringLoaded(event, widget, mask);
	return;
    }

    switch(grabType) {
	case pass:
	    DispatchEvent(event, widget, mask);
	    return;

	case ignore:
	    if ((grabList == NULL || OnGrabList(widget)) && 
		    XtIsSensitive(widget)) DispatchEvent(event, widget, mask);
	    return;

	case remap:

#define IsKeyEvent (mask & (KeyPressMask | KeyReleaseMask))

	    /* If a focus event, see if it has been focussed.  */

	    if (IsKeyEvent) dspWidget = FindFocusWidget(widget);
	    else dspWidget = widget;

	    if ((grabList == NULL || OnGrabList(dspWidget)) &&
		    XtIsSensitive(dspWidget)) {
		DispatchEvent(event, dspWidget, mask);
	    }

	    /* Also dispatch to nearest accessible spring_loaded.  Note we
	       use widget here, not dspWidget */
	    DispatchToSpringLoaded(event, widget, mask);
	    return;

#undef IsKeyEvent
    }
}

void XtDispatchEvent (event)
    XEvent  *event;
{
    CallbackList *oldDestroyList, destroyList;

    /* Skip null events from XtNextEvent */
    if (event->type == 0) return;

    /*
     * To make recursive XtDispatchEvent work, we need to do phase 2 destroys
     * only on those widgets destroyed by this particular dispatch.
     * The "right" way to do this is by passing a local variable through to
     * each recursive instance, and passing the list to XtDestroy, but that
     * causes unwieldy proliferation of arguments. We could do all this stuff
     * with signals (if we had them), but instead we have a global pointer
     * to the "current" destroy list, and XtDispatchEvent and XtDestroy
     * conspire to keep it up to date, and use the right one.
     *
     * This is pretty gross.
     */

    oldDestroyList = _XtDestroyList;
    _XtDestroyList = &destroyList;
    destroyList = NULL;

    DecideToDispatch(event);

    /* To accomodate widgets destroying other widgets in their destroy
     * callbacks, we have to make this a loop */

    while (destroyList != NULL) {
	CallbackList newList = NULL;
	_XtDestroyList = &newList;
	_XtCallCallbacks (&destroyList, (caddr_t) NULL);
	_XtRemoveAllCallbacks (&destroyList);
	destroyList = newList;
    }

    _XtDestroyList = oldDestroyList;

    if (_XtSafeToDestroy) {
	if (_XtAppDestroyCount != 0) _XtDestroyAppContexts();
	if (_XtDpyDestroyCount != 0) _XtCloseDisplays();
    }
}

static Boolean RemoveGrab();

/* ARGSUSED */
static void GrabDestroyCallback(widget, closure, call_data)
    Widget  widget;
    caddr_t closure;
    caddr_t call_data;
{
    /* Remove widget from grab list if it destroyed */
    (void)RemoveGrab(widget, False);
}

/* ARGSUSED */
static void FocusDestroyCallback(widget, closure, call_data)
    Widget  widget;
    caddr_t closure;		/* Widget */
    caddr_t call_data;
{
    /* Remove widget from grab list if it destroyed */
    (void)RemoveGrab((Widget)closure, True);
}

static GrabRec *NewGrabRec(widget, exclusive, spring_loaded, keyboard_focus)
    Widget  widget;
    Boolean exclusive;
    Boolean spring_loaded;
    Widget keyboard_focus;
{
    register GrabList    gl;

    gl		      = XtNew(GrabRec);
    gl->next	      = NULL;
    gl->widget        = widget;
    gl->exclusive     = exclusive;
    gl->spring_loaded = spring_loaded;
    gl->keyboard_focus= keyboard_focus;

    return gl;
}

void XtAddGrab(widget, exclusive, spring_loaded)
    Widget  widget;
    Boolean exclusive;
    Boolean spring_loaded;
{
    register    GrabList gl;

    if (spring_loaded && !exclusive) {
	XtAppWarningMsg(XtWidgetToApplicationContext(widget),
		"grabError", "grabDestroyCallback", "XtToolkitError",
		"XtAddGrab requires exclusive grab if spring_loaded is TRUE",
		(String *) NULL, (Cardinal *) NULL);
	exclusive = TRUE;
    }

    gl = NewGrabRec(widget, exclusive, spring_loaded, NULL);
    gl->next = grabList;
    grabList = gl;

    XtAddCallback (widget, XtNdestroyCallback, 
	    GrabDestroyCallback, (caddr_t) NULL);
}

/* add a focus record to the list, or replace the focus widget in an
   existing grab record.  Returns True if the action was not a no-op.
 */
static Boolean InsertFocusEntry(widget, keyboard_focus)
    Widget  widget;
    Widget  keyboard_focus;
{
    register GrabRec *gl, *prev;
    GrabRec* ge;
    Widget w;

    for (gl = focusList; gl != NULL; gl = gl->next) {
	if (gl->widget == widget) {
	    if (gl->keyboard_focus == keyboard_focus) return False;
	    SendFocusNotify(gl->keyboard_focus, FocusOut);
	    gl->keyboard_focus = keyboard_focus;
	    focusTraceGood = False; /* invalidate the cache */
	    return True;
	}
    }
    focusTraceGood = False;	/* invalidate the cache */

    /* Create a new record and insert it before the first entry with a widget
       that is a child of widget.  This enforces the invariant that if A is an
       ancestor of B, A will precede B on the list. */

    ge = NewGrabRec(widget, False, False, keyboard_focus);

    for (prev = NULL, gl = focusList; gl != NULL; prev = gl, gl = gl->next) {
	for (w = gl->widget; w != NULL && w != widget; w = w->core.parent) {}
	if (w == widget) break;
    }

    if (prev == NULL) focusList = ge;
    else prev->next = ge;
    ge->next = gl;

    XtAddCallback(keyboard_focus, XtNdestroyCallback,
	    FocusDestroyCallback, (caddr_t)widget);

    return True;
}

static Boolean RemoveGrab(widget, keyboard_focus)
    Widget  widget;
    Boolean keyboard_focus;
    /* returns False if no grab entry was found, True otherwise */
{
    GrabList *whichList;
    register GrabList gl, prev, next;
    register Boolean done;

    if (keyboard_focus) whichList = &focusList;
    else whichList = &grabList;

    for (prev = NULL, gl = *whichList; gl != NULL; prev = gl, gl = gl->next) {
	if (gl->widget == widget) {
	    if (!keyboard_focus || gl->keyboard_focus != NULL) break;
	}
    }

    if (gl == NULL) {
	if (!keyboard_focus) {
	    XtAppWarningMsg(XtWidgetToApplicationContext(widget),
		       "grabError","xtRemoveGrab","XtToolkitError",
		       "XtRemoveGrab asked to remove a widget not on the list",
		       (String *)NULL, (Cardinal *)NULL);
	}
	return False;
    }

    if (keyboard_focus) {
	if (gl == focusList) focusList = gl->next;
	else prev->next = gl->next;
	XtRemoveCallback(gl->keyboard_focus, XtNdestroyCallback,
		FocusDestroyCallback, widget);
	XtFree((char *)gl);
	focusTraceGood = False;	/* invalidate the cache */
	return True;
    }

    do {
	gl = grabList;
	done = (gl->widget == widget);
	grabList = gl->next;
	XtRemoveCallback(gl->widget, XtNdestroyCallback,
		GrabDestroyCallback, (caddr_t)NULL);
	XtFree((char *)gl);
    } while (! done);
    return True;
}

void XtRemoveGrab(widget)
    Widget  widget;
{
    (void)RemoveGrab(widget, False);
}

void XtMainLoop()
{
	XtAppMainLoop(_XtDefaultAppContext());
}

void XtAppMainLoop(app)
	XtAppContext app;
{
    XEvent event;

    for (;;) {
    	XtAppNextEvent(app, &event);
	XtDispatchEvent(&event);
    }
}


void _XtEventInitialize()
{
    grabList = focusList = NULL;
    focusTraceGood = False;
    _XtDestroyList = NULL;
    nullRegion = XCreateRegion();
    InitializeHash();
}

void XtAddExposureToRegion(event, region)
    XEvent   *event;
    Region   region;
{
    XRectangle rect;

    switch (event->type) {
	case Expose:
		rect.x = event->xexpose.x;
		rect.y = event->xexpose.y;
		rect.width = event->xexpose.width;
		rect.height = event->xexpose.height;
		break;
	case GraphicsExpose:
		rect.x = event->xgraphicsexpose.x;
		rect.y = event->xgraphicsexpose.y;
		rect.width = event->xgraphicsexpose.width;
		rect.height = event->xgraphicsexpose.height;
		break;
	default:
		return;
    }

    XUnionRectWithRegion(&rect, region, region);
}


void _XtFreeEventTable(event_table)
    XtEventTable *event_table;
{
    register XtEventTable event;

    event = *event_table;
    while (event != NULL) {
	register XtEventTable next = event->next;
	XtFree((char *) event);
	event = next;
    }
}


/*ARGSUSED*/
void _XtAsyncMainLoop(closure)
    caddr_t closure;
{
    XEvent event;

    XtNextEvent(&event);
    XtDispatchEvent(&event);
    XFlush(event.xany.display);
}

void XtMakeToolkitAsync() {
    XtSetAsyncEventHandler(_XtAsyncMainLoop, (caddr_t) NULL);
}


void XtSetAsyncEventHandler(handler, closure)
    XtAsyncHandler handler;
    caddr_t closure;
{
    asyncHandler = handler;
    asyncClosure = closure;
}


extern void _XtRegisterAsyncHandlers(widget)
    Widget widget;
{
    EventMask mask;

    if (asyncHandler == NULL) return;

    mask = XtBuildEventMask(widget);
#ifdef notdef
    XSelectAsyncInput(
	XtDisplay(widget), XtWindow(widget), mask,
	asyncHandler, (unsigned long)XtDisplay(widget));
#endif /*notdef*/
}

/* Stuff for XtSetKeyboardFocus */

/* ARGSUSED */
static void ForwardEvent(widget, client_data, event)
    Widget widget;
    caddr_t client_data;
    XEvent *event;
{
    /* this shouldn't have been necessary, as the keyboard grab will cause
       needed, however, a protocol bug causes us to believe we lost the
       focus during a grab */
    EventMask mask;
    GrabType grabType;

    if (XtIsSensitive(widget)) {
	ConvertTypeToMask(event->xany.type, &mask, &grabType);
	DispatchEvent(event, FindFocusWidget((Widget)client_data), mask);
    }
}

/* ARGSUSED */
static void HandleFocus(widget, client_data, event)
    Widget widget;
    caddr_t client_data;	/* child who wants focus */
    XEvent *event;
{
    Boolean add;
    Widget descendant = (Widget)client_data;

    switch( event->type ) {
      case EnterNotify:
      case LeaveNotify:
		if (!event->xcrossing.focus ||
/* a protocol bug causes us to not receive notification that we've lost
   the focus when the pointer leaves during a grab
			event->xcrossing.mode != NotifyNormal ||
*/
			event->xcrossing.detail == NotifyInferior) return;
		add = (event->type == EnterNotify);
		break;
      case FocusIn:
      case FocusOut:
		if ((event->xfocus.mode != NotifyNormal &&
		     event->xfocus.mode != NotifyWhileGrabbed) ||
		    event->xfocus.detail == NotifyInferior) return;
		add = (event->type == FocusIn);
		break;
    }

    if (add) (void) InsertFocusEntry(widget, descendant);
    else (void)RemoveGrab(widget, True);

    SendFocusNotify(descendant, add ? FocusIn : FocusOut);
}

static void AddForwardingHandler(w, descendant)
    Widget w, descendant;
{
    Window root, child;
    int root_x, root_y, win_x, win_y;
    int left, right, top, bottom;
    unsigned int mask;
    EventMask eventMask;

    /* %%%
       Until we implement a mechanism for propagating keyboard event
       interest to all ancestors for which the descendant may be the
       focus target, the following optimization requires
       XtSetKeyboardFocus calls to be executed from the inside out.
     */
    eventMask = XtBuildEventMask(descendant);
    eventMask &= KeyPressMask | KeyReleaseMask;
    if (eventMask != 0) {
	Boolean have_focus = False;
	register GrabRec *gl;
	AddEventHandler(w, eventMask, False, 
		ForwardEvent, (caddr_t)descendant, FALSE, FALSE);
	/* if we already have the focus, we'll have to change the target */
	for (gl = focusList; gl != NULL; gl = gl->next) {
	    if (gl->widget == w) {
		if (gl->keyboard_focus == descendant)
		    return;	/* simple optimization; list is unchanged */
		else {
		    have_focus = True;
		    break;
		}
	    }
	}
	if (!have_focus) {
	    /* If the widget is the shell parent of the descendent, the server
	       will send the focus notify for us -- we don't need to */
	    if (w != XtParent(descendant) ||
		!XtIsSubclass(w, shellWidgetClass)) {
		/* is the pointer already inside? */
		XQueryPointer(XtDisplay(w), XtWindow(w), &root, &child,
			      &root_x, &root_y, &win_x, &win_y, &mask );
		/* We need to take borders into consideration */
		left = top = -((int) w->core.border_width);
		right = (int) (w->core.width + (w->core.border_width << 1));
		bottom = (int) (w->core.height + (w->core.border_width << 1));
		if (win_x >= left && win_x < right &&
		    win_y >= top && win_y < bottom)
		      have_focus = True;
	    }
	}
	if (have_focus && InsertFocusEntry(w, descendant)) {
	    SendFocusNotify( descendant, FocusIn );
	}
    }
    else {
	/* in case a previous focus widget was registered... */
	RemoveEventHandler(w, XtAllEvents, True,
			   ForwardEvent, NULL, FALSE, FALSE);
    }
}

/* ARGSUSED */
static void QueryEventMask(widget, client_data, event)
    Widget widget;		/* child who gets focus */
    caddr_t client_data;	/* ancestor giving it */
    XEvent *event;
{
    if (event->type == MapNotify) {
	/* make sure ancestor still wants focus set here */
	register XtEventRec* p = ((Widget)client_data)->core.event_table;
	register XtEventHandler proc = HandleFocus; /* compiler bug */
	while (p != NULL && p->proc != proc) p = p->next;
	if (p != NULL && p->closure == (caddr_t)widget)
	    AddForwardingHandler((Widget) client_data, widget);
	RemoveEventHandler(widget, XtAllEvents, True,
			   QueryEventMask, client_data, FALSE, FALSE);
    }
}

void XtSetKeyboardFocus(widget, descendant)
    Widget widget;
    Widget descendant;
{
    EventMask mask;

    if (descendant == (Widget)None) {
        register XtEventRec* p;
        register XtEventHandler proc;
        p = widget->core.event_table;
        proc = HandleFocus;     /* compiler bug */
        while (p != NULL && p->proc != proc) p = p->next;
        if (p != NULL) {
            descendant = (Widget)p->closure;
	    RemoveEventHandler(widget, XtAllEvents, True, HandleFocus, NULL,
			   FALSE, FALSE); /* not raw, don't check closure */
        }
	RemoveEventHandler(widget, XtAllEvents, True, ForwardEvent, NULL,
			   FALSE, FALSE); /* not raw, don't check closure */

        if (RemoveGrab( widget, True ) && descendant != (Widget)None)
            SendFocusNotify( descendant, FocusOut );

	return;
    }

    /* shells are always occluded by their children */
    mask = FocusChangeMask;
    if (widget != XtParent(descendant) || 
	    !XtIsSubclass(widget, shellWidgetClass)) {
	mask |= EnterWindowMask | LeaveWindowMask;
    }

    AddEventHandler(widget, mask, False, HandleFocus, (caddr_t)descendant,
		    FALSE, FALSE);

    /* If his translations aren't installed, we'll have to wait 'till later */

    if (XtIsRealized(descendant)) AddForwardingHandler(widget, descendant);
    else AddEventHandler(descendant, StructureNotifyMask, False,
		QueryEventMask, (caddr_t)widget, FALSE, FALSE);
}

static SendFocusNotify(child, type)
    Widget child;
    int type;
{
    XEvent event;
    EventMask mask;
    GrabType grabType;

    if (XtBuildEventMask(child) & FocusChangeMask) {
	event.xfocus.serial = LastKnownRequestProcessed(XtDisplay(child));
	event.xfocus.send_event = True;
	event.xfocus.display = XtDisplay(child);
	event.xfocus.window = XtWindow(child);
	event.xfocus.type = type;
	event.xfocus.mode = NotifyNormal;
	event.xfocus.detail = NotifyAncestor;
	ConvertTypeToMask(type, &mask, &grabType);
	DispatchEvent(&event, child, mask);
    }
}
