/* xclock -- 
 *  Hacked from Tony Della Fera's much hacked clock program.
 */
#ifndef lint
static char rcsid[] = "$XConsortium: xclock.c,v 1.21 88/10/18 14:05:32 swick Exp $";
#endif  lint

#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/Shell.h>
#include <X11/Clock.h>
#include <X11/Cardinals.h>
#include "clock.bit"

extern void exit();

/* Command line options table.  Only resources are entered here...there is a
   pass over the remaining options after XtParseCommand is let loose. */

static XrmOptionDescRec options[] = {
{"-chime",	"*clock.chime",		XrmoptionNoArg,		"TRUE"},
{"-hd",		"*clock.hands",		XrmoptionSepArg,	NULL},
{"-hands",	"*clock.hands",		XrmoptionSepArg,	NULL},
{"-hl",		"*clock.highlight",	XrmoptionSepArg,	NULL},
{"-highlight",	"*clock.highlight",	XrmoptionSepArg,	NULL},
{"-update",	"*clock.update",	XrmoptionSepArg,	NULL},
{"-padding",	"*clock.padding",	XrmoptionSepArg,	NULL},
{"-d",		"*clock.analog",	XrmoptionNoArg,		"FALSE"},
{"-digital",	"*clock.analog",	XrmoptionNoArg,		"FALSE"},
{"-analog",	"*clock.analog",	XrmoptionNoArg,		"TRUE"},
};


/*
 * Report the syntax for calling xclock.
 */
Syntax(call)
	char *call;
{
	(void) printf ("Usage: %s [-analog] [-bw <pixels>] [-digital]\n", call);
	(void) printf ("       [-fg <color>] [-bg <color>] [-hd <color>]\n");
	(void) printf ("       [-hl <color>] [-bd <color>]\n");
	(void) printf ("       [-fn <font_name>] [-help] [-padding <pixels>]\n");
	(void) printf ("       [-rv] [-update <seconds>] [-display displayname]\n");
	(void) printf ("       [-geometry geom]\n\n");
	exit(1);
}

#ifndef lint
/* this silliness causes the linker to include the VendorShell
 * module from Xaw, rather than the one from Xt.
 */
static Junk()
{
#include <X11/Vendor.h>
WidgetClass junk = vendorShellWidgetClass;
}
#endif

void main(argc, argv)
    int argc;
    char **argv;
{
    Widget toplevel;
    Arg arg;
    Pixmap icon_pixmap = None;

    toplevel = XtInitialize("main", "XClock", options, XtNumber(options), &argc, argv);
    if (argc != 1) Syntax(argv[0]);

    XtSetArg(arg, XtNiconPixmap, &icon_pixmap);
    XtGetValues(toplevel, &arg, ONE);
    if (icon_pixmap == None) {
	arg.value = (XtArgVal)XCreateBitmapFromData(XtDisplay(toplevel),
				       XtScreen(toplevel)->root,
				       clock_bits, clock_width, clock_height);
	XtSetValues (toplevel, &arg, ONE);
    }

    XtCreateManagedWidget ("clock", clockWidgetClass, toplevel, NULL, ZERO);
    XtRealizeWidget (toplevel);
    XtMainLoop();
}
