/*
 * Eyes.c
 *
 * a widget which follows the mouse around
 */

# include <X11/Xos.h>
# include <stdio.h>
# include <X11/IntrinsicP.h>
# include <X11/StringDefs.h>
# include <X11/Xmu.h>
# include "EyesP.h"
# include <math.h>

#define offset(field) XtOffset(EyesWidget,eyes.field)
#define goffset(field) XtOffset(Widget,core.field)

static XtResource resources[] = {
    {XtNwidth, XtCWidth, XtRDimension, sizeof(Dimension),
	goffset(width), XtRString, "150"},
    {XtNheight, XtCHeight, XtRDimension, sizeof(Dimension),
	goffset(height), XtRString, "100"},
    {XtNforeground, XtCForeground, XtRPixel, sizeof(Pixel),
        offset(puppixel), XtRString, "Black"},
    {XtNoutline, XtCForeground, XtRPixel, sizeof(Pixel),
        offset(outline), XtRString, "Black"},
    {XtNcenterColor, XtCBackground, XtRPixel, sizeof (Pixel),
    	offset(center), XtRString, "White"},
    {XtNreverseVideo, XtCReverseVideo, XtRBoolean, sizeof (Boolean),
	offset (reverse_video), XtRString, "FALSE"},
    {XtNbackingStore, XtCBackingStore, XtRBackingStore, sizeof (int),
    	offset (backing_store), XtRString, "default"},
    {XtNuseWideLines, XtCUseWideLines, XtRBoolean, sizeof (Boolean),
        offset (use_wide_lines), XtRString, "FALSE"},
    {XtNuseBevel, XtCUseBevel, XtRBoolean, sizeof (Boolean),
	offset (use_bevel), XtRString, "FALSE"},
};

#undef offset
#undef goffset

static void Initialize(), Realize(), Destroy(), Redisplay();
static Boolean SetValues();
static int repaint_window();
static int draw_it ();

# define NUM_EYES	2
# define WIDGET_WIDTH(w)	((w)->core.width)
# define WIDGET_HEIGHT(w)	((w)->core.height)
# define EYE_THICK(w)	(((WIDGET_WIDTH(w) + WIDGET_HEIGHT(w)) / 2) / 15)
# define EYE_PAD_X(w)	(WIDGET_WIDTH(w) / 20 + EYE_THICK(w) / 2)
# define EYE_PAD_Y(w)	(WIDGET_HEIGHT(w) / 20 + EYE_THICK(w) / 2)
# define EYE_WIDTH(w)	((WIDGET_WIDTH(w) - EYE_PAD_X(w)) / NUM_EYES - EYE_PAD_X(w))
# define EYE_HEIGHT(w)	(WIDGET_HEIGHT(w) - 2 * EYE_PAD_Y(w))
# define BALL_WIDTH(w)	(((EYE_WIDTH(w) + EYE_HEIGHT(w)) / 2) / 5)
# define BALL_HEIGHT(w)	BALL_WIDTH(w)
# define BALL_DIST	0.5

# define EYE_CENTER_X(w, num)	((EYE_WIDTH(w) + EYE_PAD_X(w)) * (num) + EYE_PAD_X(w) + EYE_WIDTH(w)/2)
# define EYE_CENTER_Y(w, num)	(EYE_HEIGHT(w) / 2 + EYE_PAD_Y(w))

static int delays[] = { 50, 100, 200, 400, 0 };

static void ClassInitialize();

EyesClassRec eyesClassRec = {
    { /* core fields */
    /* superclass		*/	&widgetClassRec,
    /* class_name		*/	"Eyes",
    /* size			*/	sizeof(EyesRec),
    /* class_initialize		*/	ClassInitialize,
    /* class_part_initialize	*/	NULL,
    /* class_inited		*/	FALSE,
    /* initialize		*/	Initialize,
    /* initialize_hook		*/	NULL,
    /* realize			*/	Realize,
    /* actions			*/	NULL,
    /* num_actions		*/	0,
    /* resources		*/	resources,
    /* num_resources		*/	XtNumber(resources),
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
    /* set_values_almost	*/	NULL,
    /* get_values_hook		*/	NULL,
    /* accept_focus		*/	NULL,
    /* version			*/	XtVersion,
    /* callback_private		*/	NULL,
    /* tm_table			*/	NULL,
    /* query_geometry		*/	XtInheritQueryGeometry,
    }
};

static void ClassInitialize()
{
    XtAddConverter( XtRString, XtRBackingStore, XmuCvtStringToBackingStore,
		    NULL, 0 );
}

WidgetClass eyesWidgetClass = (WidgetClass) &eyesClassRec;

/* ARGSUSED */
static void Initialize (greq, gnew)
    Widget greq, gnew;
{
    EyesWidget w = (EyesWidget)gnew;
    XtGCMask	valuemask;
    XGCValues	myXGCV;

    /*
     * set the colors if reverse video; these are the colors used:
     *
     *     background - paper		white
     *     foreground - text, ticks	black
     *     border - border		black (foreground)
     *
     * This doesn't completely work since the parent has already made up a 
     * border.  Sigh.
     */
    if (w->eyes.reverse_video) {
	Pixel fg = w->eyes.puppixel;
	Pixel bg = w->core.background_pixel;

	if (w->core.border_pixel == fg)
 	    w->core.border_pixel = bg;
	if (w->eyes.outline == fg)
	    w->eyes.outline = bg;
	if (w->eyes.center == bg)
	    w->eyes.center = fg;
	w->eyes.puppixel = bg;
	w->core.background_pixel = fg;
    }
    w->eyes.thickness = EYE_THICK (w);

    myXGCV.foreground = w->eyes.puppixel;
    myXGCV.background = w->core.background_pixel;
    valuemask = GCForeground | GCBackground;
    w->eyes.pupGC = XtGetGC(gnew, valuemask, &myXGCV);

    myXGCV.foreground = w->eyes.outline;
    valuemask = GCForeground | GCBackground;
    if (w->eyes.use_wide_lines) {
	myXGCV.line_width = w->eyes.thickness;
	valuemask |= GCLineWidth;
	if (w->eyes.use_bevel) {
	    myXGCV.join_style = JoinBevel;
	    valuemask |= GCJoinStyle;
	}
    }
    w->eyes.outGC = XtGetGC(gnew, valuemask, &myXGCV);

    if (w->eyes.use_wide_lines)
	myXGCV.foreground = w->core.background_pixel;
    else
	myXGCV.foreground = w->eyes.center;
    myXGCV.background = w->eyes.puppixel;
    valuemask = GCForeground | GCBackground;
    w->eyes.centerGC = XtGetGC(gnew, valuemask, &myXGCV);

    w->eyes.update = 0;
    /* wait for Realize to add the timeout */
    w->eyes.interval_id = 0;

    w->eyes.pupil[0].x = w->eyes.pupil[1].x = -1;
    w->eyes.pupil[0].y = w->eyes.pupil[1].y = -1;
}

 
static void Realize (gw, valueMask, attrs)
     Widget gw;
     XtValueMask *valueMask;
     XSetWindowAttributes *attrs;
{
     EyesWidget	w = (EyesWidget)gw;

    if (w->eyes.backing_store != Always + WhenMapped + NotUseful) {
     	attrs->backing_store = w->eyes.backing_store;
	*valueMask |= CWBackingStore;
    }
    XtCreateWindow( gw, (unsigned)InputOutput, (Visual *)CopyFromParent,
		     *valueMask, attrs );
    w->eyes.interval_id =
	    XtAddTimeOut(delays[w->eyes.update], draw_it, (caddr_t)gw);
}

static void Destroy (gw)
     Widget gw;
{
     EyesWidget w = (EyesWidget)gw;
     if (w->eyes.interval_id) XtRemoveTimeOut (w->eyes.interval_id);
     XtDestroyGC (w->eyes.pupGC);
     XtDestroyGC (w->eyes.outGC);
     XtDestroyGC (w->eyes.centerGC);
}

/* ARGSUSED */
static void Redisplay(gw, event, region)
     Widget gw;
     XEvent *event;
     Region region;
{
    int		thick;
    EyesWidget	w;
    XGCValues	myXGCV;
    Display	*dpy;

    w = (EyesWidget) gw;
    thick = EYE_THICK (w);
    if (thick != w->eyes.thickness) {
	dpy = XtDisplay (w);
	w->eyes.thickness = thick;
	myXGCV.line_width = w->eyes.thickness;
	XChangeGC (dpy, w->eyes.outGC, GCLineWidth, &myXGCV);
    }
    w->eyes.pupil[0].x = -1;
    w->eyes.pupil[0].y = -1;
    w->eyes.pupil[1].x = -1;
    w->eyes.pupil[1].y = -1;
    (void) repaint_window ((EyesWidget)gw);
}

static XPoint computePupil (w, num, dx, dy)
    EyesWidget	w;
    int		num;
    int		dx, dy;
{
	int	cx, cy;
	double	dist;
	double	angle;
	double	x, y;
	double	h;
	double	a, b;
	XPoint	ret;

	dx = dx - EYE_CENTER_X(w, num);
	dy = dy - EYE_CENTER_Y(w, num);
	if (dx == 0 && dy == 0) {
		cx = EYE_CENTER_X(w, num);
		cy = EYE_CENTER_Y(w, num);
	} else {
		angle = atan2 ((double) dy, (double) dx);
		a = EYE_WIDTH(w) / 2.0;
		b = EYE_HEIGHT(w) / 2.0;
		h = hypot (b * cos (angle), a * sin (angle));
		x = a * b * cos (angle) / h;
		y = a * b * sin (angle) / h;
		dist = BALL_DIST * hypot (x, y);
		if (dist > hypot ((double) dx, (double) dy)) {
			cx = dx + EYE_CENTER_X(w, num);
			cy = dy + EYE_CENTER_Y(w, num);
		} else {
			cx = dist * cos (angle) + EYE_CENTER_X(w, num);
			cy = dist * sin (angle) + EYE_CENTER_Y(w, num);
		}
	}
	ret.x = cx;
	ret.y = cy;
	return ret;
}

static void computePupils (w, dx, dy, pupils)
    EyesWidget	w;
    int		dx, dy;
    XPoint	pupils[2];
{
    pupils[0] = computePupil (w, 0, dx, dy);
    pupils[1] = computePupil (w, 1, dx, dy);
}

/* ARGSUSED */
static int draw_it(client_data, id)
     caddr_t client_data;
     XtIntervalId id;		/* unused */
{
        EyesWidget	w = (EyesWidget)client_data;
	Window		rep_root, rep_child;
	int		rep_rootx, rep_rooty, rep_mask;
	int		dx, dy;
	Display		*dpy = XtDisplay (w);
	Window		win = XtWindow (w);
	XPoint		newpupil[2];

	if (XtIsRealized((Widget)w)) {
    		XQueryPointer (dpy, win, &rep_root, &rep_child,
 			&rep_rootx, &rep_rooty, &dx, &dy, &rep_mask);
		if (dx != w->eyes.odx || dy != w->eyes.ody) {
			computePupils (w, dx, dy, newpupil);
			if (newpupil[0].x != w->eyes.pupil[0].x ||
			    newpupil[0].y != w->eyes.pupil[0].y)
			{
			    if (w->eyes.pupil[0].x != -1 || w->eyes.pupil[0].y != -1)
				eyeBall (w, w->eyes.centerGC, 0);
			    w->eyes.pupil[0] = newpupil[0];
			    eyeBall (w, w->eyes.pupGC, 0, dx, dy);
			}
			if (newpupil[1].x != w->eyes.pupil[1].x ||
			    newpupil[1].y != w->eyes.pupil[1].y)
			{
			    if (w->eyes.pupil[1].x != -1 || w->eyes.pupil[1].y != -1)
				eyeBall (w, w->eyes.centerGC, 1);
			    w->eyes.pupil[1] = newpupil[1];
			    eyeBall (w, w->eyes.pupGC, 1, dx, dy);
			}
			XFlush(XtDisplay(w));	   /* Flush output buffers */
			w->eyes.odx = dx;
			w->eyes.ody = dy;
			w->eyes.update = 0;
		} else {
			if (delays[w->eyes.update + 1] != 0)
				++w->eyes.update;
		}
	}
	w->eyes.interval_id =
		XtAddTimeOut(delays[w->eyes.update], draw_it, (caddr_t)w);
} /* draw_it */

static
repaint_window (w)
    EyesWidget	w;
{
	if (XtIsRealized ((Widget) w)) {
		eyeLiner (w, w->eyes.outGC, w->eyes.centerGC, 0);
		eyeLiner (w, w->eyes.outGC, w->eyes.centerGC, 1);
		computePupils (w, w->eyes.odx, w->eyes.ody, w->eyes.pupil);
		eyeBall (w, w->eyes.pupGC, 0);
		eyeBall (w, w->eyes.pupGC, 1);
	}
}
    
/* ARGSUSED */
static Boolean SetValues (current, request, new)
    Widget current, request, new;
{
    EyesWidget old = (EyesWidget)current;
    EyesWidget w = (EyesWidget)new;
    return( FALSE );
}

eyeLiner (w, outgc, centergc, num)
EyesWidget	w;
GC		outgc, centergc;
int		num;
{
	Display *dpy = XtDisplay(w);
	Window win = XtWindow(w);
	int ecx = (int) EYE_CENTER_X(w, num), ecy = (int) EYE_CENTER_Y(w, num);
	int et = EYE_THICK(w);
	int ew = EYE_WIDTH(w), eh = EYE_HEIGHT(w);
	int etdiv2 = et/2;
	int ewdiv2 = ew/2, ehdiv2 = eh/2;

	if (ew < 0 || eh < 0)
		return;
	if (w->eyes.use_wide_lines) {
		XDrawArc (dpy, win, outgc,
			  (ecx - ewdiv2), (ecy - ehdiv2),
			  (unsigned) ew, (unsigned) eh,
			  90 * 64, 360 * 64);
	} else {
		XFillArc (dpy, win, outgc, 
			  (ecx - ewdiv2 - etdiv2), (ecy - ehdiv2 - etdiv2),
			  (unsigned) (ew + et), (unsigned) (eh + et),
 			  90 * 64, 360 * 64);
		if (et > ew || et > eh)
			return;
		XFillArc (dpy, win, centergc,
			  (ecx - ewdiv2 + etdiv2), (ecy - ehdiv2 + etdiv2),
			  (unsigned) (ew - et), (unsigned) (eh - et),
			  90 * 64, 360 * 64);
	}
}

eyeBall (w, gc, num)
EyesWidget	w;
GC	gc;
int	num;
{
	int	cx, cy;
	Display *dpy = XtDisplay(w);
	Window win = XtWindow(w);
	int bw = BALL_WIDTH(w), bh = BALL_HEIGHT(w);

	cx = w->eyes.pupil[num].x;
	cy = w->eyes.pupil[num].y;

	XFillArc (dpy, win, gc, (cx - bw/2), (cy - bh/2), bw, bh, 
		  90 * 64, 360 * 64);
}
