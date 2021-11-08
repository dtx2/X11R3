#include <X11/copyright.h>

/* $XConsortium: Simple.h,v 1.6 88/10/23 14:31:50 swick Exp $ */
/* Copyright	Massachusetts Institute of Technology	1987 */

#ifndef _Simple_h
#define _Simple_h

/****************************************************************
 *
 * Simple widgets
 *
 ****************************************************************/

/* Resources:

 Name		     Class		RepType		Default Value
 ----		     -----		-------		-------------
 background	     Background		Pixel		XtDefaultBackground
 border		     BorderColor	Pixel		XtDefaultForeground
 borderWidth	     BorderWidth	Dimension	1
 cursor		     Cursor		Cursor		None
 destroyCallback     Callback		Pointer		NULL
 height		     Height		Dimension	0
 insensitiveBorder   Insensitive	Pixmap		Gray
 mappedWhenManaged   MappedWhenManaged	Boolean		True
 sensitive	     Sensitive		Boolean		True
 width		     Width		Dimension	0
 x		     Position		Position	0
 y		     Position		Position	0

*/

#define XtNcursor		"cursor"
#define XtNinsensitiveBorder	"insensitiveBorder"

#define XtCCursor		"Cursor"
#define XtCInsensitive		"Insensitive"


typedef struct _SimpleClassRec	*SimpleWidgetClass;
typedef struct _SimpleRec	*SimpleWidget;

extern WidgetClass simpleWidgetClass;

#endif  _Simple_h
