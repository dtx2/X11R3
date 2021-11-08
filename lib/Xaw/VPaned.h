/*
* $XConsortium: VPaned.h,v 1.19 88/10/23 14:40:56 swick Exp $
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

#ifndef _XtVPaned_h
#define _XtVPaned_h

#include <X11/Constraint.h>

/****************************************************************
 *
 * Vertical Paned Widget (SubClass of CompositeClass)
 *
 ****************************************************************/

/* Parameters:

 Name		     Class		RepType		Default Value
 ----		     -----		-------		-------------
 allowResize	     Boolean		Boolean		False
 background	     Background		Pixel		XtDefaultBackground
 betweenCursor	     Cursor		Cursor		sb_v_double_arrow
 border		     BorderColor	Pixel		XtDefaultForeground
 borderWidth	     BorderWidth	Dimension	1
 destroyCallback     Callback		Pointer		NULL
 foreground	     Foreground		Pixel		Black
 height		     Height		Dimension	0
 gripIndent	     GripIndent		Position	16
 lowerCursor	     Cursor		Cursor		sb_down_arrow
 mappedWhenManaged   MappedWhenManaged	Boolean		True
 max		     Max		Dimension	unlimited
 min		     Min		Dimension	1
 refigureMode	     Boolean		Boolean		On
 sensitive	     Sensitive		Boolean		True
 skipAdjust	     Boolean		Boolean		False
 upperCursor	     Cursor		Cursor		sb_up_arrow
 width		     Width		Dimension	0
 x		     Position		Position	0
 y		     Position		Position	0

*/


/* New Fields */
#define XtNallowResize		"allowResize"
#define XtNbetweenCursor	"betweenCursor"
#define XtNforeground		"foreground"
#define XtNgripCursor		"gripCursor"
#define XtNgripIndent		"gripIndent"
#define XtNlowerCursor		"lowerCursor"
#define XtNrefigureMode		"refigureMode"
#define XtNposition		"position"
#define XtNmin			"min"
#define XtNmax			"max"
#define XtNskipAdjust		"skipAdjust"
#define XtNupperCursor		"upperCursor"

#define XtCGripIndent		"GripIndent"
#define XtCMin			"Min"
#define XtCMax			"Max"

/* Class record constant */
extern WidgetClass vPanedWidgetClass;

typedef struct _VPanedClassRec	*VPanedWidgetClass;
typedef struct _VPanedRec	*VPanedWidget;

/* Public Procedures */

extern void XtPanedSetMinMax( /* panedWidget, min, max */ );
    /* Widget panedWidget;	*/
    /* int    min, max;		*/

extern void XtPanedRefigureMode( /* widget, mode */ );
    /* Widget widget;		*/
    /* Boolean  mode;		*/

extern void XtPanedGetMinMax( /* panedWidget, min, max */ );
    /* Widget panedWidget;	*/
    /* int    *min, *max;	*/ /* RETURN */

extern int XtPanedGetNumSub( /* w */ );
    /* Widget w;		*/

#endif _XtVPaned_h
/* DON'T ADD STUFF AFTER THIS #endif */
