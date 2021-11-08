/*
 * $XConsortium: Mailbox.c,v 1.17 88/09/30 08:45:06 swick Exp $
 *
 * Copyright 1988 Massachusetts Institute of Technology
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted, provided
 * that the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of M.I.T. not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  M.I.T. makes no representations about the
 * suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 *
 * Author:  Jim Fulton, MIT X Consortium
 */

#include <X11/Xos.h>
#include <X11/Xlib.h>			/* for Xlib definitions */
#include <X11/cursorfont.h>		/* for cursor constants */
#include <X11/StringDefs.h>		/* for useful atom names */
#include <X11/IntrinsicP.h>		/* for toolkit stuff */
#include <X11/MailboxP.h>		/* for implementation mailbox stuff */
#include <stdio.h>			/* for printing error messages */
#include <sys/stat.h>			/* for stat() */
#include <pwd.h>			/* for getting username */
#include <X11/bitmaps/flagup>		/* for flag up (mail present) bits */
#include <X11/bitmaps/flagdown>		/* for flag down (mail not here) */

#define PictureWidth flagdown_width	/* better be same as flagup_width */
#define PictureHeight flagdown_height	/* better be same as flagup_height */

#define min(a,b) ((a) < (b) ? (a) : (b))
#define max(a,b) ((a) > (b) ? (a) : (b))


/*
 * The default user interface is to have the mailbox turn itself off whenever
 * the user presses a button in it.  Expert users might want to make this 
 * happen on EnterWindow.  It might be nice to provide support for some sort of
 * exit callback so that you can do things like press q to quit.
 */

static char defaultTranslations[] = 
  "<ButtonPress>:  unset()";

static void Check(), Set(), Unset();

static XtActionsRec actionsList[] = { 
    { "check",	Check },
    { "unset",	Unset },
    { "set",	Set },
};


/* Initialization of defaults */

#define offset(field) XtOffset(MailboxWidget,mailbox.field)
#define goffset(field) XtOffset(Widget,core.field)

static Dimension defDim = 48;

static XtResource resources[] = {
    { XtNwidth, XtCWidth, XtRDimension, sizeof (Dimension), 
	goffset (width), XtRDimension, (caddr_t)&defDim },
    { XtNheight, XtCHeight, XtRDimension, sizeof (Dimension),
	goffset (height), XtRDimension, (caddr_t)&defDim },
    { XtNupdate, XtCInterval, XtRInt, sizeof (int),
	offset (update), XtRString, "30" },
    { XtNforeground, XtCForeground, XtRPixel, sizeof (Pixel),
	offset (foreground_pixel), XtRString, "black" },
    { XtNbackground, XtCBackground, XtRPixel, sizeof (Pixel),
	goffset (background_pixel), XtRString, "white" },
    { XtNreverseVideo, XtCReverseVideo, XtRBoolean, sizeof (Boolean),
	offset (reverseVideo), XtRString, "FALSE" },
    { XtNfile, XtCFile, XtRString, sizeof (String),
	offset (filename), XtRString, NULL },
    { XtNcheckCommand, XtCCheckCommand, XtRString, sizeof(char*),
	offset (check_command), XtRString, NULL},
    { XtNvolume, XtCVolume, XtRInt, sizeof(int),
	offset (volume), XtRString, "33"},
    { XtNonceOnly, XtCBoolean, XtRBoolean, sizeof(Boolean),
	offset (once_only), XtRImmediate, (caddr_t)False },

};

#undef offset
#undef goffset

static void GetMailFile(), CloseDown();
static void check_mailbox(), redraw_mailbox(), beep();
static void Initialize(), Realize(), Destroy(), Redisplay();
static Boolean SetValues();

MailboxClassRec mailboxClassRec = {
    { /* core fields */
    /* superclass		*/	&widgetClassRec,
    /* class_name		*/	"Mailbox",
    /* widget_size		*/	sizeof(MailboxRec),
    /* class_initialize		*/	NULL,
    /* class_part_initialize	*/	NULL,
    /* class_inited		*/	FALSE,
    /* initialize		*/	Initialize,
    /* initialize_hook		*/	NULL,
    /* realize			*/	Realize,
    /* actions			*/	actionsList,
    /* num_actions		*/	XtNumber(actionsList),
    /* resources		*/	resources,
    /* resource_count		*/	XtNumber(resources),
    /* xrm_class		*/	NULL,
    /* compress_motion		*/	TRUE,
    /* compress_exposure	*/	TRUE,
    /* compress_enterleave	*/	TRUE,
    /* visible_interest		*/	FALSE,
    /* destroy			*/	Destroy,
    /* resize			*/	NULL,
    /* expose			*/	Redisplay,
    /* set_values		*/	SetValues,
    /* set_values_hook		*/	NULL,
    /* set_values_almost	*/	XtInheritSetValuesAlmost,
    /* get_values_hook		*/	NULL,
    /* accept_focus		*/	NULL,
    /* version			*/	XtVersion,
    /* callback_private		*/	NULL,
    /* tm_table			*/	defaultTranslations,
    /* query_geometry		*/	XtInheritQueryGeometry,
    /* display_accelerator	*/	XtInheritDisplayAccelerator,
    /* extension		*/	NULL
    }
};

WidgetClass mailboxWidgetClass = (WidgetClass) &mailboxClassRec;


/*
 * widget initialization
 */

/* ARGSUSED */
static void Initialize (request, new)
    Widget request, new;
{
    MailboxWidget w = (MailboxWidget) new;

    if (!w->mailbox.filename) GetMailFile (w);

    if (w->core.width <= 0) w->core.width = PictureWidth;
    if (w->core.height <= 0) w->core.height = PictureHeight;

    if (w->mailbox.reverseVideo) {
	Pixel tmp;

	tmp = w->mailbox.foreground_pixel;
	w->mailbox.foreground_pixel = w->core.background_pixel;
	w->core.background_pixel = tmp;
    }

    return;
}


/*
 * action procedures
 */

/*
 * pretend there is new mail; put widget in flagup state
 */

/* ARGSUSED */
static void Set (gw, event, params, nparams)
    Widget gw;
    XEvent *event;
    String *params;
    Cardinal *nparams;
{
    MailboxWidget w = (MailboxWidget) gw;

    w->mailbox.last_size = -1;

    check_mailbox (w, TRUE, FALSE);	/* redraw, no reset */

    return;
}


/*
 * ack the existing mail; put widget in flagdown state
 */

/* ARGSUSED */
static void Unset (gw, event, params, nparams)
    Widget gw;
    XEvent *event;
    String *params;
    Cardinal *nparams;
{
    MailboxWidget w = (MailboxWidget) gw;

    check_mailbox (w, TRUE, TRUE);	/* redraw, reset */

    return;
}


/*
 * look to see if there is new mail; if so, Set, else Unset
 */

/* ARGSUSED */
static void Check (gw, event, params, nparams)
    Widget gw;
    XEvent *event;
    String *params;
    Cardinal *nparams;
{
    MailboxWidget w = (MailboxWidget) gw;

    check_mailbox (w, TRUE, FALSE);	/* redraw, no reset */

    return;
}


/* ARGSUSED */
static void clock_tic (client_data, id)
    caddr_t client_data;
    XtIntervalId *id;
{
    MailboxWidget w = (MailboxWidget) client_data;

    check_mailbox (w, FALSE, FALSE);	/* no redraw, no reset */

    /*
     * and reset the timer
     */

    w->mailbox.interval_id = XtAddTimeOut (w->mailbox.update * 1000,
					   clock_tic, (caddr_t) w);

    return;
}

static GC get_mailbox_gc (w)
    MailboxWidget w;
{
    XtGCMask valuemask;
    XGCValues xgcv;

    valuemask = GCForeground | GCBackground | GCFunction | GCGraphicsExposures;
    xgcv.foreground = w->mailbox.foreground_pixel;
    xgcv.background = w->core.background_pixel;
    xgcv.function = GXcopy;
    xgcv.graphics_exposures = False;	/* this is Bool, not Boolean */
    return (XtGetGC ((Widget) w, valuemask, &xgcv));
}


static void Realize (gw, valuemaskp, attr)
    Widget gw;
    XtValueMask *valuemaskp;
    XSetWindowAttributes *attr;
{
    MailboxWidget w = (MailboxWidget) gw;
    register Display *dpy = XtDisplay (w);
    int depth = DefaultDepth (dpy, DefaultScreen (dpy));

    *valuemaskp |= (CWBitGravity | CWCursor);
    attr->bit_gravity = ForgetGravity;
    attr->cursor = XCreateFontCursor (dpy, XC_top_left_arrow);

    XtCreateWindow (gw, InputOutput, (Visual *) CopyFromParent,
		    *valuemaskp, attr);

    /*
     * build up the pixmaps that we'll put into the image
     */

    w->mailbox.flagup_pixmap  = 
      XCreatePixmapFromBitmapData (dpy, w->core.window, flagup_bits,
				   flagup_width, flagup_height,
				   w->core.background_pixel, 
				   w->mailbox.foreground_pixel,
				   depth);
    w->mailbox.flagdown_pixmap = 
      XCreatePixmapFromBitmapData (dpy, w->core.window, flagdown_bits,
				   flagdown_width, flagdown_height,
				   w->mailbox.foreground_pixel,
				   w->core.background_pixel,
				   depth);

    w->mailbox.gc = get_mailbox_gc (w);

    w->mailbox.interval_id = XtAddTimeOut (w->mailbox.update * 1000,
					   clock_tic, (caddr_t) w);

    return;
}


static void Destroy (gw)
    Widget gw;
{
    MailboxWidget w = (MailboxWidget) gw;

    XtFree (w->mailbox.filename);
    XtRemoveTimeOut (w->mailbox.interval_id);
    XtDestroyGC (w->mailbox.gc);
    return;
}


static void Redisplay (gw)
    Widget gw;
{
    MailboxWidget w = (MailboxWidget) gw;

    check_mailbox (w, TRUE, FALSE);
}


static void check_mailbox (w, force_redraw, reset)
    MailboxWidget w;
    Boolean force_redraw, reset;
{
    struct stat st;
    long mailboxsize = 0;

    if (w->mailbox.check_command != NULL) {
	if (system (w->mailbox.check_command) == 0)
	    mailboxsize = w->mailbox.last_size + 1;
    }
    else {
	if (stat (w->mailbox.filename, &st) == 0) {
	    mailboxsize = st.st_size;
	}
    }

    /*
     * Now check for changes.  If reset is set then we want to pretent that
     * there is no mail.  If the mailbox is empty then we want to turn off
     * the flag.  Otherwise if the mailbox has changed size then we want to
     * put the flag up.
     *
     * The cases are:
     *    o  forced reset by user                        DOWN
     *    o  no mailbox or empty (zero-sized) mailbox    DOWN
     *    o  same size as last time                      no change
     *    o  bigger than last time                       UP
     *    o  smaller than last time but non-zero         UP
     *
     * The last two cases can be expressed as different from last
     * time and non-zero.
     */

    if (reset) {			/* forced reset */
	w->mailbox.flag_up = FALSE;
	force_redraw = TRUE;
    } else if (mailboxsize == 0) {	/* no mailbox or empty */
	w->mailbox.flag_up = FALSE;
	if (w->mailbox.last_size > 0) force_redraw = TRUE;  /* if change */
    } else if (mailboxsize != w->mailbox.last_size) {  /* different size */
	if (!w->mailbox.once_only || !w->mailbox.flag_up)
	    beep(w); 
	w->mailbox.flag_up = TRUE;
	force_redraw = TRUE;
    } 

    w->mailbox.last_size = mailboxsize;
    if (force_redraw) redraw_mailbox (w);
    return;
}

/*
 * get user name for building mailbox
 */

static void GetMailFile (w)
    MailboxWidget w;
{
    char *getlogin();
    char *username;

    username = getlogin ();
    if (!username) {
	struct passwd *pw = getpwuid (getuid ());

	if (!pw) {
	    fprintf (stderr, "%s:  unable to find a username for you.\n",
		     "Mailbox widget");
	    CloseDown (w, 1);
	}
	username = pw->pw_name;
    }
    w->mailbox.filename = (String) XtMalloc (strlen (MAILBOX_DIRECTORY) + 1 +
				   	     strlen (username) + 1);
    strcpy (w->mailbox.filename, MAILBOX_DIRECTORY);
    strcat (w->mailbox.filename, "/");
    strcat (w->mailbox.filename, username);
    return;
}

static void CloseDown (w, status)
    MailboxWidget w;
    int status;
{
    Display *dpy = XtDisplay (w);

    XtDestroyWidget (w);
    XCloseDisplay (dpy);
    exit (status);
}


/* ARGSUSED */
static Boolean SetValues (gcurrent, grequest, gnew)
    Widget gcurrent, grequest, gnew;
{
    MailboxWidget current = (MailboxWidget) gcurrent;
    MailboxWidget new = (MailboxWidget) gnew;
    Boolean redisplay = FALSE;

    if (current->mailbox.update != new->mailbox.update) {
	XtRemoveTimeOut (current->mailbox.interval_id);
	new->mailbox.interval_id = XtAddTimeOut (new->mailbox.update * 1000,
						 clock_tic,
						 (caddr_t) gnew);
    }

    if (current->mailbox.foreground_pixel != new->mailbox.foreground_pixel ||
	current->core.background_pixel != new->core.background_pixel) {
	XtDestroyGC (current->mailbox.gc);
	new->mailbox.gc = get_mailbox_gc (new);
	redisplay = TRUE;
    }

    return (redisplay);
}


/*
 * drawing code
 */

static void redraw_mailbox (w)
    MailboxWidget w;
{
    register Display *dpy = XtDisplay (w);
    register Window win = XtWindow (w);
    register int x, y;
    GC gc = w->mailbox.gc;
    Pixmap picture;
    Pixel back;

    /* center the picture in the window */

    x = (((int)w->core.width) - PictureWidth) / 2;
    y = (((int)w->core.height) - PictureHeight) / 2;

    if (w->mailbox.flag_up) {		/* paint the "up" position */
	back = w->mailbox.foreground_pixel;
	picture = w->mailbox.flagup_pixmap;
    } else {				/* paint the "down" position */
	back = w->core.background_pixel;
	picture = w->mailbox.flagdown_pixmap;
    }

    XSetWindowBackground (dpy, win, back);
    XClearWindow (dpy, win);
    XCopyArea (dpy, picture, win, gc, 0, 0, PictureWidth, PictureHeight, x, y);
    return;
}


static void beep (w)
    MailboxWidget w;
{
    XBell (XtDisplay (w), w->mailbox.volume);
    return;
}
