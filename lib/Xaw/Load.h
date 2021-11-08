/*
* $XConsortium: Load.h,v 1.13 88/10/23 14:03:12 swick Exp $
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

#ifndef _XtLoad_h
#define _XtLoad_h

/***********************************************************************
 *
 * Load Widget
 *
 ***********************************************************************/

/* Parameters:

 Name		     Class		RepType		Default Value
 ----		     -----		-------		-------------
 background	     Background		Pixel		White
 border		     BorderColor	Pixel		Black
 borderWidth	     BorderWidth	Dimension	1
 destroyCallback     Callback		Pointer		NULL
 font		     Font		XFontStruct*	XtDefaultFont
 foreground	     Foreground		Pixel		Black
 getLoadProc	     Callback		Callback	(internal)
 height		     Height		Dimension	120
 highlight	     Foreground		Pixel		Black
 label		     Label		String		(empty string)
 mappedWhenManaged   MappedWhenManaged	Boolean		True
 minScale	     Scale		int		1
 reverseVideo	     ReverseVideo	Boolean		False
 scale		     Scale		int		1
 update		     Interval		int		5 (seconds)
 width		     Width		Dimension	120
 x		     Position		Position	0
 y		     Position		Position	0

*/


#define XtNupdate		"update"
#define XtNscale		"scale"
#define XtNvmunix		"vmunix"
#define XtNminScale		"minScale"
#define XtNgetLoadProc		"getLoadProc"
#define XtNhighlight		"highlight"
 
#define XtCScale		"Scale"

typedef struct _LoadRec *LoadWidget;  /* completely defined in LoadPrivate.h */
typedef struct _LoadClassRec *LoadWidgetClass;    /* completely defined in LoadPrivate.h */

extern WidgetClass loadWidgetClass;

#endif _XtLoad_h
/* DON'T ADD STUFF AFTER THIS #endif */
