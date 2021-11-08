#ifndef lint
static char Xrcsid[] = "$XConsortium: Viewport.c,v 1.28 88/10/14 15:31:37 swick Exp $";
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

#include <X11/IntrinsicP.h>
#include <X11/XawMisc.h>
#include <X11/Scroll.h>
#include <X11/StringDefs.h>
#include <X11/ViewportP.h>


static void ScrollUpDownProc(), ThumbProc();

static Boolean defFalse = False;

#define offset(field) XtOffset(ViewportWidget,viewport.field)
static XtResource resources[] = {
    {XtNforceBars, XtCBoolean, XtRBoolean, sizeof(Boolean),
	 offset(forcebars), XtRBoolean, (caddr_t)&defFalse },
    {XtNallowHoriz, XtCBoolean, XtRBoolean, sizeof(Boolean),
	 offset(allowhoriz), XtRBoolean, (caddr_t)&defFalse },
    {XtNallowVert, XtCBoolean, XtRBoolean, sizeof(Boolean),
	 offset(allowvert), XtRBoolean, (caddr_t)&defFalse },
    {XtNuseBottom, XtCBoolean, XtRBoolean, sizeof(Boolean),
	 offset(usebottom), XtRBoolean, (caddr_t)&defFalse },
    {XtNuseRight, XtCBoolean, XtRBoolean, sizeof(Boolean),
	 offset(useright), XtRBoolean, (caddr_t)&defFalse },
};
#undef offset

static void Initialize(), ConstraintInitialize(),
    Realize(), Resize(), ChangeManaged();
static Boolean SetValues(), DoLayout();
static XtGeometryResult GeometryManager(), PreferredGeometry();

#define superclass	(&formClassRec)
ViewportClassRec viewportClassRec = {
  { /* core_class fields */
    /* superclass	  */	(WidgetClass) superclass,
    /* class_name	  */	"Viewport",
    /* widget_size	  */	sizeof(ViewportRec),
    /* class_initialize	  */	NULL,
    /* class_part_init    */    NULL,
    /* class_inited	  */	FALSE,
    /* initialize	  */	Initialize,
    /* initialize_hook    */    NULL,
    /* realize		  */	Realize,
    /* actions		  */	NULL,
    /* num_actions	  */	0,
    /* resources	  */	resources,
    /* num_resources	  */	XtNumber(resources),
    /* xrm_class	  */	NULLQUARK,
    /* compress_motion	  */	TRUE,
    /* compress_exposure  */	TRUE,
    /* compress_enterleave*/    TRUE,
    /* visible_interest	  */	FALSE,
    /* destroy		  */	NULL,
    /* resize		  */	Resize,
    /* expose		  */	XtInheritExpose,
    /* set_values	  */	SetValues,
    /* set_values_hook    */    NULL,
    /* set_values_almost  */    XtInheritSetValuesAlmost,
    /* get_values_hook    */	NULL,
    /* accept_focus	  */	NULL,
    /* version            */    XtVersion,
    /* callback_private	  */	NULL,
    /* tm_table    	  */	NULL,
    /* query_geometry     */    PreferredGeometry,
    /* display_accelerator*/	XtInheritDisplayAccelerator,
    /* extension          */	NULL
  },
  { /* composite_class fields */
    /* geometry_manager	  */	GeometryManager,
    /* change_managed	  */	ChangeManaged,
    /* insert_child	  */	XtInheritInsertChild,
    /* delete_child	  */	XtInheritDeleteChild,
    /* extension          */	NULL
  },
  { /* constraint_class fields */
    /* subresourses	  */	NULL,
    /* subresource_count  */	0,
    /* constraint_size	  */	sizeof(ViewportConstraintsRec),
    /* initialize	  */	ConstraintInitialize,
    /* destroy		  */	NULL,
    /* set_values	  */	NULL,
    /* extension          */	NULL
  },
  { /* form_class fields */
    /* empty		  */	0
  },
  { /* viewport_class fields */
    /* empty		  */	0
  }
};


WidgetClass viewportWidgetClass = (WidgetClass)&viewportClassRec;

static void CreateScrollbar(w, horizontal)
    ViewportWidget w;
    Boolean horizontal;
{
    Widget clip = w->viewport.clip;
    ViewportConstraints constraints =
	(ViewportConstraints)clip->core.constraints;
    static Arg barArgs[] = {
	{XtNorientation, NULL},
	{XtNlength, NULL},
	{XtNleft, NULL},
	{XtNright, NULL},
	{XtNtop, NULL},
	{XtNbottom, NULL},
	{XtNmappedWhenManaged, False},
    };
    Widget bar;

    XtSetArg(barArgs[0], XtNorientation,
	      horizontal ? XtorientHorizontal : XtorientVertical );
    XtSetArg(barArgs[1], XtNlength,
	     horizontal ? clip->core.width : clip->core.height);
    XtSetArg(barArgs[2], XtNleft,
	     (!horizontal && w->viewport.useright) ? XtChainRight : XtChainLeft);
    XtSetArg(barArgs[3], XtNright,
	     (!horizontal && !w->viewport.useright) ? XtChainLeft : XtChainRight);
    XtSetArg(barArgs[4], XtNtop,
	     (horizontal && w->viewport.usebottom) ? XtChainBottom: XtChainTop);
    XtSetArg(barArgs[5], XtNbottom,
	     (horizontal && !w->viewport.usebottom) ? XtChainTop: XtChainBottom);

    bar = XtCreateWidget( (horizontal ? "horizontal" : "vertical"),
			  scrollbarWidgetClass, (Widget)w,
			  barArgs, XtNumber(barArgs) );
    XtAddCallback( bar, XtNscrollProc, ScrollUpDownProc, (caddr_t)w );
    XtAddCallback( bar, XtNjumpProc, ThumbProc, (caddr_t)w );

    if (horizontal) {
	Dimension bw = bar->core.border_width;
	w->viewport.horiz_bar = bar;
	constraints->form.vert_base = bar;
	XtResizeWidget( clip, clip->core.width,	/* %%% wrong, but... */
		        w->core.height - bar->core.height - bw,
		        clip->core.border_width );
    }
    else {
	Dimension bw = bar->core.border_width;
	w->viewport.vert_bar = bar;
	constraints->form.horiz_base = bar;
	XtResizeWidget( clip, w->core.width - bar->core.width - bw, /* %%% */
		        bar->core.height, clip->core.border_width );
    }

    XtManageChild( bar );

    return;
}

/* ARGSUSED */
static void Initialize(request, new)
    Widget request, new;
{
    ViewportWidget w = (ViewportWidget)new;
    static Arg clip_args[] = {
	{XtNwidth, NULL},
	{XtNheight, NULL},
	{XtNborderWidth, 0},
	{XtNleft, (XtArgVal)XtChainLeft},
	{XtNright, (XtArgVal)XtChainRight},
	{XtNtop, (XtArgVal)XtChainTop},
	{XtNbottom, (XtArgVal)XtChainBottom}
    };

    w->viewport.clip = w->viewport.child
	= w->viewport.horiz_bar = w->viewport.vert_bar
	= (Widget)NULL;

    w->form.default_spacing = 0;

    clip_args[0].value = (XtArgVal)Max(w->core.width,1);
    clip_args[1].value = (XtArgVal)Max(w->core.height,1);
    w->viewport.clip =
	XtCreateManagedWidget( "clip", widgetClass, new,
			       clip_args, XtNumber(clip_args) );

    if (w->viewport.forcebars) {
	if (w->viewport.allowhoriz) CreateScrollbar(w, True);
	if (w->viewport.allowvert)  CreateScrollbar(w, False);
    }
}

/* ARGSUSED */
static void ConstraintInitialize(request, new)
    Widget request, new;
{
    ((ViewportConstraints)new->core.constraints)->viewport.reparented = False;
}

static void Realize(widget, value_mask, attributes)
    Widget widget;
    XtValueMask *value_mask;
    XSetWindowAttributes *attributes;
{
    ViewportWidget w = (ViewportWidget)widget;
    register Widget child = w->viewport.child;
    register Widget clip = w->viewport.clip;

    *value_mask |= CWBitGravity;
    attributes->bit_gravity = NorthWestGravity;
    (*superclass->core_class.realize)(widget, value_mask, attributes);

    (*w->core.widget_class->core_class.resize)(widget);	/* turn on bars */

    if (child) {
	XtMoveWidget( child, (Position)0, (Position)0 );
	XtRealizeWidget( clip );
	XtRealizeWidget( child );
	XReparentWindow( XtDisplay(w), XtWindow(child), XtWindow(clip),
			 (Position)0, (Position)0 );
	XtMapWidget( child );
    }
}

/* ARGSUSED */
static Boolean SetValues(current, request, new)
    Widget current, request, new;
{
    ViewportWidget w = (ViewportWidget)new;
    ViewportWidget cw = (ViewportWidget)current;

    if (w->viewport.forcebars != cw->viewport.forcebars) {
	if (w->viewport.forcebars) {
	    if (w->viewport.allowhoriz && !w->viewport.horiz_bar)
		CreateScrollbar( w, True );
	    if (w->viewport.allowvert && !w->viewport.vert_bar)
		CreateScrollbar( w, False );
	}
    }

    /* take care of bars, &tc. */
    (*w->core.widget_class->core_class.resize)( new );
    return False;
}


static void ChangeManaged(widget)
    Widget widget;
{
    ViewportWidget w = (ViewportWidget)widget;
    register int num_children = w->composite.num_children;
    register Widget child, *childP;
    register int i;

    child = (Widget)NULL;
    for (childP=w->composite.children, i=0; i < num_children; childP++, i++) {
	if (XtIsManaged(*childP)
	    && *childP != w->viewport.clip
	    && *childP != w->viewport.horiz_bar
	    && *childP != w->viewport.vert_bar)
	{
	    child = *childP;
	    break;
	}
    }

    if (child != w->viewport.child) {
	w->viewport.child = child;
	if (child) {
	    XtResizeWidget( child, child->core.width,
			    child->core.height, (Dimension)0 );
	    if (XtIsRealized(widget)) {
		ViewportConstraints constraints =
		    (ViewportConstraints)child->core.constraints;
		if (!XtIsRealized(child)) {
		    Window window = XtWindow(w);
		    XtMoveWidget( child, (Position)0, (Position)0 );
#ifdef notdef
		    /* this is dirty, but it saves the following code: */
		    XtRealizeWidget( child );
		    XReparentWindow( XtDisplay(w), XtWindow(child),
				     XtWindow(w->viewport.clip),
				     (Position)0, (Position)0 );
		    if (child->core.mapped_when_managed)
			XtMapWidget( child );
#else notdef
		    w->core.window = XtWindow(w->viewport.clip);
		    XtRealizeWidget( child );
		    w->core.window = window;
#endif notdef
		    constraints->viewport.reparented = True;
		}
		else if (!constraints->viewport.reparented) {
		    XReparentWindow( XtDisplay(w), XtWindow(child),
				     XtWindow(w->viewport.clip),
				     (Position)0, (Position)0 );
		    constraints->viewport.reparented = True;
		    if (child->core.mapped_when_managed)
			XtMapWidget( child );
		}
	    }
	    /* %%% DoLayout should be FormClass method */
	    DoLayout( widget, child->core.width, child->core.height );
	    /* always want to resize, as we may no longer need bars */
	    (*widget->core.widget_class->core_class.resize)( widget );
	    /* %%% do we need to hide this child from Form?  */
	}
    }

#ifdef notdef
    (*superclass->composite_class.change_managed)( widget );
#endif
}


static void SetBar(w, top, length, total)
    Widget w;
    Position top;
    Dimension length, total;
{
    XtScrollBarSetThumb(w, (float) top / total, (float) length / total );
}

static void RedrawThumbs(w)
  ViewportWidget w;
{
    register Widget child = w->viewport.child;
    register Widget clip = w->viewport.clip;

    if (w->viewport.horiz_bar)
	SetBar( w->viewport.horiz_bar, -(child->core.x),
	        clip->core.width, child->core.width );

    if (w->viewport.vert_bar)
	SetBar( w->viewport.vert_bar, -(child->core.y),
	        clip->core.height, child->core.height );
}



static void MoveChild(w, x, y)
    ViewportWidget w;
    Position x, y;
{
    register Widget child = w->viewport.child;
    register Widget clip = w->viewport.clip;

    /* make sure we never move past right/bottom borders */
    if (-x + clip->core.width > child->core.width)
	x = -(child->core.width - clip->core.width);

    if (-y + clip->core.height > child->core.height)
	y = -(child->core.height - clip->core.height);

    /* make sure we never move past left/top borders */
    if (x >= 0) x = 0;
    if (y >= 0) y = 0;

    XtMoveWidget(child, x, y);

    RedrawThumbs(w);
}

static void Resize(widget)
    Widget widget;
{
    ViewportWidget w = (ViewportWidget)widget;
    register Widget child = w->viewport.child;
    register Widget clip = w->viewport.clip;
    ViewportConstraints constraints
	= (ViewportConstraints)clip->core.constraints;
    int   lw, lh;
    Boolean needshoriz, needsvert;
    int clip_width, clip_height;
    int child_width, child_height;

    if (child) {
	XtWidgetGeometry intended, reply;
	intended.request_mode = 0;
	if (!w->viewport.allowhoriz || child->core.width < w->core.width) {
	    intended.request_mode = CWWidth;
	    intended.width = w->core.width;
	}
	if (!w->viewport.allowvert || child->core.height < w->core.height) {
	    intended.request_mode |= CWHeight;
	    intended.height = w->core.height;
	}
	XtQueryGeometry( child, &intended, &reply );
	child_width = reply.width;
	child_height = reply.height;
    }
    else {
	child_width = 0;
	child_height = 0;
    }

    clip_width = w->core.width;
    clip_height = w->core.height;
    if (w->viewport.forcebars) {
	if (needsvert = w->viewport.allowvert)
	    clip_width -= w->viewport.vert_bar->core.width +
			  w->viewport.vert_bar->core.border_width;
	if (needshoriz = w->viewport.allowhoriz)
	    clip_height -= w->viewport.horiz_bar->core.height +
			   w->viewport.horiz_bar->core.border_width;
	AssignMax( clip_width, 1 );
	AssignMax( clip_height, 1 );
    }
    else {
	do {
	    lw = clip_width;
	    lh = clip_height;
	    if (w->viewport.allowhoriz)
		needshoriz = (Boolean)(child_width > clip_width);
	    else {
		child_width = clip_width;
		needshoriz = FALSE;
	    }
	    if (w->viewport.allowvert)
		needsvert = (Boolean)(child_height > clip_height);
	    else {
		child_height = clip_height;
		needsvert = FALSE;
	    }
	    if (! XtIsRealized(widget))
		needsvert = needshoriz = FALSE;
	    if (needshoriz && !w->viewport.horiz_bar)
		CreateScrollbar(w, True);
	    if (needsvert && !w->viewport.vert_bar)
		CreateScrollbar(w, False);
	    clip_width = w->core.width -
		(needsvert ? w->viewport.vert_bar->core.width
			     + w->viewport.vert_bar->core.border_width : 0);
	    clip_height = w->core.height -
		(needshoriz ? w->viewport.horiz_bar->core.height
			      + w->viewport.horiz_bar->core.border_width : 0);
	    AssignMax( clip_width, 1 );
	    AssignMax( clip_height, 1 );
	} while (lw != clip_width || lh != clip_height);
    }

    if (XtIsRealized(clip))
	XRaiseWindow( XtDisplay(clip), XtWindow(clip) );

    XtMoveWidget( clip,
		  needsvert ? (w->viewport.useright ? 0 :
			       w->viewport.vert_bar->core.width +
			       w->viewport.vert_bar->core.border_width) : 0,
		  needshoriz ? (w->viewport.usebottom ? 0 :
				w->viewport.horiz_bar->core.height +
			        w->viewport.horiz_bar->core.border_width) : 0);
    XtResizeWidget( clip, (Dimension)clip_width,
		    (Dimension)clip_height, (Dimension)0 );
	
    if (w->viewport.horiz_bar) {
	register Widget bar = w->viewport.horiz_bar;
	if (!needshoriz) {
	    constraints->form.vert_base = (Widget)NULL;
	    XtDestroyWidget( bar );
	    w->viewport.horiz_bar = (Widget)NULL;
	}
	else {
	    register int bw = bar->core.border_width;
	    XtResizeWidget( bar, clip_width, bar->core.height, bw );
	    XtMoveWidget( bar,
			  ((needsvert && !w->viewport.useright)
			   ? w->viewport.vert_bar->core.width
			   : -bw),
			  (w->viewport.usebottom
			    ? w->core.height - bar->core.height - bw
			    : -bw) );
	    XtSetMappedWhenManaged( bar, True );
	}
    }

    if (w->viewport.vert_bar) {
	register Widget bar = w->viewport.vert_bar;
	if (!needsvert) {
	    constraints->form.horiz_base = (Widget)NULL;
	    XtDestroyWidget( bar );
	    w->viewport.vert_bar = (Widget)NULL;
	}
	else {
	    register int bw = bar->core.border_width;
	    XtResizeWidget( bar, bar->core.width, clip_height, bw );
	    XtMoveWidget( bar,
			  (w->viewport.useright
			   ? w->core.width - bar->core.width - bw 
			   : -bw),
			  ((needshoriz && !w->viewport.usebottom)
			    ? w->viewport.horiz_bar->core.height
			    : -bw) );
	    XtSetMappedWhenManaged( bar, True );
	}
    }

    if (child) {
	XtResizeWidget( child, (Dimension)child_width,
		        (Dimension)child_height, (Dimension)0 );
	MoveChild(w, child->core.x, child->core.y);
    }
}



/* Semi-public routines */


static void ScrollUpDownProc(widget, closure, call_data)
    Widget widget;
    caddr_t closure;
    caddr_t call_data;
{
    ViewportWidget w = (ViewportWidget)closure;
    register Widget child = w->viewport.child;
    int pix = (int)call_data;
    Position x, y;

    x = child->core.x - ((widget == w->viewport.horiz_bar) ? pix : 0);
    y = child->core.y - ((widget == w->viewport.vert_bar) ? pix : 0);
    MoveChild(w, x, y);
}


/* ARGSUSED */
static void ThumbProc(widget, closure, percent)
    Widget widget;
    caddr_t closure;
    float *percent;
{
    ViewportWidget w = (ViewportWidget)closure;
    register Widget child = w->viewport.child;
    Position x, y;

    if (widget == w->viewport.horiz_bar)
#ifdef macII				/* bug in the macII A/UX 1.0 cc */
	x = (int)(-*percent * child->core.width);
#else /* else not macII */
	x = -(int)(*percent * child->core.width);
#endif /* macII */
    else
	x = child->core.x;

    if (widget == w->viewport.vert_bar)
#ifdef macII				/* bug in the macII A/UX 1.0 cc */
	y = (int)(-*percent * child->core.height);
#else /* else not macII */
	y = -(int)(*percent * child->core.height);
#endif /* macII */
    else
	y = child->core.y;

    MoveChild(w, x, y);
}


static XtGeometryResult GeometryManager(child, request, reply)
    Widget child;
    XtWidgetGeometry *request, *reply;
{
    ViewportWidget w = (ViewportWidget)child->core.parent;
    Boolean rWidth = (Boolean)(request->request_mode & CWWidth);
    Boolean rHeight = (Boolean)(request->request_mode & CWHeight);
    XtWidgetGeometry allowed, myrequest;
    XtGeometryResult result;
    Boolean reconfigured;

    if (child != w->viewport.child
	|| request->request_mode & ~(CWWidth | CWHeight | CWBorderWidth)
	|| ((request->request_mode & CWBorderWidth)
	    && request->border_width > 0))
	return XtGeometryNo;

    allowed = *request;
    result = XtGeometryYes;

    /* %%% DoLayout should be a FormClass method */
    reconfigured = DoLayout( (Widget)w,
			     (rWidth ? request->width : w->core.width),
			     (rHeight ? request->height : w->core.height) );

    if (rWidth && w->core.width != request->width) {
	if (!w->viewport.allowhoriz
	    || request->width < w->core.width)
	{
	    allowed.width = w->core.width;
	    result = XtGeometryAlmost;
	}
    }
    if (rHeight && w->core.height != request->height) {
	if (!w->viewport.allowvert
	    || request->height < w->core.height)
	{
	    allowed.height = w->core.height;
	    result = XtGeometryAlmost;
	}
    }
    *reply = allowed;
    if (result == XtGeometryYes) {
	Boolean needs_horiz = False, needs_vert = False;
	if (rWidth)  child->core.width = request->width;
	if (rHeight) child->core.height = request->height;
	myrequest.request_mode = 0;
	if (child->core.width > w->core.width) needs_horiz = True;
	if (child->core.height > w->core.height) needs_vert = True;
	if (needs_horiz && !w->viewport.horiz_bar && XtIsRealized((Widget)w)) {
	    CreateScrollbar( w, True );
	    reconfigured = True;
	    if ((myrequest.height = w->viewport.horiz_bar->core.height << 1)
		> w->core.height)
		myrequest.request_mode |= CWHeight;
	}
	if (needs_vert && !w->viewport.vert_bar && XtIsRealized((Widget)w)) {
	    CreateScrollbar( w, False );
	    reconfigured = True;
	    if ((myrequest.width = w->viewport.vert_bar->core.width << 1)
		> w->core.width)
		myrequest.request_mode |= CWWidth;
	}
	if (myrequest.request_mode) {
	    XtGeometryResult ans =
		XtMakeGeometryRequest( (Widget)w, &myrequest, &myrequest );
	    if (ans == XtGeometryAlmost)
		ans = XtMakeGeometryRequest( (Widget)w, &myrequest, NULL );
	    if (ans == XtGeometryYes)
		reconfigured = True;
	}
    }

    if (reconfigured) (*w->core.widget_class->core_class.resize)( (Widget)w );
    return result;
}


/* %%% DoLayout should be a FormClass method */
static Boolean DoLayout(w, width, height)
    Widget w;
    Dimension width, height;
{
    XtWidgetGeometry geometry;
    XtGeometryResult result;

    if (width == w->core.width && height == w->core.height)
	return False;

    geometry.request_mode = CWWidth | CWHeight;
    geometry.width = width;
    geometry.height = height;

    if (XtIsRealized(w)) {
	if (((ViewportWidget)w)->viewport.allowhoriz)
	    geometry.width = Min(w->core.width, width);
	if (((ViewportWidget)w)->viewport.allowvert)
	    geometry.height = Min(w->core.height, height);
    } else {
	/* This is the Realize call; we'll inherit a w&h iff none currently */
	if (w->core.width != 0) {
	    if (w->core.height != 0) return False;
	    geometry.width = w->core.width;
	}
	if (w->core.height != 0) geometry.height = w->core.height;
    }
    if ((result = XtMakeGeometryRequest(w, &geometry, &geometry))
	== XtGeometryAlmost)
	result = XtMakeGeometryRequest(w, &geometry, NULL);

    return (result == XtGeometryYes);
}

static XtGeometryResult PreferredGeometry(w, constraints, reply)
    Widget w;
    XtWidgetGeometry *constraints, *reply;
{
    if (((ViewportWidget)w)->viewport.child != NULL)
	return XtQueryGeometry( ((ViewportWidget)w)->viewport.child,
			       constraints, reply );
    else
	return XtGeometryYes;
}
