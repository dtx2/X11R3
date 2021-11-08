#ifndef lint
static char rcs_id[] = "$XConsortium: button.c,v 2.10 88/09/06 17:23:10 jim Exp $";
#endif lint
/*
 *			  COPYRIGHT 1987
 *		   DIGITAL EQUIPMENT CORPORATION
 *		       MAYNARD, MASSACHUSETTS
 *			ALL RIGHTS RESERVED.
 *
 * THE INFORMATION IN THIS SOFTWARE IS SUBJECT TO CHANGE WITHOUT NOTICE AND
 * SHOULD NOT BE CONSTRUED AS A COMMITMENT BY DIGITAL EQUIPMENT CORPORATION.
 * DIGITAL MAKES NO REPRESENTATIONS ABOUT THE SUITABILITY OF THIS SOFTWARE FOR
 * ANY PURPOSE.  IT IS SUPPLIED "AS IS" WITHOUT EXPRESS OR IMPLIED WARRANTY.
 *
 * IF THE SOFTWARE IS MODIFIED IN A MANNER CREATING DERIVATIVE COPYRIGHT RIGHTS,
 * APPROPRIATE LEGENDS MAY BE PLACED ON THE DERIVATIVE WORK IN ADDITION TO THAT
 * SET FORTH ABOVE.
 *
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted, provided
 * that the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting documentation,
 * and that the name of Digital Equipment Corporation not be used in advertising
 * or publicity pertaining to distribution of the software without specific,
 * written prior permission.
 */

/* button.c -- Handle a button being pressed */

#include "xmh.h"
#include "bboxint.h"


/* Highlight or unhighlight the given button.  What a hack...%%% */

FlipColors(button)
Button button;
{
    static Arg arglist[] = {
	{XtNforeground, NULL},
	{XtNbackground, NULL}
    };
    Pixel foreground, background;

    arglist[0].value = (XtArgVal)&foreground;
    arglist[1].value = (XtArgVal)&background;
    XtGetValues(button->widget, arglist, XtNumber(arglist));
    arglist[0].value = (XtArgVal)background;
    arglist[1].value = (XtArgVal)foreground;
    XtSetValues(button->widget, arglist, XtNumber(arglist));
}


/* The given button has just been pressed.  (This routine is usually called
   by the command button widget code.)  If it's a radio button, select it.
   Then call the function registered for this button. */

/*ARGSUSED*/
void DoButtonPress(w, closure, data)
Widget w;			/* unused */
caddr_t closure;		/* a Button record */
caddr_t data;			/* unused */
{
    Button button = (Button)closure;
    ButtonBox buttonbox = button->buttonbox;
    Scrn scrn = buttonbox->scrn;
    if (!button->enabled) return;
    if (buttonbox->radio && *(buttonbox->radio) != button) {
	FlipColors(*(buttonbox->radio));
	FlipColors(button);
	*(buttonbox->radio) = button;
    }
    LastButtonPressed = button;
    (*(button->func))(scrn);
}


/* Act as if the last button pressed was pressed again. */

void RedoLastButton()
{
    if (LastButtonPressed)
	DoButtonPress((Widget)NULL, (caddr_t)LastButtonPressed, NULL);
}


/* Returns the screen in which the last button was pressed. */

Scrn LastButtonScreen()
{
    return LastButtonPressed->buttonbox->scrn;
}
