#ifndef lint
static char Xrcsid[] = "$XConsortium: Load.c,v 1.44 88/10/17 15:11:38 jim Exp $";
#endif

/***********************************************************
Copyright 1987, 1988 by Digital Equipment Corporation, Maynard, Massachusetts,
and the Massachusetts Institute of Technology, Cambridge, Massachusetts.

                        All Rights Reserved

Permission to use, copy, modify, and distribute this software and its 
documentation for any purpose and without fee is hereby granted, 
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in 
supporting documentation, and that the names of Digital or MIT not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.  

DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.

******************************************************************/

#ifdef apollo
#include "/sys/ins/base.ins.c"
#include "/sys/ins/time.ins.c"
typedef struct {
	short		proc2_$state_t; /* ready, waiting, etc. */
	pinteger	usr;		/* user sr */
	linteger	upc;		/* user pc */
	linteger	usp;		/* user stack pointer */
	linteger	usb;		/* user sb ptr (A6) */
	time_$clock_t	cpu_total;	/* cumulative cpu used by process */
	unsigned short	priority;	/* process priority */
    } proc1_$info_t;

std_$call void proc1_$get_cput();
std_$call void proc1_$get_info();
#endif /* apollo */

#include <X11/IntrinsicP.h>
#include <X11/Xos.h>
#include <X11/StringDefs.h>
#include <X11/Xmu.h>
#include <stdio.h>

#ifndef macII
#ifndef apollo
#ifndef LOADSTUB
#include <nlist.h>
#endif /* LOADSTUB */
#endif /* apollo */
#endif /* macII */

#ifdef sun
#include <sys/param.h>
#endif
#ifdef mips
#include <sys/fixpoint.h>
#endif
#ifdef sequent
#include <sys/vm.h>
#endif /* sequent */
#include <X11/LoadP.h>
#ifdef macII
#include <a.out.h>
#include <sys/var.h>
#define X_AVENRUN 0
#define fxtod(i) (vec[i].high+(vec[i].low/65536.0))
struct lavnum {
    unsigned short high;
    unsigned short low;
};
#endif macII
#ifdef UTEK
#define FSCALE	100.0
#endif
#ifdef sequent
#define FSCALE	1000.0
#endif

extern long lseek();
extern void exit();

/* Private Data */

static void GetLoadPoint();
static XtCallbackProc getLoadProc = GetLoadPoint;

#define offset(field) XtOffset(LoadWidget,load.field)
#define goffset(field) XtOffset(Widget,core.field)

static Dimension defDim = 120;
static int defInterval = 5;
static int defScale = 1;

static XtResource resources[] = {
    {XtNwidth, XtCWidth, XtRDimension, sizeof(Dimension),
	goffset(width), XtRDimension, (caddr_t)&defDim},
    {XtNheight, XtCHeight, XtRDimension, sizeof(Dimension),
	goffset(height), XtRDimension, (caddr_t)&defDim},
    {XtNbackground, XtCBackground, XtRPixel, sizeof(Pixel),
	goffset(background_pixel), XtRString, "White"},
    {XtNupdate, XtCInterval, XtRInt, sizeof(int),
        offset(update), XtRInt, (caddr_t)&defInterval},
    {XtNscale, XtCScale, XtRInt, sizeof(int),
        offset(scale), XtRInt, (caddr_t)&defScale},
    {XtNminScale, XtCScale, XtRInt, sizeof(int),
        offset(min_scale), XtRInt, (caddr_t)&defScale},
    {XtNlabel, XtCLabel, XtRString, sizeof(char *),
        offset(text), XtRString, NULL},
    {XtNforeground, XtCForeground, XtRPixel, sizeof(Pixel),
        offset(fgpixel), XtRString, "Black"},
    {XtNhighlight, XtCForeground, XtRPixel, sizeof(Pixel),
        offset(hipixel), XtRString, "Black"},
    {XtNfont, XtCFont, XtRFontStruct, sizeof(XFontStruct *),
        offset(font), XtRString, XtDefaultFont},
    {XtNgetLoadProc, XtCCallback, XtRCallback, sizeof(caddr_t),
        offset(get_load), XtRFunction, (caddr_t)&getLoadProc},
    {XtNreverseVideo, XtCReverseVideo, XtRBoolean, sizeof (Boolean),
	offset (reverse_video), XtRString, "FALSE"},
};

#undef offset
#undef goffset

static void ClassInitialize();
static void Initialize(), Realize(), Destroy(), Redisplay();
static Boolean SetValues();
static int repaint_window();

LoadClassRec loadClassRec = {
    { /* core fields */
    /* superclass		*/	&widgetClassRec,
    /* class_name		*/	"Load",
    /* size			*/	sizeof(LoadRec),
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
    /* display_accelerator	*/	XtInheritDisplayAccelerator,
    /* extension		*/	NULL
    }
};

WidgetClass loadWidgetClass = (WidgetClass) &loadClassRec;

/****************************************************************
 *
 * Private Procedures
 *
 ****************************************************************/

static void draw_it();

static void ClassInitialize()
{
    XtAddConverter( XtRFunction, XtRCallback, XmuCvtFunctionToCallback,
		    NULL, 0 );
}

/* ARGSUSED */
static void Initialize (greq, gnew)
    Widget greq, gnew;
{
    LoadWidget w = (LoadWidget)gnew;
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
    if (w->load.reverse_video) {
	Pixel fg = w->load.fgpixel;
	Pixel bg = w->core.background_pixel;

	if (w->core.border_pixel == fg) w->core.border_pixel = bg;
	if (w->load.hipixel == w->load.fgpixel) w->load.hipixel = bg;
	w->load.fgpixel = bg;
	w->core.background_pixel = fg;
    }

    valuemask = GCForeground | GCBackground;
    myXGCV.foreground = w->load.fgpixel;
    if (w->load.font) {
      myXGCV.font = w->load.font->fid;
      valuemask |= GCFont;
    }
    myXGCV.background = w->core.background_pixel;
    w->load.fgGC = XtGetGC(gnew, valuemask, &myXGCV);
    myXGCV.foreground = w->load.hipixel;
    w->load.hiGC = XtGetGC(gnew, valuemask, &myXGCV);

    /*
     * Note that the second argument is a GCid -- QueryFont accepts a GCid and
     * returns the curently contained font.
     */
    if (w->load.font == NULL)
      w->load.font = XQueryFont (XtDisplay (w),
      			XGContextFromGC (DefaultGCOfScreen (XtScreen (w))));

    if (w->load.update > 0)
        w->load.interval_id =
	    XtAddTimeOut(w->load.update*1000, draw_it, (caddr_t)gnew);
    else
        w->load.interval_id = 0;

    w->load.interval = 0;
    w->load.max_value = 0.0;
}
 
static void Realize (gw, valueMask, attrs)
     Widget gw;
     XtValueMask *valueMask;
     XSetWindowAttributes *attrs;
{
     XtCreateWindow( gw, (unsigned)InputOutput, (Visual *)CopyFromParent,
		     *valueMask, attrs );
}

static void Destroy (gw)
     Widget gw;
{
     LoadWidget w = (LoadWidget)gw;
     if (w->load.interval_id) XtRemoveTimeOut (w->load.interval_id);
     XtDestroyGC (w->load.fgGC);
     XtDestroyGC (w->load.hiGC);
}

/* ARGSUSED */
static void Redisplay(gw, event, region)
     Widget gw;
     XEvent *event;
     Region region;
{
     (void) repaint_window ((LoadWidget)gw, event->xexpose.x,
			    event->xexpose.width);
}

/* ARGSUSED */
static void draw_it(client_data, id)
     caddr_t client_data;
     XtIntervalId id;		/* unused */
{
        LoadWidget w = (LoadWidget)client_data;
	double value;

	if (w->load.update > 0)
	    w->load.interval_id =
		XtAddTimeOut(w->load.update*1000, draw_it, (caddr_t)w);

	if (w->load.interval >= w->core.width) {
	    XClearWindow(XtDisplay(w), XtWindow(w));
	    w->load.interval = repaint_window(w, 0, w->core.width);
	}

	/* Get the value, stash the point and draw corresponding line. */

	XtCallCallbacks( (Widget)w, XtNgetLoadProc, (caddr_t)&value );

	/* Keep w->load.max_value up to date, and if this data point is off the
	   graph, change the scale to make it fit. */
	if (value > w->load.max_value) {
	    w->load.max_value = value;
	    if (w->load.max_value > w->load.scale) {
		w->load.scale = ((int)w->load.max_value) + 1;
		XClearWindow(XtDisplay(w), XtWindow(w));
		w->load.interval = repaint_window(w, 0, w->core.width);
	    }
	}

	w->load.valuedata[w->load.interval] = value;
	if (XtIsRealized((Widget)w)) {
	    XDrawLine(XtDisplay(w), XtWindow(w), w->load.fgGC,
		  w->load.interval, w->core.height, w->load.interval, 
		    (int)(w->core.height - (w->core.height * value) /w->load.scale));
	    XFlush(XtDisplay(w));		    /* Flush output buffers */
	}
	w->load.interval++;		    /* Next point */
} /* draw_it */

/* Blts data according to current size, then redraws the load average window.
 * Next represents the number of valid points in data.  Returns the (possibly)
 * adjusted value of next.  If next is 0, this routine draws an empty window
 * (scale - 1 lines for graph).  If next is less than the current window width,
 * the returned value is identical to the initial value of next and data is
 * unchanged.  Otherwise keeps half a window's worth of data.  If data is
 * changed, then w->load.max_value is updated to reflect the largest data point.
 */

static int repaint_window(w, left, width)
    LoadWidget w;
    int left, width;
{
    register int i, j;
    register int next = w->load.interval;
    int scale = w->load.scale;
    extern void bcopy();

    if (next >= w->core.width) {
	j = w->core.width >> 1;
	bcopy((char *)(w->load.valuedata + next - j),
	      (char *)(w->load.valuedata), j * sizeof(double));
	next = j;
	/* Since we just lost some data, recompute the w->load.max_value. */
	w->load.max_value = 0.0;
	for (i = 0; i < next; i++) {
	    if (w->load.valuedata[i] > w->load.max_value) 
	      w->load.max_value = w->load.valuedata[i];
	}
	left = 0;
	width = next; 
    }

    /* Compute the minimum scale required to graph the data, but don't go
       lower than min_scale. */
    if (w->load.interval != 0 || scale <= (int)w->load.max_value)
      scale = ((int) (w->load.max_value)) + 1;
    if (scale < w->load.min_scale)
      scale = w->load.min_scale;
    if (scale != w->load.scale) {
      w->load.scale = scale;
      left = 0;
      width = next;
      if (XtIsRealized ((Widget) w)) {
	XClearWindow (XtDisplay (w), XtWindow (w));
      }
    }

    if (XtIsRealized((Widget)w)) {
	Display *dpy = XtDisplay(w);
	Window win = XtWindow(w);

	if (w->load.text) {
	    /* Print hostname */
	    XDrawString(dpy, win, w->load.hiGC, 2, 
	         2 + w->load.font->ascent, w->load.text, strlen(w->load.text));
	}

	/* Draw graph reference lines */
	for (i = 1; i < w->load.scale; i++) {
	    j = (i * w->core.height) / w->load.scale;
	    XDrawLine(dpy, win, w->load.hiGC, 0, j,
		      (int)w->core.width, j);
	}

	if (next < (width += left)) width = next;
	/* Draw data point lines. */
	for (i = left; i < width; i++)
	    XDrawLine(dpy, win, w->load.fgGC, i, w->core.height,
		  i, (int)(w->core.height-(w->load.valuedata[i] * w->core.height)
			  /w->load.scale));
        }

    return(next);
}

#if apollo
/* ARGSUSED */
static void GetLoadPoint( w, closure, call_data )
     Widget	w;		/* unused */
     caddr_t	closure;	/* unused */
     caddr_t	call_data;	/* pointer to (double) return value */
{
     static Bool    firstTime = TRUE;
     static int     lastNullCpu;
     static int     lastClock;
     time_$clock_t  timeNow;
     double         temp;
     proc1_$info_t  info;
     status_$t      st;

     proc1_$get_info( (short) 2, info, st );
     time_$clock( timeNow );

     if (firstTime)
     {
     	*(double *)call_data = 1.0;
         firstTime = FALSE;
     }
     else {
         temp = info.cpu_total.low32 - lastNullCpu;
     	*(double *)call_data = 1.0 - temp / (timeNow.low32 - lastClock);
     }

     lastClock = timeNow.low32;
     lastNullCpu = info.cpu_total.low32;
}
#else
#ifdef LOADSTUB

/* ARGSUSED */
static void GetLoadPoint( w, closure, call_data )
     Widget	w;		/* unused */
     caddr_t	closure;	/* unused */
     caddr_t	call_data;	/* pointer to (double) return value */
{
	*(double *)call_data = 1.0;
}

#else /* LOADSTUB */

#ifndef KMEM_FILE
#define KMEM_FILE "/dev/kmem"
#endif

#ifndef KERNEL_FILE
#ifdef CRAY
#define KERNEL_FILE "/unicos"
#endif /* CRAY */
#ifdef hpux
#define KERNEL_FILE "/hp-ux"
#endif /* hpux */
#ifdef macII
#define KERNEL_FILE "/unix"
#endif /* macII */
#ifdef mips
# ifdef SYSTYPE_SYSV
# define KERNEL_FILE "/unix"
# else
# define KERNEL_FILE "/vmunix"
# endif /* SYSTYPE_SYSV */
#endif /* mips */
#ifdef sequent
#define KERNEL_FILE "/dynix"
#endif /* sequent */

/*
 * provide default for everyone else
 */
#ifndef KERNEL_FILE
#define KERNEL_FILE "/vmunix"
#endif /* KERNEL_FILE */
#endif /* KERNEL_FILE */


#ifndef KERNEL_LOAD_VARIABLE
#ifdef CRAY
#define KERNEL_LOAD_VARIABLE "avenrun"
#undef n_type
#define n_type n_value
#endif /* CRAY */
#ifdef hpux
#ifdef hp9000s800
#define KERNEL_LOAD_VARIABLE "avenrun"
#endif /* hp9000s800 */
#endif /* hpux */
#ifdef mips
# ifdef SYSTYPE_SYSV
# define KERNEL_LOAD_VARIABLE "avenrun"
# else
# define KERNEL_LOAD_VARIABLE "_avenrun"
# endif /* SYSTYPE_SYSV */
#endif /* mips */
#ifdef sequent
#define KERNEL_FILE "/dynix"
#endif /* sequent */
/*
 * provide default for everyone else
 */
#ifndef KERNEL_LOAD_VARIABLE
#define KERNEL_LOAD_VARIABLE "_avenrun"
#endif /* KERNEL_LOAD_VARIABLE */
#endif /* KERNEL_LOAD_VARIABLE */

#ifdef macII
static struct var v;
static struct nlist nl[2];
static struct lavnum vec[3];
#else /* not macII */
static struct nlist namelist[] = {	    /* namelist for vmunix grubbing */
#define LOADAV 0
    {KERNEL_LOAD_VARIABLE},
    {0}
};
#endif /* macII */


/* ARGSUSED */
static void GetLoadPoint( w, closure, call_data )
     Widget	w;		/* unused */
     caddr_t	closure;	/* unused */
     caddr_t	call_data;	/* pointer to (double) return value */
{
  	double *loadavg = (double *)call_data;
	static int init = 0;
	static kmem;
	static long loadavg_seek;
#ifdef macII
        extern nlist();

        if(!init)   {
            int i;

            strcpy(nl[0].n_name, "avenrun");
            nl[1].n_name[0] = '\0';

            kmem = open(KMEM_FILE, O_RDONLY);
            if (kmem < 0) {
	        xload_error("cannot open", KMEM_FILE);
            }

            uvar(&v);

            if (nlist( KERNEL_FILE, nl) != 0) {
		xload_error("cannot get name list from", KERNEL_FILE);
            }
            for (i = 0; i < 2; i++) {
                nl[i].n_value = (int)nl[i].n_value - v.v_kvoffset;
            }
            init = 1;
        }
#else /* not macII */
	extern void nlist();
	
	if(!init)   {
	    nlist( KERNEL_FILE, namelist);
	    if (namelist[LOADAV].n_type == 0){
		xload_error("cannot get name list from", KERNEL_FILE);
		exit(-1);
	    }
	    loadavg_seek = namelist[LOADAV].n_value;
# if defined(mips) && defined(SYSTYPE_SYSV)
	    loadavg_seek &= 0x7fffffff;
# endif /* mips && SYSTYPE_SYSV */
	    kmem = open(KMEM_FILE, O_RDONLY);
	    if (kmem < 0) xload_error("cannot open", KMEM_FILE);
	    init = 1;
	}
	

	(void) lseek(kmem, loadavg_seek, 0);
#endif /* macII */
#if defined(sun) || defined (UTEK) || defined(sequent)
	{
		long temp;
		(void) read(kmem, (char *)&temp, sizeof(long));
		*loadavg = (double)temp/FSCALE;
	}
#else /* else not sun */
# ifdef macII
        {
                lseek(kmem, (long)nl[X_AVENRUN].n_value, 0);
                read(kmem, vec, 3*sizeof(struct lavnum));
                *loadavg = fxtod(0);
        }
# else /* else not macII */
#  ifdef mips
	{
		fix temp;
		(void) read(kmem, (char *)&temp, sizeof(fix));
		*loadavg = FIX_TO_DBL(temp);
	}
#  else /* not mips */
	(void) read(kmem, (char *)loadavg, sizeof(double));
#  endif /* mips */
# endif /* macII */
#endif /* sun */
	return;
}
#endif /* LOADSTUB */
#endif /* apollo */

static xload_error(str1, str2)
char *str1, *str2;
{
    (void) fprintf(stderr,"xload: %s %s\n", str1, str2);
    exit(-1);
}

/* ARGSUSED */
static Boolean SetValues (current, request, new)
    Widget current, request, new;
{
    LoadWidget old = (LoadWidget)current;
    LoadWidget w = (LoadWidget)new;
    if (w->load.update != old->load.update) {
	XtRemoveTimeOut (old->load.interval_id);
	w->load.interval_id =
	    XtAddTimeOut(w->load.update*1000, draw_it, (caddr_t)w);
    }

    return( FALSE );
}
