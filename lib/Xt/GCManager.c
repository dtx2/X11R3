#ifndef lint
static char Xrcsid[] = "$XConsortium: GCManager.c,v 1.31 88/09/06 16:27:54 jim Exp $";
/* $oHeader: GCManager.c,v 1.4 88/08/19 14:19:51 asente Exp $ */
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

#include <stdio.h>
#include "IntrinsicI.h"


typedef struct _GCrec {
    Display	*dpy;		/* Display for GC */
    Screen	*screen;	/* Screen for GC */
    Cardinal	depth;		/* Depth for GC */
    Cardinal    ref_count;      /* # of shareholders */
    GC 		gc;		/* The GC itself. */
    XtValueMask	valueMask;	/* What fields are being used right now. */
    XGCValues 	values;		/* What values those fields have. */
    struct _GCrec *next;	/* Next GC for this widgetkind. */
} GCrec, *GCptr;

static Drawable GCparents[256]; /* static initialized to zero, K&R ss 4.9 */
static GCrec    *GClist = NULL;

static Bool Matches(ptr, valueMask, v)
	     GCptr	    ptr;
    register XtValueMask    valueMask;
    register XGCValues      *v;
{
    register XGCValues      *p = &(ptr->values);

#define CheckGCField(MaskBit,fieldName) \
    if ((valueMask & MaskBit) && (p->fieldName != v->fieldName)) return False

    /* Check most common fields specified for GCs first */
    CheckGCField( GCForeground,		foreground);
    CheckGCField( GCBackground,		background);
    CheckGCField( GCFont,		font);
    CheckGCField( GCFillStyle,		fill_style);
    CheckGCField( GCLineWidth,		line_width);
    /* Are we done yet ? */
    if (! (valueMask
        & ~(GCForeground | GCBackground | GCFont | GCFillStyle | GCLineWidth)))
	return True;

    /* Check next most common */
    CheckGCField( GCFunction,		function);
    CheckGCField( GCGraphicsExposures,	graphics_exposures);
    CheckGCField( GCTile,		tile);
    CheckGCField( GCSubwindowMode,	subwindow_mode);
    CheckGCField( GCPlaneMask,		plane_mask);
    /* Now are we done ? */
    if (! (valueMask
         & ~(GCForeground | GCBackground | GCFont | GCFillStyle | GCLineWidth
	    | GCFunction | GCGraphicsExposures | GCTile | GCSubwindowMode
	    | GCPlaneMask))) return True;

    CheckGCField( GCLineStyle,		line_style);
    CheckGCField( GCCapStyle,		cap_style);
    CheckGCField( GCJoinStyle,		join_style);
    CheckGCField( GCFillRule,		fill_rule);
    CheckGCField( GCArcMode,		arc_mode);
    CheckGCField( GCStipple,		stipple);
    CheckGCField( GCTileStipXOrigin,	ts_x_origin);
    CheckGCField( GCTileStipYOrigin,	ts_y_origin);
    CheckGCField( GCClipXOrigin,	clip_x_origin);
    CheckGCField( GCClipYOrigin,	clip_y_origin);
    CheckGCField( GCClipMask,		clip_mask);
    CheckGCField( GCDashOffset,		dash_offset);
    CheckGCField( GCDashList,		dashes);
#undef CheckGCField
    return True;
} /* Matches */


/* 
 * Return a read-only GC with the given values.  
 */

GC XtGetGC(widget, valueMask, values)
	     Widget	widget;
    register XtGCMask	valueMask;
	     XGCValues	*values;
{
	     GCptr      prev;
    register GCptr      cur;
    register Cardinal   depth   = widget->core.depth;
    register Screen     *screen = XtScreen(widget);
	     Drawable   drawable;
    register Display 	*dpy = XtDisplay(widget);

    /* Search for existing GC that matches exactly */
    for (cur = GClist, prev = NULL; cur != NULL; prev = cur, cur = cur->next) {
	if (cur->valueMask == valueMask && cur->depth == depth
		&& cur->screen == screen && cur->dpy == dpy
		&& Matches(cur, valueMask, values)) {
            cur->ref_count++;
	    /* Move this GC to front of list if not already there */
	    if (prev != NULL) {
		prev->next = cur->next;
		cur->next = GClist;
		GClist = cur;
	    }
	    return cur->gc;
	}
    }

    /* No matches, have to create a new one */
    cur		= XtNew(GCrec);
    cur->next   = GClist;
    GClist      = cur;

    cur->dpy	    = XtDisplay(widget);
    cur->screen     = screen;
    cur->depth      = depth;
    cur->ref_count  = 1;
    cur->valueMask  = valueMask;
    cur->values     = *values;
    if (XtWindow(widget) == NULL) {
	/* Have to create a bogus pixmap for the GC.  Stupid X protocol. */
	if (GCparents[depth] != 0) {
	    drawable = GCparents[depth];
        } else {
	    if (depth == DefaultDepthOfScreen(screen))
		drawable = RootWindowOfScreen(screen);
	    else 
		drawable = XCreatePixmap(cur->dpy, screen->root, 1, 1, depth);
           GCparents[depth] = drawable;
        }
    } else {
	drawable = XtWindow(widget);
    }
    cur->gc = XCreateGC(cur->dpy, drawable, valueMask, values);
    return cur->gc;
} /* XtGetGC */

void  XtReleaseGC(widget, gc)
    Widget   widget;
    GC      gc;
{
    register GCptr cur, prev;
    
    for (cur = GClist, prev = NULL; cur != NULL; prev = cur, cur = cur->next) {
	if (cur->gc == gc && cur->dpy == XtDisplay(widget)) {
	    if (--(cur->ref_count) == 0) {
		if (prev != NULL) prev->next = cur->next;
		else GClist = cur->next;
		XFreeGC(cur->dpy, gc);
		XtFree((char *) cur);
		break;
	    }
	}
    }
} /* XtReleaseGC */

/*  The following interface is broken and supplied only for backwards
 *  compatibility.  It will work properly in all cases only if there
 *  is exactly 1 Display created by the application.
 */

void XtDestroyGC(gc)
    GC      gc;
{
    register GCptr cur, prev;
    
    for (cur = GClist, prev = NULL; cur != NULL; prev = cur, cur = cur->next) {
	if (cur->gc == gc) {
	    if (--(cur->ref_count) == 0) {
		if (prev != NULL) prev->next = cur->next;
		else GClist = cur->next;
		XFreeGC(cur->dpy, gc);
		XtFree((char *) cur);
		break;
	    }
	}
    }
} /* XtDestroyGC */
