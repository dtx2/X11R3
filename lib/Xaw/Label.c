#ifndef lint
static char Xrcsid[] = "$XConsortium: Label.c,v 1.62 88/10/21 08:15:13 swick Exp $";
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

/*
 * Label.c - Label widget
 *
 */

#define XtStrlen(s)		((s) ? strlen(s) : 0)

#include <X11/IntrinsicP.h>
#include <X11/StringDefs.h>
#include <stdio.h>
#include <ctype.h>
#include <X11/LabelP.h>

/****************************************************************
 *
 * Full class record constant
 *
 ****************************************************************/

/* Private Data */

#define offset(field) XtOffset(LabelWidget, field)
static XtResource resources[] = {
    {XtNforeground, XtCForeground, XtRPixel, sizeof(Pixel),
	offset(label.foreground), XtRString, "XtDefaultForeground"},
    {XtNfont,  XtCFont, XtRFontStruct, sizeof(XFontStruct *),
	offset(label.font),XtRString, "XtDefaultFont"},
    {XtNlabel,  XtCLabel, XtRString, sizeof(String),
	offset(label.label), XtRString, NULL},
    {XtNjustify, XtCJustify, XtRJustify, sizeof(XtJustify),
	offset(label.justify), XtRImmediate, (caddr_t)XtJustifyCenter},
    {XtNinternalWidth, XtCWidth, XtRDimension,  sizeof(Dimension),
	offset(label.internal_width), XtRImmediate, (caddr_t)4},
    {XtNinternalHeight, XtCHeight, XtRDimension, sizeof(Dimension),
	offset(label.internal_height), XtRImmediate, (caddr_t)2},
    {XtNbitmap, XtCPixmap, XtRPixmap, sizeof(Pixmap),
	offset(label.pixmap), XtRPixmap, (caddr_t)None},
    {XtNresize, XtCResize, XtRBoolean, sizeof(Boolean),
	offset(label.resize), XtRImmediate, (caddr_t)True},
};

static void Initialize();
static void Realize();
static void Resize();
static void Redisplay();
static Boolean SetValues();
static void ClassInitialize();
static void Destroy();
static XtGeometryResult QueryGeometry();

LabelClassRec labelClassRec = {
  {
/* core_class fields */	
#define superclass		(&simpleClassRec)
    /* superclass	  	*/	(WidgetClass) superclass,
    /* class_name	  	*/	"Label",
    /* widget_size	  	*/	sizeof(LabelRec),
    /* class_initialize   	*/	ClassInitialize,
    /* class_part_initialize	*/	NULL,
    /* class_inited       	*/	FALSE,
    /* initialize	  	*/	Initialize,
    /* initialize_hook		*/	NULL,
    /* realize		  	*/	Realize,
    /* actions		  	*/	NULL,
    /* num_actions	  	*/	0,
    /* resources	  	*/	resources,
    /* num_resources	  	*/	XtNumber(resources),
    /* xrm_class	  	*/	NULLQUARK,
    /* compress_motion	  	*/	TRUE,
    /* compress_exposure  	*/	TRUE,
    /* compress_enterleave	*/	TRUE,
    /* visible_interest	  	*/	FALSE,
    /* destroy		  	*/	Destroy,
    /* resize		  	*/	Resize,
    /* expose		  	*/	Redisplay,
    /* set_values	  	*/	SetValues,
    /* set_values_hook		*/	NULL,
    /* set_values_almost	*/	XtInheritSetValuesAlmost,
    /* get_values_hook		*/	NULL,
    /* accept_focus	 	*/	NULL,
    /* version			*/	XtVersion,
    /* callback_private   	*/	NULL,
    /* tm_table		   	*/	NULL,
    /* query_geometry		*/	QueryGeometry,
    /* display_accelerator	*/	XtInheritDisplayAccelerator,
    /* extension		*/	NULL
  }
};
WidgetClass labelWidgetClass = (WidgetClass)&labelClassRec;
/****************************************************************
 *
 * Private Procedures
 *
 ****************************************************************/

static void ClassInitialize()
{
    XtAddConverter( XtRString, XtRJustify, XmuCvtStringToJustify, NULL, 0 );
} /* ClassInitialize */

/*
 * Calculate width and height of displayed text in pixels
 */

static void SetTextWidthAndHeight(lw)
    LabelWidget lw;
{
    register XFontStruct	*fs = lw->label.font;

    if (lw->label.pixmap != None) {
	Window root;
	int x, y;
	unsigned int width, height, bw, depth;
	if (XGetGeometry(XtDisplay(lw), lw->label.pixmap, &root, &x, &y,
			 &width, &height, &bw, &depth)) {
	    lw->label.label_height = height;
	    lw->label.label_width = width;
	    lw->label.label_len = depth;
	    return;
	}
    }

    lw->label.label_len = XtStrlen(lw->label.label);
    lw->label.label_height = fs->max_bounds.ascent + fs->max_bounds.descent;
    lw->label.label_width = XTextWidth(
	fs, lw->label.label, (int) lw->label.label_len);
}

static void GetnormalGC(lw)
    LabelWidget lw;
{
    XGCValues	values;

    values.foreground	= lw->label.foreground;
    values.background	= lw->core.background_pixel;
    values.font		= lw->label.font->fid;

    lw->label.normal_GC = XtGetGC(
	(Widget)lw,
	(unsigned) GCForeground | GCBackground | GCFont,
	&values);
}

static void GetgrayGC(lw)
    LabelWidget lw;
{
    XGCValues	values;

    values.foreground = lw->label.foreground;
    values.background = lw->core.background_pixel;
    values.font	      = lw->label.font->fid;
    values.fill_style = FillTiled;
    values.tile       = XmuCreateStippledPixmap(XtScreen((Widget)lw),
						lw->label.foreground, 
						lw->core.background_pixel,
						lw->core.depth);

    lw->label.gray_GC = XtGetGC((Widget)lw, 
				(unsigned) GCForeground | GCBackground |
					   GCFont | GCTile | GCFillStyle,
				&values);
}

/* ARGSUSED */
static void Initialize(request, new)
 Widget request, new;
{
    LabelWidget lw = (LabelWidget) new;

    if (lw->label.label == NULL) 
        lw->label.label = lw->core.name;
    else {
        lw->label.label = strcpy( XtMalloc( strlen(lw->label.label) + 1 ),
				  lw->label.label );
    }

    GetnormalGC(lw);
    GetgrayGC(lw);

    SetTextWidthAndHeight(lw);

    if (lw->core.width == 0)
        lw->core.width = lw->label.label_width + 2 * lw->label.internal_width;
    if (lw->core.height == 0)
        lw->core.height = lw->label.label_height + 2*lw->label.internal_height;

    (*XtClass(new)->core_class.resize) ((Widget)lw);

} /* Initialize */


static void Realize(w, valueMask, attributes)
    register Widget w;
    Mask *valueMask;
    XSetWindowAttributes *attributes;
{
    *valueMask |= CWBitGravity;
    attributes->bit_gravity = NorthWestGravity;
    (*superclass->core_class.realize) (w, valueMask, attributes);
    
} /* Realize */



/*
 * Repaint the widget window
 */

/* ARGSUSED */
static void Redisplay(w, event, region)
    Widget w;
    XEvent *event;
    Region region;
{
   LabelWidget lw = (LabelWidget) w;
   GC gc;

   if (region != NULL &&
       XRectInRegion(region, lw->label.label_x, lw->label.label_y,
		     lw->label.label_width, lw->label.label_height)
	     == RectangleOut)
       return;

   gc = XtIsSensitive(lw) ? lw->label.normal_GC : lw->label.gray_GC;
#ifdef notdef
   if (region != NULL) XSetRegion(XtDisplay(w), gc, region);
#endif /*notdef*/
   if (lw->label.pixmap == None) {
       XDrawString(
		   XtDisplay(w), XtWindow(w), gc, lw->label.label_x,
		   lw->label.label_y + lw->label.font->max_bounds.ascent,
		   lw->label.label, (int) lw->label.label_len);
   } else if (lw->label.label_len == 1) { /* depth */
       XCopyPlane(
		  XtDisplay(w), lw->label.pixmap, XtWindow(w), gc,
		  0, 0, lw->label.label_width, lw->label.label_height,
		  lw->label.label_x, lw->label.label_y, 1L);
   } else {
       XCopyArea(
		 XtDisplay(w), lw->label.pixmap, XtWindow(w), gc,
		 0, 0, lw->label.label_width, lw->label.label_height,
		 lw->label.label_x, lw->label.label_y);
   }
#ifdef notdef
   if (region != NULL) XSetClipMask(XtDisplay(w), gc, (Pixmap)None);
#endif notdef
}

static void _Reposition(lw, width, height, dx, dy)
    register LabelWidget lw;
    Dimension width, height;
    Position *dx, *dy;
{
    Position newPos;
    switch (lw->label.justify) {

	case XtJustifyLeft   :
	    newPos = lw->label.internal_width;
	    break;

	case XtJustifyRight  :
	    newPos = width -
		(lw->label.label_width + lw->label.internal_width);
	    break;

	case XtJustifyCenter :
	    newPos = (width - lw->label.label_width) / 2;
	    break;
    }
    if (newPos < (Position)lw->label.internal_width)
	newPos = lw->label.internal_width;
    *dx = newPos - lw->label.label_x;
    lw->label.label_x = newPos;
    *dy = (newPos = (height - lw->label.label_height) / 2) - lw->label.label_y;
    lw->label.label_y = newPos;
    return;
}


static void Resize(w)
    Widget w;
{
    LabelWidget lw = (LabelWidget)w;
    Position dx, dy;
    _Reposition(lw, w->core.width, w->core.height, &dx, &dy);
    if ((dx || dy) && XtIsRealized(w)) {
	int old_x = lw->label.label_x - dx;
	int old_y = lw->label.label_y - dy;
	XCopyArea(XtDisplay(w), XtWindow(w), XtWindow(w), lw->label.normal_GC,
		  (int)old_x, (int)old_y,
		  (unsigned)lw->label.label_width,
		  (unsigned)lw->label.label_height,
		  (int)lw->label.label_x, (int)lw->label.label_y);
	if (dx) {
	    XClearArea(XtDisplay(w), XtWindow(w),
		       (dx > 0) ? old_x
		             : (int)lw->label.label_x + lw->label.label_width,
		       old_y, (unsigned) ((dx < 0) ? -dx : dx),
		       (unsigned)lw->label.label_height, False);
	}
	if (dy) {
	    XClearArea(XtDisplay(w), XtWindow(w),
		       old_x,
		       (dy > 0) ? old_y
			     : (int)lw->label.label_y + lw->label.label_height,
		       (unsigned)lw->label.label_width, 
		       (unsigned) ((dy < 0) ? -dy : dy), False);
	}
    }
}

/*
 * Set specified arguments into widget
 */

/* ARGSUSED */
static Boolean SetValues(current, request, new)
    Widget current, request, new;
{
    LabelWidget curlw = (LabelWidget) current;
    LabelWidget reqlw = (LabelWidget) request;
    LabelWidget newlw = (LabelWidget) new;
    Boolean was_resized = False;
    Boolean redisplay = False;

    if (newlw->label.label == NULL) {
	newlw->label.label = newlw->core.name;
    }

    /* note that there is no way to change the label and force the window */
    /* to keep it's current size (and possibly clip the text) perhaps we */
    /* should make the user set width and height to 0 when they set the */
    /* label if they want the label to recompute size based on the new */
    /* label? */
    if (curlw->label.label != newlw->label.label) {
        if (curlw->label.label != curlw->core.name)
	    XtFree( (char *)curlw->label.label );

	if (newlw->label.label != newlw->core.name) {
	    newlw->label.label = strcpy(
	        XtMalloc((unsigned) XtStrlen(newlw->label.label) + 1),
		newlw->label.label);
	}
	was_resized = True;
    }

    if (was_resized
	|| (curlw->label.font != newlw->label.font)
	|| (curlw->label.justify != newlw->label.justify)
	|| (curlw->label.pixmap != newlw->label.pixmap)) {

	SetTextWidthAndHeight(newlw);
	was_resized = True;

	}

    /* calculate the window size */
    if (newlw->label.resize) {
	if (curlw->core.width == reqlw->core.width)
	    newlw->core.width =
		newlw->label.label_width +2*newlw->label.internal_width;

	if (curlw->core.height == reqlw->core.height)
	    newlw->core.height =
		newlw->label.label_height + 2*newlw->label.internal_height;
    }

    if (curlw->label.foreground != newlw->label.foreground
	|| curlw->label.font->fid != newlw->label.font->fid) {

	XtReleaseGC(new, curlw->label.normal_GC);
	XtReleaseGC(new, curlw->label.gray_GC);
	GetnormalGC(newlw);
	GetgrayGC(newlw);
	redisplay = True;
    }

    if ((curlw->label.internal_width != newlw->label.internal_width)
        || (curlw->label.internal_height != newlw->label.internal_height)
	|| was_resized) {
	/* Resize() will be called if geometry changes succeed */
	Position dx, dy;
	_Reposition(newlw, curlw->core.width, curlw->core.height, &dx, &dy);
    }

    return was_resized || redisplay ||
	   XtIsSensitive(current) != XtIsSensitive(new);
}


static void Destroy(w)
    Widget w;
{
    LabelWidget lw = (LabelWidget)w;

    XtReleaseGC( w, lw->label.normal_GC );
    XtReleaseGC( w, lw->label.gray_GC);
}


static XtGeometryResult QueryGeometry(w, intended, preferred)
    Widget w;
    XtWidgetGeometry *intended, *preferred;
{
    register LabelWidget lw = (LabelWidget)w;

    preferred->request_mode = CWWidth | CWHeight;
    preferred->width = lw->label.label_width + 2 * lw->label.internal_width;
    preferred->height = lw->label.label_height + 2*lw->label.internal_height;
    if (  ((intended->request_mode & (CWWidth | CWHeight))
	   	== (CWWidth | CWHeight)) &&
	  intended->width == preferred->width &&
	  intended->height == preferred->height)
	return XtGeometryYes;
    else if (preferred->width == w->core.width &&
	     preferred->height == w->core.height)
	return XtGeometryNo;
    else
	return XtGeometryAlmost;
}
