/* LINTLIBRARY */
#ifndef lint
static char Xrcsid[] = "$XConsortium: WindowObj.c,v 1.12 88/10/21 18:52:27 swick Exp $";
/* $oHeader: WindowObj.c,v 1.5 88/09/01 12:05:03 asente Exp $ */
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

#define WINDOWOBJ
#include "IntrinsicI.h"
#include "StringDefs.h"
/******************************************************************
 *
 * WindowObj Resources
 *
 ******************************************************************/
int XtUnspecifiedPixmap = 2;
static Boolean true = TRUE;
externaldef(xtinherittranslations) int _XtInheritTranslations = NULL;
static XtResource resources[] = {
    {XtNscreen, XtCScreen, XtRPointer, sizeof(int),
      XtOffset(WindowObj,win_obj.screen), XtRCallProc, (caddr_t)XtCopyScreen},
/*XtCopyFromParent does not work for screen because the Display
parameter is not passed through to the XtRCallProc routines */
    {XtNdepth, XtCDepth, XtRInt,sizeof(int),
         XtOffset(WindowObj,win_obj.depth), XtRCallProc, (caddr_t)XtCopyFromParent},
    {XtNcolormap, XtCColormap, XtRPointer, sizeof(Colormap),
      XtOffset(WindowObj,win_obj.colormap), XtRCallProc,(caddr_t)XtCopyFromParent},
    {XtNbackground, XtCBackground, XtRPixel,sizeof(Pixel),
         XtOffset(WindowObj,win_obj.background_pixel),
	 XtRString,(caddr_t)"XtDefaultBackground"},
    {XtNbackgroundPixmap, XtCPixmap, XtRPixmap, sizeof(Pixmap),
         XtOffset(WindowObj,win_obj.background_pixmap),
	 XtRInt, (caddr_t) &XtUnspecifiedPixmap},
    {XtNborderColor, XtCBorderColor, XtRPixel,sizeof(Pixel),
         XtOffset(WindowObj,win_obj.border_pixel),
         XtRString,(caddr_t)"XtDefaultForeground"},
    {XtNborderPixmap, XtCPixmap, XtRPixmap, sizeof(Pixmap),
         XtOffset(WindowObj,win_obj.border_pixmap),
	 XtRInt, (caddr_t) &XtUnspecifiedPixmap},
    {XtNmappedWhenManaged, XtCMappedWhenManaged, XtRBoolean, sizeof(Boolean),
         XtOffset(WindowObj,win_obj.mapped_when_managed),
	 XtRBoolean, (caddr_t)&true},
    {XtNtranslations, XtCTranslations, XtRTranslationTable,
        sizeof(XtTranslations), XtOffset(WindowObj,win_obj.tm.translations),
        XtRTranslationTable, (caddr_t)NULL},
    {XtNaccelerators, XtCAccelerators, XtRAcceleratorTable,
        sizeof(XtTranslations), XtOffset(WindowObj,win_obj.accelerators),
        XtRTranslationTable, (caddr_t)NULL}
    };

static void WindowObjInitialize();
static void WindowObjClassPartInitialize();
static void WindowObjDestroy();
static void WindowObjRealize ();
static Boolean WindowObjSetValues ();
static void WindowObjSetValuesAlmost();

externaldef(windowobjclassrec) WindowObjClassRec windowObjClassRec = {
  {
    /* superclass	  */	(WidgetClass)&rectObjClassRec,
    /* class_name	  */	"WindowObj",
    /* widget_size	  */	sizeof(WindowObjRec),
    /* class_initialize   */    NULL,
    /* class_part_initialize*/	WindowObjClassPartInitialize,
    /* class_inited       */	FALSE,
    /* initialize	  */	WindowObjInitialize,
    /* initialize_hook    */	NULL,		
    /* realize		  */	WindowObjRealize,
    /* actions		  */	NULL,
    /* num_actions	  */	0,
    /* resources	  */	resources,
    /* num_resources	  */	XtNumber(resources),
    /* xrm_class	  */	NULLQUARK,
    /* compress_motion	  */	FALSE,
    /* compress_exposure  */	TRUE,
    /* compress_enterleave*/ 	FALSE,
    /* visible_interest	  */	FALSE,
    /* destroy		  */	WindowObjDestroy,
    /* resize		  */	NULL,
    /* expose		  */	NULL,
    /* set_values	  */	WindowObjSetValues,
    /* set_values_hook    */	NULL,			
    /* set_values_almost  */	WindowObjSetValuesAlmost,  
    /* get_values_hook    */	NULL,			
    /* accept_focus	  */	NULL,
    /* version		  */	XtVersion,
    /* callback_offsets   */    NULL,
    /* tm_table           */    NULL,
    /* query_geometry	    */  NULL,
    /* display_accelerator  */	NULL,
    /* extension	    */  NULL
  }
};

externaldef(WindowObjClass) WindowObjClass windowObjClass
                                                  = &windowObjClassRec;

/*
 * Start of Window object methods
 */


static void WindowObjClassPartInitialize(wc)
    register WidgetClass wc;
{
#ifdef lint
    /* ||| GROSS!!! do the right thing after .h split!!! */
    extern void  XrmCompileResourceList();
    extern Opaque _CompileActionTable();
#endif

    /* We don't need to check for null super since we'll get to object
       eventually, and it had better define them!  */
    register WindowObjClass woc = (WindowObjClass)wc;
    register WindowObjClass super =((WindowObjClass)
                                      woc->window_class.superclass);

    if (woc->window_class.realize == XtInheritRealize) {
	woc->window_class.realize = super->window_class.realize;
    }

    if (woc->window_class.accept_focus == XtInheritAcceptFocus) {
	woc->window_class.accept_focus = super->window_class.accept_focus;
    }


    if (woc->window_class.display_accelerator == XtInheritDisplayAccelerator) {
	woc->window_class.display_accelerator = 
		super->window_class.display_accelerator;
    }
    if (woc->window_class.tm_table == (char *) XtInheritTranslations) {
	woc->window_class.tm_table =
		((WindowObjClass)woc->window_class.superclass)
                    ->window_class.tm_table;
    } else if (woc->window_class.tm_table != NULL) {
	woc->window_class.tm_table =
	      (String) _XtParseTranslationTable(woc->window_class.tm_table);
    }
    if (woc->window_class.actions != NULL) {
	/* Compile the action table into a more efficient form */
        woc->window_class.actions = (XtActionList) _CompileActionTable(
	    woc->window_class.actions, woc->window_class.num_actions);
    }
}
/* ARGSUSED */
static void WindowObjInitialize(requested_widget, new_widget)
    Widget   requested_widget;
    register Widget new_widget;
{
    register WindowObj new_w_widget = (WindowObj)new_widget;
    XtTranslations save;
    new_w_widget->win_obj.window = (Window) NULL;
    new_w_widget->win_obj.visible = TRUE;
    new_w_widget->win_obj.event_table = NULL;
    new_w_widget->win_obj.popup_list = NULL;
    new_w_widget->win_obj.num_popups = 0;
    new_w_widget->win_obj.tm.proc_table = NULL;
    new_w_widget->win_obj.tm.current_state = NULL;
    new_w_widget->win_obj.tm.lastEventTime = 0;
    save = new_w_widget->win_obj.tm.translations;
    new_w_widget->win_obj.tm.translations =
	(XtTranslations)((WindowObjClass)new_w_widget->object.widget_class)
                          ->window_class.tm_table;
    if (save!= NULL) {
        switch ((int)(save->operation)) {
               case XtTableReplace:
                  new_w_widget->win_obj.tm.translations = save;
                  break;
               case XtTableAugment:
                  XtAugmentTranslations(new_widget,save);
                  break;
               case XtTableOverride:
                  XtOverrideTranslations(new_widget,save);
                  break;
        }
     }

}

static void WindowObjRealize(widget, value_mask, attributes)
    Widget		 widget;
    Mask		 *value_mask;
    XSetWindowAttributes *attributes;
{
    XtCreateWindow(widget, (unsigned int) InputOutput,
	(Visual *) CopyFromParent, *value_mask, attributes);
} /* WindowObjRealize */

static void WindowObjDestroy (widget)
     Widget    widget;
{
    int i;

    register WindowObj winObj = (WindowObj) widget;
    XtFree((char *) (winObj->win_obj.name));
/* since resources in arg-lists do not go through type conversion, we
   don't know if the storage belongs to the application or to a
   type converter. We can't free storage associated with resources .

   if (winObj->win_obj.background_pixmap > XtUnspecifiedPixmap)
        XFreePixmap(XtObjDisplay(winObj), winObj->win_obj.background_pixmap);
    if (winObj->win_obj.border_pixmap > XtUnspecifiedPixmap)
        XFreePixmap(XtObjDisplay(winObj), winObj->win_obj.border_pixmap);
*/
    _XtFreeEventTable(&winObj->win_obj.event_table);
    XtFree((char *) winObj->win_obj.tm.proc_table);
    _XtUnregisterWindow(winObj->win_obj.window, (Widget) winObj);

    if (winObj->win_obj.popup_list != NULL)
        XtFree((char *)winObj->win_obj.popup_list);

} /* WindowObjDestroy */

/* ARGSUSED */
static Boolean WindowObjSetValues(old, reference, new)
    Widget old, reference, new;
{
    Boolean redisplay;
    Mask    window_mask;
    XSetWindowAttributes attributes;
    XtTranslations save;

    WindowObj wold = (WindowObj)old;
    WindowObj wnew = (WindowObj)new;
    redisplay = FALSE;
    if  (wold->win_obj.tm.translations != wnew->win_obj.tm.translations) {
        switch (wnew->win_obj.tm.translations->operation) {
            case XtTableAugment:
                save = wnew->win_obj.tm.translations;
                wnew->win_obj.tm.translations = wold->win_obj.tm.translations;
                XtAugmentTranslations(new,save);
                break;
            case XtTableOverride:
                save = wnew->win_obj.tm.translations;
                wnew->win_obj.tm.translations = wold->win_obj.tm.translations;
                XtOverrideTranslations(new,save);
                break;
        }
    }       

    /* Check everything that depends upon window being realized */
    if (XtIsRealized(old)) {
	window_mask = 0;
	/* Check window attributes */
	if (wold->win_obj.background_pixel != wnew->win_obj.background_pixel) {
	   window_mask |= CWBackPixel;
	   redisplay = TRUE;
	}	
	if (wold->win_obj.background_pixmap != wnew->win_obj.background_pixmap) {
	   window_mask |= CWBackPixmap;
	   redisplay = TRUE;
	}	
	if (wold->win_obj.border_pixel != wnew->win_obj.border_pixel)
	   window_mask |= CWBorderPixel;
	if (wold->win_obj.border_pixmap != wnew->win_obj.border_pixmap)
	   window_mask |= CWBorderPixmap;
	if (wold->win_obj.depth != wnew->win_obj.depth) {
	   XtAppWarningMsg(XtWidgetToApplicationContext(wold),
		    "invalidDepth","setValues","XtToolkitError",
               "Can't change widget depth", (String *)NULL, (Cardinal *)NULL);
	   wnew->win_obj.depth = wold->win_obj.depth;
	}
	if (window_mask != 0) {
	    /* Actually change X window attributes */
	    attributes.background_pixmap = wnew->win_obj.background_pixmap;
	    attributes.background_pixel  = wnew->win_obj.background_pixel;
	    attributes.border_pixmap     = wnew->win_obj.border_pixmap;
	    attributes.border_pixel      = wnew->win_obj.border_pixel;
	    XChangeWindowAttributes(
		XtDisplay(new), XtWindow(new), window_mask, &attributes);
	}

	if (wold->win_obj.mapped_when_managed != wnew->win_obj.mapped_when_managed) {
	    Boolean mapped_when_managed = new->core.mapped_when_managed;
	    new->core.mapped_when_managed = !mapped_when_managed;
	    XtSetMappedWhenManaged(new, mapped_when_managed);
	} 

	/* Translation table and state */
	if (wold->win_obj.tm.translations != wnew->win_obj.tm.translations) {
	    XtUninstallTranslations((Widget)wold);
	    wnew->win_obj.tm.proc_table = NULL;
	    _XtBindActions(new, &wnew->win_obj.tm, 0);
	    _XtInstallTranslations((Widget) wnew, wnew->win_obj.tm.translations);
	}
    } /* if realized */

    return redisplay;
} /* WindowObjSetValues */

/*ARGSUSED*/
static void WindowObjSetValuesAlmost(old, new, request, reply)
    Widget		old;
    Widget		new;
    XtWidgetGeometry    *request;
    XtWidgetGeometry    *reply;
{
    *request = *reply;
}
