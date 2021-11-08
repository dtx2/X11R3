#ifndef lint
static char Xrcsid[] = "$XConsortium: Create.c,v 1.48 88/12/02 12:49:46 swick Exp $";
/* $oHeader: Create.c,v 1.5 88/09/01 11:26:22 asente Exp $ */
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
#include "StringDefs.h"
#include "Shell.h"
#include "ShellP.h"
#include <stdio.h>

extern void bcopy();

static void CallClassPartInit(ancestor, wc)
     WidgetClass ancestor, wc;
{
    if (ancestor->core_class.superclass != NULL) {
	CallClassPartInit(ancestor->core_class.superclass, wc);
    }
    if (ancestor->core_class.class_part_initialize != NULL) {
	(*(ancestor->core_class.class_part_initialize)) (wc);
    }
}

static void ClassInit(wc)
    WidgetClass wc;
{
    String param[3];
    Cardinal num_params=3;
    if (wc->core_class.version != XtVersion &&
	    wc->core_class.version != XtVersionDontCheck) {
        param[0] =  wc->core_class.class_name;
	param[1] =  (String)wc->core_class.version;
        param[2] = (String)XtVersion;
	XtWarningMsg("versionMismatch","widget","XtToolkitError",
          "Widget class %s version mismatch:\n  widget %d vs. intrinsics %d.",
          param,&num_params);
	if (wc->core_class.version == (2 * 1000 + 2)) /* MIT R2 */
	    XtErrorMsg("versionMismatch","widget","XtToolkitError",
		       "Widget class %s must be re-compiled.",
		       param, (Cardinal)1);
    }

    if ((wc->core_class.superclass != NULL) 
	    && (!(wc->core_class.superclass->core_class.class_inited)))
 	ClassInit(wc->core_class.superclass);
 
    if (wc->core_class.class_initialize != NULL)
	(*(wc->core_class.class_initialize))();
    CallClassPartInit(wc, wc);
    wc->core_class.class_inited = TRUE;
}

static void CallInitialize (class, req_widget, new_widget, args, num_args)
    WidgetClass class;
    Widget      req_widget;
    Widget      new_widget;
    ArgList     args;
    Cardinal    num_args;
{
    if (class->core_class.superclass)
        CallInitialize (class->core_class.superclass,
	    req_widget, new_widget, args, num_args);
    if (class->core_class.initialize != NULL)
	(*class->core_class.initialize) (req_widget, new_widget);
    if (class->core_class.initialize_hook != NULL)
	(*class->core_class.initialize_hook) (new_widget, args, &num_args);
}

static void CallConstraintInitialize (class, req_widget, new_widget)
    ConstraintWidgetClass class;
    Widget	req_widget, new_widget;
{
    if (class->core_class.superclass != constraintWidgetClass)
	CallConstraintInitialize(
	    (ConstraintWidgetClass) class->core_class.superclass,
	    req_widget, new_widget);
    if (class->constraint_class.initialize != NULL)
        (*class->constraint_class.initialize) (req_widget, new_widget);
}

Widget _XtCreate(
	name, class, widgetClass, parent, default_screen,
	args, num_args, parent_constraint_class)
    char        *name, *class;
    WidgetClass widgetClass;
    Widget      parent;
    Screen*     default_screen;
    ArgList     args;
    Cardinal    num_args;
    ConstraintWidgetClass parent_constraint_class;
        /* NULL if not a subclass of Constraint or if child is popup shell */
{
    register _XtOffsetList  offsetList;
    XtCallbackList          *pCallbacks;
    char                    widget_cache[600];
    Widget                  req_widget;
    char                    constraint_cache[100];
    char                    *req_constraints;
    Cardinal                size;
    register Widget widget;

    if (! (widgetClass->core_class.class_inited))
	ClassInit(widgetClass);
    widget = (Widget) XtMalloc((unsigned)widgetClass->core_class.widget_size);
    widget->core.self = widget;
    widget->core.parent = parent;
    widget->core.widget_class = widgetClass;
    widget->core.xrm_name = StringToName((name != NULL) ? name : "");
    widget->core.being_destroyed =
	(parent != NULL ? parent->core.being_destroyed : FALSE);
    widget->core.constraints = NULL;
    if (parent_constraint_class != NULL) 
       	widget->core.constraints = 
	    (caddr_t) XtMalloc((unsigned)parent_constraint_class->
                       constraint_class.constraint_size);
    if (XtIsWindowObject(widget)) {
	widget->core.name = XtNewString((name != NULL) ? name : "");
        widget->core.screen = default_screen;
        widget->core.tm.translations = NULL;
    };
    if (XtIsSubclass(widget, applicationShellWidgetClass)) {
	ApplicationShellWidget a = (ApplicationShellWidget) widget;
	if (class != NULL) a->application.class = XtNewString(class);
	else a->application.class = widgetClass->core_class.class_name;
	a->application.xrm_class = StringToClass(a->application.class);
    }

    /* fetch resources */
    XtGetResources(widget, args, num_args);

    /* Compile any callback lists into internal form */
    for (offsetList = widget->core.widget_class->core_class.callback_private;
	 offsetList != NULL;
	 offsetList = offsetList->next) {
	 pCallbacks = (XtCallbackList *) ((int)widget - offsetList->offset - 1);
	if (*pCallbacks != NULL) {
	    extern CallbackStruct* _XtCompileCallbackList();
	    *pCallbacks =
		(XtCallbackList) _XtCompileCallbackList(widget, *pCallbacks);
	}
    }

    size = XtClass(widget)->core_class.widget_size;
    req_widget = (Widget) XtStackAlloc(size, widget_cache);
    bcopy ((char *) widget, (char *) req_widget, (int) size);
    CallInitialize (XtClass(widget), req_widget, widget, args, num_args);
    if (parent_constraint_class != NULL) {
	size = parent_constraint_class->constraint_class.constraint_size;
	req_constraints = XtStackAlloc(size, constraint_cache);
	bcopy(widget->core.constraints, req_constraints, (int) size);
	req_widget->core.constraints = (caddr_t) req_constraints;
	CallConstraintInitialize(parent_constraint_class, req_widget, widget);
	XtStackFree(req_constraints, constraint_cache);
    }
    XtStackFree((char *) req_widget, widget_cache);
    return (widget);
}


Widget XtCreateWidget(name, widgetClass, parent, args, num_args)
    String      name;
    WidgetClass widgetClass;
    Widget      parent;
    ArgList     args;
    Cardinal    num_args;
{
    register Widget	    widget;
    ConstraintWidgetClass   cwc;
    XtWidgetProc	    insert_child;
    Screen*                 default_screen;

    if (parent == NULL) {
	XtErrorMsg("invalidParent","xtCreateWidget","XtToolkitError",
                "XtCreateWidget requires non-NULL parent",
                  (String *)NULL, (Cardinal *)NULL);
    } else if (widgetClass == NULL) {
	XtAppErrorMsg(XtWidgetToApplicationContext(parent),
		"invalidClass","xtCreateWidget","XtToolkitError",
                "XtCreateWidget requires non-NULL widget class",
                  (String *)NULL, (Cardinal *)NULL);
    }
    if (XtIsConstraint(parent)) {
	cwc = (ConstraintWidgetClass) parent->core.widget_class;
    } else {
	cwc = NULL;
    }
    default_screen = parent->core.screen;
    widget = _XtCreate(name, (char *)NULL, widgetClass, parent,
                          default_screen,args, num_args, cwc);
    if (XtIsComposite(parent)) {
        insert_child = ((CompositeWidgetClass) parent->core.widget_class)->
	    composite_class.insert_child;

    } else if (XtIsCompositeObject(parent)) {
        insert_child = ((CompositeObjectClass) parent->core.widget_class)->
            composite_class.insert_child;
    } else {
	return(widget);
    }
    if (insert_child == NULL) {
	XtAppErrorMsg(XtWidgetToApplicationContext(parent),
		"nullProc","insertChild","XtToolkitError",
                "NULL insert_child procedure",
                  (String *)NULL, (Cardinal *)NULL);
    } else {
	(*insert_child) (widget);
    }
    return (widget);
}

Widget XtCreateManagedWidget(name, widgetClass, parent, args, num_args)
    String      name;
    WidgetClass widgetClass;
    Widget      parent;
    ArgList     args;
    Cardinal    num_args;
{
    register Widget	    widget;

    widget = XtCreateWidget(name, widgetClass, parent, args, num_args);
    XtManageChild(widget);
    return widget;
}

/*ARGSUSED*/
static void RemovePopupFromParent(widget,closure,call_data)
    Widget  widget;
    caddr_t closure;
    caddr_t call_data;
{
    int i;
    Boolean found = FALSE;
    register Widget parent;
    parent = widget->core.parent;
    if (parent == NULL || parent->core.num_popups == 0)
        XtAppErrorMsg(XtWidgetToApplicationContext(widget),
		"invalidParameter","removePopupFromParent","XtToolkitError",
                "RemovePopupFromParent requires non-NULL popuplist",
                  (String *)NULL, (Cardinal *)NULL);

    for (i=0; i<parent->core.num_popups; i++)
        if (parent->core.popup_list[i] == widget){
            found = TRUE; break;
        }
    if (found == FALSE) {
        XtAppWarningMsg(XtWidgetToApplicationContext(widget),
		  "invalidWidget","removePopupFromParent","XtToolkitError",
                  "RemovePopupFromParent, widget not on parent list",
                   (String *)NULL, (Cardinal *)NULL);
        return;
    }
    if (parent->core.being_destroyed) {
	/* then we're (probably) not the target of the XtDestroyWidget,
	 * so our window won't get destroyed automatically...
	 */
	Window win;
        if ((win = XtWindow(widget)) != NULL)
	    XDestroyWindow( XtDisplay(widget), win );

	return;
	/* don't update parent's popup_list, as we won't then be able to find
	 * this child for Phase2Destroy.  This also allows for the possibility
	 * that a destroy callback higher up in the hierarchy may care to
	 * know that this popup child once existed.
	 */
    }
    parent->core.num_popups--;
    for (/*i=i*/; i<parent->core.num_popups; i++)
        parent->core.popup_list[i]= parent->core.popup_list[i+1];

}

Widget XtCreatePopupShell(name, widgetClass, parent, args, num_args)
    String      name;
    WidgetClass widgetClass;
    Widget      parent;
    ArgList     args;
    Cardinal    num_args;
{
    register Widget widget;
    Screen* default_screen;

    if (parent == NULL) {
	XtErrorMsg("invalidParent","xtCreatePopupShell","XtToolkitError",
                "XtCreatePopupShell requires non-NULL parent",
                  (String *)NULL, (Cardinal *)NULL);
    } else if (widgetClass == NULL) {
	XtAppErrorMsg(XtWidgetToApplicationContext(parent),
		"invalidClass","xtCreatePopupShell","XtToolkitError",
                "XtCreatePopupShell requires non-NULL widget class",
                  (String *)NULL, (Cardinal *)NULL);
    }
    default_screen = parent->core.screen;
    widget = _XtCreate(
        name, (char *)NULL, widgetClass, parent, default_screen,
	args, num_args, (ConstraintWidgetClass) NULL);

    parent->core.popup_list =
	(WidgetList) XtRealloc((caddr_t) parent->core.popup_list,
               (unsigned) (parent->core.num_popups+1) * sizeof(Widget));
    parent->core.popup_list[parent->core.num_popups++] = widget;
    XtAddCallback(
       widget,XtNdestroyCallback,RemovePopupFromParent, (caddr_t)NULL);
    return(widget);
} /* XtCreatePopupShell */

Widget XtAppCreateShell(name, class, widgetClass, display, args, num_args)
    String      name, class;
    WidgetClass widgetClass;
    Display*    display;
    ArgList     args;
    Cardinal    num_args;

{
    if (widgetClass == NULL) {
	XtAppErrorMsg(_XtDisplayToApplicationContext(display),
	       "invalidClass","xtAppCreateShell","XtToolkitError",
               "XtAppCreateShell requires non-NULL widget class",
                 (String *)NULL, (Cardinal *)NULL);
    }

    if (name == NULL)
	name = XrmNameToString(_XtGetPerDisplay(display)->name);

    return _XtCreate(name, class, widgetClass, (Widget)NULL,
	    DefaultScreenOfDisplay(display),
	    args, num_args, (ConstraintWidgetClass) NULL);
} /* XtAppCreateShell */

Widget XtCreateApplicationShell(name, widgetClass, args, num_args)
    String      name;		/* unused in R3 */
    WidgetClass widgetClass;
    ArgList     args;
    Cardinal    num_args;
{
    Display *dpy = _XtDefaultAppContext()->list[0];
    XrmClass class = _XtGetPerDisplay(dpy)->class;

    return XtAppCreateShell(NULL, XrmQuarkToString((XrmQuark)class),
			    widgetClass, dpy, args, num_args);
}

