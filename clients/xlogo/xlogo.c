#ifndef lint
static char rcsid[] = "$XConsortium: xlogo.c,v 1.5 88/09/12 15:32:49 jim Exp $";
#endif  lint

#include <X11/Xlib.h>
#include <X11/StringDefs.h>
#include <X11/Intrinsic.h>
#include <X11/Shell.h>
#include <X11/Logo.h>
#include <X11/Cardinals.h>

extern void exit();

/*
 * Report the syntax for calling xclock.
 */
Syntax(call)
	char *call;
{
	(void) printf ("Usage: %s [-fg <color>] [-bg <color>] [-rv]\n", call);
	(void) printf ("       [-bw <pixels>] [-bd <color>]\n");
	(void) printf ("       [-d [<host>]:[<vs>]]\n");
	(void) printf ("       [-g [<width>][x<height>][<+-><xoff>[<+-><yoff>]]]\n\n");
	exit(1);
}

void main(argc, argv)
    int argc;
    char **argv;
{
    Widget toplevel;
    Pixmap icon;
    Arg arg;
    XGCValues  gcv;
    GC gcFore, gcBack;

    toplevel = XtInitialize("main", "XLogo", NULL, 0, &argc, argv);
    if (argc != 1) Syntax(argv[0]);
    icon = XCreatePixmap(XtDisplay(toplevel), XtScreen(toplevel)->root,
			 32, 32, 1);
    gcv.foreground = 1;
    gcFore = XCreateGC(XtDisplay(toplevel), icon, GCForeground, &gcv);
    gcv.foreground = 0;
    gcBack = XCreateGC(XtDisplay(toplevel), icon, GCForeground, &gcv);
    XDrawLogo(XtDisplay(toplevel), icon, gcFore, gcBack, 0, 0, 32, 32);
    XFreeGC(XtDisplay(toplevel), gcFore);
    XFreeGC(XtDisplay(toplevel), gcBack);
    arg.name = XtNiconPixmap;
    arg.value = (XtArgVal) icon;
    XtSetValues (toplevel, &arg, ONE); 
    XtCreateManagedWidget("xlogo", logoWidgetClass, toplevel, NULL, ZERO);
    XtRealizeWidget(toplevel);
    XtMainLoop();
}
