/* $XConsortium: popup.c,v 2.6 88/09/06 17:23:31 jim Exp $ */
/* popup.c -- Handle pop-up widgets. */

#include "xmh.h"

/*
static Widget confirmwidget = NULL;
static char *confirmstring;

static Widget confirmparent;
static count = 0;

static Widget promptwidget;
static int (*promptfunc)();
static char promptstring[210];

extern TellPrompt();
void DestroyPromptWidget();

ArgList arglist, promptarglist;
*/

/*
InitPopup()
{
}
*/

static Scrn lastscrn = NULL;
static char laststr[500];

static Widget confirmwidget = NULL;
static int buttoncount = 0;
static Widget promptwidget = NULL;
static void (*promptfunction)();


void CenterWidget(parent, child)
Widget parent, child;
{
    int x, y;
    x = (GetWidth(parent) - GetWidth(child)) / 2;
    y = (GetHeight(parent) - GetHeight(child)) / 2;
    if (x < 0) x = 0;
    XtMoveWidget(child, x, y);
}


void DestroyConfirmWidget()
{
    lastscrn = NULL;
    *laststr = 0;
    if (confirmwidget) {
	XtDestroyWidget(confirmwidget);
	confirmwidget = NULL;
    }
}


int Confirm(scrn, str)
Scrn scrn;
char *str;
{
    Arg args[1];
    extern void RedoLastButton();
    if (lastscrn == scrn && strcmp(str, laststr) == 0) {
	DestroyConfirmWidget();
	return TRUE;
    }
    DestroyConfirmWidget();
    lastscrn = scrn;
    scrn = LastButtonScreen();
    (void) strcpy(laststr, str);
    XtSetArg( args[0], XtNlabel, str );
    confirmwidget = XtCreateWidget( "confirm", dialogWidgetClass,
				    scrn->widget, args, XtNumber(args) );
    XtDialogAddButton(confirmwidget, "yes", RedoLastButton, (caddr_t)NULL);
    XtDialogAddButton(confirmwidget, "no", DestroyConfirmWidget,(caddr_t)NULL);
    XtRealizeWidget( confirmwidget );
    CenterWidget(scrn->widget, confirmwidget);
    XtMapWidget( confirmwidget );
    buttoncount = 0;
    return FALSE;
}


HandleConfirmEvent(event)
XEvent *event;
{
    if (confirmwidget &&
	    (event->type == ButtonRelease || event->type == KeyPress)) {
	if (++buttoncount > 1)
	    DestroyConfirmWidget();
    }
}


/* ARGSUSED */
void DestroyPromptWidget(widget, client_data, call_data)
    Widget widget;		/* unused */
    caddr_t client_data;	/* scrn */
    caddr_t call_data;		/* unused */
{
    if (promptwidget) {
	Scrn scrn = (Scrn)client_data;
	XtSetKeyboardFocus(scrn->parent, scrn->viewwidget);
	XtDestroyWidget(promptwidget);
	promptwidget = NULL;
    }
}


/* ARGSUSED */
void TellPrompt(widget, client_data, call_data)
    Widget widget;
    caddr_t client_data;	/* scrn */
    caddr_t call_data;

{
    (*promptfunction)(XtDialogGetValueString(promptwidget));
    DestroyPromptWidget(widget, client_data, call_data);
}

MakePrompt(scrn, prompt, func)
Scrn scrn;
char *prompt;
void (*func)();
{
    static Arg args[] = {
	{XtNlabel, NULL},
	{XtNvalue, NULL},
    };
    args[0].value = (XtArgVal)prompt;
    args[1].value = (XtArgVal)"";
    DestroyPromptWidget((Widget)NULL, (caddr_t)scrn, NULL);
    promptwidget = XtCreateWidget( "prompt", dialogWidgetClass, scrn->widget,
				   args, (Cardinal)2 );
    XtDialogAddButton(promptwidget, "goAhead", TellPrompt, (caddr_t)scrn);
    XtDialogAddButton(promptwidget, "cancel", DestroyPromptWidget, (caddr_t)scrn);
    XtRealizeWidget(promptwidget);
    XtSetKeyboardFocus(promptwidget, XtNameToWidget(promptwidget,"value"));
    XtSetKeyboardFocus(scrn->parent, (Widget)None);
    CenterWidget(scrn->widget, promptwidget);
    XtMapWidget( promptwidget );
    promptfunction = func;
}
