#ifndef lint
static char Xrcsid[] = "$XConsortium: VPaned.c,v 1.45 88/10/19 09:39:56 swick Exp $";
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
 * VPaned.c - VPaned composite Widget (Converted to classing toolkit)
 *
 */

#include <ctype.h>
#include <X11/IntrinsicP.h>
#include <X11/cursorfont.h>
#include <X11/StringDefs.h>
#include <X11/Grip.h>
#include <X11/VPanedP.h>
#include <X11/XawMisc.h>

typedef enum {UpperPane='U', LowerPane='L', ThisBorderOnly='T'} Direction;

#define PaneInfo(w)	((Pane)(w)->core.constraints)
#define IsPane(w)	(PaneInfo(w)->position >= 0)
#define PaneIndex(w)	(-PaneInfo(w)->position - 1)

/*****************************************************************************
 *
 * Full instance record declaration
 *
 ***************************************************************** ***********/

static char defGripTranslations[] =
    "<Btn1Down>:		GripAction(Start, UpperPane) \n\
     <Btn2Down>:		GripAction(Start, ThisBorderOnly) \n\
     <Btn3Down>:		GripAction(Start, LowerPane) \n\
     <Btn1Motion>:		GripAction(Move, Upper) \n\
     <Btn2Motion>:		GripAction(Move, ThisBorder) \n\
     <Btn3Motion>:		GripAction(Move, Lower) \n\
     Any<BtnUp>:		GripAction(Commit)";

#define offset(field) XtOffset(VPanedWidget, v_paned.field)

static XtResource resources[] = {
    {XtNforeground, XtCForeground, XtRPixel, sizeof(Pixel),
         offset(foreground_pixel), XtRString, "Black"},
    {XtNgripIndent, XtCGripIndent, XtRPosition, sizeof(Position),
	 offset(grip_indent), XtRImmediate, (caddr_t)10},
    {XtNrefigureMode, XtCBoolean, XtRBoolean, sizeof(int),
         offset(refiguremode), XtRString, "On"},
    {XtNgripTranslations, XtCTranslations, XtRTranslationTable,
         sizeof(XtTranslations),
         offset(grip_translations), XtRString, (caddr_t)defGripTranslations},
    {XtNgripCursor, XtCCursor, XtRCursor, sizeof(Cursor),
         offset(grip_cursor), XtRString, "sb_v_double_arrow"},
    {XtNupperCursor, XtCCursor, XtRCursor, sizeof(Cursor),
         offset(adjust_upper_cursor), XtRString, "sb_up_arrow"},
    {XtNbetweenCursor, XtCCursor, XtRCursor, sizeof(Cursor),
         offset(adjust_this_cursor), XtRString, "sb_left_arrow"},
    {XtNlowerCursor, XtCCursor, XtRCursor, sizeof(Cursor),
         offset(adjust_lower_cursor), XtRString, "sb_down_arrow"},
};

#undef offset

static Boolean defFalse = False;
static int def0 = 0;
static Dimension def1 = 1;
static Dimension defM1 = (Dimension)~0;

#define offset(field) XtOffset(Pane, field)

static XtResource subresources[] = {
    {XtNallowResize, XtCBoolean, XtRBoolean, sizeof(Boolean),
	 offset(allow_resize), XtRBoolean, (caddr_t)&defFalse},
    {XtNposition, XtCPosition, XtRInt, sizeof(int),
         offset(position), XtRInt, (caddr_t)&def0},
    {XtNmin, XtCMin, XtRDimension, sizeof(Dimension),
         offset(min), XtRDimension, (caddr_t)&def1},
    {XtNmax, XtCMax, XtRDimension, sizeof(Dimension),
         offset(max), XtRDimension, (caddr_t)&defM1},
    {XtNskipAdjust, XtCBoolean, XtRBoolean, sizeof(Boolean),
         offset(skip_adjust), XtRBoolean, (caddr_t)&defFalse},
};

#undef offset

static void ClassPartInitialize();
static void Initialize();
static void Realize();
static void Destroy();
static void Resize();
static Boolean SetValues();
static XtGeometryResult GeometryManager();
static void ChangeManaged();
static void InsertChild();
static void DeleteChild();
static Boolean PaneSetValues();
static void SetMinMax();
static void RefigureMode();


#define SuperClass ((ConstraintWidgetClass)&constraintClassRec)

VPanedClassRec vPanedClassRec = {
   {
/* core class fields */
    /* superclass         */   (WidgetClass) SuperClass,
    /* class name         */   "VPaned",
    /* size               */   sizeof(VPanedRec),
    /* class initialize   */   NULL,
    /* class_part init    */   ClassPartInitialize,
    /* class_inited       */   FALSE,
    /* initialize         */   Initialize,
    /* initialize_hook    */   NULL,
    /* realize            */   Realize,
    /* actions            */   NULL,
    /* num_actions        */   0,
    /* resourses          */   resources,
    /* resource_count     */   XtNumber(resources),
    /* xrm_class          */   NULLQUARK,
    /* compress_motion    */   TRUE,
    /* compress_exposure  */   TRUE,
    /* compress_enterleave*/   TRUE,
    /* visible_interest   */   FALSE,
    /* destroy            */   Destroy,
    /* resize             */   Resize,
    /* expose             */   NULL,
    /* set_values         */   SetValues,
    /* set_values_hook    */   NULL,
    /* set_values_almost  */   XtInheritSetValuesAlmost,
    /* get_values_hook    */   NULL,
    /* accept_focus       */   NULL,
    /* version            */   XtVersion,
    /* callback_private   */   NULL,
    /* tm_table           */   NULL,
    /* query_geometry	  */   XtInheritQueryGeometry,
    /* display_accelerator*/   XtInheritDisplayAccelerator,
    /* extension          */   NULL
   }, {
/* composite class fields */
    /* geometry_manager   */   GeometryManager,
    /* change_managed     */   ChangeManaged,
    /* insert_child       */   InsertChild,
    /* delete_child       */   DeleteChild,
    /* extension          */   NULL
   }, {
/* constraint class fields */
    /* subresourses       */   subresources,
    /* subresource_count  */   XtNumber(subresources),
    /* constraint_size    */   sizeof(VPanedConstraintsRec),
    /* initialize         */   NULL,
    /* destroy            */   NULL,
    /* set_values         */   PaneSetValues,
    /* extension          */   NULL
   }, {
/* vpaned class fields */
    /* SetMinMax          */   SetMinMax,
    /* RefigureMode       */   RefigureMode
   }
};

WidgetClass vPanedWidgetClass = (WidgetClass) &vPanedClassRec;

static void ClassPartInitialize(class)
    WidgetClass class;
{
    VPanedWidgetClass c = (VPanedWidgetClass)class;

    if (c->vpaned_class.SetMinMax == XtInheritSetMinMax)
	c->vpaned_class.SetMinMax = SetMinMax;

    if (c->vpaned_class.RefigureMode == XtInheritRefigureMode)
	c->vpaned_class.RefigureMode = RefigureMode;
}

static Dimension NeedsAdjusting(vpw)
VPanedWidget vpw;
{

   int needed, i;
   int num_panes = vpw->v_paned.num_panes;

   needed = 0;
   for (i = 0; i < num_panes; i++) {
       needed += PaneInfo(vpw->composite.children[i])->dheight + BORDERWIDTH;
   }
   needed -= BORDERWIDTH;

   return (needed > vpw->core.height) ? needed : 0;
}

static void AdjustVPanedHeight(vpw, newheight)
  VPanedWidget vpw;
  Dimension newheight;
{
    Dimension replyWidth, replyHeight;
    XtGeometryResult result;

    if (newheight < 1) newheight = 1;
    result = XtMakeResizeRequest( (Widget)vpw, vpw->core.width, newheight,
				  &replyWidth, &replyHeight );
    if (result == XtGeometryAlmost) {
	result = XtMakeResizeRequest( (Widget)vpw, replyWidth, replyHeight,
				      NULL, NULL );
    }
}


static void ResetHeights(vpw)
  VPanedWidget vpw;
{
    WidgetList children = vpw->composite.children;
    int num_panes = vpw->v_paned.num_panes;
    Widget *childP;
    int i;

    for (childP = children, i = 0; i < num_panes; childP++, i++)
        PaneInfo(*childP)->dheight = (*childP)->core.height;
}

static void RefigureLocations(vpw, index, dir)
  VPanedWidget vpw;
  int index;
  Direction dir;
{
    WidgetList children = vpw->composite.children;
    int num_panes = vpw->v_paned.num_panes;
    int _dir = (dir == UpperPane) ? 1 : (dir == LowerPane) ? -1 : 0;
    Pane pane;
    register Widget *childP;
    Position y;
    Dimension heightused;
    int cdir;
    int i;

    if (num_panes == 0 || !vpw->v_paned.refiguremode)
	return;

    do {
	heightused = 0;
	for (childP = children, i = 0; i < num_panes; childP++, i++) {
	    pane = PaneInfo(*childP);
	    if (pane->dheight < pane->min) pane->dheight = pane->min;
	    if (pane->dheight > pane->max) pane->dheight = pane->max;
	    heightused += pane->dheight + BORDERWIDTH;
	}
        heightused -= BORDERWIDTH;
	if (dir == ThisBorderOnly && heightused != vpw->core.height) {
	    for (childP = children, i = 0; i < num_panes; childP++, i++) {
		pane = PaneInfo(*childP);
		if (pane->dheight != (*childP)->core.height)
		    pane->dheight += vpw->core.height - heightused;
            }
	}
    } while (dir == ThisBorderOnly && heightused != vpw->core.height);

    childP = children + index;
    if (dir == UpperPane) childP++;
    cdir = _dir;
    while (heightused != vpw->core.height) {
	pane = PaneInfo(*childP);
	if (!pane->skip_adjust || cdir != _dir) {
	    Dimension old = pane->dheight;
	    if (heightused < vpw->core.height)
	        pane->dheight+= vpw->core.height - heightused;
	    else
	        if (heightused - vpw->core.height < pane->dheight)
		    pane->dheight-= heightused - vpw->core.height;
	        else
		    pane->dheight = 1;
	    if (pane->dheight < pane->min) pane->dheight = pane->min;
	    if (pane->dheight > pane->max) pane->dheight = pane->max;
	    heightused += (pane->dheight - old);
	}
	childP+= cdir;
	while (childP < children || childP - children >= num_panes) {
	    cdir = -cdir;
	    if (cdir == _dir)
	        goto triplebreak; /* reversed twice */
	    childP = children + index + cdir;
	}
    }
triplebreak:
    y = -BORDERWIDTH;
    for (childP = children, i = 0; i < num_panes; childP++, i++) {
       PaneInfo(*childP)->dy = y;
       y += PaneInfo(*childP)->dheight + BORDERWIDTH;
    }
}


static void CommitNewLocations(vpw)
VPanedWidget vpw;
{
    WidgetList children = vpw->composite.children;
    int num_panes = vpw->v_paned.num_panes;
    register Widget *childP;
    XWindowChanges changes;
    int i;

    changes.stack_mode = Above;

    for (childP = children, i = 0; i < num_panes; childP++, i++) {
	register Pane pane = PaneInfo(*childP);
	register Widget grip = pane->grip;

	XtMoveWidget(*childP, -BORDERWIDTH, pane->dy);
	XtResizeWidget(*childP, vpw->core.width, pane->dheight,
		       (Dimension)BORDERWIDTH);

	/* Move and Display the Grip */
	changes.x = vpw->core.width - vpw->v_paned.grip_indent
		    - grip->core.width - grip->core.border_width*2;
	changes.y = (*childP)->core.y + (*childP)->core.height -
		    grip->core.height/2 - grip->core.border_width + 1;

	/* This should match XtMoveWidget, except that we're
	 * also insuring the grip is Raised in the same request */

	grip->core.x = changes.x;
	grip->core.y = changes.y;

	if (XtIsRealized(pane->grip))
	    XConfigureWindow( XtDisplay(pane->grip), XtWindow(pane->grip),
			      CWX | CWY | CWStackMode, &changes );
    }
}


static void RefigureLocationsAndCommit(vpw, index, dir)
  VPanedWidget vpw;
  Direction dir;
{
    if (vpw->v_paned.refiguremode) {
	RefigureLocations(vpw, index, dir);
	CommitNewLocations(vpw);
    }
}



static void EraseInternalBorders(vpw)
  VPanedWidget vpw;
{
    Widget *childP;
    Widget *children = vpw->composite.children;
    int num_panes = vpw->v_paned.num_panes;

    for (childP = children + 1; childP - children < num_panes; childP++)
	XFillRectangle(XtDisplay(vpw), XtWindow(vpw), vpw->v_paned.invgc,
		       0, (*childP)->core.y, vpw->core.width, BORDERWIDTH);
}

static void DrawInternalBorders(vpw)
  VPanedWidget vpw;
{
    Widget *childP;
    Widget *children = vpw->composite.children;
    int num_panes = vpw->v_paned.num_panes;

    for (childP = children + 1; childP - children < num_panes; childP++)
	XFillRectangle(XtDisplay(vpw), XtWindow(vpw), vpw->v_paned.normgc,
		       0, (*childP)->core.y, vpw->core.width, BORDERWIDTH);
}


static void DrawTrackLines(vpw)
  VPanedWidget vpw;
{
    Widget *childP;
    Pane pane;
    Widget *children = vpw->composite.children;
    int num_panes = vpw->v_paned.num_panes;

    for (childP = children + 1; childP - children < num_panes; childP++) {
        pane = PaneInfo(*childP);
	if (pane->olddy != pane->dy) {
	    XDrawLine(XtDisplay(vpw), XtWindow(vpw), vpw->v_paned.flipgc,
		      0, pane->olddy, (Position) vpw->core.width, pane->olddy);
	    XDrawLine(XtDisplay(vpw), XtWindow(vpw), vpw->v_paned.flipgc,
		      0, pane->dy, (Position) vpw->core.width, pane->dy);
	    pane->olddy = pane->dy;
	}
    }
}

static void EraseTrackLines(vpw)
  VPanedWidget vpw;
{
    Widget *childP;
    Widget *children = vpw->composite.children;
    int num_panes = vpw->v_paned.num_panes;

    for (childP = children + 1; childP - children < num_panes; childP++)
	XDrawLine(XtDisplay(vpw), XtWindow(vpw), vpw->v_paned.flipgc,
		  0, PaneInfo(*childP)->olddy,
		  (Position) vpw->core.width, PaneInfo(*childP)->olddy);
}

/* ARGSUSED */
static void HandleGrip(w, closure, callData)
  Widget	w;
  caddr_t	closure;	/* (paned) Widget (unused) */
  caddr_t	callData;	/* GripCallData */
{
    GripCallData call_data = (GripCallData)callData;
    VPanedWidget vpw = (VPanedWidget)w->core.parent;
    WidgetList children = vpw->composite.children;
    int num_panes = vpw->v_paned.num_panes;
    int     diff, y, index;
    Widget *childP;
    Pane pane;
    char ActionType;
    Direction direction;

    if (call_data->num_params == 0 ||
	(*call_data->params[0] != 'C' && call_data->num_params < 2))
      	XtError( "VPaned GripAction has too few parameters." );

    ActionType = *call_data->params[0];
    if (islower(ActionType)) ActionType = toupper(ActionType);

    switch (call_data->event->xany.type) {
        case ButtonPress:
	case ButtonRelease: y = call_data->event->xbutton.y_root;
			    break;

	case KeyPress:
	case KeyRelease:    y = call_data->event->xkey.y_root;
			    break;

        case MotionNotify:  y = call_data->event->xmotion.y_root;
			    break;

	default:	    y = vpw->v_paned.starty;
    }

    if (ActionType != 'C') {
	int dummyi;

        direction = (Direction)*call_data->params[1];
	dummyi = (int) direction;
	if (isascii(dummyi) && islower(dummyi))
	    direction = (Direction)toupper(dummyi);
    }

    switch (ActionType) {
	case 'S':		/* Start adjustment */
	    vpw->v_paned.whichadd = vpw->v_paned.whichsub = (Pane)NULL;
            if (direction == LowerPane || direction == ThisBorderOnly) {
	        index = PaneIndex(w) + 1;
		pane = PaneInfo(vpw->composite.children[index]);
		while (pane->max == pane->min && index < num_panes - 1)
		    pane = PaneInfo(vpw->composite.children[++index]);
		vpw->v_paned.whichsub = pane;
	    }
            if (direction == UpperPane || direction == ThisBorderOnly) {
	        index = PaneIndex(w);
		pane = PaneInfo(vpw->composite.children[index]);
		while (pane->max == pane->min && index > 0)
		    pane = PaneInfo(vpw->composite.children[--index]);
		vpw->v_paned.whichadd = pane;
	    }
	    XDefineCursor(XtDisplay(w), XtWindow(w), 
			  (direction == UpperPane)
			    ? vpw->v_paned.adjust_upper_cursor
			    : (direction == LowerPane)
			      ? vpw->v_paned.adjust_lower_cursor
			      : vpw->v_paned.adjust_this_cursor
			  );
	    vpw->v_paned.starty = y;
	    EraseInternalBorders(vpw);
	    for (childP = children; childP - children < num_panes; childP++)
		PaneInfo(*childP)->olddy = -99;
	    break;

	case 'M': 
	    ResetHeights( vpw );
	    diff = y - vpw->v_paned.starty;
	    if (vpw->v_paned.whichadd) {
		/* make sure heights don't go negative */
		if ((-diff) >= (int)vpw->v_paned.whichadd->dheight) {
		    /* can't add as much to other pane */
		    diff = -vpw->v_paned.whichadd->dheight + 1;
		    vpw->v_paned.whichadd->dheight = 1;
		}
		else
		    vpw->v_paned.whichadd->dheight += diff;
	    }
	    if (vpw->v_paned.whichsub) {
		if (diff >= (int)vpw->v_paned.whichsub->dheight) {
		    if (vpw->v_paned.whichadd)
			/* added too much to other pane */
			vpw->v_paned.whichadd->dheight -=
			    diff - vpw->v_paned.whichsub->dheight - 1;
		    vpw->v_paned.whichsub->dheight = 1;
		}
		else
		    vpw->v_paned.whichsub->dheight -= diff;
	    }
	    RefigureLocations(vpw, PaneIndex(w), direction);
	    DrawTrackLines(vpw);
	    break;

	case 'C':
	    XDefineCursor(XtDisplay(w), XtWindow(w), vpw->v_paned.grip_cursor);
	    EraseTrackLines(vpw);
	    CommitNewLocations(vpw);
	    DrawInternalBorders(vpw);
	    break;

	default:
	    XtError( "VPaned GripAction(); 1st parameter invalid" );
     }
}


/* Semi-public routines. */

/* The Geometry Manager only allows changes after Realize if
 * allow_resize is True in the constraints record.  It
 * only allows height changes, but offers the requested height
 * as a compromise if both width and height changes were requested.
 * As all good Geometry Managers should, we will return No if the
 * request will have no effect; i.e. when the requestor is already
 * of the desired geometry.
 */

static XtGeometryResult GeometryManager(w, request, reply)
Widget w;
XtWidgetGeometry *request, *reply;
{
    VPanedWidget vpw = (VPanedWidget) w->core.parent;
    XtGeometryMask mask = request->request_mode;
    Dimension old_dheight, new_height;
    XtWidgetGeometry allowed;
    Pane pane = PaneInfo(w);
    
    if (XtIsRealized((Widget)vpw) && !pane->allow_resize)
      return XtGeometryNo;

    if (!(mask & CWHeight) || mask & ~(CWWidth | CWHeight)
	|| request->height == w->core.height) {
        return XtGeometryNo;
    }

    allowed.request_mode = mask;
    allowed.width = vpw->core.width;

    /* try out the new size */
    ResetHeights( vpw );
    old_dheight = pane->dheight;
    pane->dheight = request->height;

    if (new_height = NeedsAdjusting(vpw)) {
        Dimension old_height = vpw->core.height;
        AdjustVPanedHeight( vpw, new_height );
        allowed.height = w->core.height + (vpw->core.height - old_height);
    }
    else
        allowed.height = request->height;

    if (((mask & CWWidth) && request->width != allowed.width)
	|| request->height != allowed.height) {
      pane->dheight = old_dheight;
      *reply = allowed;
      return XtGeometryAlmost;
    }

    if (pane->min > pane->dheight) pane->min = pane->dheight;
    if (pane->max < pane->dheight) pane->max = pane->dheight;
    RefigureLocationsAndCommit(vpw, PaneIndex(pane->grip), UpperPane);
    return XtGeometryDone;
}

/* ARGSUSED */
static void Initialize(request, new)
Widget request, new;
{
    ((VPanedWidget)new)->v_paned.recursively_called = False;
}


static void Realize(w, valueMask, attributes)
Widget w;
Mask *valueMask;
XSetWindowAttributes *attributes;
{
    register VPanedWidget vpw = (VPanedWidget)w;
    WidgetList children = vpw->composite.children;
    int num_children = vpw->composite.num_children;
    unsigned long valuemask;
    XGCValues values;
    Widget *childP;

    attributes->bit_gravity = NorthWestGravity;
    *valueMask |= CWBitGravity;

    XtCreateWindow(w, (unsigned)InputOutput, (Visual *)CopyFromParent,
		   *valueMask, attributes);

    values.foreground = vpw->v_paned.foreground_pixel;
    values.subwindow_mode = IncludeInferiors;
    valuemask = GCForeground | GCSubwindowMode;
    vpw->v_paned.normgc = XtGetGC(w, valuemask, &values);
    values.foreground = vpw->core.background_pixel;
    vpw->v_paned.invgc = XtGetGC(w, valuemask, &values);
    values.function = GXinvert;
    values.plane_mask = vpw->v_paned.foreground_pixel ^
			vpw->core.background_pixel;
#if BORDERWIDTH == 1
    values.line_width = 0;	/* Take advantage of fast server lines. */
#else
    values.line_width = BORDERWIDTH;
#endif
    values.line_style = LineSolid;
    valuemask &= ~GCForeground;
    valuemask |= GCPlaneMask | GCLineWidth | GCLineStyle | GCFunction;
    vpw->v_paned.flipgc = XtGetGC(w, valuemask, &values);

    /* one last time, in case we grew to try to return an
     * XtGeometryAlmost for a child, but the child decided not to grow */
    (*XtClass(w)->core_class.resize) (w);

    /* now we have to make sure all the grips are on above their
     * panes, which means that we have to realize all our children
     * here and now.  If we realize from the beginning of the list,
     * then the grips (which are at the end) will be Above by default. */

    for (childP = children; childP - children < num_children; childP++)
        if (XtIsManaged(*childP)) 
	    XtRealizeWidget( *childP );

} /* Realize */

static void Destroy(w)
Widget w;
{
    register VPanedWidget vpw = (VPanedWidget)w;

    XtDestroyGC( vpw->v_paned.normgc );
    XtDestroyGC( vpw->v_paned.invgc  );
    XtDestroyGC( vpw->v_paned.flipgc );

} /* Destroy */


static void InsertChild(w, args, argcount)
register Widget w;
ArgList args;
Cardinal *argcount;
{
   static Arg gripargs[] = {
      {XtNtranslations, (XtArgVal) 0},
      {XtNcursor, (XtArgVal) NULL},
   };

   VPanedWidget vpw = (VPanedWidget)w->core.parent;
   Pane pane = PaneInfo(w);

   /* insert the child widget in the composite children list with the */
   /* superclass insert_child routine.                                */
   (*SuperClass->composite_class.insert_child)(w, args, argcount);

   if (w->core.widget_class == gripWidgetClass)
       return;

   /* ||| Panes will be added in the order they are created, temporarilly */

   /* Create the Grip Widget */
   gripargs[0].value = (XtArgVal) vpw->v_paned.grip_translations;
   gripargs[1].value = (XtArgVal) vpw->v_paned.grip_cursor;
   pane->grip = XtCreateWidget("grip", gripWidgetClass, (Widget)vpw,
			       gripargs, XtNumber(gripargs));
   XtAddCallback(pane->grip, XtNcallback, HandleGrip, (caddr_t)w);
   PaneInfo(pane->grip)->grip = w;
   PaneInfo(pane->grip)->position = -1;

} /* InsertChild */

static void DeleteChild(w)
Widget w;
{
   VPanedWidget vpw = (VPanedWidget)w->core.parent;

   /* remove the subwidget info and destroy the grip */
   if (w->core.widget_class != gripWidgetClass)
       XtDestroyWidget(PaneInfo(w)->grip);

   /* delete the child widget in the composite children list with the */
   /* superclass delete_child routine.                                */
   (*SuperClass->composite_class.delete_child) (w);

} /* DeleteChild */

static void ChangeManaged(w)
   Widget w;
{
   VPanedWidget vpw = (VPanedWidget)w;
   Widget *children = vpw->composite.children;
   int num_children = vpw->composite.num_children;
   int i, index;
   Dimension needed;
   register Widget *childP;
   WidgetList managed_grips, unmanaged_grips;
   Widget *managedP, *unmanagedP;
   XtWidgetGeometry new_geom, reply_geom;
   int num_panes = 0;

   if (vpw->v_paned.recursively_called++)
       return;

   managed_grips = (WidgetList)XtMalloc((unsigned)sizeof(Widget)*num_children/2);
   unmanaged_grips = (WidgetList)XtMalloc((unsigned)sizeof(Widget)*num_children/2);
   managedP = managed_grips;
   unmanagedP = unmanaged_grips;

   if (vpw->core.width == 0) {	/* can only happen before Realize */
       /* use width of widest child */
       int width = 0;
       int height = vpw->core.height;
       for (childP = children, i = 0; i < num_children; childP++, i++)
	   if (XtIsManaged(*childP) && (*childP)->core.width > width)
	       width = (*childP)->core.width;
       if (width < 1) width = 1;
       if (XtMakeResizeRequest(w, width, height, &width, &height)
	   == XtGeometryAlmost)
	   XtMakeResizeRequest(w, width, height, &width, &height);
   }

   new_geom.request_mode = CWWidth;
   new_geom.width = vpw->core.width;

   /* first make sure the associated grips are {,un}managed as appropriate */
   for (childP = children, i = 0; i < num_children; childP++, i++) {
      if (IsPane(*childP))
	  if (XtIsManaged(*childP)) {
	      Widget grip = PaneInfo(*childP)->grip;
	      XtQueryGeometry( *childP, &new_geom, &reply_geom );
	      AssignMax(reply_geom.height, 1);
	      PaneInfo(*childP)->wp_height = 
		  PaneInfo(*childP)->dheight = reply_geom.height;
	      XtResizeWidget( *childP, vpw->core.width, reply_geom.height,
			      (Dimension)BORDERWIDTH );
	      *managedP++ = grip;
	      num_panes++;
	      if (XtIsRealized(grip))
		  XRaiseWindow( XtDisplay(grip), XtWindow(grip) );
	  } else
	      *unmanagedP++ = PaneInfo(*childP)->grip;
   }
   vpw->v_paned.num_panes = num_panes;

   if (managedP != managed_grips) /* last grip is never managed */
       *unmanagedP++ = *--managedP;

   if (managedP != managed_grips)
       XtManageChildren( managed_grips, (Cardinal)(managedP - managed_grips) );

   if (unmanagedP != unmanaged_grips)
       XtUnmanageChildren( unmanaged_grips,
			   (Cardinal)(unmanagedP - unmanaged_grips) );

   XtFree((caddr_t)managed_grips);
   XtFree((caddr_t)unmanaged_grips);
   vpw->v_paned.recursively_called = False;

   /* now re-sort the list so all managed panes are at the top
    * and set the index values in the associated grips */
   unmanagedP = NULL;
   for (childP = children, index = 0; index < num_panes; childP++) {
       if (!IsPane(*childP) || !XtIsManaged(*childP)) {
	   if (!unmanagedP)	     /* we only keep track of the first */
	       unmanagedP = childP;
       }
       else {			     /* must be a managed pane */
	   PaneInfo(PaneInfo(*childP)->grip)->position = -(index++) - 1;
	   if (unmanagedP) {	     /* if an earlier widget was */
	       Widget child = *unmanagedP;  /* not a managed pane, then swap */
	       *unmanagedP = *childP;
	       *childP = child;
	       childP = unmanagedP;  /* easiest to just back-track */
	       unmanagedP = NULL;    /* in case there is another unmanaged */
	   }
       }
   }

   /* see if the height of the VPaned needs to be adjusted to fit all the panes */
   if (needed = NeedsAdjusting(vpw))
      AdjustVPanedHeight(vpw, needed);

   ResetHeights( vpw );
   RefigureLocations( vpw, 0, UpperPane );

   if (XtIsRealized((Widget)vpw))
       CommitNewLocations( vpw );

} /* ChangeManaged */

static void Resize(w)
    Widget w;
{
    VPanedWidget vpw = (VPanedWidget)w;
    WidgetList children = vpw->composite.children;
    int num_panes = vpw->v_paned.num_panes;
    Widget *childP;
    int i;
    XtWidgetGeometry intended, preferred;

    intended.request_mode = CWWidth;
    intended.width = vpw->core.width;
    for (childP = children, i = 0; i < num_panes; childP++, i++) {
	XtQueryGeometry( *childP, &intended, &preferred );
	PaneInfo(*childP)->dheight = 
	    PaneInfo(*childP)->wp_height = preferred.height;
    }

    RefigureLocationsAndCommit( vpw, num_panes - 1, LowerPane );

} /* Resize */


/* ARGSUSED */
static Boolean SetValues(oldvpw, requestvpw, newvpw)
    VPanedWidget oldvpw, requestvpw, newvpw;
{
    return (FALSE);
} /* SetValues */


/* ARGSUSED */
static Boolean PaneSetValues(old, request, new)
    Widget old, request, new;
{
    Pane old_pane = PaneInfo(old);
    Pane new_pane = PaneInfo(new);

    if (old_pane->min != new_pane->min || old_pane->max != new_pane->max)
	XtPanedSetMinMax(new, (int)new_pane->min, (int)new_pane->max);

  /* ||| need to look at position changes */
    return False;
}

/* Change the min and max size of the given sub window. */

static void SetMinMax(panedWidget, min, max)
    Widget panedWidget;
    int    min, max;
{
    Pane pane = PaneInfo(panedWidget);

    pane->min = min;
    pane->max = max;

    if (XtIsRealized(panedWidget->core.parent) && XtIsManaged(panedWidget))
	RefigureLocationsAndCommit( (VPanedWidget)panedWidget->core.parent,
				    PaneIndex(pane->grip),
				    UpperPane );
}


static void RefigureMode(w, mode)
    Widget w;
    Boolean  mode;
{
    VPanedWidget vpw = (VPanedWidget)w;
    int num_panes = vpw->v_paned.num_panes;

    vpw->v_paned.refiguremode = mode;
    if (XtIsRealized(w) && mode && num_panes > 0) {
        RefigureLocationsAndCommit( vpw, num_panes - 1, LowerPane );
    }
}


void XtPanedSetMinMax(panedWidget, min, max)
    Widget panedWidget;
    int    min, max;
{
    (*((VPanedWidgetClass)XtClass(panedWidget->core.parent))->
      vpaned_class.SetMinMax) (panedWidget, min, max);
}

/* XXX
 * This is a kludge (for now?) because various compilers barf when
 * comparing (void (*)()) entities.
 */
typedef int (*XtComp)();

void XtInheritPanedSetMinMax(panedWidget, min, max)
    Widget panedWidget;
    int    min, max;
{
    VPanedWidgetClass class = (VPanedWidgetClass)XtClass(panedWidget);
    void (*SetMinMax)();

    if (! XtIsSubclass(panedWidget, vPanedWidgetClass)) {
	XtError("Must be a subclass of VPaned to inherit SetMinMax");
    }

    SetMinMax =
      ((VPanedWidgetClass)XtSuperclass(panedWidget))->vpaned_class.SetMinMax;

    if (SetMinMax) {
  	/* Here we fake runtime casting by changing the class field in  */
	/* the current class record and resetting it when we return.    */
	XtClass(panedWidget) = XtSuperclass(panedWidget);
	(*SetMinMax) (panedWidget, min, max);
	XtClass(panedWidget) = (WidgetClass)class;
    }

    if ((XtComp)class->vpaned_class.SetMinMax == (XtComp)XtInheritPanedSetMinMax)
      class->vpaned_class.SetMinMax = SetMinMax;
}


void XtPanedSetRefigureMode(w, mode)
    Widget w;
    Boolean mode;
{
    (*((VPanedWidgetClass)XtClass(w))->vpaned_class.RefigureMode) (w, mode);
}


void XtInheritPanedSetRefigureMode(w, mode)
    Widget w;
    Boolean mode;
{
    VPanedWidgetClass class = (VPanedWidgetClass)XtClass(w);
    void (*RefigureMode)();

    if (! XtIsSubclass(w, vPanedWidgetClass)) {
	XtError("Must be a subclass of VPaned to inherit SetRefigureMode");
    }

    RefigureMode =
      ((VPanedWidgetClass)XtSuperclass(w))->vpaned_class.RefigureMode;

    if (RefigureMode) {
  	/* Here we fake runtime casting by changing the class field in  */
	/* the current class record and resetting it when we return.    */
	XtClass(w) = XtSuperclass(w);
	(*RefigureMode) (w, mode);
	XtClass(w) = (WidgetClass)class;
    }

    if ((XtComp)class->vpaned_class.RefigureMode == (XtComp)XtInheritPanedSetRefigureMode)
      class->vpaned_class.RefigureMode = RefigureMode;
}


/* Get the min and max size of the given sub window. */

void XtPanedGetMinMax(panedWidget, min, max)
    Widget panedWidget;
    int    *min, *max;
{
    Pane pane = PaneInfo(panedWidget);

    *min = pane->min;
    *max = pane->max;
}

int XtPanedGetNumSub(w)
    Widget w;
{
    return ((VPanedWidget)w)->v_paned.num_panes;
}

void XtPanedAllowResize(panedWidget, allow_resize)
     Widget panedWidget;
     Boolean allow_resize;
{
    Pane pane = PaneInfo(panedWidget);

    pane->allow_resize = allow_resize;
}
