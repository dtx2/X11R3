/* $XConsortium: Composite.h,v 1.8 88/09/06 16:27:11 jim Exp $ */
/* $oHeader: Composite.h,v 1.2 asente Exp $ */
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

#ifndef _XtComposite_h
#define _XtComposite_h

typedef struct _CompositeClassRec *CompositeWidgetClass;

extern void XtManageChildren ();
    /* WidgetList children; */
    /* Cardinal   num_children; */

extern void XtManageChild ();
    /* Widget    child; */

extern void XtUnmanageChildren ();
    /* WidgetList children; */
    /* Cardinal   num_children; */

extern void XtUnmanageChild ();
    /* Widget child; */


#ifndef COMPOSITE
externalref WidgetClass compositeWidgetClass;
#endif

#endif _XtComposite_h
/* DON'T ADD STUFF AFTER THIS #endif */
