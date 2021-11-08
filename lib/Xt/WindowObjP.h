/* $XConsortium: WindowObjP.h,v 1.6 88/09/26 11:43:54 swick Exp $ */
/* $oHeader: WindowObjP.h,v 1.2 88/08/18 15:56:58 asente Exp $ */
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

#ifndef _Xt_WindowObjP_h_
#define _Xt_WindowObjP_h_

#include "RectObjP.h"

externalref int _XtInheritTranslations;
#define XtObjDisplay(widget)  ((widget)->win_obj.screen->display)

#define XtInheritTranslations  ((String) &_XtInheritTranslations)
#define XtInheritRealize ((XtRealizeProc) _XtInherit)
#define XtInheritResize ((XtWidgetProc) _XtInherit)
#define XtInheritExpose ((XtExposeProc) _XtInherit)
#define XtInheritSetValuesAlmost ((XtAlmostProc) _XtInherit)
#define XtInheritAcceptFocus ((XtAcceptFocusProc) _XtInherit)
#define XtInheritQueryGeometry ((XtGeometryHandler) _XtInherit)
#define XtInheritDisplayAccelerator ((XtStringProc) _XtInherit)

/**********************************************************
 * Window Object Instance Data Structures
 *
 **********************************************************/
/* these fields match CorePart and can not be changed */

typedef struct _WindowObjPart {
    XtEventTable    event_table;        /* private to event dispatcher       */
    XtTMRec         tm;                 /* translation management            */
    XtTranslations  accelerators;       /* accelerator translations          */
    Pixel	    border_pixel;	/* window border pixel		     */
    Pixmap          border_pixmap;	/* window border pixmap or NULL      */
    WidgetList      popup_list;         /* list of popups                    */
    Cardinal        num_popups;         /* how many popups                   */
    String          name;               /* widget resource name              */
    Screen          *screen;            /* window's screen                   */
    Colormap        colormap;           /* colormap                          */
    Window          window;             /* window ID                         */
    Cardinal        depth;              /* number of planes in window        */
    Pixel           background_pixel;   /* window background pixel           */
    Pixmap          background_pixmap;  /* window background pixmap or NULL  */
    Boolean         visible;            /* is window mapped and not occluded?*/
    Boolean         mapped_when_managed;/* map window if it's managed?       */
} WindowObjPart;

typedef struct _WindowObjRec {
    ObjectPart  object;
    RectObjPart rect;
    WindowObjPart win_obj;
}WindowObjRec;


/********************************************************
 * Window Object Class Data Structures
 *
 ********************************************************/
/* these fields match CoreClassPart and can not be changed */
/* ideally these structures would only contain the fields required;
   but because the CoreClassPart cannot be changed at this late date
   extraneous fields are necessary to make the field offsets match */

typedef struct _WindowObjClassPart {

    WidgetClass     superclass;         /* pointer to superclass ClassRec   */
    String          class_name;         /* widget resource class name       */
    Cardinal        widget_size;        /* size in bytes of widget record   */
    XtProc          class_initialize;   /* class initialization proc        */
    XtWidgetClassProc class_part_initialize; /* dynamic initialization      */
    Boolean         class_inited;       /* has class been initialized?      */
    XtInitProc      initialize;         /* initialize subclass fields       */
    XtArgsProc      initialize_hook;    /* notify that initialize called    */
    XtRealizeProc   realize;            /* XCreateWindow for widget         */
    XtActionList    actions;            /* widget semantics name to proc map */
    Cardinal        num_actions;        /* number of entries in actions     */
    XtResourceList  resources;          /* resources for subclass fields    */
    Cardinal        num_resources;      /* number of entries in resources   */
    XrmClass        xrm_class;          /* resource class quarkified        */
    Boolean         compress_motion;    /* compress MotionNotify for widget */
    Boolean         compress_exposure;  /* compress Expose events for widget*/
    Boolean         compress_enterleave;/* compress enter and leave events  */
    Boolean         visible_interest;   /* select for VisibilityNotify      */
    XtWidgetProc    destroy;            /* free data for subclass pointers  */
    XtWidgetProc    resize;             /* geom manager changed widget size */
    XtExposeProc    expose;             /* rediplay window                  */
    XtSetValuesFunc set_values;         /* set subclass resource values     */
    XtArgsFunc      set_values_hook;    /* notify that set_values called    */
    XtAlmostProc    set_values_almost;  /* set_values got "Almost" geo reply */
    XtArgsProc      get_values_hook;    /* notify that get_values called    */
    XtAcceptFocusProc accept_focus;      /* assign input focus to widget     */
    XtVersionType   version;            /* version of intrinsics used       */
    struct _XtOffsetRec *callback_private;/* list of callback offsets       */
    String          tm_table;           /* state machine                    */
    XtGeometryHandler query_geometry;   /* return preferred geometry        */
    XtStringProc    display_accelerator;/* display your accelerator         */
    caddr_t         extension;          /* pointer to extension record      */
}WindowObjClassPart;

typedef struct _WindowObjClassRec {
    WindowObjClassPart window_class;
} WindowObjClassRec;

externalref WindowObjClassRec windowObjClassRec;

#endif /*_Xt_WindowObjP_h_*/
