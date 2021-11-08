#ifndef lint
static char rcsid[] = "$XConsortium: xload.c,v 1.16 88/10/18 14:07:18 swick Exp $";
#endif  lint

#include <X11/Intrinsic.h>
#include <X11/Xatom.h>
#include <X11/StringDefs.h>
#include <X11/Shell.h>
#include <X11/Load.h>
#include <stdio.h> 
#include "xload.bit"

extern void exit();

/* Command line options table.  Only resources are entered here...there is a
   pass over the remaining options after XtParseCommand is let loose. */

static XrmOptionDescRec options[] = {
{"-scale",	"*load.minScale",	XrmoptionSepArg,	 NULL},
{"-update",	"*load.update",		XrmoptionSepArg,	 NULL},
{"-hl",		"*load.highlight",	XrmoptionSepArg,	 NULL},
{"-highlight",	"*load.highlight",	XrmoptionSepArg,	 NULL},
};


/* Exit with message describing command line format */

void usage()
{
    fprintf(stderr,
"usage: xload [-fn {font}] [-update {seconds}] [-scale {integer}] [-rv]\n"
);
    fprintf(stderr,
"             [-geometry [{width}][x{height}][{+-}{xoff}[{+-}{yoff}]]] [-display [{host}]:[{vs}]]\n"
);
    fprintf(stderr,
"             [-fg {color}] [-bg {color}] [-bd {color}] [-bw {pixels}]\n");
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
    char host[256];
    Widget toplevel;
    Widget load;
    Arg arg;
    char *labelname = NULL;
    Pixmap icon_pixmap = None;
    
    toplevel = XtInitialize("main", "XLoad", options, XtNumber(options), &argc, argv);
      
    if (argc != 1) usage();
    
    XtSetArg (arg, XtNiconPixmap, &icon_pixmap);
    XtGetValues(toplevel, &arg, 1);
    if (icon_pixmap == None) {
	XtSetArg(arg, XtNiconPixmap, 
		 XCreateBitmapFromData(XtDisplay(toplevel),
				       XtScreen(toplevel)->root,
				       xload_bits, xload_width, xload_height));
	XtSetValues (toplevel, &arg, 1);
    }

    XtSetArg (arg, XtNlabel, &labelname);
    load = XtCreateManagedWidget ("load", loadWidgetClass, toplevel, NULL, 0);
    XtGetValues(load, &arg, 1);
    if (!labelname) {
       (void) gethostname (host, 255);
       XtSetArg (arg, XtNlabel, host);
       XtSetValues (load, &arg, 1);
    }
    XtRealizeWidget (toplevel);
    XtMainLoop();
}
