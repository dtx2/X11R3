#ifndef lint
static char Xrcsid[] = "$XConsortium: Selection.c,v 1.10 88/09/06 16:28:52 jim Exp $";
/* $oHeader: Selection.c,v 1.8 88/09/01 11:53:42 asente Exp $ */
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
#include "Selection.h"
#include "StringDefs.h"
#include "SelectionI.h"
#include "EventI.h"

extern void bcopy();

#define LOCAL (Opaque) 0x80000000

void _XtSetDefaultSelectionTimeout(timeout)
	unsigned long *timeout;
{
	*timeout = 5000; /* default to 5 seconds */
}

void XtSetSelectionTimeout(timeout)
	unsigned long timeout;
{
	XtAppSetSelectionTimeout(_XtDefaultAppContext(), timeout);
}

void XtAppSetSelectionTimeout(app, timeout)
	XtAppContext app;
	unsigned long timeout;
{
	app->selectionTimeout = timeout;
}

unsigned int XtGetSelectionTimeout()
{
	return XtAppGetSelectionTimeout(_XtDefaultAppContext());
}

unsigned int XtAppGetSelectionTimeout(app)
	XtAppContext app;
{
	return app->selectionTimeout;
}


/* General utilities */

XContext selectPropertyContext = NULL;

static Atom GetSelectionProperty(dpy)
Display *dpy;
{
 SelectionProp p;
 char propname[80];
 int propCount; 
 PropList sarray;

 if (selectPropertyContext == NULL)
   selectPropertyContext = XUniqueContext();
 if (XFindContext(dpy, DefaultRootWindow(dpy), selectPropertyContext,
    (caddr_t *)&sarray)) {
   sarray = (PropList) XtMalloc((unsigned) sizeof(PropListRec));
   sarray->propCount = 1;
   sarray->list = (SelectionProp)XtMalloc((unsigned) sizeof(SelectionPropRec));
   sarray->list[0].prop = XInternAtom(dpy, "_SELECTION_0", FALSE);
   sarray->list[0].avail = FALSE;
   (void) XSaveContext(
	    dpy, DefaultRootWindow(dpy), selectPropertyContext, 
	    (caddr_t) sarray);
   return(sarray->list[0].prop);
 }
 for (p = sarray->list, propCount=sarray->propCount;
	propCount; 
	p++, propCount--) {
   if (p->avail) {
      p->avail = FALSE;
      return(p->prop);
      }
    }
 propCount = sarray->propCount++;
 sarray->list = (SelectionProp) XtRealloc((char *) sarray->list, 
  		(unsigned) ((propCount+1)*sizeof(SelectionPropRec)));
 (void) sprintf(propname, "%s%d", "_SELECTION_", propCount);
 sarray->list[propCount].prop = XInternAtom(dpy, propname, FALSE);
 sarray->list[propCount].avail = FALSE;
 return(sarray->list[propCount].prop);
}

static FreeSelectionProperty(dpy, prop)
Display *dpy;
Atom prop;
{
 SelectionProp p;
 PropList sarray;
 if (prop == None) return;
 if (XFindContext(dpy, DefaultRootWindow(dpy), selectPropertyContext,
    (caddr_t *)&sarray)) 
    XtAppErrorMsg(_XtDisplayToApplicationContext(dpy),
	    "noSelectionProperties", "freeSelectionProperty", "XtToolkitError",
		"internal error: no selection property context for display",
		 (String *)NULL,  (Cardinal *)NULL );
 for (p = sarray->list; p; p++) 
   if (p->prop == prop) {
      p->avail = TRUE;
      return;
      }
}

static CallBackInfo MakeInfo(
	callback, closures, count, widget, time, incremental)
XtSelectionCallbackProc callback;
Opaque *closures;
int count;
Widget widget;
Time time;
Boolean incremental;
{
    	void HandleSelectionReplies();
    	CallBackInfo info;
	info = (CallBackInfo) XtMalloc(
		(unsigned) sizeof(CallBackInfoRec));
	info->callback = callback;
	info->req_closure = (Opaque *)XtMalloc((unsigned) (count * sizeof(Opaque)));
	bcopy((char *) closures, (char *) info->req_closure, count * sizeof(Opaque));
        info->property = GetSelectionProperty(XtDisplay(widget));
	info->proc = HandleSelectionReplies;
	info->widget = widget;
	info->time = time;
	info->incremental = incremental;
	return (info);
}

static RequestSelectionValue(info, ctx, selection, target)
CallBackInfo info;
Select ctx;
Atom selection;
Atom target;
{
    void HandleSelectionReplies();
    void ReqTimedOut();
#ifndef DEBUG
    XtAppContext app = XtWidgetToApplicationContext(info->widget);
	info->timeout = XtAppAddTimeOut(app,
			 app->selectionTimeout, ReqTimedOut, (Opaque) info);
#endif 
	XtAddEventHandler(info->widget, (EventMask) NULL, TRUE,
			  HandleSelectionReplies, (Opaque) info);
	XConvertSelection(ctx->dpy, selection, target, 
			  info->property, XtWindow(info->widget), info->time);
}



static Select FindCtx(dpy, selection)
Display *dpy;
Atom selection;
{
    static XContext selectContext = NULL;
    Select ctx;

    if (selectContext == NULL)
	selectContext = XUniqueContext();
    if (XFindContext(dpy, (Window)selection, selectContext, (caddr_t *)&ctx)) {
	ctx = (Select) XtMalloc((unsigned) sizeof(SelectRec));
	ctx->dpy = dpy;
	ctx->selection = selection;
	ctx->widget = NULL;
	ctx->incrList = NULL;
 	ctx->refcount = 0;
	ctx->incremental_atom = XInternAtom(dpy, "INCR", FALSE);
	ctx->indirect_atom = XInternAtom(dpy, "MULTIPLE", FALSE);
	(void) XSaveContext(
	    dpy, (Window)selection, selectContext, (caddr_t) ctx);
    }
    return ctx;
}


static Incremental FindIncr(ctx, window, prop)
   Select ctx;
   Window window;
   Atom prop;
{
   Incremental incr;
   for (incr = ctx->incrList; incr; incr = incr->next) {
	   if ((window == incr->requestor) && (prop == incr->property)) 
		return(incr);
   }
   return((Incremental)NULL);
}

/*ARGSUSED*/
static void WidgetDestroyed(widget, closure, data)
Widget widget;
Opaque closure, data;
{
    Select ctx = (Select) closure;
    if (ctx->widget == widget)
	ctx->widget = NULL;
}

/* Selection Owner code */

static Boolean LoseSelection(ctx, widget, selection, time)
Select ctx;
Widget widget;
Atom selection;
Time time;
{
    void HandleSelectionEvents();

    if ((ctx->widget == widget)
       && (ctx->selection == selection) /* paranoia */
       && ((time == CurrentTime) || (time >= ctx->time))) {
	    if (--ctx->refcount) {
		XtRemoveEventHandler(widget, (EventMask) NULL, TRUE,
			HandleSelectionEvents, (Opaque) ctx); 
	        XtRemoveCallback(widget, XtNdestroyCallback, 
			WidgetDestroyed, (Opaque) ctx); 
	    };
	    ctx->widget = NULL; /* widget officially loses ownership */
	    /* now inform widget */
	    if (ctx->loses) { 
		if (ctx->incremental)  
		   (*ctx->loses)(widget, &ctx->selection, ctx->owner_closure);
		else  (*ctx->loses)(widget, &ctx->selection);
	    }
	    return(TRUE);
	}
   else return(FALSE);
}

static void AddHandler(dpy, window, widget, mask, proc, closure)
Display *dpy;
Window window;
Widget widget;
EventMask mask;
XtEventHandler proc;
Opaque closure;
{
    /* see if there is already a widget associated with the window */
    if (XtWindowToWidget(dpy, window)==NULL) {
	_XtRegisterWindow(window, widget);
	XtAddRawEventHandler(widget, mask, FALSE, proc, closure);
	XSelectInput(dpy, window, mask);
    } else if (XtWindow(widget) != window) 
        /* raw handler already added */
	return; 
    else {
      XtAddEventHandler(widget, mask, TRUE, proc, closure);
    }
}

static void RemoveHandler(dpy, window, widget, mask, proc, closure)
Display *dpy;
Window window;
Widget widget;
EventMask mask;
XtEventHandler proc;
Opaque closure;
{
    if ((XtWindowToWidget(dpy, window) == widget) && 
        (XtWindow(widget) != window)) {
    /* we had to hang this window onto our widget; take it off */
      XtRemoveRawEventHandler(widget, mask, TRUE, proc, closure);
      _XtUnregisterWindow(window, widget);
      XSelectInput(dpy, window, 0L);
    } else {
        XtRemoveEventHandler(widget, mask, TRUE,  proc, closure); 
    }
}

/* ARGSUSED */
static void OwnerTimedOut(closure, id)
Opaque closure;
XtIntervalId   *id;
{
    PropGone rec = (PropGone)closure;
    Select ctx = rec->ctx;
    Incremental incr;
    void HandlePropertyGone();

    incr = FindIncr(ctx, rec->window, rec->property);
    if (incr != NULL) {
	if ((incr->incr_callback) && (incr->owner_cancel != NULL)) 
		(*incr->owner_cancel)(rec->widget, &ctx->selection, 
		&rec->target, (Opaque)((int)rec->property | (int)rec->window),
		incr->owner_closure);
	if (incr->prev==NULL) ctx->incrList = ctx->incrList->next;
	else incr->prev->next = incr->next;
	if (incr->next != NULL) incr->next->prev = incr->prev;
	if (ctx->notify == NULL) XtFree((char *)incr->value);
	XtFree((char *)incr); 
     }
    /* should we really call the owner's notify proc? the requestor
       hasn't received it, but we are done with it; owner needs
       the chance to free it */
    if (rec->notify) 
	if (ctx->incremental)
	      (*rec->notify)(rec->widget, &ctx->selection, &rec->target, 
				rec->property, ctx->owner_closure);
	else
	      (*rec->notify)(rec->widget, &ctx->selection, &rec->target);
     RemoveHandler(ctx->dpy, rec->window, rec->widget,
	  	(EventMask) PropertyChangeMask, HandlePropertyGone, closure); 
     XtFree((char *)rec);
}

static void SendIncrement(ctx, incr)
    Select ctx;
    Incremental incr;
{
    int incrSize = MAX_SELECTION_INCR(ctx->dpy);
    if (incrSize >  incr->bytelength - incr->offset)
        incrSize = incr->bytelength - incr->offset;
    XChangeProperty(ctx->dpy, incr->requestor, incr->property, 
    	    incr->type, incr->format, PropModeReplace, 
	    (unsigned char *)&incr->value[incr->offset], 
	    NUMELEM(incrSize, incr->format));
    incr->offset += incrSize;
}

static AllSent(ctx, incr, rec)
Select ctx;
Incremental incr;
PropGone rec;

{
   		  XChangeProperty(ctx->dpy, incr->requestor, 
		  	incr->property, incr->type,  incr->format, 
			PropModeReplace, (unsigned char *) NULL, 0);
		  rec->allSent = TRUE;
 		  /* remove from incrList */
	          if (incr->prev==NULL) ctx->incrList = ctx->incrList->next;
	  	  else incr->prev->next = incr->next;
		  if (incr->next != NULL) incr->next->prev = incr->prev;
		  if (ctx->notify == NULL) XtFree((char *)incr->value);
		  XtFree((char *)incr); 
}

static void HandlePropertyGone(widget, closure, ev)
Widget widget;
Opaque closure;
XEvent *ev;
{


    XPropertyEvent *event = (XPropertyEvent *) ev;
    PropGone rec = (PropGone) closure;
    Select ctx = rec->ctx;
    Incremental incr;

    if ((event->type != PropertyNotify) ||
        (event->state != PropertyDelete) ||
	(rec->property != event->atom)) return;
#ifndef DEBUG
    XtRemoveTimeOut(rec->timeout);
#endif 
    if (rec->allSent) { 
          if (rec->notify)  
		if (ctx->incremental)
	      		(*rec->notify)(widget, &ctx->selection, 
			    &rec->target, rec->property, ctx->owner_closure);
		else (*rec->notify)(widget, &ctx->selection, &rec->target);
	  RemoveHandler(event->display, event->window, widget,
	  	(EventMask) PropertyChangeMask, HandlePropertyGone, closure); 
          XtFree((char *)rec);
    } else  { /* is this part of an incremental transfer? */ 
           incr = FindIncr(ctx, event->window, event->atom);
   	   if (incr == NULL) return;
	   if (incr->incr_callback) {
	     if (incr->bytelength == 0)
		AllSent(ctx, incr, rec);
	     else {
    		SendIncrement(ctx, incr);
		(*incr->convert)(ctx->widget, &ctx->selection, &rec->target, 
			    &incr->type, &incr->value, 
			    &incr->bytelength, &incr->format,
			    MAX_SELECTION_INCR(ctx->dpy), incr->owner_closure,
			    (Opaque)((int)rec->property | (int)rec->window));
		incr->bytelength = BYTELENGTH(incr->bytelength,incr->format);
		incr->offset = 0;
	    }
	   } else {
	       if (incr->offset < incr->bytelength) 
	          SendIncrement(ctx, incr);
	       else AllSent(ctx, incr, rec);
	  }
#ifndef DEBUG
	  {
	  XtAppContext app = XtWidgetToApplicationContext(rec->widget);
    	   rec->timeout = XtAppAddTimeOut(app,
			 app->selectionTimeout, OwnerTimedOut, (Opaque) rec);
	  }
#endif 
     }
}

static PrepareIncremental(ctx, widget, window, property, target, 
	 targetType, value, length, format)
Select ctx;
Widget widget;
Window window;
Atom target;
Atom property;
Atom targetType;
caddr_t value;
int length, format;
{
    Incremental incr;
    PropGone rec;
    int size;

	incr = (Incremental) XtMalloc((unsigned) sizeof(IncrementalRec));
	incr->requestor = window;
	incr->type = targetType;
	incr->property = property;
	incr->value = value;
	incr->incr_callback = ctx->incremental;
	incr->convert = ctx->convert;
	incr->owner_cancel = ctx->owner_cancel;
	incr->owner_closure = ctx->owner_closure;
	incr->bytelength = BYTELENGTH(length,format);
	incr->format = format;
	incr->offset = 0;
	incr->prev = NULL;
	incr->next = ctx->incrList;
	ctx->incrList = incr;
 	rec = (PropGone) XtMalloc((unsigned) sizeof(PropGoneRec));
	rec->ctx = ctx;
	rec->notify = ctx->notify;
	rec->target = target;
	rec->property = property;
	rec->widget = widget;
	rec->window = window;
	rec->allSent = FALSE;
#ifndef DEBUG
	{
	XtAppContext app = XtWidgetToApplicationContext(rec->widget);
	rec->timeout = XtAppAddTimeOut(app,
			 app->selectionTimeout, OwnerTimedOut, (Opaque) rec);
	}
#endif 
	AddHandler(ctx->dpy, window, widget, 
			(EventMask) PropertyChangeMask, 
	       		HandlePropertyGone, (Opaque) rec);
/* now send client INCREMENT property */
	size = BYTELENGTH(length,format);
	XChangeProperty(ctx->dpy, window,
 			    incr->property, ctx->incremental_atom,
			    8, PropModeReplace, (unsigned char *)&size, 
			    	sizeof(size));
}

static Boolean GetConversion(ctx, selection, target, property, widget, window,
			     incremental)
Select ctx;
Atom selection;
Atom target;
Atom property;
Widget widget;
Window window;
Boolean *incremental;
{
    caddr_t value;
    int length, format;
    Atom targetType;
    PropGone rec;

    if (ctx->incremental == TRUE) {
         if ((*ctx->convert)(ctx->widget, &selection, &target,
			    &targetType, &value, &length, &format,
			    MAX_SELECTION_INCR(ctx->dpy), ctx->owner_closure,
			    (Opaque)((int)property | (int)window)) == FALSE)
		return(FALSE);
	 PrepareIncremental(ctx, widget, window,  property, target,
			    targetType, value, length, format);
	 *incremental = True;
	 return(TRUE);
    }
    if ((*ctx->convert)(ctx->widget, &selection, &target,
			    &targetType, &value, &length, &format) == FALSE)
	return(FALSE);
    if (BYTELENGTH(length,format) <= MAX_SELECTION_INCR(ctx->dpy)) {
	if (ctx->notify != NULL) {
		  rec = (PropGone) XtMalloc((unsigned) sizeof(PropGoneRec));
		  rec->ctx = ctx;
		  rec->notify = ctx->notify;
		  rec->target = target;
		  rec->property = property;
		  rec->widget = ctx->widget;
		  rec->window = window;
		  rec->allSent = TRUE;
#ifndef DEBUG
		  {
		  XtAppContext app = XtWidgetToApplicationContext(rec->widget);
		  rec->timeout = XtAppAddTimeOut(app,
			 app->selectionTimeout, OwnerTimedOut, (Opaque) rec);
		  }
#endif 
	          AddHandler(ctx->dpy, window,
		        ctx->widget, (EventMask) PropertyChangeMask, 
	       		HandlePropertyGone, (Opaque) rec);
        }
	XChangeProperty(ctx->dpy, window, property, 
			    targetType, format, PropModeReplace,
			    (unsigned char *)value, length);
	/* free storage for client if no notify proc */
	if (ctx->notify == NULL) XtFree((char *)value);
	*incremental = FALSE;
    } else {
	 PrepareIncremental(ctx, widget, window, property, target,
			    targetType, value, length, format);
	 *incremental = True;
    }
    return(TRUE);
}

/*ARGSUSED*/
static void HandleSelectionEvents(widget, closure, event)
Widget widget;
Opaque closure;
XEvent *event;
{
    Select ctx;
    XSelectionEvent ev;
    Boolean incremental;
    unsigned long bytesafter, length;
    IndirectPair *p;
    char *value;
    int format;
    Atom target;
    int count;
    Boolean writeback = FALSE;

    switch (event->type) {
      case SelectionClear:
	ctx = FindCtx(event->xselectionclear.display,
		      event->xselectionclear.selection);
	(void) LoseSelection(ctx, widget, event->xselectionclear.selection,
			event->xselectionclear.time);
	break;
      case SelectionRequest:
	ctx = FindCtx(event->xselectionrequest.display,
		      event->xselectionrequest.selection);
	ev.type = SelectionNotify;
	ev.display = event->xselectionrequest.display;
	ev.requestor = event->xselectionrequest.requestor;
	ev.selection = event->xselectionrequest.selection;
	ev.time = event->xselectionrequest.time;
	ev.target = event->xselectionrequest.target;
	if (event->xselectionrequest.property == None) /* obsolete requestor */
	   event->xselectionrequest.property = event->xselectionrequest.target;
	if (!(ctx->widget)
	   || ((event->xselectionrequest.time != CurrentTime)
	        && (event->xselectionrequest.time < ctx->time)))
	    ev.property = None;
         else {
	   if (ev.target == ctx->indirect_atom) {
	        ev.property = event->xselectionrequest.property;
		(void) XGetWindowProperty(ev.display, ev.requestor,
			event->xselectionrequest.property, 0L, 1000000,
			False, AnyPropertyType, &target, &format, &length,
			&bytesafter, (unsigned char **)&value);
		count = BYTELENGTH(length, format) / sizeof(IndirectPair);
		for (p = (IndirectPair *)value; count; p++, count--) {
		  if (GetConversion(ctx, ev.selection, p->target,
			p->property, widget,
			event->xselectionrequest.requestor, &incremental)) {
			if (incremental) {
				p->target = ctx->incremental_atom;
				writeback = TRUE;
			}
		  } else {
			p->property = None;
			writeback = TRUE;
		  }
		}
	      if (writeback) 
		XChangeProperty(ev.display, ev.requestor, 
			event->xselectionrequest.property, target,
			format, PropModeReplace,
			(unsigned char *) value, (int) length);
	   } else {
	       if (GetConversion(ctx, ev.selection,
			    event->xselectionrequest.target,
			    event->xselectionrequest.property,
			    widget, event->xselectionrequest.requestor,
			    &incremental)) {
	        if (incremental) ev.target = ctx->incremental_atom;
	        ev.property = event->xselectionrequest.property;
              } else
	        ev.property = None;
	   }
      }
      (void) XSendEvent(ctx->dpy, ev.requestor, False, (unsigned long)NULL,
		   (XEvent *) &ev);
      break;
    }
}



Boolean XtOwnSelection(widget, selection, time, newconvert, newloses, newnotify)
Widget widget;
Atom selection;
Time time;
XtConvertSelectionProc newconvert;
XtLoseSelectionProc newloses;
XtSelectionDoneProc newnotify;
{
    Select ctx;
    Window window;
    ctx = FindCtx(XtDisplay(widget), selection);
    if (ctx->widget != widget) {
        XSetSelectionOwner(ctx->dpy, selection, window = XtWindow(widget),
		 time);
        if (XGetSelectionOwner(ctx->dpy, selection) != window)
		return(FALSE);
	if (ctx->widget)
	    (void) LoseSelection(ctx, ctx->widget, selection, ctx->time);
    }
    ctx->widget = widget;
    if (time != CurrentTime) ctx->time = time;
    ctx->convert = newconvert;
    ctx->loses = newloses;
    ctx->notify = newnotify;
    ctx->incremental = FALSE;
    ctx->refcount += 1;
    XtAddEventHandler(widget, (EventMask)NULL, TRUE,
		      HandleSelectionEvents, (Opaque) ctx);
    XtAddCallback(widget, XtNdestroyCallback, 
	WidgetDestroyed, (Opaque) ctx);
   return(TRUE);
}

void XtDisownSelection(widget, selection, time)
Widget widget;
Atom selection;
Time time;
{
    Select ctx;
    ctx = FindCtx(XtDisplay(widget), selection);
    if (LoseSelection(ctx, widget, selection, time))
	XSetSelectionOwner(XtDisplay(widget), selection, None, time);
}

/* Selection Requestor code */

static void ReqCleanup(widget, closure, ev)
Widget widget;
Opaque closure;
XEvent *ev;
{
    CallBackInfo info = (CallBackInfo)closure;
    unsigned long bytesafter, length;
    char *value;
    int format;
    Atom target;
    void HandleGetIncrement();

    if ((ev->type & 0x7f) == SelectionNotify) {
	XSelectionEvent *event = (XSelectionEvent *) ev;
	if (!MATCH_SELECT(event, info)) return; /* not really for us */
         XtRemoveEventHandler(widget, (EventMask) NULL, TRUE,
			   ReqCleanup, (Opaque) info );
	if (event->target != info->ctx->incremental_atom) {
	   if (event->property != None) 
		XDeleteProperty(event->display, XtWindow(widget),
				event->property);
           FreeSelectionProperty(XtDisplay(widget), info->property);
	   XtFree((caddr_t)info->req_closure);
	   XtFree((caddr_t)info->target);
           XtFree((caddr_t) info);
	} else {
	   info->proc = HandleGetIncrement;
	   XtAddEventHandler(info->widget, (EventMask) PropertyChangeMask, 
		FALSE, ReqCleanup, (Opaque) info);
	}
    } else if (((ev->type & 0x7f) == PropertyNotify) && 
		(ev->xproperty.state == PropertyNewValue) &&
	        (ev->xproperty.atom == info->property)) {
	XPropertyEvent *event = (XPropertyEvent *) ev;
        (void) XGetWindowProperty(event->display, XtWindow(widget), 
			   event->atom, 0L, 1000000, True, AnyPropertyType,
			   &target, &format, &length, &bytesafter, 
			   (unsigned char **) &value);
	XtFree(value);
	if (length == 0) {
           XtRemoveEventHandler(widget, (EventMask) PropertyChangeMask, FALSE,
			   ReqCleanup, (Opaque) info );
           FreeSelectionProperty(XtDisplay(widget), info->property);
	   XtFree((caddr_t)info->req_closure);
	   XtFree((caddr_t)info->target);
           XtFree((caddr_t) info);
	}
    }
}

/* ARGSUSED */
static void ReqTimedOut(closure, id)
Opaque closure;
XtIntervalId   *id;
{
    caddr_t value = NULL;
    int length = 0;
    int format = 8;
    Atom resulttype = XT_CONVERT_FAIL;
    CallBackInfo info = (CallBackInfo)closure;
    Incremental incr;
    unsigned long bytesafter;
    unsigned long proplength;
    Atom type;
    IndirectPair *pairs;
    Opaque *c;
    void HandleSelectionReplies();
    /* call requestor with XT_CONVERT_FAIL */
    if ((info->incremental) && (info->req_cancel != NULL))
	(*info->req_cancel)(info->widget, &info->ctx->selection, 
		*info->req_closure);
    if (*info->target == info->ctx->indirect_atom) {
        (void) XGetWindowProperty(XtDisplay(info->widget), 
			   XtWindow(info->widget), info->property, 0L,
			   10000000, True, AnyPropertyType, &type, &format,
			   &proplength, &bytesafter, (unsigned char **) &pairs);
       XtFree((caddr_t)pairs);
       for (proplength = proplength / IndirectPairWordSize, c = info->req_closure;
	           proplength; proplength--, c++) 
	    (*info->callback)(info->widget, *c, 
	    	&info->ctx->selection, &resulttype, value, &length, &format);
    } else {
	(*info->callback)(info->widget, *info->req_closure, 
	    &info->ctx->selection, &resulttype, value, &length, &format);
    }
    incr = FindIncr(info->ctx, XtWindow(info->widget), info->property);
    if (incr) {
	  /* remove from incrList */
          if (incr->prev==NULL) 
		info->ctx->incrList = info->ctx->incrList->next;
  	  else incr->prev->next = incr->next;
	  if (incr->next != NULL) incr->next->prev = incr->prev;
	  if (info->ctx->notify == NULL) XtFree((char *)incr->value);
	  XtFree((char *)incr);
    } 
    /* change event handlers for straggler events */
    if (info->proc == (XtEventHandler)HandleSelectionReplies) {
        XtRemoveEventHandler(info->widget, (EventMask) NULL, 
			TRUE, info->proc, (Opaque) info);
	XtAddEventHandler(info->widget, (EventMask) NULL, TRUE,
		ReqCleanup, (Opaque) info);
    } else {
        XtRemoveEventHandler(info->widget,(EventMask) PropertyChangeMask, 
			FALSE, info->proc, (Opaque) info);
	XtAddEventHandler(info->widget, (EventMask) PropertyChangeMask, 
		FALSE, ReqCleanup, (Opaque) info);
    }

}

static void HandleGetIncrement(widget, closure, ev)
Widget widget;
Opaque closure;
XEvent *ev;
{
    XPropertyEvent *event = (XPropertyEvent *) ev;
    CallBackInfo info = (CallBackInfo) closure;
    Select ctx = info->ctx; 
    char *value;
    unsigned long bytesafter;
    unsigned long length;
    Incremental incr;
    int bad;
    void HandleSelectionReplies();

    if ((event->state != PropertyNewValue) || (event->atom != info->property))
	 return;
    incr = FindIncr(ctx, event->window, event->atom);
    if (incr == NULL) return; 
    bad = XGetWindowProperty(event->display, incr->requestor, event->atom, 0L,
			   10000000, True, AnyPropertyType, &incr->type, 
			   &incr->format, &length, &bytesafter, 
			   (unsigned char **) &value);
    if (bad) 
      return;
#ifndef DEBUG
    XtRemoveTimeOut(info->timeout); 
#endif 
    if (length == 0) {
       (*info->callback)(widget, *info->req_closure, &ctx->selection, 
			  &incr->type, 
			  (incr->offset == 0 ? NULL : incr->value), 
			  &incr->offset, &incr->format);
       XtRemoveEventHandler(widget, (EventMask) PropertyChangeMask, FALSE, 
		HandleGetIncrement, (Opaque) info);
       FreeSelectionProperty(event->display, info->property);
       XtFree((caddr_t)info->req_closure);
       XtFree((caddr_t)info->target);
       XtFree((caddr_t) info);
    } else { /* add increment to collection */
      if (info->incremental) {
        (*info->callback)(widget, *info->req_closure, &ctx->selection, 
			  &incr->type, value, &length, &incr->format);
      } else {
          if ((BYTELENGTH(length,incr->format)+incr->offset) 
			> incr->bytelength) {
  	    incr->value = (char *)XtRealloc((char *) incr->value, 
					 (unsigned) (incr->bytelength *= 2));
          }
          bcopy(value, &incr->value[incr->offset], 
		(int) BYTELENGTH(length, incr->format));
          incr->offset += BYTELENGTH(length, incr->format);
         XtFree(value);
     }
     /* reset timer */
#ifndef DEBUG
     {
     XtAppContext app = XtWidgetToApplicationContext(info->widget);
     info->timeout = XtAppAddTimeOut(app,
			 app->selectionTimeout, ReqTimedOut, (Opaque) info);
     }
#endif 
   }
}
static HandleNone(widget, callback, closure, selection)
Widget widget;
XtSelectionCallbackProc callback;
Opaque closure;
Atom selection;
{
    unsigned long length = 0;
    int format = 8;
    Atom type = None;

    (*callback)(widget, closure, &selection, 
		&type, NULL, &length, &format);
}

static HandleNormal(dpy, widget, property, callback, closure, selection)
Display *dpy;
Widget widget;
Atom property;
XtSelectionCallbackProc callback;
Opaque closure;
Atom selection;
{
    unsigned long bytesafter;
    unsigned long length;
    int format;
    Atom type;
    char *value;

    (void) XGetWindowProperty(dpy, XtWindow(widget), property, 0L,
			   10000000, True, AnyPropertyType, &type, &format,
			   &length, &bytesafter, (unsigned char **) &value);
     (*callback)(widget, closure, &selection, 
			  &type, (length == 0 ? NULL : value), 
			  &length, &format);
}

static HandleIncremental(dpy, widget, property, info, ctx)
Display *dpy;
Widget widget;
Atom property;
CallBackInfo info;
Select ctx;
{
    unsigned long bytesafter;
    unsigned long length;
    int format;
    Atom type;
    int *size;
    Incremental incr;

    XtAddEventHandler(widget, (EventMask) PropertyChangeMask, FALSE,
	  	HandleGetIncrement, (Opaque) info);
    (void) XGetWindowProperty(dpy, XtWindow(widget), property, 0L,
			   10000000, True, AnyPropertyType, &type, &format,
			   &length, &bytesafter, (unsigned char **) &size);
    incr = (Incremental) XtMalloc((unsigned) sizeof(IncrementalRec));
    incr->requestor = XtWindow(widget);
    incr->type = None;
    incr->property = property;
    incr->incr_callback = FALSE;
    incr->bytelength = *size;
    incr->value = (char *) XtMalloc((unsigned) incr->bytelength);
    incr->format = 0;
    incr->offset = 0;
    incr->prev = NULL;
    incr->next = ctx->incrList;
    if (ctx->incrList != NULL)
		ctx->incrList->prev = incr;
    ctx->incrList = incr;
    /* reset the timer */
     info->proc = HandleGetIncrement;
#ifndef DEBUG
    {
    XtAppContext app = XtWidgetToApplicationContext(info->widget);
    info->timeout = XtAppAddTimeOut(app,
			 app->selectionTimeout, ReqTimedOut, (Opaque) info);
    }
#endif 
}

static void HandleSelectionReplies(widget, closure, ev)
Widget widget;
Opaque closure;
XEvent *ev;
{
    XSelectionEvent *event = (XSelectionEvent *) ev;
    Display *dpy = event->display;
    CallBackInfo info = (CallBackInfo) closure;
    Select ctx;
    IndirectPair *pairs, *p;
    unsigned long bytesafter;
    unsigned long length;
    int format;
    Atom type;
    Opaque *c;
    Atom *t;
    CallBackInfo newinfo;

    if ((event->type & 0x7f) != SelectionNotify) return;
    if (!MATCH_SELECT(event, info)) return; /* not really for us */
#ifndef DEBUG
    XtRemoveTimeOut(info->timeout); 
#endif 
    XtRemoveEventHandler(widget, (EventMask) NULL, TRUE,
		HandleSelectionReplies, (Opaque) info );
    ctx = FindCtx(dpy, event->selection);
    if (event->target == ctx->indirect_atom) {
        (void) XGetWindowProperty(dpy, XtWindow(widget), info->property, 0L,
			   10000000, True, AnyPropertyType, &type, &format,
			   &length, &bytesafter, (unsigned char **) &pairs);
       for (length = length / IndirectPairWordSize, p = pairs, 
		   c = info->req_closure, t = info->target+1;
	           length; length--, p++, c++, t++) {
	    if ((event->property == None) || (format != 32) || 
		 (p->property == None)) {
		HandleNone(widget, info->callback, *c, event->selection);
		if (p->property != None)
                    FreeSelectionProperty(XtDisplay(widget), p->property);
	    } else if (p->target != ctx->incremental_atom) {
		HandleNormal(dpy, widget, p->property, info->callback, *c, 
				event->selection);
                FreeSelectionProperty(XtDisplay(widget), p->property);
	    } else {
		newinfo = (CallBackInfo) XtMalloc(
			(unsigned) sizeof(CallBackInfoRec));
		newinfo->callback = info->callback;
		newinfo->req_closure = (Opaque *)XtMalloc(sizeof(Opaque));
		*newinfo->req_closure = *c;
		newinfo->property = p->property;
		newinfo->widget = info->widget;
		newinfo->time = info->time;
		newinfo->target = (Atom *)XtMalloc(sizeof(Atom));
		*newinfo->target = *t;
		newinfo->ctx = info->ctx;
		HandleIncremental(dpy, widget, p->property, newinfo, ctx);
	    }
       }
       XtFree((caddr_t)pairs);
       FreeSelectionProperty(dpy, info->property);
       XtFree((caddr_t)info->req_closure); 
       XtFree((caddr_t)info->target); 
       XtFree((caddr_t) info);
    } else if (event->property == None) {
	HandleNone(widget, info->callback, *info->req_closure, event->selection);
        FreeSelectionProperty(XtDisplay(widget), info->property);
        XtFree((caddr_t)info->req_closure);
        XtFree((caddr_t)info->target); 
        XtFree((caddr_t) info);
    } else if (event->target != ctx->incremental_atom) {
	HandleNormal(dpy, widget, event->property, info->callback, 
			*info->req_closure, event->selection);
        FreeSelectionProperty(XtDisplay(widget), info->property);
        XtFree((caddr_t)info->req_closure);
        XtFree((caddr_t)info->target); 
        XtFree((caddr_t) info);
    } else {
	HandleIncremental(dpy, widget, event->property, info, ctx); 
    }
}

static DoLocalTransfer(ctx, selection, target, widget,
		       callback, closure, incremental)
Select ctx;
Atom selection;
Atom target;
Widget widget;			/* The widget requesting the value. */
XtSelectionCallbackProc callback;
Opaque closure;			/* the closure for the callback, not the conversion */
Boolean incremental;
{
    caddr_t value, temp, total = NULL;
    int length, format;
    Atom resulttype;
    int totallength = 0;
    Boolean notFirst = FALSE;
    

	if (ctx->incremental) {
	   if (!(*ctx->convert)(ctx->widget, &selection, &target,
			    &resulttype, &value, &length, &format,
			    MAX_SELECTION_INCR(ctx->dpy), ctx->owner_closure,
			    (Opaque)LOCAL)) {
	    HandleNone(widget, callback, closure, selection);
	  } else {
		if (incremental) {
	          while (length || notFirst) {
		      notFirst = TRUE;
	    	      if (ctx->notify && (value != NULL)) {
              	        int bytelength = BYTELENGTH(length,format);
	                /* both sides think they own this storage */
	                temp = XtMalloc((unsigned)bytelength);
	                bcopy(value, temp, bytelength);
	                value = temp;
	              }
		     (*callback)(widget, closure, &selection, 
			&resulttype, value, &length, &format);
		     (*ctx->convert)(ctx->widget, &selection, &target,
			    &resulttype, &value, &length, &format,
			    MAX_SELECTION_INCR(ctx->dpy), ctx->owner_closure,
			    (Opaque)LOCAL);
		  }
	        } else {
	          while (length) {
		    int bytelength = BYTELENGTH(length, format);
		    total = XtRealloc(total, 
			    (unsigned) (totallength += bytelength));
		    bcopy(value, &total[totallength-bytelength], bytelength);
		    if (ctx->notify == NULL) XtFree(value);
		    (*ctx->convert)(ctx->widget, &selection, &target, 
			    &resulttype, &value, &length, &format,
			    MAX_SELECTION_INCR(ctx->dpy), ctx->owner_closure,
			    (Opaque)LOCAL);
		  }
		  totallength = NUMELEM(totallength, format); 
		  (*callback)(widget, closure, &selection, &resulttype, 
		    total,  &totallength, &format);
		}
	        if (ctx->notify) 
		   (*ctx->notify)(ctx->widget, &selection, &target, 
				  (Opaque)LOCAL, ctx->owner_closure);
         }
	} else { /* not incremental owner */
	  if (!(*ctx->convert)(ctx->widget, &selection, &target, 
			     &resulttype, &value, &length, &format)) {
	    HandleNone(widget, callback, closure, selection);
	  } else {
	      if (ctx->notify && (value != NULL)) {
                int bytelength = BYTELENGTH(length,format);
	        /* both sides think they own this storage; better copy */
	        temp = XtMalloc((unsigned)bytelength);
	        bcopy(value, temp, bytelength);
	        value = temp;
	      }
	      (*callback)(widget, closure, &selection, &resulttype, 
		    (length == 0 ? NULL : value), 
		    &length, &format);
	      if (ctx->notify)
	         (*ctx->notify)(ctx->widget, &selection, &target);
	  }
      }
}

void XtGetSelectionValue(widget, selection, target, callback, closure, time)
Widget widget;
Atom selection;
Atom target;
XtSelectionCallbackProc callback;
Opaque closure;
Time time;
{
    Select ctx;
    CallBackInfo info;

    ctx = FindCtx(XtDisplay(widget), selection);
    if (ctx->widget) 
	DoLocalTransfer(ctx, selection, target, widget,
			callback, closure, FALSE);
    else {
	info = MakeInfo(callback, &closure, 1, widget, time, FALSE);
	info->target = (Atom *)XtMalloc((unsigned) sizeof(Atom));
	 *(info->target) = target;
	info->ctx = ctx;
	RequestSelectionValue(info, ctx, selection, target);
    }
}

void XtGetSelectionValues(widget, selection, targets, count, callback, 
	closures, time)
Widget widget;
Atom selection;
Atom *targets;
int count;
XtSelectionCallbackProc callback;
Opaque *closures;
Time time;
{
    Select ctx;
    CallBackInfo info;
    IndirectPair *pairs, *p;
    Atom *t;

    if (count == 0) return;
    ctx = FindCtx(XtDisplay(widget), selection);
    if (ctx->widget) {
       for (; count; count--, targets++, closures++ ) 
	 DoLocalTransfer(ctx, selection, *targets, widget,
			 callback, *closures, FALSE);
    } else {
	info = MakeInfo(callback, closures, count, widget, time, FALSE);
	info->target = (Atom *)XtMalloc((unsigned) ((count+1) * sizeof(Atom)));
        (*info->target) = ctx->indirect_atom;
	bcopy((char *) targets, (char *) info->target+sizeof(Atom), count * sizeof(Atom));
	info->ctx = ctx;
	pairs = (IndirectPair *) XtMalloc((unsigned) (count*sizeof(IndirectPair)));
	for (p = &pairs[count-1], t = &targets[count-1];
	     p >= pairs;  p--, t-- ) {
	   p->target = *t;
	   p->property = GetSelectionProperty(XtDisplay(widget));
	}
	XChangeProperty(XtDisplay(widget), XtWindow(widget), 
			info->property, info->property,
			32, PropModeReplace, (unsigned char *) pairs, 
			count * IndirectPairWordSize);
	XtFree((char *) pairs);
	RequestSelectionValue(info, ctx, selection, ctx->indirect_atom);
    }
}
