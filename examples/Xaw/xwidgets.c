#ifndef lint
static char rcsid[] = "$XConsortium: xwidgets.c,v 1.20 88/09/30 09:27:32 jim Exp $";
#endif	lint

#include <stdio.h>
#include <X11/Xatom.h>
#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/Box.h>
#include <X11/Clock.h>
#include <X11/Command.h>
#include <X11/Dialog.h>
#include <X11/Label.h>
#undef XtNhighlight			/* defined up in Clock.h */
#include <X11/Load.h>
#include <X11/Scroll.h>
#include <X11/AsciiText.h>
#include <X11/VPaned.h>
#include <X11/Viewport.h>
#include <X11/Cardinals.h>

/* Command line options table.  Only resources are entered here...there is a
   pass over the remaining options after XtParseCommand is let loose. */

static Widget textWidget;
static char message[] = "text widget             ";
static int leftMargin = 2;

static XrmOptionDescRec options[] = {
{"-label",	XtNlabel,	XrmoptionSepArg,	NULL}
};


/* ARGSUSED */
Syntax(call)
	char *call;
{
}

/* ARGSUSED */
void Quit(widget,closure,callData)
    Widget widget;
    caddr_t closure;		/* Widget */
    caddr_t callData;
{
    fprintf(stderr,"command callback.\n");
    XtDestroyWidget((Widget)closure);
}

/* ARGSUSED */
void Count(widget, closure, callData)
   Widget widget;
   caddr_t closure;		/* Widget */
   caddr_t callData;
{
   Arg arg[1];
   char text[10];
   static int count = 0;

   sprintf( text, " %d ", ++count );
   XtSetArg( arg[0], XtNlabel, text );
   XtSetValues( (Widget)closure, arg, (Cardinal)1 );
}

/* ARGSUSED */
void Toggle(widget, clientData, callData)
    Widget widget;
    caddr_t clientData;		/* Boolean* */
    caddr_t callData;
{
    Boolean *ok = (Boolean*)clientData;
    Arg args[1];

    if (*ok = !*ok)
	XtSetArg( args[0], XtNlabel, "ok" );
    else
	XtSetArg( args[0], XtNlabel, "not ok" );

    XtSetValues( widget, args, ONE );
}


/* ARGSUSED */
void Scrolled(widget, closure, callData)
    Widget widget;
    caddr_t closure;		/* unused */
    int callData;
{
    Arg args[1];
    Dimension height;
    XtSetArg( args[0], XtNheight, &height );
    XtGetValues( widget, args, ONE );
    height += callData;
    if (height < 4 || height > 9999) height = 4;
    XtSetArg( args[0], XtNheight, height );
    XtSetValues( widget, args, ONE );
}

/* ARGSUSED */
void Thumbed(widget, closure, top)
    Widget widget;
    caddr_t closure;		/* unused */
    float top;
{
    Arg args[1];

    sprintf( message, " Thumbed to %d%% ", (int)(top*100) );
    message[strlen(message)] = ' ';

    /* this is a complete crock, but Text needs to be re-done anyway.
     * the point is to just tickle the SetValues proc into doing a redisplay */
    leftMargin = (leftMargin == 2) ? 3 : 2;
    XtSetArg( args[0], XtNleftMargin, leftMargin );
    XtSetValues( textWidget, args, ONE );
}


/* ARGSUSED */
void Finish(client_data, id)
    caddr_t	 client_data;
    XtIntervalId id;
{
    XCloseDisplay((Display*)client_data);
    fprintf(stderr, "Bye!\n");
    exit(0);
}


/* ARGSUSED */
void Destroyed(widget, closure, callData)
    Widget widget;
    caddr_t closure;		/* unused */
    caddr_t callData;		/* unused */
{
    fprintf( stderr, "everything now destroyed; setting 5 second timer.\n" );
    XtAddTimeOut(5000, Finish, (caddr_t)XtDisplay(widget));
}


void main(argc, argv)
    unsigned int argc;
    char **argv;
{
    Widget toplevel, outer, form, viewport, box1, label, button, box2;
    Widget command;
    static XtCallbackRec callback[2]; /* K&R: initialized to NULL */
    Arg arg[10];
    Boolean ok = True;

    toplevel = XtInitialize( NULL, "XBoxes", options, XtNumber(options),
			     &argc, argv);

    if (argc != 1) Syntax(argv[0]);

    XtSetArg( arg[0], XtNwidth, 200 );
    outer = XtCreateManagedWidget( "vpaned", vPanedWidgetClass, toplevel,
				   arg, ONE );
    XtAddCallback( outer, XtNdestroyCallback, Destroyed, NULL );

    callback[0].callback = Quit;
    callback[0].closure = (caddr_t)toplevel;
    XtSetArg( arg[0], XtNcallback, callback );
    XtCreateManagedWidget( "quit", commandWidgetClass, outer, arg, ONE );

    XtSetArg( arg[0], XtNlabel, "at least one of everything!" );
    XtCreateManagedWidget( "label", labelWidgetClass, outer, arg, ONE );

    XtSetArg( arg[0], XtNlabel, "I am a dialog form" );
    XtSetArg( arg[1], XtNvalue, "enter new value here" );
    form = XtCreateManagedWidget( "form", dialogWidgetClass, outer, arg, TWO );

	XtSetArg( arg[0], XtNlabel, "ok" );
	command = XtCreateManagedWidget( "command", commandWidgetClass,
					 form, arg, ONE );

        XtAddCallback( command, XtNcallback, Toggle, (caddr_t)&ok );

    XtSetArg( arg[0], XtNallowVert, True );
    XtSetArg( arg[1], XtNallowHoriz, True );
    viewport = XtCreateManagedWidget( "viewport", viewportWidgetClass, outer,
				      arg, TWO );

	box1 = XtCreateManagedWidget( NULL, boxWidgetClass, viewport,
				      NULL, ZERO );

	    label = XtCreateManagedWidget(  "label", labelWidgetClass,
					    box1, NULL, ZERO );
	    button = XtCreateManagedWidget( "command", commandWidgetClass,
					    box1, NULL, ZERO );
	    XtAddCallback( button, XtNcallback, Count, (caddr_t)label );

    XtSetArg( arg[0], XtNmin, 16 );
    XtCreateManagedWidget( NULL, clockWidgetClass, outer, arg, ONE );

    XtCreateManagedWidget( NULL, loadWidgetClass, outer, NULL, ZERO );

    box2 = XtCreateManagedWidget( NULL, boxWidgetClass, outer, NULL, ZERO );

    XtSetArg( arg[0], XtNallowResize, True );
    XtSetValues( box2, arg, ONE );

    {
	XtCallbackRec thumbcallback[2];
	XtSetArg( arg[0], XtNheight, 100 );
	XtSetArg( arg[1], XtNscrollProc, callback );
        callback[0].callback = Scrolled;
        callback[0].closure = NULL;
	XtSetArg( arg[2], XtNthumbProc, thumbcallback );
	thumbcallback[0].callback = Thumbed;
	thumbcallback[1].closure = NULL;
	thumbcallback[1].callback = NULL;
	XtCreateManagedWidget( NULL, scrollbarWidgetClass, box2, arg, THREE );

	XtSetArg( arg[0], XtNstring, message );
	XtSetArg( arg[1], XtNleftMargin, leftMargin );
	textWidget = XtCreateManagedWidget( NULL, asciiStringWidgetClass,
					    box2, arg, TWO );
    }

    XtSetArg( arg[0], XtNfile, "/etc/motd" );
    XtSetArg( arg[1], XtNtextOptions, scrollHorizontal );
    XtCreateManagedWidget( NULL, asciiDiskWidgetClass, outer, arg, TWO );

    XtRealizeWidget(toplevel);
    XtMainLoop();
}
