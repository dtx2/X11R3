/* $XConsortium: SelectionI.h,v 1.5 88/09/06 16:29:00 jim Exp $ */
/* $oHeader: SelectionI.h,v 1.3 88/08/19 14:02:44 asente Exp $ */
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

#ifndef _XtselectionI_h
#define _XtselectionI_h

#include "Intrinsic.h"

#define BYTELENGTH(length, format) ((length)*((format)>>3))
#define NUMELEM(bytelength, format) ((bytelength) / ((format)>>3))

/* incremental interface -- lots of these aren't really used. */

typedef void (*XtLoseSelectionIncrProc) (); 
	/* widget, selection, closure) */
	/*	Widget	widget,*/
	/*	Atom	*selection,*/
	/*	Opaque	closure*/	/* selection owner specified */

typedef void (*XtSelectionDoneIncrProc) ();
/* widget, selection, target, receiver_id, closure */
	/*	Widget	widget,*/
	/*	Atom	*selection,*/
	/*	Atom	*target,*/
	/*	Opaque	receiver_id */
	/*	Opaque	closure */

typedef Boolean (*XtConvertSelectionIncrProc)();
 /* widget, selection, target, type, value, length,
    format, max_length, closure, receiver_id */
	/*	Widget	widget,*/
	/*	Atom	*selection,*/
	/*	Atom	*target,*/
	/*	Atom	*type, */
	/*	caddr_t	*value, */
	/*	unsigned long	*length, */
	/*	int	*format */	 
	/*	unsigned long	max_length,*/
	/*	Opaque	closure,*/
	/*	Opaque	receiver_id,*/

typedef void (*XtCancelSelectionCallbackProc)();
/* widget, selection, target, closure */
	/*	Widget	widget,*/
	/*	Atom	*selection,*/
	/*	Opaque	closure*/

typedef void (*XtCancelConvertSelectionProc)();
/* widget, selection, target, closure */
	/*	Widget	widget,*/
	/*	Atom	*selection,*/
	/*	Atom	*target, */
	/*	Opaque	receiver_id*/
	/*	Opaque	closure*/

typedef void (*XtSelectionIncrCallbackProc) ();
/* widget, closure, selection, type, value, length, format */
	/*	Widget	widget,*/
	/*	Opaque	closure, */
	/*	Atom	*selection,*/
	/*	Atom	*type,*/
	/*	caddr_t	value,*/
	/*	unsigned long	*length,*/
	/*	int	*format  */

typedef struct _IncrementalRec *Incremental;

typedef struct _IncrementalRec {
   Window requestor;
   Atom type;
   Atom property;
   int format;
   Incremental next, prev;
   Boolean incr_callback;
   XtConvertSelectionIncrProc convert;
   XtCancelSelectionCallbackProc owner_cancel;
   Opaque owner_closure;
   char *value;
   int bytelength;
   int offset;
} IncrementalRec;


typedef struct {
    Atom selection; 			/* constant */
    Display *dpy; 			/* constant */
    Widget widget;
    Time time;
    int refcount;
    XtConvertSelectionProc convert;
    XtLoseSelectionProc loses;
    XtSelectionDoneProc notify;
    XtCancelSelectionCallbackProc owner_cancel;
    Incremental incrList;
    Boolean incremental;
    Opaque owner_closure;
    Atom incremental_atom; 		/* constant */
    Atom indirect_atom; 		/* constant */
} SelectRec, *Select;

typedef struct {
    XtSelectionCallbackProc callback;
    Opaque *req_closure;
    Atom property;
    Atom *target;
    XtIntervalId timeout;
    XtEventHandler proc;
    Widget widget;
    Time time;
    Select ctx;
    Boolean incremental;
    XtCancelConvertSelectionProc req_cancel;
} CallBackInfoRec, *CallBackInfo;

typedef struct {
  Atom target;
  Atom property;
} IndirectPair;

#define IndirectPairWordSize 2

typedef struct {
   Select ctx;
   XtSelectionDoneProc notify;
   Atom target;
   Atom property;
   Window window;
   Widget widget;
   Boolean allSent;
   XtIntervalId timeout;
} PropGoneRec, *PropGone;

 typedef struct {
  Atom prop;
  Boolean avail;
} SelectionPropRec, *SelectionProp;

typedef struct {
  int propCount;
  SelectionProp list;
} PropListRec, *PropList;

#define MAX_SELECTION_INCR(dpy) (((65536 < dpy->max_request_size) ? \
	(65536 << 2)  : (dpy->max_request_size << 2))-100)

#define MATCH_SELECT(event, info) ((event->time == info->time) && \
	    (event->requestor == XtWindow(info->widget)) && \
	    ((event->target == *info->target) || \
	     ((event->target == info->ctx->incremental_atom) && \
	      (event->property == info->property))))


#endif _XtselectionI_h
/* DON'T ADD STUFF AFTER THIS #endif */
