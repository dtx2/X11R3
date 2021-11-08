#include <stdio.h>
#include <X11/Intrinsic.h>
#include <X11/Command.h>

static XrmOptionDescRec options[] = {
{"-label",	"*label",	XrmoptionSepArg,	NULL}
};

Syntax(call)
    char *call;
{
    fprintf( stderr, "Usage: %s\n", call );
}


/* ARGSUSED */
void Activate(w, closure, call_data)
    Widget w;
    caddr_t closure;
    caddr_t call_data;
{
    printf( "button was activated.\n" );
}


void main(argc, argv)
    unsigned int argc;
    char **argv;
{
    Widget toplevel;
    static XtCallbackRec callbacks[] = {
      { Activate, NULL },
      { NULL, NULL },
    };

    static Arg args[] = {
      { XtNcallback, (XtArgVal)callbacks },
    };

    toplevel = XtInitialize( NULL, "Demo", options, XtNumber(options),
			     &argc, argv );

    if (argc != 1) Syntax(argv[0]);

    XtCreateManagedWidget( "command", commandWidgetClass, toplevel,
			   args, XtNumber(args) );
    
    XtRealizeWidget(toplevel);
    XtMainLoop();
}
