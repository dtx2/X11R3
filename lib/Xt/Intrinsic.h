/*
* $XConsortium: Intrinsic.h,v 1.88 88/10/18 10:41:40 swick Exp $
* $oHeader: Intrinsic.h,v 1.10 88/09/01 10:33:34 asente Exp $
*/

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

#ifndef _XtIntrinsic_h
#define _XtIntrinsic_h
#include	<X11/Xlib.h>
#include	<X11/Xutil.h>
#include	<X11/Xresource.h>
#include	<X11/Xos.h>

#ifndef NULL
#define NULL 0
#endif

#define externalref extern
#define externaldef(psect)

#ifndef FALSE
#define FALSE 0
#define TRUE 1
#endif

#define XtNumber(arr)		((Cardinal) (sizeof(arr) / sizeof(arr[0])))

typedef char *String;
typedef struct _WidgetRec *Widget;
typedef Widget *WidgetList;
typedef struct _WidgetClassRec *WidgetClass;
typedef struct _CompositeRec *CompositeWidget;
typedef struct _XtActionsRec *XtActionList;
typedef struct _XtEventRec *XtEventTable;
typedef struct _XtBoundAccActionRec *XtBoundAccActions;
typedef unsigned int   Cardinal;
typedef unsigned short  ShortCard;
typedef char	Boolean;
typedef unsigned long	*Opaque;
typedef struct _XtAppStruct *XtAppContext;
typedef unsigned long   XtValueMask;
typedef unsigned long   XtIntervalId;
typedef unsigned long   XtInputId;
typedef unsigned long	XtWorkProcId;
typedef unsigned int    XtGeometryMask;
typedef unsigned long   XtGCMask;   /* Mask of values that are used by widget*/
typedef unsigned long   Pixel;	    /* Index into colormap	        */
typedef short		Position;   /* Offset from 0 coordinate	        */
typedef ShortCard	Dimension;  /* Size in pixels		        */

/****************************************************************
 *
 * System Dependent Definitions
 *
 *
 * XtArgVal ought to be a union of caddr_t, char *, long, int *, and proc *
 * but casting to union types is not really supported.
 *
 * So the typedef for XtArgVal should be chosen such that
 *
 *      sizeof (XtArgVal) >=	sizeof(caddr_t)
 *				sizeof(char *)
 *				sizeof(long)
 *				sizeof(int *)
 *				sizeof(proc *)
 *
 * ArgLists rely heavily on the above typedef.
 *
 ****************************************************************/
typedef long XtArgVal;

#include "Core.h"
#include "Composite.h"
#include "Constraint.h"

/***************************************************************
 *
 * Resource Conversions
 *
 ****************************************************************/

extern void _XtConvertInitialize();

extern void XtStringConversionWarning();
    /* String   from;       */  /* String attempted to convert.     */
    /* String   toType;     */  /* Type attempted to convert it to. */

typedef void (*XtConverter)();
    /* XrmValue		    *args;	    */
    /* Cardinal		    *num_args;       */
    /* XrmValue		    *from;	    */
    /* Xrmvalue		    *to;	    */

typedef enum {
/* address mode     	parameter representation    */
/* ------------     	------------------------    */
    XtAddress,      	/* address		    */
    XtBaseOffset,   	/* offset		    */
    XtImmediate,    	/* constant		    */
    XtResourceString,	/* resource name string     */
    XtResourceQuark	/* resource name quark      */
} XtAddressMode;

typedef struct {
    XtAddressMode   address_mode;
    caddr_t	    address_id;
    Cardinal	    size;
} XtConvertArgRec, *XtConvertArgList;

extern XtConvertArgRec colorConvertArgs[];
extern XtConvertArgRec screenConvertArg[];

extern void XtAppAddConverter();
    /* XtAppContext	    app;	*/
    /* String		    from_type;      */
    /* String		    to_type;	    */
    /* XtConverter      converter;      */
    /* XtConvertArgList     convert_args;   */
    /* Cardinal		    num_args;   */

extern void XtAddConverter();
    /* String		    from_type;      */
    /* String		    to_type;	    */
    /* XtConverter      converter;      */
    /* XtConvertArgList     convert_args;   */
    /* Cardinal		    num_args;   */

extern void XtConvert();
    /* Widget		    widget;	    */
    /* String		    from_type;	    */
    /* XrmValue		    *from;	    */
    /* String		    to_type;	    */
    /* XrmValue		    *to;	    */  /* RETURN */

extern void XtDirectConvert();
    /* XtConverter	    converter;      */
    /* XrmValuePtr	    args;	    */
    /* Cardinal		    num_args;       */
    /* XrmValuePtr	    from;	    */
    /* XrmValue		    *to;	    */  /* RETURN */

/****************************************************************
 *
 * Translation Management
 *
 ****************************************************************/

typedef struct _TranslationData *XtTranslations;
typedef struct _TranslationData *XtAccelerators;
typedef unsigned int Modifiers;

typedef void (*XtActionProc)();
    /* Widget widget; */
    /* XEvent *event; */
    /* String *params; */
    /* Cardinal *num_params; */

typedef XtActionProc* XtBoundActions;

typedef struct _XtActionsRec{
    char    *string;
    XtActionProc proc;
} XtActionsRec;

extern XtTranslations XtParseTranslationTable(); /* source */
    /* String *source; */

extern XtAccelerators XtParseAcceleratorTable();
   /* String *source */

extern void XtOverrideTranslations(); /* widget, new */
    /* Widget widget; */
    /* XtTranslations new; */

extern void XtAugmentTranslations(); /* widget, new */
    /* Widget widget; */
    /* XtTranslations new; */

extern void XtInstallAccelerators();
   /* Widget destination,source */

extern void XtInstallAllAccelerators();
  /* Widget destination,source */

extern void XtUninstallTranslations();
   /* Widget widget */

extern void XtAppAddActions(); /* app, action, num_actions */
   /* XtAppContext app */
   /* XtActionList action */
   /* Cardinal num_actions */

extern void XtAddActions(); /* action, num_actions */
   /* XtActionList action */
   /* Cardinal num_actions */

/***************************************************************
 *
 * Keycode and Keysym procedures for translation management
 *
 ****************************************************************/

typedef void (*XtKeyProc)();
/*
    Display *dpy;
    KeyCode *keycode;
    Modifiers *modifiers;
    Modifiers *modifiers_return;
    KeySym *keysym_return;

*/

extern void XtTranslateKeycode();
/*
    Display *dpy;
    KeyCode keycode;
    Modifiers modifiers;
    Modifiers *modifiers_return;
    KeySym *keysym_return;
*/

extern void XtTranslateKey();
/*
    Display *dpy;
    KeyCode *keycode;
    Modifiers *modifiers;
    Modifiers *modifiers_return;
    KeySym *keysym_return;

*/

extern void XtSetKeyTranslator();
/*  Display *dpy;
    XtKeyProc proc;
*/

typedef void (*XtCaseProc)();
/*
    KeySym *keysym;
    KeySym *lower_return;
    KeySym *upper_return;
*/

extern void XtRegisterCaseConverter();
/*  Display *dpy;
    XtCaseProc proc;
    KeySym start;
    KeySym stop;
*/

extern void XtConvertCase();
/*  Display *dpy;
    KeySym keysym;
    KeySym *lower_return;
    KeySym *upper_return;
*/

/****************************************************************
 *
 * Event Management
 *
 ****************************************************************/

typedef void (*XtEventHandler)(); /* widget, closure, event */
    /* Widget  widget   */
    /* caddr_t closure  */
    /* XEvent  *event;  */

typedef void (*XtAsyncHandler)(); /* closure */
    /* Opaque closure; */

typedef unsigned long EventMask;
#define XtAllEvents ((EventMask) -1L)

extern void XtAddEventHandler(); /* widget, eventMask, nonmaskable, proc, closure */
    /* Widget		widget      */
    /* EventMask        eventMask;  */
    /* Boolean          nonmaskable; */
    /* XtEventHandler   proc;       */
    /* caddr_t		closure ;   */

extern void XtRemoveEventHandler(); /* widget, eventMask, nonmaskable, proc, closure */
    /* Widget		widget      */
    /* EventMask        eventMask;  */
    /* Boolean          nonmaskable;*/
    /* XtEventHandler   proc;       */
    /* caddr_t		closure ;   */

extern void XtAddRawEventHandler(); /* widget, eventMask, nonmaskable, proc, closure */
    /* Widget		widget      */
    /* EventMask        eventMask;  */
    /* Boolean          nonmaskable; */
    /* XtEventHandler   proc;       */
    /* caddr_t		closure ;   */

extern void XtRemoveRawEventHandler(); /* widget, eventMask, nonmaskable, proc, closure */
    /* Widget		widget      */
    /* EventMask        eventMask;  */
    /* Boolean          nonmaskable;*/
    /* XtEventHandler   proc;       */
    /* caddr_t		closure ;   */

extern EventMask XtBuildEventMask(); /* widget */
    /* Widget widget; */

extern void XtAddGrab();
    /* Widget   widget;		    */
    /* Boolean  exclusive;	    */
    /* Boolean  spring_loaded;      */

extern void XtRemoveGrab();
    /* Widget   widget;		    */

extern void XtDispatchEvent(); /* event */
    /* XEvent	*event; */

extern void XtProcessEvent();
    /* XtInputMask mask; */

extern void XtAppProcessEvent();
    /* XtAppContext app; */
    /* XtInputMask mask; */

extern void XtMainLoop();

extern void XtAppMainLoop();
    /* XtAppContext app; */

extern void XtAddExposureToRegion();
    /* XEvent   *event;	*/
    /* Region   region;	*/

extern void XtSetAsyncEventHandler(); /* handler, closure */
    /* XtAsyncHandler handler; */
    /* Opaque	closure; */

extern void XtMakeToolkitAsync();

extern void XtSetKeyboardFocus();
    /* Widget subtree, descendent; */

/****************************************************************
 *
 * Event Gathering Routines
 *
 ****************************************************************/

typedef unsigned long	XtInputMask;

#define XtInputNoneMask		0L
#define XtInputReadMask		(1L<<0)
#define XtInputWriteMask	(1L<<1)
#define XtInputExceptMask	(1L<<2)

typedef void (*XtTimerCallbackProc)();
    /* caddr_t closure         */
    /* XtIntervalId   *id      */

extern XtIntervalId XtAddTimeOut();
    /* unsigned long interval */
    /* XtTimerCallbackProc  proc  */
    /* caddr_t   closure      */

extern XtIntervalId XtAppAddTimeOut();
    /* XtAppContext app */
    /* unsigned long interval */
    /* XtTimerCallbackProc  proc  */
    /* caddr_t   closure       */

extern void XtRemoveTimeOut();
    /* XtIntervalId timer;      */

typedef void (* XtInputCallbackProc)();
    /* caddr_t closure;          */
    /* int    *source;           */
    /* XtInputId  *id;           */

extern XtInputId XtAddInput(); /* source, condition, proc, closure */
    /* int source;		*/
    /* Opaque condition;        */
    /* XtInputCallbackProc proc;*/
    /* caddr_t closure;         */

extern XtInputId XtAppAddInput(); /* source, condition, proc, closure */
    /*  XtAppContext app; */
    /* int source;		*/
    /* Opaque condition;        */
    /* XtInputCallbackProc proc;*/
    /* caddr_t closure;         */

extern void XtRemoveInput(); /* id */
    /* XtInputid id;		*/

extern void XtNextEvent(); /* event */
    /* XEvent *event;		*/

extern void XtAppNextEvent();
    /* XtAppContext appContext	*/
    /* XEvent *event;		*/

extern Boolean XtPeekEvent(); /* event */
    /* XEvent *event;		*/

extern Boolean XtAppPeekEvent();
    /* XtAppContext appContext	*/
    /* XEvent *event;		*/

#define XtIMXEvent		1
#define XtIMTimer		2
#define XtIMAlternateInput	4
#define XtIMAll (XtIMXEvent | XtIMTimer | XtIMAlternateInput)

extern XtInputMask XtPending ();

extern XtInputMask XtAppPending();
    /* XtAppContext appContext	*/

/****************************************************************
 *
 * Random utility routines
 *
 ****************************************************************/

extern Boolean XtIsSubclass ();
    /* Widget       widget;	    */
    /* WidgetClass  widgetClass;    */

#define XtIsComposite(widget)       XtIsSubclass(widget, (WidgetClass) compositeWidgetClass)
#define XtIsConstraint(widget)      XtIsSubclass(widget, (WidgetClass) constraintWidgetClass)
#define XtIsShell(widget)	    XtIsSubclass(widget, (WidgetClass) shellWidgetClass)

extern void XtRealizeWidget ();
    /* Widget    widget      */

void XtUnrealizeWidget (); /* widget */
    /* Widget		widget; */

extern void XtDestroyWidget ();
    /* Widget widget */

extern void XtSetSensitive ();
    /* Widget    widget;    */
    /* Boolean   sensitive; */

extern void XtSetMappedWhenManaged ();
    /* Widget    widget;    */
    /* Boolean   mappedWhenManaged; */

extern Widget XtNameToWidget(); /* root, name */
    /* Widget root; */
    /* String name; */

extern Widget XtWindowToWidget(); /* window */
    /* Display *display; */
    /* Window window; */

/***************************************************************
 *
 * Arg lists
 *
 ****************************************************************/

typedef struct {
    String	name;
    XtArgVal	value;
} Arg, *ArgList;

#define XtSetArg(arg, n, d) \
    ( (arg).name = (n), (arg).value = (XtArgVal)(d) )

extern ArgList XtMergeArgLists(); /* args1, num_args1, args2, num_args2 */
    /* ArgList  args1;      */
    /* Cardinal num_args1;  */
    /* ArgList  args2;      */
    /* Cardinal num_args2;  */

/*************************************************************
 *
 * Information routines
 *
 ************************************************************/

#ifndef _XtIntrinsicP_h

/* We're not included from the private file, so define these */

extern Display *XtDisplay();
    /*	Widget widget;		*/

extern Screen *XtScreen();
    /*	Widget widget;		*/

extern Window XtWindow();
    /*	Widget widget;		*/

extern WidgetClass XtSuperclass();
    /*	Widget widget;		*/

extern WidgetClass XtClass();
    /*	Widget widget;		*/

extern Boolean XtIsManaged();
    /*	Widget widget;		*/

extern Boolean XtIsRealized ();
    /* Widget    widget; */

extern Boolean XtIsSensitive();
    /* Widget	widget; */

extern Widget XtParent();
    /* Widget	widget; */

#endif _XtIntrinsicP_h

#define XtMapWidget(widget)	XMapWindow(XtDisplay(widget), XtWindow(widget))
#define XtUnmapWidget(widget)	\
		XUnmapWindow(XtDisplay(widget), XtWindow(widget))

typedef struct _XtCallbackRec*    XtCallbackList;

typedef void (*XtCallbackProc)();
    /* Widget widget; */
    /* caddr_t closure;  data the application registered */
    /* caddr_t callData; widget instance specific data passed to application*/

typedef struct _XtCallbackRec {
    XtCallbackProc  callback;
    caddr_t         closure;
} XtCallbackRec;

extern void XtAddCallback ();
    /* Widget           widget;         */
    /* String           callback_name;  */
    /* XtCallbackProc   callback;       */
    /* caddr_t          closure;        */

extern void XtRemoveCallback ();
    /* Widget           widget;         */
    /* String           callback_name;  */
    /* XtCallbackProc   callback;       */
    /* caddr_t          closure;        */

extern void XtAddCallbacks ();
    /* Widget           widget;         */
    /* String           callback_name;  */
    /* XtCallbackList   callbacks;      */

extern void XtRemoveCallbacks ();
    /* Widget           widget;         */
    /* String           callback_name;  */
    /* XtCallbackList   callbacks;      */

extern void XtRemoveAllCallbacks ();
    /* Widget           widget;         */
    /* String           callback_name;  */


extern void XtCallCallbacks ();
    /* Widget           widget;         */
    /* String           callback_name;   */
    /* caddr_t          callData;       */

/* These utilities are here on Leo's request. We should think about them */
/* and decide if they really belong in the intrinsics, or a level above */

typedef enum {
        XtCallbackNoList,
        XtCallbackHasNone,
        XtCallbackHasSome
} XtCallbackStatus;

extern XtCallbackStatus XtHasCallbacks(); /* widget, callback_name */
    /* Widget           widget; */
    /* String           callback_name; */



/****************************************************************
 *
 * Geometry Management
 *
 ****************************************************************/


/* Additions to Xlib geometry requests: ask what would happen, don't do it */
#define XtCWQueryOnly   (1 << 7)

/* Additions to Xlib stack modes: don't change stack order */
#define XtSMDontChange  5


typedef struct {
    XtGeometryMask request_mode;
    Position x, y;
    Dimension width, height, border_width;
    Widget sibling;
    int stack_mode;    /* Above, Below, TopIf, BottomIf, Opposite, DontChange */
} XtWidgetGeometry;

typedef enum  {
    XtGeometryYes,        /* Request accepted. */
    XtGeometryNo,         /* Request denied. */
    XtGeometryAlmost,     /* Request denied, but willing to take replyBox. */
    XtGeometryDone        /* Request accepted and done. */
} XtGeometryResult;

extern XtGeometryResult XtMakeGeometryRequest();
    /*  widget, request, reply		*/
    /* Widget   widget;			*/
    /* XtWidgetGeometry *request;	*/
    /* XtWidgetGeometry *reply;		*//* RETURN */

extern XtGeometryResult XtMakeResizeRequest ();
    /* Widget    widget;        */
    /* Dimension width, height; */
    /* Dimension *replyWidth, *replyHeight; */

extern void XtTransformCoords();
    /* register Widget w;       */
    /* Position x, y;           */
    /* register Position *rootx, *rooty; */

extern XtGeometryResult XtQueryGeometry();
    /* Widget   widget;                 */
    /* XtWidgetGeometry *intended;      */
    /* XtWidgetGeometry *reply_return;  */


/* Class record constants */

extern Widget XtCreatePopupShell();

    /* String       name;           */
    /* WidgetClass  widgetClass;    */
    /* Widget       parent;         */
    /* ArgList      args;           */
    /* Cardinal     num_args;       */

typedef enum {XtGrabNone, XtGrabNonexclusive, XtGrabExclusive} XtGrabKind;

extern void XtPopup();
    /* Widget       widget;         */
    /* XtGrabKind   grab_kind;      */

extern void XtCallbackNone();
    /* Widget       widget;         */
    /* caddr_t      closure;        */
    /* caddr_t      call_data;      */

extern void XtCallbackNonexclusive();
    /* Widget       widget;         */
    /* caddr_t      closure;        */
    /* caddr_t      call_data;      */

extern void XtCallbackExclusive();
    /* Widget       widget;         */
    /* caddr_t      closure;        */
    /* caddr_t      call_data;      */

extern void XtPopdown();
    /* Widget       widget;         */

typedef struct {
    Widget  shell_widget;
    Widget  enable_widget;
} XtPopdownIDRec, *XtPopdownID;

extern void XtCallbackPopdown();
    /* Widget       widget;         */
    /* caddr_t      closure;        */
    /* caddr_t      call_data;      */


extern Widget XtCreateWidget ();
    /* String       name;           */
    /* WidgetClass  widget_class;    */
    /* Widget       parent;         */
    /* ArgList      args;           */
    /* Cardinal     num_args;       */

extern Widget XtCreateManagedWidget ();
    /* String       name;           */
    /* WidgetClass  widget_class;    */
    /* Widget       parent;         */
    /* ArgList      args;           */
    /* Cardinal     num_args;       */

extern Widget XtCreateApplicationShell ();
    /* String       name;	    */
    /* WidgetClass  widget_class;   */
    /* ArgList      args;           */
    /* Cardinal     num_args;       */

extern Widget XtAppCreateShell ();
    /* String       name, class;    */
    /* WidgetClass  widget_class;   */
    /* Display      *display        */
    /* ArgList      args;           */
    /* Cardinal     num_args;       */


/****************************************************************
 *
 * Toolkit initialization
 *
 ****************************************************************/

extern void XtToolkitInitialize();

extern void XtDisplayInitialize();
    /* XtAppContext	    appContext */
    /* Display              *d */
    /* String               name, class; */
    /* XrmOptionsDescRec    options;    */
    /* Cardinal             num_options;  */
    /* Cardinal             *argc; */ /* returns count of args not processed */
    /* char                 **argv;     */

extern Widget XtInitialize();
    /* String               name;       */
    /* String               class;      */
    /* XrmOptionsDescRec    options;    */
    /* Cardinal             num_options;  */
    /* Cardinal             *argc; */ /* returns count of args not processed */
    /* char                 **argv;     */

extern Display *XtOpenDisplay();        /* displayName */
   /*   XtAppContext	    appContext */
   /*   String displayName, applName, className;        */
   /*   XrmOptionDescRec *urlist;       */
   /*   Cardinal num_urs;       */
   /*   Cardinal *argc; */
   /*   char *argv[];   */

extern XtAppContext XtCreateApplicationContext();

extern void XtDestroyApplicationContext();
    /* XtAppContext appContext	*/

extern XtAppContext XtWidgetToApplicationContext();
    /* Widget	widget */

extern XrmDatabase XtDatabase();
    /*  Display *dpy; */

extern void XtCloseDisplay();           /* display */
    /* Display *display;        */


typedef struct {
    XrmQuark	xrm_name;	  /* Resource name quark 		*/
    XrmQuark	xrm_class;	  /* Resource class quark 		*/
    XrmQuark	xrm_type;	  /* Resource representation type quark */
    Cardinal	xrm_size;	  /* Size in bytes of representation	*/
    long int	xrm_offset;	  /* -offset-1				*/
    XrmQuark	xrm_default_type; /* Default representation type quark 	*/
    caddr_t	xrm_default_addr; /* Default resource address		*/
} XrmResource, *XrmResourceList;

extern void XtCopyFromParent();

extern void XtCopyDefaultDepth();

extern void XtCopyDefaultColormap();

extern void XtCopyAncestorSensitive();

extern void XtCopyScreen();

typedef struct _XtResource *XtResourceList;

extern void XrmCompileResourceList(); /* resources, num_resources */
    /* XtResourceList resources; */
    /* Cardinal     num_resources; */


extern void XtGetSubresources();
    /* Widget           widget;         */
    /* caddr_t          base;           */
    /* String           name;           */
    /* String           class;          */
    /* XtResourceList   resources;      */
    /* Cardinal         num_resources;  */
    /* ArgList          args;           */
    /* Cardinal         num_args;       */

extern void XtSetValues();
    /* Widget           widget;         */
    /* ArgList          args;           */
    /* Cardinal         num_args;       */

extern void XtGetValues();
    /* Widget           widget;         */
    /* ArgList          args;           */
    /* Cardinal         num_args;       */

extern void XtSetSubvalues();
    /* caddr_t          base;           */
    /* XtResourceList   resources;      */
    /* Cardinal         num_resources;  */
    /* ArgList          args;           */
    /* Cardinal         num_args;       */

extern void XtGetSubvalues();
    /* caddr_t          base;           */
    /* XtResourceList   resources;      */
    /* Cardinal         num_resources;  */
    /* ArgList          args;           */
    /* Cardinal         num_args;       */

extern void XtGetResourceList();
    /* WidgetClass	widget_class;		*/
    /* XtResourceList	*resources_return;	*/
    /* Cardinal		*num_resources_return;	*/

typedef struct _XtResource {
    String     resource_name;   /* Resource name                            */
    String     resource_class;  /* Resource class                           */
    String     resource_type;   /* Representation type desired              */
    Cardinal    resource_size;  /* Size in bytes of representation          */
    Cardinal    resource_offset;/* Offset from base to put resource value   */
    String     default_type;    /* representation type of specified default */
    caddr_t     default_addr;   /* Address of default resource              */
} XtResource;

#define XtDefaultForeground	"XtDefaultForeground"
#define XtDefaultBackground	"XtDefaultBackground"
#define XtDefaultFont		"XtDefaultFont"

/*
 * If we're going to go sticking proc *'s into default_addr, we should use a
 * different type for the field. Possibly XtArgVal?
 */

#define XtOffset(type,field) \
        ((unsigned int) (((char *) (&(((type)NULL)->field))) - \
                         ((char *) NULL)))

#ifdef notdef
/* this doesn't work on picky compilers */
#define XtOffset(type,field)    ((unsigned int)&(((type)NULL)->field))
#endif

/*************************************************************
 *
 * Error Handling
 *
 ************************************************************/

typedef void (*XtErrorMsgHandler)();
/*  String name,type,class,defaultp;
    String* params;
    Cardinal* num_params;
*/

extern void XtAppSetErrorMsgHandler();
    /* XtAppContext	   app;		*/
    /* XtErrorMsgHandler   handler;    */

extern void XtSetErrorMsgHandler();
    /* XtErrorMsgHandler   handler;    */

extern void XtAppSetWarningMsgHandler();
    /* XtAppContext	   app;		*/
    /* XtErrorMsgHandler   handler;    */

extern void XtSetWarningMsgHandler();
    /* XtErrorMsgHandler   handler;    */

extern void XtAppErrorMsg();
    /* XtAppContext	   app;		*/
/*  String name,type,class,defaultp;
    String* params;
    Cardinal* num_params;
*/

extern void XtErrorMsg();
/*  String name,type,class,defaultp;
    String* params;
    Cardinal* num_params;
*/

extern void XtAppWarningMsg();
    /* XtAppContext	   app;		*/
/*  String name,type,class,defaultp;
    String* params;
    Cardinal* num_params;
*/

extern void XtWarningMsg();
/*  String name,type,class,defaultp;
    String* params;
    Cardinal* num_params;
*/

typedef void (*XtErrorHandler)();
/*  String msg;
*/

extern void XtAppSetErrorHandler();
    /* XtAppContext	   app;		*/
    /* XtErrorHandler   handler;    */

extern void XtSetErrorHandler();
    /* XtErrorHandler   handler;    */

extern void XtAppSetWarningHandler();
    /* XtAppContext	   app;		*/
    /* XtErrorHandler   handler;    */

extern void XtSetWarningHandler();
    /* XtErrorHandler   handler;    */

extern void XtAppError();
    /* XtAppContext	   app;		*/
/*  String message;
*/

extern void XtError();
/*  String message;
*/

extern void XtAppWarning();
    /* XtAppContext	   app;		*/
/*  String message;
*/

extern void XtWarning();
/*  String message;
*/

extern XrmDatabase *XtAppGetErrorDatabase();
    /* XtAppContext	   app;		*/

extern XrmDatabase *XtGetErrorDatabase();
/* no paramters, returns toolkit error database*/


extern void XtAppGetErrorDatabaseText();
    /* XtAppContext	   app;		*/
/*  char *name, *type,*class;
    char *defaultp;
    char *buffer;
    int nbytes;
    XrmDatabase database;
*/

extern void XtGetErrorDatabaseText();
/*  char *name, *type,*class;
    char *defaultp;
    char *buffer;
    int nbytes;
*/

/****************************************************************
 *
 * Memory Management
 *
 ****************************************************************/

#define XtNew(type) ((type *) XtMalloc((unsigned) sizeof(type)))
#define XtNewString(str) (strcpy(XtMalloc((unsigned) strlen(str) + 1), str))

extern char *XtMalloc(); /* size */
    /* Cardinal size; */

extern char *XtCalloc(); /* num, size */
    /* Cardinal num, size; */

extern char *XtRealloc(); /* ptr, num */
    /* char     *ptr; */
    /* Cardinal num; */

extern void XtFree(); /* ptr */
        /* char  *ptr */


/*************************************************************
 *
 *  Work procs
 *
 **************************************************************/

typedef Boolean (*XtWorkProc)();
    /* caddr_t closure;  data the application registered */
    /* returns whether to remove this work procedure or not */

extern XtWorkProcId XtAddWorkProc();
    /*  XtWorkProc proc; */
    /*  caddr_t closure; */

extern XtWorkProcId XtAppAddWorkProc();
    /*  XtAppContext app; */
    /*  XtWorkProc proc; */
    /*  caddr_t closure; */

extern void  XtRemoveWorkProc();
    /*  XtWorkProcId id; */


/****************************************************************
 *
 * Graphic Context Management
 *****************************************************************/

extern GC XtGetGC(); /* widget, valueMask, values */
    /* Widget    widget */
    /* XtGCMask valueMask; */
    /* XGCValues *values; */

extern void XtDestroyGC (); /* widget, gc */
    /* Widget widget; */
    /* GC gc; */
/* we pass in the widget because XFreeGC needs a display, and there isn't */
/* one stored in the GC record. */

#endif _XtIntrinsic_h
/* DON'T ADD STUFF AFTER THIS #endif */

