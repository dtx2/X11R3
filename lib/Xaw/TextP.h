/*
* $XConsortium: TextP.h,v 1.28 88/10/18 13:12:54 swick Exp $
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

#ifndef _XtTextP_h
#define _XtTextP_h


#include <X11/Text.h>
#include <X11/SimpleP.h>
#include <X11/TextSrcP.h>

/****************************************************************
 *
 * Text widget private
 *
 ****************************************************************/
#define MAXCUT	30000	/* Maximum number of characters that can be cut. */

#define LF	0x0a
#define CR	0x0d
#define TAB	0x09
#define BS	0x08
#define SP	0x20
#define DEL	0x7f
#define BSLASH	'\\'

/* constants that subclasses may want to know */
#define DEFAULT_TEXT_HEIGHT ((Dimension)~0)
#define  yMargin 2


/* displayable text management data structures */

typedef struct {
    XtTextPosition position;
    Position x, y, endX;
    } XtTextLineTableEntry, *XtTextLineTableEntryPtr;

/* Line Tables are n+1 long - last position displayed is in last lt entry */
typedef struct {
    XtTextPosition	 top;	/* Top of the displayed text.		*/
    int			 lines;	/* How many lines in this table.	*/
    XtTextLineTableEntry *info;	/* A dynamic array, one entry per line  */
    } XtTextLineTable, *XtTextLineTablePtr;

#define IsPositionVisible(ctx, pos) \
		(pos >= ctx->text.lt.info[0].position && \
		 pos < ctx->text.lt.info[ctx->text.lt.lines].position)

/* Private Text Definitions */

typedef int (*ActionProc)();

/* New fields for the Text widget class record */

typedef struct {int empty;} TextClassPart;

/* Full class record declaration */
typedef struct _TextClassRec {
    CoreClassPart	core_class;
    SimpleClassPart	simple_class;
    TextClassPart	text_class;
} TextClassRec;

extern TextClassRec textClassRec;

/* New fields for the Text widget record */
typedef struct _TextPart {
    /* resources */
    XtTextSource	source;
    XtTextSink		sink;
    XtTextPosition	insertPos;
    XtTextSelection	s;
    XtTextSelectType	*sarray;	   /* Array to cycle for selections. */
    Dimension		client_leftmargin;   /* client-visible resource */
    int			options;	     /* wordbreak, scroll, etc. */
    int			dialog_horiz_offset; /* position for popup dialog */
    int			dialog_vert_offset;  /* position for popup dialog */
    /* private state */
    XtTextLineTable	lt;
    XtTextScanDirection extendDir;
    XtTextSelection	origSel;    /* the selection being modified */
    Dimension	    leftmargin;	    /* Width of left margin. */
    Time	    lasttime;	    /* timestamp of last processed action */
    Time	    time;	    /* time of last key or button action */ 
    Position	    ev_x, ev_y;	    /* x, y coords for key or button action */
    Widget	    sbar;	    /* The vertical scroll bar (none = 0).  */
    Widget	    outer;	    /* Parent of scrollbar & text (if any) */
    XtTextPosition  *updateFrom;    /* Array of start positions for update. */
    XtTextPosition  *updateTo;	    /* Array of end positions for update. */
    int		    numranges;	    /* How many update ranges there are. */
    int		    maxranges;	    /* How many ranges we have space for */
    Boolean	    showposition;   /* True if we need to show the position. */
    XtTextPosition  lastPos;	    /* Last position of source. */
    struct _dialog {
	TextWidget  text;	    /* the dialog's parent */
	Widget      widget;	    /* the dialog widget */
	Widget	    doit;	    /* the confirm button */
	Widget	    message;	    /* the (occasional) error message */
	Boolean	    mapped;	    /* True if this dialog is in-use */
	struct _dialog *next;	    /* a list of dialogs */
    } *dialog;			    /* InsertFile pop-up widget */
    GC              gc;
    Boolean         hasfocus;       /* TRUE if we currently have input focus.*/
    Boolean	    update_disabled; /* TRUE if display updating turned off */
    XtTextPosition  old_insert;      /* Last insertPos for batched updates */
} TextPart;

/****************************************************************
 *
 * Full instance record declaration
 *
 ****************************************************************/

typedef struct _TextRec {
    CorePart	core;
    SimplePart	simple;
    TextPart	text;
} TextRec;


#endif _XtTextP_h
