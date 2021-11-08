#ifndef lint
static char Xrcsid[] = "$XConsortium: Intrinsic.c,v 1.122 88/09/22 16:04:29 swick Exp $";
/* $oHeader: Intrinsic.c,v 1.4 88/08/18 15:40:35 asente Exp $ */
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

#define INTRINSIC_C

#include "IntrinsicI.h"
#include "StringDefs.h"


static void SetAncestorSensitive();

Boolean XtIsSubclass(widget, widgetClass)
    Widget    widget;
    WidgetClass widgetClass;
{
    register WidgetClass w;

    for (w = widget->core.widget_class; w != NULL; w = w->core_class.superclass)
	if (w == widgetClass) return (TRUE);
    return (FALSE);
} /* XtIsSubclass */

Bool _XtClassIsSubclass(subWidgetClass,widgetClass)
    WidgetClass  subWidgetClass,widgetClass;
{
    register WidgetClass w;
    for (w=subWidgetClass;w != NULL;
                          w = w->core_class.superclass)
        if (w == widgetClass) return (TRUE);
    return (FALSE);
}; /*_XtClassIsSubclass */
    

static void ComputeWindowAttributes(widget,value_mask,values)
    Widget		 widget;
    XtValueMask		 *value_mask;
    XSetWindowAttributes *values;
{
    *value_mask = CWEventMask;
    (*values).event_mask = XtBuildEventMask(widget);
    if (widget->core.background_pixmap != XtUnspecifiedPixmap) {
	*value_mask |= CWBackPixmap;
	(*values).background_pixmap = widget->core.background_pixmap;
    } else {
	*value_mask |= CWBackPixel;
	(*values).background_pixel = widget->core.background_pixel;
    }
    if (widget->core.border_pixmap != XtUnspecifiedPixmap) {
	*value_mask |= CWBorderPixmap;
	(*values).border_pixmap = widget->core.border_pixmap;
    } else {
	*value_mask |= CWBorderPixel;
	(*values).border_pixel = widget->core.border_pixel;
    }
    if (widget->core.widget_class->core_class.expose == (XtExposeProc) NULL) {
	/* Try to avoid redisplay upon resize by making bit_gravity the same
	   as the default win_gravity */
	*value_mask |= CWBitGravity;
	(*values).bit_gravity = NorthWestGravity;
    }
} /* ComputeWindowAttributes */

static void CallChangeManaged(widget)
    register Widget		widget;
{
    register Cardinal		i;
    XtWidgetProc		change_managed;
    register WidgetList		children;
    int    			managed_children = 0;

    register CompositePtr cpPtr;
    register CompositePartPtr clPtr;
   
    if (XtIsComposite (widget)) {
	cpPtr = (CompositePtr)&((CompositeWidget) widget)->composite;
        clPtr = (CompositePartPtr)&((CompositeWidgetClass)
                   widget->core.widget_class)->composite_class;
    } else if (XtIsCompositeObject(widget)) {
        cpPtr = (CompositePtr)&((CompositeObject) widget)->composite;
        clPtr = (CompositePartPtr)&((CompositeObjectClass)
                  widget->core.widget_class)->composite_class;
    } else return;

    children = cpPtr->children;
    change_managed = clPtr->change_managed;

    /* CallChangeManaged for all children */
    for (i = cpPtr->num_children; i != 0; --i) {
	CallChangeManaged (children[i-1]);
	if (XtIsManaged(children[i-1])) managed_children++;
    }

    if (change_managed != NULL && managed_children != 0) {
	(*change_managed) (widget);
    }
} /* CallChangeManaged */


static void MapChildren(cwp)
    CompositePart *cwp;
{
    Cardinal i;
    WidgetList children;
    register Widget child;

    children = cwp->children;
    for (i = 0; i <  cwp->num_children; i++) {
	child = children[i];
	if (XtIsWindowObject (child)){
	    if (child->core.managed && child->core.mapped_when_managed) {
		XtMapWidget (children[i]);
	    }
	} else if (XtIsCompositeObject(child)) {
	    MapChildren(&((CompositeObject)child)->composite);
	}
    }
} /* MapChildren */


static Boolean ShouldMapAllChildren(cwp)
    CompositePart *cwp;
{
    Cardinal i;
    WidgetList children;
    register Widget child;

    children = cwp->children;
    for (i = 0; i < cwp->num_children; i++) {
	child = children[i];
	if (XtIsWindowObject(child)) {
	    if (XtIsRealized(child) && (! (child->core.managed 
					  && child->core.mapped_when_managed))){
		    return False;
	    }
	} else if (XtIsCompositeObject(child)) {
	    if (! ShouldMapAllChildren(&((CompositeObject)child)->composite)) {
		return False;
	    }
	}
    }

    return True;
} /* ShouldMapAllChildren */


static void RealizeWidget(widget)
    register Widget		widget;
{
    register CompositePart	*cwp;
    XtValueMask			value_mask;
    XSetWindowAttributes	values;
    register Cardinal		i;
    XtRealizeProc		realize;
    register WidgetList		children;
    Window			window;
    Display			*dpy;

    if (XtIsWindowObject(widget)) {
	if (XtIsRealized(widget)) return;

	dpy = XtDisplay(widget);

	if (widget->core.tm.proc_table == NULL)
	    _XtBindActions(widget, &widget->core.tm,0);
	_XtInstallTranslations(widget, widget->core.tm.translations);

	ComputeWindowAttributes (widget, &value_mask, &values);
	realize = widget->core.widget_class->core_class.realize;
	if (realize == NULL)
	    XtErrorMsg("invalidProcedure","realizeProc","XtToolkitError",
                "No realize class procedure defined",
                  (String *)NULL, (Cardinal *)NULL);
	else (*realize) (widget, &value_mask, &values);
	window = XtWindow(widget);

	_XtRegisterAsyncHandlers(widget);
	_XtRegisterGrabs(widget,&widget->core.tm);
	_XtRegisterWindow (window, widget);
    } else {
	/* No window associated with object, look up parent chain */
	Widget parent;
	parent = widget->core.parent;
	while (parent != NULL && ! XtIsWindowObject(parent)) {
	    parent = parent->core.parent;
	}
	if (parent == NULL) { /* Should never happen */
	    XtErrorMsg("invalidParent", "realize", "XtToolkitError",
		"Application shell is not a windowed widget?",
		(String *) NULL, (Cardinal *)NULL);
	} else {
	    window = XtWindow(parent);
	    dpy = XtDisplay(parent);
	}
    }

    cwp = NULL;
    if (XtIsComposite (widget)) {
	cwp = &(((CompositeWidget) widget)->composite);
    } else if (XtIsCompositeObject(widget)) {
	cwp = &(((CompositeObject) widget)->composite);
    }
    if (cwp != NULL) {
	children = cwp->children;
	/* Realize all children */
	for (i = cwp->num_children; i != 0; --i) {
	    RealizeWidget (children[i-1]);
	}
	/* Map children that are managed and mapped_when_managed */

	if (cwp->num_children != 0) {
	    if (ShouldMapAllChildren(cwp)) {
		XMapSubwindows (dpy, window);
	    } else {
		MapChildren(cwp);
	    }
	}
    }

    /* If this is the application's popup shell, map it */
    if (widget->core.parent == NULL && widget->core.mapped_when_managed) {
	XtMapWidget (widget);
    }
} /* RealizeWidget */

void XtRealizeWidget (widget)
    register Widget		widget;
{
    if (XtIsRealized (widget)) return;

    CallChangeManaged(widget);
    RealizeWidget(widget);
} /* XtRealizeWidget */


static void UnrealizeWidget(widget)
    register Widget		widget;
{
    register CompositeWidget	cw;
    register Cardinal		i;
    register WidgetList		children;

    if (! XtIsRealized(widget)) return;

    /* If this is the application's popup shell, unmap it? */
    /* no, the window is being destroyed */

    /* Recurse on children */
    if (XtIsComposite (widget)) {
	cw = (CompositeWidget) widget;
	children = cw->composite.children;
	/* Unrealize all children */
	for (i = cw->composite.num_children; i != 0; --i) {
	    UnrealizeWidget (children[i-1]);
	}
	/* Unmap children that are managed and mapped_when_managed? */
	/* No, it's ok to be managed and unrealized as long as your parent */
	/* is unrealized. XtUnrealize widget makes sure the "top" widget */
	/* is unmanaged, we can ignore all descendents */
    }

    /* Unregister window */
    _XtUnregisterWindow(XtWindow(widget), widget);

    /* Remove Event Handlers */
    /* remove async handlers, how? */
    /* remove grabs. Happens automatically when window is destroyed. */

    /* Destroy X Window, done at outer level with one request */
    widget->core.window = NULL;

    /* Unbind actions? Nope, we check in realize to see if done. */
    /* Uninstall Translations? */
    XtUninstallTranslations(widget);

} /* UnrealizeWidget */


void XtUnrealizeWidget (widget)
    register Widget		widget;
{
    Window window = XtWindow(widget);

    if (! XtIsRealized (widget)) return;

    if (widget->core.parent != NULL) XtUnmanageChild(widget);

    UnrealizeWidget(widget);

    if (window != NULL) XDestroyWindow(XtDisplay(widget), window);
} /* XtUnrealizeWidget */


void _XtInherit()

{
    XtErrorMsg("invalidProcedure","inheritanceProc","XtToolkitError",
            "Unresolved inheritance operation",
              (String *)NULL, (Cardinal *)NULL);
}

void XtCreateWindow(widget, window_class, visual, value_mask, attributes)
    Widget		 widget;
    unsigned int	 window_class;
    Visual		 *visual;
    Mask		 value_mask;
    XSetWindowAttributes *attributes;
{
    if (widget->core.window == None) {
	if (widget->core.width == 0 || widget->core.height == 0) {
	    Cardinal count = 1;
	    XtErrorMsg("invalidDimension", "xtCreateWindow", "XtToolkitError",
		       "Widget %s has zero width and/or height",
		       &widget->core.name, &count);
	}
	widget->core.window =
	    XCreateWindow (
		XtDisplay (widget),
		(widget->core.parent ?
		    widget->core.parent->core.window :
		    widget->core.screen->root),
		(int)widget->core.x, (int)widget->core.y,
		(unsigned)widget->core.width, (unsigned)widget->core.height,
		(unsigned)widget->core.border_width, (int) widget->core.depth,
		window_class, visual, value_mask, attributes);
    }
} /* XtCreateWindow */
			
void XtSetSensitive(widget, sensitive)
    register Widget widget;
    Boolean	    sensitive;
{
    Arg			args[1];
    register Cardinal   i;
    register WidgetList children;

    if (widget->core.sensitive == sensitive) return;

    XtSetArg(args[0], XtNsensitive, sensitive);
    XtSetValues(widget, args, XtNumber(args));

    /* If widget's ancestor_sensitive is TRUE, propagate new sensitive to
       children's ancestor_sensitive; else do nothing as children's
       ancestor_sensitive is already FALSE */
    
    if (widget->core.ancestor_sensitive && XtIsComposite (widget)) {
	children = ((CompositeWidget) widget)->composite.children;
	for (i = 0; i < ((CompositeWidget)widget)->composite.num_children; i++){
	    SetAncestorSensitive (children[i], sensitive);
	}
    }
} /* XtSetSensitive */

static void SetAncestorSensitive(widget, ancestor_sensitive)
    register Widget widget;
    Boolean	    ancestor_sensitive;
{
    Arg			args[1];
    register Cardinal   i;
    register WidgetList children;

    if (widget->core.ancestor_sensitive == ancestor_sensitive) return;

    XtSetArg(args[0], XtNancestorSensitive, ancestor_sensitive);
    XtSetValues(widget, args, XtNumber(args));

    /* If widget's sensitive is TRUE, propagate new ancestor_sensitive to
       children's ancestor_sensitive; else do nothing as children's
       ancestor_sensitive is already FALSE */
    
    if (widget->core.sensitive && XtIsComposite(widget)) {
	children = ((CompositeWidget) widget)->composite.children;
	for (i = 0; i < ((CompositeWidget)widget)->composite.num_children; i++){
	    SetAncestorSensitive (children[i], ancestor_sensitive);
	}
    }
} /* SetAncestorSensitive */

/* ---------------- XtNameToWidget ----------------- */

static Widget NameListToWidget(root, names)
    register Widget root;
    XrmNameList     names;
{
    register Cardinal   i;
    register WidgetList children;
    register XrmName    name;

    name = *names;
    if (name == NULLQUARK) return root;
    if (XtIsComposite(root)) {
        children = ((CompositeWidget) root)->composite.children;
        for (i = 0;
                i < ((CompositeWidget) root)->composite.num_children; i++) {
            if (name == children[i]->core.xrm_name)
	        return NameListToWidget(children[i], &names[1]);
        }
    }
    children = root->core.popup_list;
    for (i = 0; i < root->core.num_popups; i++) {
	if (name == children[i]->core.xrm_name)
	    return NameListToWidget(children[i], &names[1]);
    }
    return NULL;
} /* NameListToWidget */

Widget XtNameToWidget(root, name)
    Widget root;
    String name;
{
    XrmName	names[100];
    XrmStringToNameList(name, names);
    if (names[0] == NULLQUARK) return NULL;
    return NameListToWidget(root, names);
} /* XtNameToWidget */

/* Define user versions of intrinsics macros */

#undef XtDisplay

Display *XtDisplay(widget)
	Widget widget;
{
	return widget->core.screen->display;
}

#undef XtScreen

Screen *XtScreen(widget)
	Widget widget;
{
	return widget->core.screen;
}

#undef XtWindow

Window XtWindow(widget)
	Widget widget;
{
	return widget->core.window;
}

#undef XtSuperclass

WidgetClass XtSuperclass(widget)
	Widget widget;
{
	return XtClass(widget)->core_class.superclass;
}

#undef XtClass

WidgetClass XtClass(widget)
	Widget widget;
{
	return widget->core.widget_class;
}

#undef XtIsManaged

Boolean XtIsManaged(widget)
	Widget widget;
{
	return widget->core.managed;
}

#undef XtIsRealized

Boolean XtIsRealized (widget)
	Widget   widget;
{
	return widget->core.window != NULL;
} /* XtIsRealized */

#undef XtIsSensitive

Boolean XtIsSensitive(widget)
	Widget	widget;
{
	return widget->core.sensitive && widget->core.ancestor_sensitive;
}

#undef XtParent

Widget XtParent(widget)
	Widget widget;
{
	return widget->core.parent;
}

