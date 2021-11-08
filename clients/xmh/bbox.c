#ifndef lint
static char rcs_id[] = "$XConsortium: bbox.c,v 2.19 88/09/16 13:31:14 swick Exp $";
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

/* bbox.c -- management of buttons and buttonbox's. */

/* This module implements a simple interface to buttonboxes, allowing a client
   to create new buttonboxes and manage their contents.  It is a layer hiding
   the toolkit interfaces. */

#include <X11/Cardinals.h>
#include "xmh.h"
#include "bboxint.h"


/* Free the storage for the given button. */

FreeButton(button)
Button button;
{
    XtFree(button->name);
    XtFree((char *) button);
}


/* Create a new button box.  The widget for it will be a child of the given
   scrn's widget, and it will be added to the scrn's pane. */

ButtonBox BBoxRadioCreate(scrn, position, name, radio)
  Scrn scrn;
  int position;			/* Position to put it in the pane. */
  char *name;			/* Name of the buttonbox widget. */
  Button *radio;		/* Pointer to place to store which radio
				   button is active. */
{
    static Arg arglist[] = {
	{XtNallowVert, True},
	{XtNskipAdjust, True},
    };
    ButtonBox buttonbox;

    buttonbox = XtNew(ButtonBoxRec);
    bzero((char *)buttonbox, sizeof(ButtonBoxRec));
    buttonbox->updatemode = TRUE;
    buttonbox->scrn = scrn;
    buttonbox->outer =
	XtCreateManagedWidget(name, viewportWidgetClass, scrn->widget,
			      arglist, XtNumber(arglist));
    buttonbox->inner =
	XtCreateManagedWidget(name, boxWidgetClass, buttonbox->outer,
			      NULL, (Cardinal)0);
    buttonbox->numbuttons = 0;
    buttonbox->button = (Button *) XtMalloc((unsigned) 1);
    buttonbox->maxheight = 5;
    buttonbox->radio = radio;
    if (radio) *radio = NULL;
    return buttonbox;
}



/* Create a new buttonbox which does not manage radio buttons. */

ButtonBox BBoxCreate(scrn, position, name)
  Scrn scrn;
  int position;
  char *name;
{
    return BBoxRadioCreate(scrn, position, name, (Button *)NULL);
}



/* Set the current button in a radio buttonbox. */

void BBoxSetRadio(buttonbox, button)
ButtonBox buttonbox;
Button button;
{
    if (buttonbox->radio && *(buttonbox->radio) != button) {
	if (*(buttonbox->radio)) FlipColors(*(buttonbox->radio));
	FlipColors(button);
	*(buttonbox->radio) = button;
    }
}



/* Some buttons have been added to the buttonbox structure; go and actually
   add the button widgets to the buttonbox widget.  (It is much more
   efficient to add several buttons at once rather than one at a time.) */

static ProcessAddedButtons(buttonbox)
ButtonBox buttonbox;
{
    int i;
    WidgetList widgetlist, ptr;
    Button button;
    if (buttonbox->updatemode == FALSE) {
	buttonbox->needsadding = TRUE;
	return;
    }
    ptr = widgetlist = (WidgetList)
	XtMalloc((unsigned)sizeof(Widget) * (buttonbox->numbuttons + 1));
    for (i=0 ; i<buttonbox->numbuttons ; i++) {
	button = buttonbox->button[i]; /* %%% position? */
	if (button->needsadding) {
	    *ptr++ = button->widget;
	    button->needsadding = FALSE;
	}
    }
    if (ptr != widgetlist)
	XtManageChildren(widgetlist, (Cardinal) (ptr-widgetlist));
    XtFree((char *) widgetlist);
    buttonbox->needsadding = FALSE;
}



/* Create a new button, and add it to a buttonbox. */

void BBoxAddButton(buttonbox, name, func, position, enabled, extra)
ButtonBox buttonbox;
char *name;			/* Name of button. */
void (*func)();			/* Func to call when button pressed. */
int position;			/* Position to put button in box. */
int enabled;			/* Whether button is initially enabled. */
char **extra;			/* Extra translation bindings. */
{
    extern void DoButtonPress();
    Button button;
    int i;
    static XtCallbackRec callback[] = { {DoButtonPress, NULL}, {NULL, NULL} };
    Arg arglist[1];

    if (position > buttonbox->numbuttons) position = buttonbox->numbuttons;
    buttonbox->numbuttons++;
    buttonbox->button = (Button *)
	XtRealloc((char *) buttonbox->button,
		  (unsigned) buttonbox->numbuttons * sizeof(Button));
    for (i=buttonbox->numbuttons-1 ; i>position ; i--)
	buttonbox->button[i] = buttonbox->button[i-1];
    button = buttonbox->button[position] = XtNew(ButtonRec);
    bzero((char *) button, sizeof(ButtonRec));
    callback[0].closure = (caddr_t)button;
    button->buttonbox = buttonbox;
    button->name = MallocACopy(name);
    XtSetArg(arglist[0], XtNcallback, callback);
    button->widget = XtCreateWidget(name, commandWidgetClass,
				    buttonbox->inner, arglist, ONE);
/*    if (extra) XtAugmentTranslations(button->widget,
				     XtParseTranslationTable(extra));*/
    button->func = func;
    button->needsadding = TRUE;
    button->enabled = TRUE;
    if (!enabled) BBoxDisable(button);
    ProcessAddedButtons(buttonbox);
    if (buttonbox->radio && *(buttonbox->radio) == NULL) {
	*(buttonbox->radio) = button;
	FlipColors(button);
    }
}



/* Remove the given button from its buttonbox.  The button widget is
   destroyed.  If it was the current button in a radio buttonbox, then the
   current button becomes the first button in the box. */

void BBoxDeleteButton(button)
Button button;
{
    ButtonBox buttonbox = button->buttonbox;
    int i, found, reradio;
    found = FALSE;
    for (i=0 ; i<buttonbox->numbuttons; i++) {
	if (found) buttonbox->button[i-1] = buttonbox->button[i];
	else if (buttonbox->button[i] == button) {
	    found = TRUE;
	    XtDestroyWidget(button->widget);
	    reradio = (buttonbox->radio && *(buttonbox->radio) == button);
	    FreeButton(button);
	}
    }
    if (found) {
	buttonbox->numbuttons--;
	if (reradio) {
	    *(buttonbox->radio) = NULL;
	    if (buttonbox->numbuttons)
		BBoxSetRadio(buttonbox, buttonbox->button[0]);
	}
    }
}
	    


/* Enable or disable the given command button widget. */

static void SendEnableMsg(widget, value)
Widget widget;
int value;			/* TRUE for enable, FALSE for disable. */
{
    static Arg arglist[] = {XtNsensitive, NULL};
    arglist[0].value = (XtArgVal) value;
    XtSetValues(widget, arglist, XtNumber(arglist));
}



/* Enable the given button (if it's not already). */

void BBoxEnable(button)
Button button;
{
    if (!button->enabled) {
	button->enabled = TRUE;
	SendEnableMsg(button->widget, TRUE);
    }
}



/* Disable the given button (if it's not already). */

void BBoxDisable(button)
Button button;
{
    if (button->enabled) {
	button->enabled = FALSE;
	SendEnableMsg(button->widget, FALSE);
    }
}


/* Given a buttonbox and a button name, find the button in the box with that
   name. */

Button BBoxFindButtonNamed(buttonbox, name)
ButtonBox buttonbox;
char *name;
{
    int i;
    for (i=0 ; i<buttonbox->numbuttons; i++)
	if (strcmp(name, buttonbox->button[i]->name) == 0)
	    return buttonbox->button[i];
    return NULL;
}



/* Return the nth button in the given buttonbox. */

Button BBoxButtonNumber(buttonbox, n)
ButtonBox buttonbox;
int n;
{
    return buttonbox->button[n];
}



/* Return how many buttons are in a buttonbox. */

int BBoxNumButtons(buttonbox)
ButtonBox buttonbox;
{
    return buttonbox->numbuttons;
}


/* Given a button, return its name. */

char *BBoxNameOfButton(button)
Button button;
{
    return button->name;
}



/* The client calls this routine before doing massive updates to a buttonbox.
   It then must call BBoxStartUpdate when it's finished.  This allows us to
   optimize things.  Right now, the only optimization performed is to batch
   together requests to add buttons to a buttonbox. */

void BBoxStopUpdate(buttonbox)
ButtonBox buttonbox;
{
    buttonbox->updatemode = FALSE;
}



/* The client has finished its massive updates; go and handle any batched
   requests. */

void BBoxStartUpdate(buttonbox)
ButtonBox buttonbox;
{
    buttonbox->updatemode = TRUE;
    if (buttonbox->needsadding) ProcessAddedButtons(buttonbox);
}



/* Set the minimum and maximum size for a bbox so that it cannot be resized
   any bigger than its total height. */

void BBoxLockSize(buttonbox)
ButtonBox buttonbox;
{
#ifdef notyet
    static Arg args[] = {
	{XtNmax, NULL},		/* first is for VPaned */
/*	{XtNmin, 5}, */		/* let user select this */
    };

    buttonbox->maxheight = GetHeight(buttonbox->inner);
    args[0].value = (XtArgVal)buttonbox->maxheight;
    XtSetValues(buttonbox->outer, args, XtNumber(args));
#endif
}





/* Destroy the given buttonbox. */

void BBoxDeleteBox(buttonbox)
ButtonBox buttonbox;
{
    if (buttonbox->radio) *(buttonbox->radio) = NULL;
    XtDestroyWidget(buttonbox->outer);
}



/* Change the borderwidth of the given button. */

void BBoxChangeBorderWidth(button, borderWidth)
Button button;
unsigned int borderWidth;
{
    static Arg arglist[] = {XtNborderWidth, NULL};
    arglist[0].value = (XtArgVal) borderWidth;
    XtSetValues(button->widget, arglist, XtNumber(arglist));
}
