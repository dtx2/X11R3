/* $XConsortium: InitialI.h,v 1.10 89/02/23 18:56:12 swick Exp $ */
/* $oHeader: InitializeI.h,v 1.8 88/09/01 11:25:04 asente Exp $ */
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

/****************************************************************
 *
 * Displays
 *
 ****************************************************************/

#include <sys/param.h>				/* to get MAXPATHLEN */

#ifndef MAX
#define MAX(a,b) (((a) > (b)) ? (a) : (b))
#endif

#ifndef MIN
#define MIN(a,b) (((a) < (b)) ? (a) : (b))
#endif


#include "fd.h"

typedef struct _TimerEventRec {
        struct timeval   te_timer_value;
	struct _TimerEventRec *te_next;
	Display *te_dpy;
	XtTimerCallbackProc	te_proc;
	XtAppContext app;
	caddr_t	te_closure;
} TimerEventRec;

typedef struct _InputEvent {
	XtInputCallbackProc  ie_proc;
	caddr_t ie_closure;
	struct	_InputEvent	*ie_next;
	struct  _InputEvent	*ie_oq;
	XtAppContext app;
	int	ie_source;
} InputEvent;

typedef struct _WorkProcRec {
	XtWorkProc proc;
	caddr_t closure;
	struct _WorkProcRec *next;
	XtAppContext app;
} WorkProcRec;


typedef struct 
{
  	Fd_set rmask;
	Fd_set wmask;
	Fd_set emask;
	int	nfds;
	int	count;
} FdStruct;

typedef struct _ProcessContextRec {
    XtAppContext	defaultAppContext;
    XtAppContext	appContextList;
    ConverterTable	globalConverterTable;
} ProcessContextRec, *ProcessContext;

typedef struct _XtAppStruct {
    XtAppContext next;		/* link to next app in process context */
    ProcessContext process;	/* back pointer to our process context */
    Display **list;
    TimerEventRec *timerQueue;
    WorkProcRec *workQueue;
    InputEvent *selectRqueue[NOFILE], *selectWqueue[NOFILE],
	    *selectEqueue[NOFILE];
    InputEvent *outstandingQueue;
    XrmDatabase errorDB;
    XtErrorMsgHandler errorMsgHandler, warningMsgHandler;
    XtErrorHandler errorHandler, warningHandler;
    struct _ActionListRec *action_table;
    ConverterTable converterTable;
    unsigned long selectionTimeout;
    FdStruct fds;
    short count, max, last;
    Boolean sync, rv, being_destroyed, error_inited;
} XtAppStruct;

extern void _XtSetDefaultErrorHandlers();
extern void _XtSetDefaultSelectionTimeout();
extern void _XtSetDefaultConverterTable();
extern void _XtFreeConverterTable();

extern XtAppContext _XtDefaultAppContext();
extern ProcessContext _XtGetProcessContext();
extern void _XtDestroyAppContexts();
extern void _XtCloseDisplays();
extern int _XtAppDestroyCount;
extern int _XtDpyDestroyCount;

extern int _XtwaitForSomething(); /* ignoreTimers, ignoreInputs, ignoreEvents,
				     block, howlong, appContext */
    /* Boolean ignoreTimers; */
    /* Boolean ignoreInputs; */
    /* Boolean ignoreEvents; */
    /* Boolean block; */
    /* unsigned long *howlong; */
    /* XtAppContext app */

typedef struct _XtPerDisplayStruct {
    Region region;
    XtCaseProc defaultCaseConverter;
    XtKeyProc defaultKeycodeTranslator;
    XtAppContext appContext;
    KeySym *keysyms;                   /* keycode to keysym table */
    int keysyms_per_keycode;           /* number of keysyms for each keycode */
    KeySym *modKeysyms;                /* keysym values for modToKeysysm */
    ModToKeysymTable *modsToKeysyms;   /* modifiers to Keysysms index table*/
    Boolean being_destroyed;
    XrmName name;		       /* resolved app name */
    XrmClass class;		       /* R2 compatibility only */
} XtPerDisplayStruct, *XtPerDisplay;

extern void _XtPerDisplayInitialize();

extern XtPerDisplay _XtGetPerDisplay();
    /* Display *dpy */

extern XtAppContext _XtDisplayToApplicationContext();
    /* Display *dpy */

extern void _XtDisplayInitialize();
    /* 	Display *dpy; */
    /* 	String name, classname; */
    /* 	XrmOptionDescRec *urlist */;
    /* 	Cardinal num_urs; */
    /* 	Cardinal *argc; */
    /* 	char *argv[];  */

