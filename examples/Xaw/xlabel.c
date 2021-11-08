#include <X11/Intrinsic.h>
#include <X11/Cardinals.h>
#include <X11/Label.h>
#include <stdio.h>

/* Command line options table.  Only resources are entered here...there is a
   pass over the remaining options after XtParseCommand is let loose. */

static XrmOptionDescRec options[] = {
{"-label",	"*label",	XrmoptionSepArg,	NULL}
};


/*
 * Report the syntax for calling xlabel
 */
Syntax(call)
    char *call;
{
    fprintf( stderr, "Usage: %s\n", call );
}

void main(argc, argv)
    unsigned int argc;
    char **argv;
{
    Widget toplevel;

    toplevel = XtInitialize( NULL, "XLabel",
			     options, XtNumber(options),
			     &argc, argv );

    if (argc != 1) Syntax(argv[0]);

    XtCreateManagedWidget( "label", labelWidgetClass, toplevel, NULL, ZERO );
    
    XtRealizeWidget(toplevel);
    XtMainLoop();
}
