#ifndef lint
static char rcsid[] = "$XConsortium: xbuttonbox.c,v 1.12 88/09/06 18:27:06 jim Exp $";
#endif  lint

#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/Shell.h>
#include <X11/Label.h>
#include <X11/Box.h>
#include <X11/Command.h>
#include <X11/Cardinals.h>
#include "icon.bits"

extern void exit();

/* Command line options table.  Only resources are entered here...there is a
   pass over the remaining options after XtParseCommand is let loose. */

static XrmOptionDescRec options[] = {
    {"-hspace",	"*Box.hSpace",	XrmoptionSepArg,	NULL},
    {"-vspace",	"*Box.vSpace",	XrmoptionSepArg,	NULL},
};

static Widget sensitivityButton;
static Widget button;
static Widget labels[10];

/*
 * Report the syntax for calling xbox.
 */
Syntax(call)
    char *call;
{
    (void) printf ("Usage: %s [-hspace <pixels>] [-vspace <pixels>]\n", call);
}

static void SetSensitivity( w, closure, call_data )
    Widget w;
    caddr_t closure, call_data;
{
    static Boolean sensitivity = TRUE;
    int i;

    sensitivity = !sensitivity;

    XtSetSensitive( button, sensitivity );

    for (i=1; i<10; i+=2)
        XtSetSensitive( labels[i], sensitivity );
}

static void Activate( w, closure, call_data )
    Widget w;
    caddr_t closure, call_data;
{
    static int presses = 0;
    static char label[100];
    static Arg arg[] = { {XtNlabel, (XtArgVal)label} };

    sprintf( label, "button pressed %d times", ++presses );
    XtSetValues( w, arg, XtNumber(arg) );
}

void main(argc, argv)
    int argc;
    char **argv;
{
    Display *dpy;
    Widget toplevel, w;
    Arg 	args[10];
    Cardinal	i;
    char	name[100];
    static XtCallbackRec callback[2]; /* K&R: initialized to NULL */

    if (argc != 1) Syntax(argv[0]);

    toplevel = XtInitialize( NULL, "XBox",
			     options, XtNumber(options),
			     &argc, argv );

    XtSetArg(args[0], XtNiconPixmap,
	     XCreateBitmapFromData (XtDisplay(toplevel),
				    XtScreen(toplevel)->root, box_bits,
				    box_width, box_height));
    XtSetArg(args[1], XtNallowShellResize, TRUE);
    XtSetValues (toplevel, args, TWO);

    w = XtCreateManagedWidget (argv[0], boxWidgetClass,
			       toplevel, NULL, ZERO);

    callback[0].callback = SetSensitivity;
    XtSetArg( args[0], XtNcallback, callback );
    sensitivityButton =	XtCreateManagedWidget("CommandButtonWidget0",
					      commandWidgetClass, w, args, ONE);

    callback[0].callback = Activate;
    button = XtCreateManagedWidget("CommandButtonWidget1",
				   commandWidgetClass, w, args, ONE);

    for (i = 0; i < 10; i++) {
	(void) sprintf(name, "%s%d", "longLabelWidgetName", i*i*i);
	XtSetArg( args[0], XtNsensitive, (i % 2) );
        labels[i] = XtCreateManagedWidget(name, labelWidgetClass,
					  w, args, ONE);
    }

    XtRealizeWidget (toplevel);
    XtMainLoop();
}
