#ifndef lint
static char rcsid[] = "$XConsortium: xscroll.c,v 1.7 88/09/06 18:27:02 jim Exp $";
#endif lint

#include <X11/Intrinsic.h>
#include <X11/Scroll.h>
#include <stdio.h>

/* Command line options table.  Only resources are entered here...there is a
   pass over the remaining options after XtParseCommand is let loose. */

static XrmOptionDescRec options[] = {
{"-label",	"label",		 XrmoptionSepArg, NULL},
{"-horiz",	"scrollbar.orientation", XrmoptionNoArg,  "horizontal"}
};

char *ProgramName;


/*
 * Report the syntax for calling xcommand
 */
Syntax()
{
    fprintf( stderr, "Usage: %s\n", ProgramName );
}


void Scrolled(w, closure, call_data)
    Widget w;
    caddr_t closure;
    int call_data;
{
    printf( "scrolled by %d pixels.\n", call_data );
}


void Thumbed(w, closure, top)
    Widget w;
    caddr_t closure;
    float top;
{
    printf( "thumbed to %f%%\n", top );
}


void main(argc, argv)
    unsigned int argc;
    char **argv;
{
    Widget toplevel;
    static XtCallbackRec scrollCallbacks[] = {
      { Scrolled, NULL },
      { NULL, NULL },
    };

    static XtCallbackRec thumbCallbacks[] = {
      { Thumbed, NULL },
      { NULL, NULL },
    };

    static Arg args[] = {
      { XtNscrollProc,	(XtArgVal)scrollCallbacks },
      { XtNthumbProc,	(XtArgVal)thumbCallbacks },
      { XtNlength,	(XtArgVal)200 },
    };

    ProgramName = argv[0];

    toplevel = XtInitialize( NULL, "Demo", options, XtNumber(options),
			     &argc, argv );

    if (argc != 1) Syntax ();

    XtCreateManagedWidget( "scrollbar", scrollbarWidgetClass, toplevel,
			   (ArgList)args, XtNumber(args) );
    
    XtRealizeWidget(toplevel);
    XtMainLoop();
}
