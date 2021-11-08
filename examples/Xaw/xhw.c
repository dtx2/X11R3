#include <X11/Xlib.h>
#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/Label.h>

#define	STRING	"Hello,  World"

Arg wargs[] = {
    XtNlabel,	(XtArgVal) STRING,
};

main(argc, argv)
    int argc;
    char **argv;
{
    Widget      toplevel;

    /*
     * Create the Widget that represents the window.
     * See Section 14 of the Toolkit manual.
     */
    toplevel = XtInitialize(argv[0], "XLabel", NULL, 0, &argc, argv);

    /*
     * Create a Widget to display the string,  using wargs to set
     * the string as its value.  See Section 9.1.  Since we don't
     * need to change the child, we can also add it to the toplevel
     * widget's managed set right away, and we don't need to save
     * it's widget id.   See Section 13.5.2.
     */
    XtCreateManagedWidget(argv[0], labelWidgetClass,
			  toplevel, wargs, XtNumber(wargs));

    /*
     * Create the windows,  and set their attributes according
     * to the Widget data.  See Section 9.2.
     */
    XtRealizeWidget(toplevel);

    /*
     * Now process the events.  See Section 16.6.2.
     */
    XtMainLoop();
}
