#ifndef lint
static char Xrcsid[] = "$XConsortium: Dialog.c,v 1.18 88/09/06 16:41:13 jim Exp $";
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

/* NOTE: THIS IS NOT A WIDGET!  Rather, this is an interface to a widget.
   It implements policy, and gives a (hopefully) easier-to-use interface
   than just directly making your own form. */


#include <X11/Xlib.h>
#include <X11/Xos.h>
#include <X11/IntrinsicP.h>
#include <X11/XawMisc.h>
#include <X11/StringDefs.h>
#include <X11/AsciiText.h>
#include <X11/Command.h>
#include <X11/Label.h>
#include <X11/DialogP.h>


static XtResource resources[] = {
  {XtNlabel, XtCLabel, XtRString, sizeof(String),
     XtOffset(DialogWidget, dialog.label), XtRString, NULL},
  {XtNvalue, XtCValue, XtRString, sizeof(String),
     XtOffset(DialogWidget, dialog.value), XtRString, NULL},
  {XtNmaximumLength, XtCMax, XtRInt, sizeof(int),
     XtOffset(DialogWidget, dialog.max_length), XtRString, "256"}
};

static void Initialize(), ConstraintInitialize();
static Boolean SetValues();

DialogClassRec dialogClassRec = {
  { /* core_class fields */
    /* superclass         */    (WidgetClass) &formClassRec,
    /* class_name         */    "Dialog",
    /* widget_size        */    sizeof(DialogRec),
    /* class_initialize   */    NULL,
    /* class_part init    */    NULL,
    /* class_inited       */    FALSE,
    /* initialize         */    Initialize,
    /* initialize_hook    */    NULL,
    /* realize            */    XtInheritRealize,
    /* actions            */    NULL,
    /* num_actions        */    0,
    /* resources          */    resources,
    /* num_resources      */    XtNumber(resources),
    /* xrm_class          */    NULLQUARK,
    /* compress_motion    */    TRUE,
    /* compress_exposure  */    TRUE,
    /* compress_enterleave*/    TRUE,
    /* visible_interest   */    FALSE,
    /* destroy            */    NULL,
    /* resize             */    XtInheritResize,
    /* expose             */    XtInheritExpose,
    /* set_values         */    SetValues,
    /* set_values_hook    */    NULL,
    /* set_values_almost  */    XtInheritSetValuesAlmost,
    /* get_values_hook    */    NULL,
    /* accept_focus       */    NULL,
    /* version            */    XtVersion,
    /* callback_private   */    NULL,
    /* tm_table           */    NULL,
    /* query_geometry     */	XtInheritQueryGeometry,
    /* display_accelerator*/	XtInheritDisplayAccelerator,
    /* extension          */	NULL
  },
  { /* composite_class fields */
    /* geometry_manager   */   XtInheritGeometryManager,
    /* change_managed     */   XtInheritChangeManaged,
    /* insert_child       */   XtInheritInsertChild,
    /* delete_child       */   XtInheritDeleteChild,
    /* extension          */   NULL
  },
  { /* constraint_class fields */
    /* subresourses       */   NULL,
    /* subresource_count  */   0,
    /* constraint_size    */   sizeof(DialogConstraintsRec),
    /* initialize         */   ConstraintInitialize,
    /* destroy            */   NULL,
    /* set_values         */   NULL,
    /* extension          */   NULL
  },
  { /* form_class fields */
    /* empty              */   0
  },
  { /* dialog_class fields */
    /* empty              */   0
  }
};

WidgetClass dialogWidgetClass = (WidgetClass)&dialogClassRec;


/* ARGSUSED */
static void Initialize(request, new)
Widget request, new;
{
    DialogWidget dw = (DialogWidget)new;
    static Arg label_args[] = {
	{XtNlabel, (XtArgVal)NULL},
	{XtNborderWidth, (XtArgVal) 0}
    };
    static Arg text_args[] = {
	{XtNwidth, (XtArgVal)NULL},
	{XtNstring, (XtArgVal)NULL},
	{XtNlength, (XtArgVal)0},
	{XtNfromVert, (XtArgVal)NULL},
	{XtNresizable, (XtArgVal)TRUE},
	{XtNtextOptions, (XtArgVal)(resizeWidth | resizeHeight)},
	{XtNeditType, (XtArgVal)XttextEdit},
	{XtNright, (XtArgVal)XtChainRight}
    };
    Widget children[2], *childP = children;

    label_args[0].value = (XtArgVal)dw->dialog.label;
    dw->dialog.labelW = XtCreateWidget( "label", labelWidgetClass, new,
				        label_args, XtNumber(label_args) );
    *childP++ = dw->dialog.labelW;

    if (dw->dialog.value) {
        String initial_value = dw->dialog.value;
	Cardinal length = Max( dw->dialog.max_length, strlen(initial_value) );
	dw->dialog.value = XtMalloc( length );
	strcpy( dw->dialog.value, initial_value );
	text_args[0].value = (XtArgVal)dw->dialog.labelW->core.width; /*|||hack*/
	text_args[1].value = (XtArgVal)dw->dialog.value;
	text_args[2].value = (XtArgVal)length;
	text_args[3].value = (XtArgVal)dw->dialog.labelW;
	dw->dialog.valueW = XtCreateWidget("value",asciiStringWidgetClass,new,
					   text_args, XtNumber(text_args) );
	*childP++ = dw->dialog.valueW;
#ifdef notdef
	static int grabfocus;
	static Resource resources[] = {
	    {XtNgrabFocus, XtCGrabFocus, XtRBoolean, sizeof(int),
		 (caddr_t)&grabfocus, (caddr_t)NULL}
	};
	XrmNameList names;
	XrmClassList classes;
	grabfocus = FALSE;
	XtGetResources(dpy, resources, XtNumber(resources), args, argCount,
		       parent, "dialog", "Dialog", &names, &classes);
	XrmFreeNameList(names);
	XrmFreeClassList(classes);
	if (grabfocus) XSetInputFocus(dpy, data->value, RevertToParent,
				      CurrentTime); /* !!! Hackish. |||*/
#endif notdef
    } else {
        dw->dialog.valueW = NULL;
    }

    XtManageChildren( children, (Cardinal)(childP - children) );
}


/* ARGSUSED */
static void ConstraintInitialize(request, new)
Widget request, new;
{
    DialogWidget dw = (DialogWidget)new->core.parent;
    WidgetList children = dw->composite.children;
    DialogConstraints constraint = (DialogConstraints)new->core.constraints;
    Widget *childP;

    if (!XtIsSubclass(new, commandWidgetClass))	/* if not a button */
	return;					/* then just use defaults */

    constraint->form.left = constraint->form.right = XtChainLeft;
    constraint->form.vert_base = dw->dialog.valueW
				 ? dw->dialog.valueW
				 : dw->dialog.labelW;

    if (dw->composite.num_children > 1) {
        for (childP = children + dw->composite.num_children - 1;
	     childP >= children; childP-- ) {
	    if (*childP == dw->dialog.labelW || *childP == dw->dialog.valueW)
	        break;
	    if (XtIsManaged(*childP) &&
		XtIsSubclass(*childP, commandWidgetClass)) {
	        constraint->form.horiz_base = *childP;
		break;
	    }
	}
    }
}


/* ARGSUSED */
static Boolean SetValues(current, request, new)
Widget current, request, new;
{
    DialogWidget w = (DialogWidget)new;
    DialogWidget old = (DialogWidget)current;

    if (w->dialog.label != old->dialog.label
	|| (w->dialog.label != NULL
	    && old->dialog.label != NULL
	    && strcmp(w->dialog.label, old->dialog.label))
	)
    {
	Arg args[1];
	XtSetArg( args[1], XtNlabel, w->dialog.label );
	XtSetValues( w->dialog.labelW, args, XtNumber(args) );
    }

    return False;
}


void XtDialogAddButton(dialog, name, function, param)
Widget dialog;
char *name;
void (*function)();
caddr_t param;
{
    DialogWidget parent = (DialogWidget)dialog;
    static XtCallbackRec callback[] = { {NULL, NULL}, {NULL, NULL} };
    static Arg arglist[] = {
	{XtNcallback, (XtArgVal) callback},
	{XtNfromVert, (XtArgVal) NULL},
	{XtNleft, (XtArgVal) XtChainLeft},
	{XtNright, (XtArgVal) XtChainLeft}
    };

    callback[0].callback = function;
    callback[0].closure =  param;

    if (parent->dialog.value)
       arglist[1].value = (XtArgVal) parent->dialog.value;
    else
       arglist[1].value = (XtArgVal) parent->dialog.label;

    XtCreateManagedWidget( name, commandWidgetClass, dialog, 
			   arglist, XtNumber(arglist) );
}


char *XtDialogGetValueString(w)
Widget w;
{
    return ((DialogWidget)w)->dialog.value;
}
