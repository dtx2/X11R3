/*
 * xman - X window system manual page display program.
 *
 * $XConsortium: tkfuncs.c,v 1.3 89/01/06 18:42:30 kit Exp $
 *
 * Copyright 1987, 1988 Massachusetts Institute of Technology
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted, provided
 * that the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of M.I.T. not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  M.I.T. makes no representations about the
 * suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 *
 * Author:    Chris D. Peterson, MIT Project Athena
 * Created:   February 6, 1988
 */

#if ( !defined(lint) && !defined(SABER))
  static char rcs_version[] = "$Athena: tkfuncs.c,v 4.5 88/12/19 13:48:24 kit Exp $";
#endif

#include <X11/X.h>
#include <X11/Xos.h>
#include <X11/Xlib.h>
#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/IntrinsicP.h>

/* 
 * I am doing the "wrong" thing here by looking in the core field for the
 * widget to get this info, the "right" thing to do is to do a XtGetValues
 * to get this information.
 */

/*	Function Name: Width
 *	Description: finds the width of a widget.
 *	Arguments: w - the widget.
 *	Returns: the width of that widget.
 */

int
Width(w)
Widget w;
{
  return( (int) w->core.width );
}

/*	Function Name: Height
 *	Description: finds the height of a widget.
 *	Arguments: w - the widget.
 *	Returns: the height of that widget.
 */

int
Height(w)
Widget w;
{
  return( (int) w->core.height );
}

/*	Function Name: BorderWidth
 *	Description: finds the BorderWidth of a widget.
 *	Arguments: w - the widget.
 *	Returns: the BorderWidth of that widget.
 */

int
BorderWidth(w)
Widget w;
{
  return( (int) w->core.border_width );
}

/* 
 * These functions have got to be able to get at the widget tree, I don't see
 * any way around this one.
 */

/*	Function Name: PopupChild
 *	Description: This function returns the correct popup child
 *	Arguments: w - widget
 *                 child - which popup child to get.
 *	Returns: the popup child.
 */

Widget
PopupChild(w,child)
Widget w;
int child;
{
  return( (Widget) w->core.popup_list[child]);
}

/*	Function Name: Child
 *	Description: This function returns the correct child
 *	Arguments: w - widget
 *                 child - which child to get.
 *	Returns: the child.
 */

Widget
Child(w,child)
Widget w;
int child;
{
  CompositeWidget cw = (CompositeWidget) w;
  return( (Widget) cw->composite.children[child]);
}

/*	Function Name: Name
 *	Description: This function returns the correct popup child
 *	Arguments: w - widget
 *	Returns: the popup child.
 */

char * 
Name(w)
Widget w;
{
  return( w->core.name);
}


/*	Function Name: MakeLong
 *	Description: This function make all widgets of the parent the same
 *                   length as the longest child.
 *	Arguments: widget - the parent widget.
 *	Returns: TRUE if sucessful, cannot fail when parent is a Box and this
 *               is the only place I use it, so I never look at this.
 */

Boolean
MakeLong(widget)
Widget widget;
{
  CompositeWidget top;		/* the parent of the room,
				 also the parent of the lable.*/
  Widget w;
  XtGeometryResult answer;	/* result of geometery test. */
  Dimension awidth,aheight;	/* The height and width of the reply. */
  int width = 0;		/* The width of the biggest button */
  int i;

  top = (CompositeWidget) widget;

/* Find the width of the longest child. */

  for ( i = 0; i < top->composite.num_children; i++) {
    w = top->composite.children[i];
    if (width < Width(w) )
      width = Width(w);
  }

/* 
 * Attempt to resize the children to the width we just found to be the longest.
 */

  for ( i = 0; i < top->composite.num_children; i++) {
    w = top->composite.children[i];
    answer = XtMakeResizeRequest( w, (Cardinal) width, (Cardinal) Height(w),
				 &awidth, &aheight);
    
    switch(answer) {
    case XtGeometryYes:
      break;
    case XtGeometryNo:
      return(FALSE);
    case XtGeometryAlmost:
      (void) XtMakeResizeRequest( w, awidth, aheight, &awidth, &aheight);
    }
  }
  return(TRUE);
}
