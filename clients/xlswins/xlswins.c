#include <stdio.h>
#include <X11/Xos.h>
#include <X11/Xlib.h>

char *ProgramName;
static char *output_format = "0x%lx";

static void usage ()
{
    static char *help[] = {
"    -display displayname             X server to contact",
"    -format {hex, decimal, octal}    format used to print window id",
"    -indent number                   amount to indent per level",
"    -long                            print a long listing",
"",
NULL};
    char **cpp;

    fprintf (stderr, "usage:\n        %s [-options ...] [windowid] ...\n\n",
	     ProgramName);
    fprintf (stderr, "where options include:\n");
    for (cpp = help; *cpp; cpp++) {
	fprintf (stderr, "%s\n", *cpp);
    }
    exit (1);
}

static long parse_long (s)
    char *s;
{
    char *fmt = "%lu";
    long retval = 0;

    if (s && *s) {
	if (*s == '0') s++, fmt = "%lo";
	if (*s == 'x' || *s == 'X') s++, fmt = "%lx";
	(void) sscanf (s, fmt, &retval);
    }
    return retval;
}

static int got_xerror = 0;

static int myxerror (dpy, rep)
    Display *dpy;
    XErrorEvent *rep;
{
    char buffer[BUFSIZ];
    char mesg[BUFSIZ];
    char number[32];
    char *mtype = "XlibMessage";

    got_xerror++;
    if (rep->error_code == BadWindow) {
	fflush (stdout);
	fprintf (stderr, "%s:  no such window ", ProgramName);
	fprintf (stderr, output_format, rep->resourceid);
	fprintf (stderr, "\n");
	return 0;
    }
    XGetErrorText(dpy, rep->error_code, buffer, BUFSIZ);
    XGetErrorDatabaseText(dpy, mtype, "XError", "X Error", mesg, BUFSIZ);
    (void) fprintf(stderr, "%s: %s\n  ", mesg, buffer);
    XGetErrorDatabaseText(dpy, mtype, "MajorCode", "Request Major code %d", 
        mesg, BUFSIZ);
    (void) fprintf(stderr, mesg, rep->request_code);
    sprintf(number, "%d", rep->request_code);
    XGetErrorDatabaseText(dpy, "XRequest", number, "",  buffer, BUFSIZ);
    (void) fprintf(stderr, " %s", buffer);
    fputs("\n  ", stderr);
    XGetErrorDatabaseText(dpy, mtype, "MinorCode", "Request Minor code", 
        mesg, BUFSIZ);
    (void) fprintf(stderr, mesg, rep->minor_code);
    fputs("\n  ", stderr);
    XGetErrorDatabaseText(dpy, mtype, "ResourceID", "ResourceID 0x%x",
        mesg, BUFSIZ);
    (void) fprintf(stderr, mesg, rep->resourceid);
    fputs("\n  ", stderr);
    XGetErrorDatabaseText(dpy, mtype, "ErrorSerial", "Error Serial #%d", 
        mesg, BUFSIZ);
    (void) fprintf(stderr, mesg, rep->serial);
    fputs("\n  ", stderr);
    XGetErrorDatabaseText(dpy, mtype, "CurrentSerial", "Current Serial #%d",
        mesg, BUFSIZ);
    (void) fprintf(stderr, mesg, dpy->request);
    fputs("\n", stderr);
    if (rep->error_code == BadImplementation) return 0;
    exit (1);
}


main (argc, argv)
    int argc;
    char *argv[];
{
    char *displayname = NULL;
    Display *dpy;
    Bool long_version = False;
    int i;
    int indent = 2;

    ProgramName = argv[0];

    for (i = 1; i < argc; i++) {
	char *arg = argv[i];

	if (arg[0] == '-') {
	    switch (arg[1]) {
	      case 'd':			/* -display displayname */
		if (++i >= argc) usage ();
		displayname = argv[i];
		continue;
	      case 'i':			/* -indent number */
		if (++i >= argc) usage ();
		indent = atoi (argv[i]);
		continue;
	      case 'l':
		long_version = True;	/* -long */
		continue;
	      case 'f':			/* -format [odh] */
		if (++i >= argc) usage ();
		switch (argv[i][0]) {
		  case 'd':		/* decimal */
		    output_format = "%lu";
		    continue;
		  case 'o':		/* octal */
		    output_format = "0%lo";
		    continue;
		  case 'h':		/* hex */
		    output_format = "0x%lx";
		    continue;
		}
		usage ();

	      default:
		usage ();
	    }
	} else {
	    break;			/* out of for loop */
	}
    }

    dpy = XOpenDisplay (displayname);
    if (!dpy) {
	fprintf (stderr, "%s:  unable to open display \"%s\"\n",
		 ProgramName, XDisplayName (displayname));
	exit (1);
    }

    XSetErrorHandler (myxerror);

    if (i >= argc) {
	list_window (dpy, RootWindow (dpy, DefaultScreen (dpy)), 0, 
		     indent, long_version);
    } else {
	for (; i < argc; i++) {
	    Window w = parse_long (argv[i]);
	    if (w == 0) {
		fprintf (stderr, "%s:  bad window number \"%s\"\n",
			 ProgramName, argv[i]);
	    } else {
		list_window (dpy, w, 0, indent, long_version);
	    }
	}
    }

    XCloseDisplay (dpy);
    exit (0);
}


list_window (dpy, w, depth, indent, long_version)
    Display *dpy;
    Window w;
    int depth;
    int indent;
    Bool long_version;
{
    Window root, parent;
    unsigned int nchildren;
    Window *children = NULL;
    Status status;
    int n;
    char *name = NULL;

    if (long_version) {
	printf ("%d:  ", depth);
    }
    for (n = depth * indent; n > 0; n--) {
	putchar (' ');
    }
    printf (output_format, w);

    /*
     * if we put anything before the XFetchName then we'll have to change the
     * error handler
     */
    got_xerror = 0;
    if (XFetchName (dpy, w, &name)) {
	printf ("  (%s)", name ? name : "nil");
    } else {
	printf ("  ()");
    }
    if (name) free (name);
    if (long_version && got_xerror == 0) {
	int x, y, rx, ry;
	unsigned int width, height, bw, depth;

	if (XGetGeometry(dpy, w, &root, &x,&y,&width,&height, &bw, &depth)) {
	    Window child;

	    printf ("    %ux%u+%d+%d", width, height, x, y);
	    if (XTranslateCoordinates (dpy, w, root, 0, 0, &rx, &ry, &child)) {
		printf ("  +%d+%d", rx - bw, ry - bw);
	    }
	}
    }
    putchar ('\n');

    if (got_xerror) return;

    if (!XQueryTree (dpy, w, &root, &parent, &children, &nchildren))
      return;

    for (n = 0; n < nchildren; n++) {
	list_window (dpy, children[n], depth + 1, indent, long_version);
    }

    if (children) free ((char *) children);
    return;
}

