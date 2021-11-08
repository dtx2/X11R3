#ifndef lint
static char Xrcsid[] = "$XConsortium: Simple.c,v 1.16 88/09/27 16:44:32 swick Exp $";
#endif lint

/* Copyright	Massachusetts Institute of Technology	1987 */

#include <X11/IntrinsicP.h>
#include <X11/copyright.h>
#include <X11/StringDefs.h>
#include <X11/SimpleP.h>
#include <X11/Vendor.h>		/* hack; force Xaw/Vendor.o to be loaded */

#define UnspecifiedPixmap 2	/* %%% should be NULL, according to the spec */
#define IsSensitive(w) ((w)->core.sensitive && (w)->core.ancestor_sensitive)

static Cursor defaultCursor = None;
static Pixmap defaultPixmap = NULL;

static XtResource resources[] = {
#define offset(field) XtOffset(SimpleWidget, simple.field)
  {XtNcursor, XtCCursor, XtRCursor, sizeof(Cursor),
     offset(cursor), XtRCursor, (caddr_t)&defaultCursor},
  {XtNinsensitiveBorder, XtCInsensitive, XtRPixmap, sizeof(Pixmap),
     offset(insensitive_border), XtRPixmap, (caddr_t)&defaultPixmap}
#undef offset
};

static void ClassPartInitialize(), Realize();
static Boolean SetValues(), ChangeSensitive();

SimpleClassRec simpleClassRec = {
  { /* core fields */
    /* superclass		*/	(WidgetClass) &widgetClassRec,
    /* class_name		*/	"Simple",
    /* widget_size		*/	sizeof(SimpleRec),
    /* class_initialize		*/	NULL,
    /* class_part_initialize	*/	ClassPartInitialize,
    /* class_inited		*/	FALSE,
    /* initialize		*/	NULL,
    /* initialize_hook		*/	NULL,
    /* realize			*/	Realize,
    /* actions			*/	NULL,
    /* num_actions		*/	0,
    /* resources		*/	resources,
    /* num_resources		*/	XtNumber(resources),
    /* xrm_class		*/	NULLQUARK,
    /* compress_motion		*/	TRUE,
    /* compress_exposure	*/	TRUE,
    /* compress_enterleave	*/	TRUE,
    /* visible_interest		*/	FALSE,
    /* destroy			*/	NULL,
    /* resize			*/	NULL,
    /* expose			*/	NULL,
    /* set_values		*/	SetValues,
    /* set_values_hook		*/	NULL,
    /* set_values_almost	*/	XtInheritSetValuesAlmost,
    /* get_values_hook		*/	NULL,
    /* accept_focus		*/	NULL,
    /* version			*/	XtVersion,
    /* callback_private		*/	NULL,
    /* tm_table			*/	NULL,
    /* query_geometry		*/	XtInheritQueryGeometry,
    /* display_accelerator	*/	XtInheritDisplayAccelerator,
    /* extension		*/	NULL
  },
  { /* simple fields */
    /* change_sensitive		*/	ChangeSensitive
  }
};

WidgetClass simpleWidgetClass = (WidgetClass)&simpleClassRec;

static void ClassPartInitialize(class)
    WidgetClass class;
{
    register SimpleWidgetClass c = (SimpleWidgetClass)class;
#ifndef lint
    /* this silliness causes the linker to include the VendorShell
     * module from Xaw, rather than the one from Xt.
     */
    WidgetClass junk = vendorShellWidgetClass;
#endif

    if (c->simple_class.change_sensitive == XtInheritChangeSensitive)
	c->simple_class.change_sensitive = ChangeSensitive;
}


/* ARGSUSED */
static void Realize(w, valueMask, attributes)
    register Widget w;
    Mask *valueMask;
    XSetWindowAttributes *attributes;
{
    Pixmap border_pixmap;

    if (!IsSensitive(w)) {
	/* change border to gray; have to remember the old one,
	 * so XtDestroyWidget deletes the proper one */
	if (((SimpleWidget)w)->simple.insensitive_border == NULL)
	    ((SimpleWidget)w)->simple.insensitive_border =
		XmuCreateStippledPixmap(XtScreen(w),
					w->core.border_pixel, 
					w->core.background_pixel,
					w->core.depth);
        border_pixmap = w->core.border_pixmap;
	attributes->border_pixmap =
	  w->core.border_pixmap = ((SimpleWidget)w)->simple.insensitive_border;

	*valueMask |= CWBorderPixmap;
	*valueMask &= ~CWBorderPixel;
    }

    if ((attributes->cursor = ((SimpleWidget)w)->simple.cursor) != None)
	*valueMask |= CWCursor;

    XtCreateWindow( w, (unsigned int)InputOutput, (Visual *)CopyFromParent,
		    *valueMask, attributes );

    if (!IsSensitive(w))
	w->core.border_pixmap = border_pixmap;

}


/* ARGSUSED */
static Boolean SetValues(current, request, new)
    Widget current, request, new;
{
    if ((current->core.sensitive != new->core.sensitive ||
	 current->core.ancestor_sensitive != new->core.ancestor_sensitive))
	ChangeSensitive( new );

    return False;
}


static Boolean ChangeSensitive(w)
    register Widget w;
{
    if (XtIsRealized(w)) {
	if (IsSensitive(w))
	    if (w->core.border_pixmap != UnspecifiedPixmap)
		XSetWindowBorderPixmap( XtDisplay(w), XtWindow(w),
				        w->core.border_pixmap );
	    else
		XSetWindowBorder( XtDisplay(w), XtWindow(w), 
				  w->core.border_pixel );
	else {
	    if (((SimpleWidget)w)->simple.insensitive_border == NULL)
		((SimpleWidget)w)->simple.insensitive_border =
		    XmuCreateStippledPixmap(XtScreen(w),
					    w->core.border_pixel, 
					    w->core.background_pixel,
					    w->core.depth);
	    XSetWindowBorderPixmap( XtDisplay(w), XtWindow(w),
				    ((SimpleWidget)w)->
				        simple.insensitive_border );
	}
    }
    return False;
}

