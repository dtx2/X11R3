/*
 * $XConsortium: pseudoroot.c,v 1.12 88/09/06 14:26:54 jim Exp $
 *
 * Copyright 1988 Massachusetts Institute of Technology
 *
 *
 *				 W A R N I N G
 *
 * This is experimental code for implementing pseudo-root windows as specified
 * by the Inter-Client Communications Conventions Manual.  The interfaces that
 * it provides should be considered private to the MIT implementation of Xlib
 * and are SUBJECT TO CHANGE WITHOUT NOTICE.  They should not be incorporated
 * into any toolkits or applications.  When they change, no effort will be
 * made to provide backwards compatibility.
 *
 * Current questions about the implementation are delimited by triple X's.
 *
 * If you think this all sounds severe, you're right.
 */

#include <stdio.h>
#include <X11/Xos.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/bitmaps/gray1>
#include <ctype.h>
#include <signal.h>
#include "PseudoRoot.h"

/*
 * flags for indicating what was set on the command line
 */
#define PSWhitePixel (1L << 0L)
#define PSBlackPixel (1L << 1L)
#define PSMaxMaps (1L << 2L)
#define PSMinMaps (1L << 3L)
#define PSBackingStore (1L << 4L)
#define PSSaveUnders (1L << 5L)


/*
 * global data
 */
char *ProgramName;
static int abort_flag = 0;


/*
 * utility routines
 */
static void usage (fmt, arg)
    char *fmt, *arg;
{
    static char *msg[] = {
"    -display displayname        X server to contact",
"    -geometry geomspec          size and location of pseudo root",
"    -name string                window name",
"    -visuals visualid ...       list of visuals to provide",
"    -colormap colormapid        colormap to use by default",
"    -Colormap visualid          make a colormap from the indicated visual",
"    -white pixel                pixel number to use for WhitePixel",
"    -White colorname            color to use for WhitePixel",
"    -black pixel                pixel number to use for BlackPixel",
"    -Black colorname            color to use for BlackPixel",
"    -empty                      don't allocate BlackPixel and WhitePixel",
"    -max number                 maximum number of colormaps installed",
"    -min number                 minimum number of colormaps installed",
"    -backingstore when          NotUseful, WhenMapped, or Always",
"    -saveunders boolean         true or false",
"",
NULL};
    char **cpp;

    if (fmt) {
	fprintf (stderr, "%s:  ", ProgramName);
	fprintf (stderr, fmt, arg);
	fprintf (stderr, "\n\n");
    }
    fprintf (stderr,
"usage:\n        %s [-options ...] propname\n\nwhere options include:\n",
	     ProgramName);
    for (cpp = msg; *cpp; cpp++) {
	fprintf (stderr, "%s\n", *cpp);
    }
    exit (1);
}

static int intr_bump_flag (sig)
    int sig;
{
    signal (sig, intr_bump_flag);
    abort_flag++;
    return 0;
}

static XID parse_xid (s)
    register char *s;
{
    char *fmt = "%lu";
    long retval = 0;

    if (s && *s) {
	if (*s == '0') s++, fmt = "%lo";
	if (*s == 'x' || *s == 'X') s++, fmt = "%lx";
	(void) sscanf (s, fmt, &retval);
    }
    return (XID) retval;
}

static long parse_backing_store (s)
    register char *s;
{
    register char *cp = s;
    int len = 0;

    for (; *cp; cp++) {
	register char c = *cp;
	if (isascii (c) && isupper (c)) *cp = tolower (c);
    }
    len = cp - s;

    if (strncmp (s, "notuseful", len) == 0) return NotUseful;
    if (strncmp (s, "whenmapped", len) == 0) return WhenMapped;
    if (strncmp (s, "always", len) == 0) return Always;

    return NotUseful;
}

static Bool parse_bool (s)
    register char *s;
{
    register char *cp = s;
    int len = 0;

    for (; *cp; cp++) {
	register char c = *cp;
	if (isascii (c) && isupper (c)) *cp = tolower (c);
    }
    len = cp - s;

    if (strncmp (s, "true", len) == 0 || strncmp (s, "yes", len) == 0 ||
	strncmp (s, "on", 2) == 0) return True;
    return False;
}

void Exit (dpy, stat)
    Display *dpy;
    int stat;
{
    if (dpy) XCloseDisplay (dpy);
    exit (stat);
}

static Bool is_visualid_okay (visualid, pseudoDepths, pseudoVisualsList,
			      ndepths, allVisualInfos, nVisualInfos)
    VisualID visualid;
    PseudoDepth *pseudoDepths;
    PseudoVisual **pseudoVisualsList;
    int ndepths;
    XVisualInfo *allVisualInfos;
    int nVisualInfos;
{
    int i, j;

    if (pseudoDepths) {
	for (i = 0; i < ndepths; i++) {
	    for (j = 0; j < pseudoDepths[i].nvisuals; j++) {
		if (visualid == pseudoVisualsList[i][j].visualid)
		  return True;
	    }
	}
    } else {
	for (i = 0; i < nVisualInfos; i++) {
	    if (visualid == allVisualInfos[i].visualid)
	      return True;
	}
    }
    return False;
}

/*
 * create the pseudo root window; since we don't want to select on it directly,
 * create a frame in which to store it
 */
static Window create_window (dpy, screenno, ps, hintsp, name, borderwidth,
			     allVisualInfos, nVisualInfos, framePtr)
    Display *dpy;
    int screenno;
    PseudoScreen *ps;
    XSizeHints *hintsp;
    char *name;
    int borderwidth;
    XVisualInfo *allVisualInfos;
    int nVisualInfos;
    Window *framePtr;
{
    XSetWindowAttributes attr;
    Window realRoot = RootWindow (dpy, screenno);
    int i;
    XVisualInfo *v = NULL;
    Pixmap bg;
    Window frame, pseudoRoot;
    unsigned long mask = 0;

    for (i = 0; i < nVisualInfos; i++) {
	if (ps->root_visual == allVisualInfos[i].visualid)
	  v = &allVisualInfos[i];
    }

    if (!v) {
	fprintf (stderr,
		 "%s:  unable to find visual for root visual id 0x%lx\n",
		 ProgramName, ps->root_visual);
	Exit (dpy, 1);
    }

    bg = XCreatePixmapFromBitmapData (dpy, realRoot, gray1_bits, gray1_width,
				      gray1_height, ps->black_pixel,
				      ps->white_pixel, v->depth);
    if (bg == None) {
	fprintf (stderr, "%s:  unable to create gray pixmap\n",
		 ProgramName);
	Exit (dpy, 1);
    }

    /*
     * We need to create a frame window so that we have something on which to
     * select for events.  Afterword, we'll create an inner window of the same
     * size.
     */

    attr.background_pixmap = None;
    attr.border_pixel = ps->black_pixel;
    attr.backing_store = NotUseful;
    attr.event_mask = StructureNotifyMask;
    attr.colormap = ps->cmap;
    mask = (CWBackPixmap | CWBorderPixel | CWBackingStore | CWEventMask |
	    CWColormap);
			      
    frame = XCreateWindow (dpy, realRoot, hintsp->x, hintsp->y, hintsp->width,
			   hintsp->height, borderwidth, v->depth, InputOutput,
			   v->visual, mask, &attr);
    XStoreName (dpy, frame, name);
    XSetNormalHints (dpy, frame, hintsp);

    attr.background_pixmap = bg;
    attr.border_pixmap = None;
    attr.backing_store = ps->backing_store;
    attr.save_under = ps->save_unders;
    attr.event_mask = 0;
    attr.colormap = ps->cmap;
    mask = (CWBackPixmap | CWBorderPixel | CWBackingStore | CWEventMask |
	    CWColormap);

    pseudoRoot = XCreateWindow (dpy, frame, 0,0, hintsp->width, hintsp->height,
				0, v->depth, InputOutput, v->visual, mask,
				&attr);
    {
	char buf[256];

	sprintf (buf, "%s root window", name);
	XStoreName (dpy, pseudoRoot, buf);
    }
    XMapWindow (dpy, pseudoRoot);

    *framePtr = frame;
    return pseudoRoot;
}

/*
 * recompute the size fields
 */
static void set_size (dpy, screenno, ps, width, height)
    Display *dpy;
    int screenno;
    PseudoScreen *ps;
    int width, height;
{
    ps->width = width;
    ps->height = height;
    ps->mwidth = ((((unsigned long) DisplayWidthMM (dpy, screenno)) *
		       ps->width) /
		      (unsigned long) DisplayWidth(dpy, screenno));

    ps->mheight = ((((unsigned long) DisplayHeightMM (dpy, screenno)) *
		       ps->height) /
		      (unsigned long) DisplayHeight(dpy, screenno));
    return;
}


static Bool is_id (s)
    char *s;
{
    char *allowed = "0123456789";

    if (*s == '0') s++, allowed = "01234567";
    if (*s == 'x' || *s == 'X') s++, allowed = "0123456789abcdefABCDEF";
    for (; *s; s++) {
	if (index (allowed, *s) == NULL) return False;
    }
    return True;
}


/*
 * the main program; it could use a little more modularity
 */

main (argc, argv)
    int argc;
    char *argv[];
{
    char *displayname = NULL;		/* name of X server to contact */
    char *geometry = NULL;		/* size of initial root window */
    int borderwidth = 2;		/* size of border around pseudo root */
    char namebuf[256], *name = NULL;	/* for identifying frame */
    char *propName = NULL;		/* property on which to store */
    Atom screenProp;			/* id of property */
    Atom screenType = None;		/* filled in later */
    int nvisualids = 0;			/* number of visual ids to copy */
    VisualID *visualids = NULL;		/* visuals to copy */
    PseudoScreen pscreen;		/* pseudo screen data */
    PseudoDepth *pseudoDepths = NULL;	/* list of specific pseudo depths */
    PseudoVisual **pseudoVisualsList = NULL; /* corresponding pseudo visuals */
    int ndepths;			/* number of above arrays */
    XVisualInfo *allVisualInfos;	/* all visuals on this screen */
    int nVisualInfos;			/* count of above array */
    VisualID colormap_visualid = None;	/* visual from which to make cmap */
    Bool empty_colormap = False;	/* t: don't alloc B&W in new cmaps */
    char *white_name = NULL, *black_name = NULL;  /* colors for b&w pixels */
    int i, j, n;			/* temp variables */
    unsigned long psmask = 0L;		/* mask of things that are preset */
    Display *dpy;			/* connection to X server */
    int screenno;			/* default screen number */
    Screen *scr;			/* struct for screenno */
    Window rootw, frame;		/* root and windows to create */
    XSizeHints hints;			/* for creating window */

    ProgramName = argv[0];
    pscreen.root = None;
    pscreen.width = pscreen.height = pscreen.mwidth = pscreen.mheight = 0;
    pscreen.root_visual = None;
    pscreen.cmap = None;
    pscreen.white_pixel = pscreen.black_pixel = 0;
    pscreen.max_maps = pscreen.min_maps = 0;
    pscreen.backing_store = NotUseful;
    pscreen.save_unders = False;
    pscreen.root_input_mask = 0;
    pscreen.depths = None;

    for (i = 1; i < argc; i++) {
	char *arg = argv[i];

	if (arg[0] == '-') {
	    switch (arg[1]) {
	      case 'h':			/* -help */
		usage (NULL, NULL);
	      case 'd':			/* -display displayname */
		if (++i >= argc) usage ("missing displayname on -display",
					NULL);
		displayname = argv[i];
		continue;
	      case 'g':			/* -geometry geomspec */
		if (++i >= argc) usage ("missing geomspec on -geometry", NULL);
		geometry = argv[i];
		continue;
	      case 'v':			/* -visuals visualid ... */
		if (++i >= argc) usage ("missing visualid on -visuals", NULL);
		for (j = i;
		     j < argc && argv[j][0] != '-' && is_id (argv[j]);
		     j++) ;

		/* j is one beyond list of visualids */
		n = j - i;
		if (visualids) {
		    visualids = (VisualID *) realloc (visualids,
						      n * sizeof(VisualID));
		} else {
		    visualids = (VisualID *) malloc (n * sizeof(VisualID));
		}
		if (!visualids) {
		    fprintf (stderr, 
		 "%s:  unable to allocate %d bytes for visual id list\n",
			     ProgramName, n * sizeof(VisualID));
		    exit (1);
		}
		for (; i < j; i++) {
		    visualids[nvisualids++] = parse_xid (argv[i]);
		}
		i--;
		continue;
	      case 'r':			/* -rootvisual visualid */
		if (++i >= argc) usage ("missing visualid on -rootvisual",
					NULL);
		pscreen.root_visual = parse_xid (argv[i]);
		continue;
	      case 'c':			/* -colormap colormapid */
		if (++i >= argc) usage ("missing colormap on -colormap", NULL);
		pscreen.cmap = parse_xid (argv[i]);
		continue;
	      case 'C':			/* -Colormap visualid */
		if (++i >= argc) usage ("missing visualid on -Colormap", NULL);
		colormap_visualid = parse_xid (argv[i]);
		continue;
	      case 'w':			/* -white pixel */
		if (++i >= argc) usage ("missing pixel on -white", NULL);
		pscreen.white_pixel = parse_xid (argv[i]);
		psmask |= PSWhitePixel;
		continue;
	      case 'W':			/* -White colorname */
		if (++i >= argc) usage ("missing color name on -White", NULL);
		white_name = argv[i];
		continue;
	      case 'b':
		switch (arg[2]) {
		  case 'l':		/* -black pixel */
		    if (++i >= argc) usage ("missing pixel on -black", NULL);
		    pscreen.black_pixel = parse_xid (argv[i]);
		    psmask |= PSBlackPixel;
		    continue;
		  case 'a':	/* -backingstore NotUseful,WhenMapped,Always */
		    if (++i >= argc) usage ("missing when on -backingstore",
					    NULL);
		    pscreen.backing_store = parse_backing_store (argv[i]);
		    continue;
		}
		break;			/* out of switch */
	      case 'B':			/* -BLACK colorname */
		if (++i >= argc) usage ("missing color name on -Black", NULL);
		black_name = argv[i];
		continue;
	      case 'm':
		switch (arg[2]) {
		  case 'a':		/* -max number */
		    if (++i >= argc) usage ("missing count on -max", NULL);
		    pscreen.max_maps = atoi (argv[i]);
		    continue;
		  case 'i':		/* -min number */
		    if (++i >= argc) usage ("missing count on -min", NULL);
		    pscreen.min_maps = atoi (argv[i]);
		    continue;
		}
		break;			/* out of switch */
	      case 'e':			/* -empty */
		empty_colormap = True;
		continue;
	      case 's':			/* -saveunders boolean */
		if (++i >= argc) usage ("missing boolean on -saveunders",NULL);
		pscreen.save_unders = parse_bool (argv[i]);
		continue;
	      case 'n':			/* -name string */
		if (++i >= argc) usage ("missing string on -name", NULL);
		name = argv[i];
		continue;
	    }
	    usage ("unknown command line option \"%s\"", arg);
	} else {
	    if (propName) usage ("extra property name \"%s\"", argv[i]);
	    propName = argv[i];
	    continue;
	}
    }

    /*
     * do initial requirements
     */
    if (!propName) usage("no property name given", NULL);


    /*
     * set up initial data structures
     */
    if (!name) {
	strcpy (namebuf, propName);
	name = namebuf;
    }

    dpy = XOpenDisplay (displayname);
    if (!dpy) {
	fprintf (stderr, "%s:  unable to open display \"%s\"\n",
		 ProgramName, XDisplayName (displayname));
	exit (1);
    }
    screenno = DefaultScreen (dpy);
    rootw = RootWindow (dpy, screenno);
    scr = ScreenOfDisplay (dpy, screenno);

    screenProp = XInternAtom (dpy, propName, False);
    if (!screenProp) {
	fprintf (stderr, "%s:  unable to create atom for property \"%s\"\n",
		 ProgramName, propName);
	Exit (dpy, 1);
    }

    screenType = XInternAtom (dpy, PSEUDO_SCREEN_TYPE, False);
    if (screenType == None) {
	fprintf (stderr, 
		 "%s:  unable to create atom for screen type \"%s\"\n",
		 ProgramName, PSEUDO_SCREEN_TYPE);
	Exit (dpy, 1);
    }

    /*
     * if we have specified a list of visuals, create a set of PseudoDepths
     * and a matching array of visuals.
     */
    get_pseudo_depths_and_visuals (dpy, screenno, visualids, nvisualids,
				   &allVisualInfos, &nVisualInfos,
				   &pseudoDepths, &pseudoVisualsList,
				   &ndepths);
    /*
     * verify that the root visual is one of the ones supported
     */
    if (pscreen.root_visual == None) {
	pscreen.root_visual = XVisualIDFromVisual(DefaultVisual(dpy,screenno));
    }

    if (!is_visualid_okay (pscreen.root_visual,
			   pseudoDepths, pseudoVisualsList, ndepths,
			   allVisualInfos, nVisualInfos)) {
	fprintf (stderr,
		 "%s:  root visual 0x%lx not valid for pseudo root\n",
		 ProgramName, pscreen.root_visual);
	Exit (dpy, 1);
    }

    /*
     * figure out if we need to create a new colormap
     */
    if (!(psmask & PSWhitePixel)) {
	pscreen.white_pixel = WhitePixel (dpy, screenno);
	psmask &= PSWhitePixel;
    }
    if (!(psmask & PSBlackPixel)) {
	pscreen.black_pixel = BlackPixel (dpy, screenno);
	psmask &= PSBlackPixel;
    }
    if (pscreen.cmap == None) {
	if (colormap_visualid == None) {
	    if (pscreen.root_visual == 
		XVisualIDFromVisual (DefaultVisual (dpy, screenno))) {
		pscreen.cmap = DefaultColormap (dpy, screenno);
	    } else {
		colormap_visualid = pscreen.root_visual;
	    }
	}
	if (colormap_visualid != None) {
	    if (!is_visualid_okay (colormap_visualid,
				   pseudoDepths, pseudoVisualsList, ndepths,
				   allVisualInfos, nVisualInfos)) {
		fprintf (stderr,
		        "%s:  visual 0x%lx not found, can't create colormap\n",
			 ProgramName, colormap_visualid);
		Exit (dpy, 1);
	    }
	    create_colormap (dpy, screenno, colormap_visualid, &pscreen,
			     allVisualInfos, nVisualInfos, white_name,
			     black_name, empty_colormap);
	}
	/* okay, pscreen.cmap is now set */
    }

    /*
     * fill in missing fields
     */
    hints.flags =  XGeometry (dpy, screenno, geometry, "600x400+200+10", 
			      borderwidth, 1, 1, 0, 0, &hints.x, &hints.y,
			      &hints.width, &hints.height);

    set_size (dpy, screenno, &pscreen, hints.width, hints.height);

    if (!(psmask & PSMaxMaps)) {
	pscreen.max_maps = MaxCmapsOfScreen (scr);
	psmask |= PSMaxMaps;
    }
    if (!(psmask & PSMinMaps)) {
	pscreen.min_maps = MinCmapsOfScreen (scr);
	psmask |= PSMinMaps;
    }
    if (!(psmask & PSBackingStore)) {
	pscreen.backing_store = DoesBackingStore (scr);
	psmask |= PSBackingStore;
    }
    if (!(psmask & PSSaveUnders)) {
	pscreen.save_unders = DoesSaveUnders (scr);
	psmask |= PSSaveUnders;
    }

    pscreen.root = create_window (dpy, screenno, &pscreen, &hints, name,
				  borderwidth, allVisualInfos, nVisualInfos,
				  &frame);

    /*
     * now store the data on the window, taking care to establish 
     * signal handlers first; otherwise, we could end up leaving garbage
     * properties hanging around
     */
    (void) signal (SIGHUP, intr_bump_flag);
    (void) signal (SIGINT, intr_bump_flag);
    (void) signal (SIGTERM, intr_bump_flag);

    if (nvisualids > 0) {
	store_depths_and_visuals (dpy, rootw, &pscreen, name, pseudoDepths,
				  pseudoVisualsList, ndepths);
    }
    store_pseudo_screen (dpy, rootw, &pscreen, screenProp, screenType);

    /*
     * Everything is all set; make it visual and go into an event handling
     * loop; at this time, all we do is wait for reconfigure notices so that
     * we can reset the size fields in the PseudoScreen structure.
     */
    XMapWindow (dpy, frame);
    while (!abort_flag) {
	XEvent event;

	if (!XPending (dpy)) {		/* likely to block */
	    if (!wait_for_input (dpy)) break;
	}
	XNextEvent (dpy, &event);

	if (event.type == ConfigureNotify) {
	    XConfigureEvent *e = (XConfigureEvent *) &event;
	    
	    if (e->window == frame) {
		set_size (dpy, screenno, &pscreen, e->width, e->height);
		store_pseudo_screen (dpy, rootw, &pscreen, screenProp,
				     screenType);
	    }
	}
	/* ignore other events */
    }
    XUnmapWindow (dpy, frame);
    delete_properties (dpy, rootw, screenProp, &pscreen,pseudoDepths, ndepths);
    Exit (dpy, 0);
}


/*
 * cruddy routine needed to catch ^C
 */
int wait_for_input (dpy)
    Display *dpy;
{
    int fd = ConnectionNumber (dpy);
    int nfds, readfds;

    readfds = (1 << fd);
    nfds = select (fd+1, &readfds, (int *) NULL, (int *) NULL, 
		   (struct timeval *) NULL);
    return (nfds != -1 ? 1 : 0);
}


/*
 * Create a new colormap from the indicated visual, taking into account the
 * prefered colors for black and white if possible.  Make sure that the
 * screen's BlackPixel and WhitePixel still map to the right place.
 */
create_colormap (dpy, screenno, visualid, ps, allVisualInfos, nVisualInfos, 
		 white_name, black_name, empty)
    Display *dpy;
    int screenno;
    VisualID visualid;
    PseudoScreen *ps;
    XVisualInfo *allVisualInfos;
    int nVisualInfos;
    char *white_name, *black_name;
    Bool empty;
{
    int i, j, k;
    unsigned long nentries = 0;
    XColor whitecdef, blackcdef;
    unsigned long planes[1], *pixels;

    whitecdef.pixel = ps->white_pixel;
    whitecdef.red = whitecdef.green = whitecdef.blue = ~0;
    whitecdef.flags = DoRed | DoGreen | DoBlue;

    blackcdef.pixel = ps->black_pixel;
    whitecdef.red = whitecdef.green = whitecdef.blue = 0;
    blackcdef.flags = DoRed | DoGreen | DoBlue;
    
    /*
     * now that we know it is okay, create a colormap for it
     */
    ps->cmap = None;
    for (i = 0; i < nVisualInfos; i++) {
	if (visualid == allVisualInfos[i].visualid) {
	    nentries = allVisualInfos[i].colormap_size;
	    ps->cmap = XCreateColormap (dpy, RootWindow(dpy, screenno),
					    allVisualInfos[i].visual,
					    AllocNone);
	    
	    break;
	}
    }

    if (ps->cmap == None) {
	fprintf (stderr, 
		 "%s:  unable to create colormap for visual 0x%lx\n",
		 ProgramName, visualid);
	Exit (dpy, 1);
    }


    /*
     * now check the pixel values
     */
    if (ps->white_pixel >= nentries) {
	fprintf (stderr,
		 "%s:  WhitePixel %lu is larger than %lu entry colormap\n",
		 ProgramName, ps->white_pixel, nentries);
	Exit (dpy, 1);
    }
    if (ps->black_pixel >= nentries) {
	fprintf (stderr,
		 "%s:  BlackPixel %lu is larger than %lu entry colormap\n",
		 ProgramName, ps->black_pixel, nentries);
	Exit (dpy, 1);
    }

    /*
     * If we don't want to preallocate BlackPixel and WhitePixel (to squeeze
     * out the maximum number of colors in the colormap) then skip the rest.
     */
    if (empty) return;


    /*
     * if a color name was given, try to use it
     */
    if (white_name && !XParseColor (dpy, ps->cmap, white_name, &whitecdef)) {
	fprintf (stderr, "%s:  warning, ignoring bad color name \"%s\"\n",
		 ProgramName, white_name);
    }
    if (black_name && !XParseColor (dpy, ps->cmap, black_name, &blackcdef)) {
	fprintf (stderr, "%s:  warning, ignoring bad color name \"%s\"\n",
		 ProgramName, black_name);
    }


    /* 
     * now allocate all of the cells so that we can set black pixel and 
     * white pixel if possible
     */

    pixels = (unsigned long *) malloc (nentries * sizeof(unsigned long));
    if (!pixels) {
	fprintf (stderr,
		 "%s:  unable to allocate %d pixels from new colormap\n",
		 ProgramName, nentries);
	Exit (dpy, 1);
    }

    if (!XAllocColorCells (dpy, ps->cmap, False, planes, 0, pixels, 
			   nentries)) {
	fprintf (stderr,
		 "%s:  unable to allocate %d colors from new colormap\n",
		 ProgramName, nentries);
	Exit (dpy, 1);
    }

    /*
     * iterate over pixels, freeing the ones that we aren't using; this is a
     * bit of a hack since real pseudo root applications probably won't care
     * what BlackPixel and WhitePixel are set to, but since this is a 
     * diagnostic program....
     */
    for (j = k = 0; j < nentries; j++) {
	if (pixels[j] == ps->black_pixel || pixels[j] == ps->white_pixel) {
	    if (j > k) {
		XFreeColors (dpy, ps->cmap, pixels + k, j - k, 0);
		k = j + 1;
	    }
	    XStoreColor (dpy, ps->cmap,
			 (pixels[j] == ps->black_pixel ?
			  &blackcdef : &whitecdef));
	}
    }
    if (j > k) {
	XFreeColors (dpy, ps->cmap, pixels + k, j - k, 0);
    }

    free ((char *) pixels);
    return;
}


/*
 * Fetch the PseudoDepths and PseudoVisuals, and create the structures that 
 * will used to build the parallel Depth and Visual trees.  There is a little
 * bit of hair since we need to source the visuals into depth sets.
 */
get_pseudo_depths_and_visuals (dpy, screenno, visualids, nvisualids,
			       allVisualInfosPtr, nVisualInfosPtr,
			       pseudoDepthsPtr, pseudoVisualsListPtr,
			       ndepthsPtr)
    Display *dpy;
    int screenno;
    VisualID *visualids;
    int nvisualids;
    XVisualInfo **allVisualInfosPtr;	/* RETURN */
    int *nVisualInfosPtr;		/* RETURN */
    PseudoDepth **pseudoDepthsPtr;	/* RETURN */
    PseudoVisual ***pseudoVisualsListPtr;  /* RETURN */
    int *ndepthsPtr;			/* RETURN */
{
    int i, j, n;
    XVisualInfo *allVisualInfos, **realVisualsList, proto, *vip;
    struct _viplist {
	int count;
	XVisualInfo *visual;
	struct _viplist *next;
    } *vl, *vlp;
    PseudoDepth *pseudoDepths = NULL;
    PseudoVisual **pseudoVisualsList = NULL;
    int ndepths = 0;
    int nvisualsonscreen;
    
    /*
     * get all of the available visuals on this screen
     */
    proto.screen = screenno;
    allVisualInfos = XGetVisualInfo (dpy, VisualScreenMask, &proto, 
				     &nvisualsonscreen);
    if (!allVisualInfos) {
	fprintf (stderr, "%s:  unable to get visuals for screen %d\n",
		 ProgramName, proto.screen);
	Exit (dpy, 1);
    }

    *allVisualInfosPtr = allVisualInfos;
    *nVisualInfosPtr = nvisualsonscreen;
    *pseudoDepthsPtr = NULL;
    *pseudoVisualsListPtr = NULL;
    *ndepthsPtr = 0;

    if (nvisualids == 0)
      return;

    /*
     * okay, since we do have some specific visuals, we need to create
     * a list to look at
     */
    realVisualsList = (XVisualInfo **) malloc (nvisualids *
					       sizeof(XVisualInfo *));
    if (!realVisualsList) {
	fprintf (stderr,
		 "%s:  unable to get space for %d visual pointers\n",
		 ProgramName, nvisualids);
	Exit (dpy, 1);
    }
    
    /*
     * walk the list of specified visuals checking to see if each one is in
     * the list of visuals on this screen
     */
    for (i = 0; i < nvisualids; i++) {
	realVisualsList[i] = NULL;
	for (j = 0; j < nvisualsonscreen; j++) {
	    if (visualids[i] == allVisualInfos[j].visualid) {
		realVisualsList[i] = allVisualInfos + j;
		break;			/* out of for loop */
	    }
	}
	if (!realVisualsList[i]) {
	    fprintf (stderr, "%s:  no such visual 0x%lx on screen %d\n",
		     ProgramName, visualids[i], screenno);
	    Exit (dpy, 1);
	}
    }

    /*
     * allocate a list of pointers to the records
     */

    vl = (struct _viplist *) malloc (nvisualids *
				     sizeof(struct _viplist));
    for (i = 0, vlp = vl; i < nvisualids; i++, vlp++) {
	vlp->count = 1;
	vlp->visual = realVisualsList[i];
	vlp->next = NULL;
    }

    /*
     * run down the list from top to bottom building a set of counted
     * linked lists; this also allows us to count the number of chains
     */

    ndepths = 0;
    n = nvisualids - 1;
    vlp = vl + n;
    for (i = n; i >= 0; i--, vlp--) {
	struct _viplist *prev = vlp - 1;
	
	for (j = i - 1; j >= 0; j--, prev--) {
	    if (vlp->visual->depth == prev->visual->depth) {
		prev->count = vlp->count + 1;
		prev->next = vlp;
		break;		/* just out of inner loop */
	    }
	}
	if (j < 0) ndepths++;
    }

    /*
     * allocate PseudoDepth and PseudoVisual structures (or places for
     * lists in the latter case)
     */
    pseudoDepths = (PseudoDepth *) calloc (ndepths,
					   sizeof(PseudoDepth));
    pseudoVisualsList = (PseudoVisual **) calloc (ndepths,
						  sizeof(PseudoVisual *));
    if (!pseudoDepths || !pseudoVisualsList) {
	fprintf (stderr,
		 "%s:  unable to allocate %d pseudo depth and visuals\n",
		 ProgramName, ndepths);
	Exit (dpy, 1);
    }
    
    /*
     * Run down the list of visuals building up Depth and Visual trees
     * from the chains that we just created.  We set the visuals field in
     * the PseudoDepth to be the atom XA_VISUALID to indicate that the
     * associated visual tree needs to be copied out.  We don't just free
     * space used by the uncopied visuals since they make life easier later.
     */
    n = 0;
    for (i = 0, vlp = vl; i < nvisualids; i++, vlp++) {
	if (vlp->count > 0) {
	    PseudoDepth *d = pseudoDepths + n;
	    PseudoVisual *v;
	    struct _viplist *srcvlp;
	    
	    d->depth = vlp->visual->depth;
	    d->nvisuals = vlp->count;
	    d->visuals = XA_VISUALID;
	    v = (PseudoVisual *) calloc ((unsigned) d->nvisuals,
					 sizeof(PseudoVisual));
	    if (!v) {
		fprintf (stderr,
			 "%s:  unable to allocate %u pseudo visuals\n",
			 ProgramName, (unsigned) d->nvisuals);
		Exit (dpy, 1);
	    }
	    pseudoVisualsList[n] = v;
	    /*
	     * run down list creating pseudo visual and marking
	     */
	    srcvlp = vlp;		/* points to real visual infos */
	    do {
		vip = srcvlp->visual;
		
		v->visualid = vip->visualid;
		v->class = vip->class;
		v->red_mask = vip->red_mask;
		v->green_mask = vip->green_mask;
		v->blue_mask = vip->blue_mask;
		v->bits_per_rgb = vip->bits_per_rgb;
		v->map_entries = vip->colormap_size;
		v++;
		
		srcvlp->count *= -1;  /* mark it */
		srcvlp = srcvlp->next;
	    } while (srcvlp);
	    n++;
	}
    }

    /*
     * Walk down each of the visuals lists to see if we have all of them
     * for a given depth.  If so, then we don't need to define a pseudo
     * visual structure.  If there are no duplicates in either the list of
     * real visuals or the list of our visuals, we can simply count the
     * numbers of each depth and match them up.  This means that if you are
     * creating multiple instances of visuals that you won't get the
     * optimization, but then again you're really screwing around anyway.
     */

    for (i = 0; i < nvisualsonscreen; i++) {
	for (j = 0; j < i; j++) {
	    if (allVisualInfos[j].visualid == allVisualInfos[i].visualid) {
		goto done;		/* C sucks */
	    }
	}
    }
    for (i = 0; i < nvisualids; i++) {
	for (j = 0; j < i; j++) {
	    if (realVisualsList[j]->visualid == realVisualsList[i]->visualid) {
		goto done;		/* C sucks */
	    }
	}
    }

    /*
     * no duplicates found, so go ahead and optimize
     */
    for (i = 0; i < ndepths; i++) {
	int depth = pseudoDepths[i].depth;
	/*
	 * count the number of real visuals that have this depth
	 */
	n = 0;
	for (j = 0; j < nvisualsonscreen; j++) {
	    if (depth == allVisualInfos[j].depth) n++;
	}
	/*
	 * see if it matches the number that we are storing
	 */
	if (n == pseudoDepths[i].nvisuals) {
	    pseudoDepths[i].visuals = None;
	}
    }

  done:
    *pseudoDepthsPtr = pseudoDepths;
    *pseudoVisualsListPtr = pseudoVisualsList;
    *ndepthsPtr = ndepths;
    return;
}


/*
 * create properties for the visuals and depths; this routine should only be
 * be called once.
 */
store_depths_and_visuals (dpy, rw, ps, screenPropName, pseudoDepths,
			  pseudoVisualsList, ndepths)
    Display *dpy;
    Window rw;
    PseudoScreen *ps;
    char *screenPropName;
    PseudoDepth *pseudoDepths;
    PseudoVisual **pseudoVisualsList;
    int ndepths;
{
    int i;				/* iterator variable */
    Atom id;				/* for setting props */
    Atom depthsType, visualsType;	/* if needed */
    char propname[256];		/* for holding names */

    if (ndepths <= 0) {
	ps->depths = None;
	return;
    }

    depthsType = XInternAtom (dpy, PSEUDO_DEPTHS_TYPE, False);
    if (!depthsType) {
	fprintf (stderr, "%s:  unable to create depths atom type \"%s\"\n",
		 ProgramName, PSEUDO_DEPTHS_TYPE);
	Exit (dpy, 1);
    }

    visualsType = XInternAtom (dpy, PSEUDO_VISUALS_TYPE, False);
    if (visualsType == None) {
	fprintf (stderr,
		 "%s:  unable to create visuals atom type \"%s\"\n",
		 ProgramName, PSEUDO_VISUALS_TYPE);
	Exit (dpy, 1);
    }

    for (i = 0; i < ndepths; i++) {
	if (pseudoDepths[i].visuals != None) {
	    /* build a unique visual id */
	    sprintf (propname, "%s_%s_%u", screenPropName, 
		     PSEUDO_VISUALS_TYPE, i + 1);
	    id = XInternAtom (dpy, propname, False);
	    if (id == None) {
		fprintf (stderr, "%s:  unable to create visual atom \"%s\"\n",
			 ProgramName, propname);
		Exit (dpy, 1);
	    }
	    if (!_XSetPseudoVisuals (dpy, rw, id, visualsType,
				     pseudoVisualsList[i],
				     (int) pseudoDepths[i].nvisuals)) {
		fprintf (stderr, 
			 "%s:  unable to set %d visuals in property \"%s\"\n",
			 ProgramName, (int) pseudoDepths[i].nvisuals,
			 propname);
		goto hack_abort;
	    }
	    /*
	     * and bash the depth array; this means that this routine can
	     * only be called once
	     */
	    pseudoDepths[i].visuals = id;
	}
    }

    /*
     * store the depth 
     */
    sprintf (propname, "%s_%s", screenPropName, PSEUDO_DEPTHS_TYPE);
    id = XInternAtom (dpy, propname, False);
    if (id == None) {
	fprintf (stderr, "%s:  unable to create depth atom \"%s\"\n",
			 ProgramName, propname);
	Exit (dpy, 1);
    }

    if (!_XSetPseudoDepths (dpy, rw, id, depthsType, pseudoDepths, ndepths)) {
	fprintf (stderr,
		 "%s:  unable to set property \"%s\" to %d depths\n",
		 ProgramName, propname, ndepths);
      hack_abort:
	for (i = 0; i < ndepths; i++) {
	    if (pseudoDepths[i].visuals != None &&
		pseudoDepths[i].visuals != XA_VISUALID) {
		XDeleteProperty (dpy, rw, pseudoDepths[i].visuals);
	    }
	}
	Exit (dpy, 1);
    }
    ps->depths = id;

    return;
}


/*
 * Store the PseudoScreen onto the property; called initially and whenever
 * the pseudo root changes size.
 */
store_pseudo_screen (dpy, rw, ps, screenProp, screenType)
    Display *dpy;
    Window rw;
    PseudoScreen *ps;
    Atom screenProp, screenType;
{
    XResizeWindow (dpy, ps->root, ps->width, ps->height);
    if (!_XSetPseudoScreen (dpy, rw, screenProp, screenType, ps)) {
	fprintf (stderr, "%s:  unable to set pseudo screen\n",
		 ProgramName);
	Exit (dpy, 1);
    }
    return;
}


/*
 * Clean up after one's self
 */
delete_properties (dpy, rw, screenProp, ps, pseudoDepths, ndepths)
    Display *dpy;
    Window rw;
    Atom screenProp;
    PseudoScreen *ps;
    PseudoDepth *pseudoDepths;
    int ndepths;
{
    int i;

    for (i = 0; i < ndepths; i++) {
	if (pseudoDepths[i].visuals != None) {
	    XDeleteProperty (dpy, rw, pseudoDepths[i].visuals);
	}
    }
    if (ps->depths != None) {
	XDeleteProperty (dpy, rw, ps->depths);
    }
    XDeleteProperty (dpy, rw, screenProp);
    XSync (dpy, 0);
    return;
}
