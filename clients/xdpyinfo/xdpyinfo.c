/*
 * $XConsortium: xdpyinfo.c,v 1.10 88/09/26 16:25:28 jim Exp $
 * 
 * xdpyinfo - print information about X display connecton
 *
 * Copyright 1988 by the Massachusetts Institute of Technology
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted, provided 
 * that the above copyright notice appear in all copies and that both that 
 * copyright notice and this permission notice appear in supporting 
 * documentation, and that the name of M.I.T. not be used in advertising
 * or publicity pertaining to distribution of the software without specific, 
 * written prior permission. M.I.T. makes no representations about the 
 * suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 *
 * Author:  Jim Fulton, MIT X Consortium
 */

#include <stdio.h>
#include <X11/Xos.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

char *ProgramName;

/*
 * There needs to be some sort of interface to the PixmapFormats in the
 * display structure.  But, since it is opaque, we aren't allowed to refer
 * to them.  Sigh.
 */

static void usage ()
{
    fprintf (stderr, "usage:  %s [-display displayname]\n",
	     ProgramName);
    exit (1);
}

main (argc, argv)
    int argc;
    char *argv[];
{
    Display *dpy;			/* X connection */
    char *displayname = NULL;		/* server to contact */
    int i;				/* temp variable:  iterator */

    ProgramName = argv[0];

    for (i = 1; i < argc; i++) {
	char *arg = argv[i];

	if (arg[0] == '-') {
	    switch (arg[1]) {
	      case 'd':
		if (++i >= argc) usage ();
		displayname = argv[i];
		continue;
	      default:
		usage ();
	    }
	} else
	  usage ();
    }

    dpy = XOpenDisplay (displayname);
    if (!dpy) {
	fprintf (stderr, "%s:  unable to open display \"%s\".\n",
		 ProgramName, XDisplayName (displayname));
	exit (1);
    }

    print_display_info (dpy);
    for (i = 0; i < ScreenCount (dpy); i++) {
	print_screen_info (dpy, i);
    }

    XCloseDisplay (dpy);
    exit (0);
}

print_display_info (dpy)
    Display *dpy;
{
    char dummybuf[40];
    char *cp;
    int minkeycode, maxkeycode;
    int i, n;

    printf ("name of display:    %s\n", DisplayString (dpy));
    printf ("version number:    %d.%d\n",
	    ProtocolVersion (dpy), ProtocolRevision (dpy));
    printf ("vendor string:    %s\n", ServerVendor (dpy));
    printf ("vendor release number:    %d\n", VendorRelease (dpy));
    printf ("maximum request size:  %ld longwords (%ld bytes)\n",
	    XMaxRequestSize (dpy), XMaxRequestSize (dpy) * sizeof (long));
    printf ("motion buffer size:  %d\n", XDisplayMotionBufferSize (dpy));

    switch (BitmapBitOrder (dpy)) {
      case LSBFirst:    cp = "LSBFirst"; break;
      case MSBFirst:    cp = "MSBFirst"; break;
      default:    
	sprintf (dummybuf, "unknown order %d", BitmapBitOrder (dpy));
	cp = dummybuf;
	break;
    }
    printf ("bitmap unit, bit order, padding:    %d, %s, %d\n",
	    BitmapUnit (dpy), cp, BitmapPad (dpy));

    switch (ImageByteOrder (dpy)) {
      case LSBFirst:    cp = "LSBFirst"; break;
      case MSBFirst:    cp = "MSBFirst"; break;
      default:    
	sprintf (dummybuf, "unknown order %d", ImageByteOrder (dpy));
	cp = dummybuf;
	break;
    }
    printf ("image byte order:    %s\n", cp);

    /*
     * when we get interfaces to the PixmapFormat stuff, insert code here
     */

    XDisplayKeycodes (dpy, &minkeycode, &maxkeycode);
    printf ("keycode range:    minimum %d, maximum %d\n",
	    minkeycode, maxkeycode);

    printf ("default screen number:    %d\n", DefaultScreen (dpy));
    printf ("number of screens:    %d\n", ScreenCount (dpy));

    return;
}

print_screen_info (dpy, scr)
    Display *dpy;
    int scr;
{
    Screen *s = ScreenOfDisplay (dpy, scr);  /* opaque structure */
    XVisualInfo viproto;		/* fill in for getting info */
    XVisualInfo *vip;			/* retured info */
    int nvi;				/* number of elements returned */
    int nscr;				/* number of screens */
    int i;				/* temp variable: iterator */
    char eventbuf[80];			/* want 79 chars per line + nul */
    static char *yes = "YES", *no = "NO";
    double xres, yres;

    /*
     * there are 2.54 centimeters to an inch; so there are 25.4 millimeters.
     *
     *     dpi = N pixels / (M millimeters / (25.4 millimeters / 1 inch))
     *         = N pixels / (M inch / 25.4)
     *         = N * 25.4 pixels / M inch
     */

    xres = ((((double) DisplayWidth(dpy,scr)) * 25.4) / 
	    ((double) DisplayWidthMM(dpy,scr)));
    yres = ((((double) DisplayHeight(dpy,scr)) * 25.4) / 
	    ((double) DisplayHeightMM(dpy,scr)));

    printf ("\n");
    printf ("screen #%d:\n", scr);
    printf ("  dimensions:    %dx%d pixels (%dx%d millimeters)\n",
	    DisplayWidth (dpy, scr), DisplayHeight (dpy, scr),
	    DisplayWidthMM(dpy, scr), DisplayHeightMM (dpy, scr));
    printf ("  resolution:    %dx%d dots per inch\n", 
	    (int) (xres + 0.5), (int) (yres + 0.5));
    printf ("  root window id:    0x%lx\n", RootWindow (dpy, scr));
    printf ("  depth of root window:    %d plane%s\n",
	    DisplayPlanes (dpy, scr),
	    DisplayPlanes (dpy, scr) == 1 ? "" : "s");
    printf ("  number of colormaps:    minimum %d, maximum %d\n",
	    MinCmapsOfScreen(s), MaxCmapsOfScreen(s));
    printf ("  default colormap:    0x%lx\n", DefaultColormap (dpy, scr));
    printf ("  default number of colormap cells:    %d\n",
	    DisplayCells (dpy, scr));
    printf ("  preallocated pixels:    black %d, white %d\n",
	    BlackPixel (dpy, scr), WhitePixel (dpy, scr));
    printf ("  options:    backing-store %s, save-unders %s\n",
	    DoesBackingStore (s) ? yes : no,
	    DoesSaveUnders (s) ? yes : no);
    printf ("  current input event mask:    0x%lx\n", EventMaskOfScreen (s));
    (void) print_event_mask (eventbuf, 79, 4, EventMaskOfScreen (s));
		      

    nvi = 0;
    viproto.screen = scr;
    vip = XGetVisualInfo (dpy, VisualScreenMask, &viproto, &nvi);
    printf ("  number of visuals:    %d\n", nvi);
    printf ("  default visual id:  0x%lx\n", 
	    XVisualIDFromVisual (DefaultVisual (dpy, scr)));
    for (i = 0; i < nvi; i++) {
	print_visual_info (dpy, vip+i);
    }
    if (vip) XFree ((char *) vip);

    return;
}


print_visual_info (dpy, vip)
    Display *dpy;
    XVisualInfo *vip;
{
    char errorbuf[40];			/* for sprintfing into */
    char *class = NULL;			/* for printing */

    switch (vip->class) {
      case StaticGray:    class = "StaticGray"; break;
      case GrayScale:    class = "GrayScale"; break;
      case StaticColor:    class = "StaticColor"; break;
      case PseudoColor:    class = "PseudoColor"; break;
      case TrueColor:    class = "TrueColor"; break;
      case DirectColor:    class = "DirectColor"; break;
      default:    
	sprintf (errorbuf, "unknown class %d", vip->class);
	class = errorbuf;
	break;
    }

    printf ("  visual:\n");
    printf ("    visual id:    0x%lx\n", vip->visualid);
    printf ("    class:    %s\n", class);
    printf ("    depth:    %d plane%s\n", vip->depth, 
	    vip->depth == 1 ? "" : "s");
    printf ("    size of colormap:    %d entries\n", vip->colormap_size);
    printf ("    red, green, blue masks:    0x%lx, 0x%lx, 0x%lx\n",
	    vip->red_mask, vip->green_mask, vip->blue_mask);
    printf ("    significant bits in color specification:    %d bits\n",
	    vip->bits_per_rgb);

    return;
}

/*
 * The following routine prints out an event mask, wrapping events at nice
 * boundaries.
 */

#define MASK_NAME_WIDTH 25

static struct _event_table {
    char *name;
    long value;
} event_table[] = {
    { "KeyPressMask             ", KeyPressMask },
    { "KeyReleaseMask           ", KeyReleaseMask },
    { "ButtonPressMask          ", ButtonPressMask },
    { "ButtonReleaseMask        ", ButtonReleaseMask },
    { "EnterWindowMask          ", EnterWindowMask },
    { "LeaveWindowMask          ", LeaveWindowMask },
    { "PointerMotionMask        ", PointerMotionMask },
    { "PointerMotionHintMask    ", PointerMotionHintMask },
    { "Button1MotionMask        ", Button1MotionMask },
    { "Button2MotionMask        ", Button2MotionMask },
    { "Button3MotionMask        ", Button3MotionMask },
    { "Button4MotionMask        ", Button4MotionMask },
    { "Button5MotionMask        ", Button5MotionMask },
    { "ButtonMotionMask         ", ButtonMotionMask },
    { "KeymapStateMask          ", KeymapStateMask },
    { "ExposureMask             ", ExposureMask },
    { "VisibilityChangeMask     ", VisibilityChangeMask },
    { "StructureNotifyMask      ", StructureNotifyMask },
    { "ResizeRedirectMask       ", ResizeRedirectMask },
    { "SubstructureNotifyMask   ", SubstructureNotifyMask },
    { "SubstructureRedirectMask ", SubstructureRedirectMask },
    { "FocusChangeMask          ", FocusChangeMask },
    { "PropertyChangeMask       ", PropertyChangeMask },
    { "ColormapChangeMask       ", ColormapChangeMask },
    { "OwnerGrabButtonMask      ", OwnerGrabButtonMask },
    { NULL, 0 }};

int print_event_mask (buf, lastcol, indent, mask)
    char *buf;				/* string to write into */
    int lastcol;			/* strlen(buf)+1 */
    int indent;				/* amount by which to indent */
    long mask;				/* event mask */
{
    struct _event_table *etp;
    int len;
    int bitsfound = 0;

    buf[0] = buf[lastcol] = '\0';	/* just in case */

#define INDENT() { register int i; len = indent; \
		   for (i = 0; i < indent; i++) buf[i] = ' '; }

    INDENT ();

    for (etp = event_table; etp->name; etp++) {
	if (mask & etp->value) {
	    if (len + MASK_NAME_WIDTH > lastcol) {
		puts (buf);
		INDENT ();
	    }
	    strcpy (buf+len, etp->name);
	    len += MASK_NAME_WIDTH;
	    bitsfound++;
	}
    }

    if (bitsfound) puts (buf);

#undef INDENT

    return (bitsfound);
}