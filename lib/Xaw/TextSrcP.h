/*
* $XConsortium: TextSrcP.h,v 1.1 88/10/18 13:13:08 swick Exp $
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

#ifndef _XtTextSrcP_h
#define _XtTextSrcP_h


#include <X11/Text.h>

typedef enum {XtsdLeft, XtsdRight} XtTextScanDirection;
typedef enum {XtstPositions, XtstWhiteSpace, XtstEOL, XtstParagraph, XtstAll}
    XtTextScanType;

typedef struct _XtTextSource {
    XtTextPosition	(*Read)();
    int			(*Replace)();
    XtTextPosition	(*GetLastPos)();
    int			(*SetLastPos)();
    XtTextPosition	(*Scan)();
    void		(*AddWidget)( /* source, widget */ );
    void		(*RemoveWidget)( /* source, widget */ );
    Boolean		(*GetSelection)( /* source, left, right, selection */);
    void		(*SetSelection)( /* source, left, right, selection */);
    Boolean		(*ConvertSelection)( /* Display*, source, ... */ );
    XtTextEditType	edit_mode;
    caddr_t		data;	    
    };

typedef struct _XtTextSink {
    XFontStruct	*font;
    int foreground;
    int (*Display)();
    int (*InsertCursor)();
    int (*ClearToBackground)();
    int (*FindPosition)();
    int (*FindDistance)();
    int (*Resolve)();
    int (*MaxLines)();
    int (*MaxHeight)();
    void (*SetTabs)();		/* widget, offset, tab_count, *tabs */
    caddr_t data;
    };

typedef enum {XtisOn, XtisOff} XtTextInsertState;

typedef enum {XtsmTextSelect, XtsmTextExtend} XtTextSelectionMode;

typedef enum {XtactionStart, XtactionAdjust, XtactionEnd}
    XtTextSelectionAction;

typedef struct {
    XtTextPosition   left, right;
    XtTextSelectType type;
    Atom*	     selections;
    int		     atom_count;
    int		     array_size;
} XtTextSelection;

typedef enum  {Normal, Selected }highlightType;

/* for backwards compatibility only */

#define EditDone	XawEditDone
#define EditError	XawEditError
#define PositionError	XawPositionError

#endif _XtTextSrcP_h
