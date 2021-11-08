#ifndef lint
static char Xrcsid[] = "$XConsortium: AsciiSink.c,v 1.26 88/10/19 20:08:51 swick Exp $";
#endif lint


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

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/IntrinsicP.h>
#include <X11/StringDefs.h>
#include <X11/TextP.h>


#define GETLASTPOS (*source->Scan)(source, 0, XtstAll, XtsdRight, 1, TRUE)
/* Private Ascii TextSink Definitions */

static unsigned bufferSize = 200;

typedef struct _AsciiSinkData {
    Pixel foreground;
    GC normgc, invgc, xorgc;
    XFontStruct *font;
    int em;
    Pixmap insertCursorOn;
    XtTextInsertState laststate;
    int tab_count;
    Position *tabs;
} AsciiSinkData, *AsciiSinkPtr;

static char *buf = NULL;

/* XXX foreground default should be XtDefaultFGPixel. How do i do that?? */

static XtResource SinkResources[] = {
    {XtNfont, XtCFont, XtRFontStruct, sizeof (XFontStruct *),
        XtOffset(AsciiSinkPtr, font), XtRString, "Fixed"},
    {XtNforeground, XtCForeground, XtRPixel, sizeof (int),
        XtOffset(AsciiSinkPtr, foreground), XtRString, "Black"},    
};

/* Utilities */

static int CharWidth (w, x, c)
  Widget w;
  int x;
  char c;
{
    AsciiSinkData *data = (AsciiSinkData*) ((TextWidget)w)->text.sink->data;
    int     width, nonPrinting;
    XFontStruct *font = data->font;

    if (c == '\t') {
	int i;
	Position *tab;
	if (x >= w->core.width) return 0;
	for (i=0, tab=data->tabs; i<data->tab_count; i++, tab++) {
	    if (x < *tab) {
		if (*tab < w->core.width)
		    return *tab - x;
		else
		    return 0;
	    }
	}
	return 0;
    }
    if (c == LF)
	c = SP;
    nonPrinting = (c < SP);
    if (nonPrinting) c += '@';

    if (font->per_char &&
	    (c >= font->min_char_or_byte2 && c <= font->max_char_or_byte2))
	width = font->per_char[c - font->min_char_or_byte2].width;
    else
	width = font->min_bounds.width;

    if (nonPrinting)
	width += CharWidth(w, x, '^');

    return width;
}

/* Sink Object Functions */

static int AsciiDisplayText (w, x, y, pos1, pos2, highlight)
  Widget w;
  Position x, y;
  int highlight;
  XtTextPosition pos1, pos2;
{
    XtTextSink sink = ((TextWidget)w)->text.sink;
    XtTextSource source = ((TextWidget)w)->text.source;
    AsciiSinkData *data = (AsciiSinkData *) sink->data ;

    XFontStruct *font = data->font;
    int     j, k;
    Dimension width;
    XtTextBlock blk;
    GC gc = highlight ? data->invgc : data->normgc;
    GC invgc = highlight ? data->normgc : data->invgc;

    y += font->ascent;
    j = 0;
    while (pos1 < pos2) {
	pos1 = (*source->Read)(source, pos1, &blk, pos2 - pos1);
	for (k = 0; k < blk.length; k++) {
	    if (j >= bufferSize - 5) {
		bufferSize *= 2;
		buf = XtRealloc(buf, bufferSize);
	    }
	    buf[j] = blk.ptr[k];
	    if (buf[j] == LF)
		buf[j] = ' ';
	    else if (buf[j] == '\t') {
	        XDrawImageString(XtDisplay(w), XtWindow(w),
			gc, x, y, buf, j);
		buf[j] = 0;
		x += XTextWidth(data->font, buf, j);
		width = CharWidth(w, x, '\t');
		XFillRectangle(XtDisplay(w), XtWindow(w), invgc, x,
			       y - font->ascent, width,
			       (Dimension) (data->font->ascent +
					    data->font->descent));
		x += width;
		j = -1;
	    }
	    else
		if (buf[j] < ' ') {
		    buf[j + 1] = buf[j] + '@';
		    buf[j] = '^';
		    j++;
		}
	    j++;
	}
    }
    XDrawImageString(XtDisplay(w), XtWindow(w), gc, x, y, buf, j);
}


#define insertCursor_width 6
#define insertCursor_height 3
static char insertCursor_bits[] = {0x0c, 0x1e, 0x33};

#ifdef SERVERNOTBROKEN
static Pixmap CreateInsertCursor(s)
Screen *s;
{
    return (XCreateBitmapFromData (DisplayOfScreen(s), RootWindowOfScreen(s),
        insertCursor_bits, insertCursor_width, insertCursor_height));
}
#endif

/*
 * The following procedure manages the "insert" cursor.
 */

static AsciiInsertCursor (w, x, y, state)
  Widget w;
  Position x, y;
  XtTextInsertState state;
{
    XtTextSink sink = ((TextWidget)w)->text.sink;
    AsciiSinkData *data = (AsciiSinkData *) sink->data;

/*
    XCopyArea(sink->dpy,
	      (state == XtisOn) ? data->insertCursorOn : data->insertCursorOff,
	      w, data->normgc, 0, 0, insertCursor_width, insertCursor_height,
	      x - (insertCursor_width >> 1), y - (insertCursor_height));
*/

    if (state != data->laststate && XtIsRealized(w)) {
#ifdef SERVERNOTBROKEN
	XCopyPlane(XtDisplay(w),
		  data->insertCursorOn, XtWindow(w),
		  data->xorgc, 0, 0, insertCursor_width, insertCursor_height,
		  x - (insertCursor_width >> 1), y - (insertCursor_height), 1);
#else /* SERVER is BROKEN */
	/*
	 * See the comment down at the bottom where the pixmap gets built
	 * for why we are doing this this way.
	 */
	XCopyArea (XtDisplay(w), data->insertCursorOn, XtWindow (w),
		   data->xorgc, 0, 0, insertCursor_width, insertCursor_height,
		   x - (insertCursor_width >> 1), y - (insertCursor_height));
#endif /* SERVERNOTBROKEN */
    }
    data->laststate = state;
}

/*
 * Clear the passed region to the background color.
 */

static AsciiClearToBackground (w, x, y, width, height)
  Widget w;
  Position x, y;
  Dimension width, height;
{
#ifndef USE_CLEAR_AREA
    XFillRectangle(XtDisplay(w), XtWindow(w),
		   ((AsciiSinkData*)((TextWidget)w)->text.sink->data)->invgc,
		   x, y, width, height);
#else
    XClearArea(XtDisplay(w), XtWindow(w), x, y, width, height, False);
#endif /*USE_CLEAR_AREA*/
}

/*
 * Given two positions, find the distance between them.
 */

static AsciiFindDistance (w, fromPos, fromx, toPos,
			  resWidth, resPos, resHeight)
  Widget w;
  XtTextPosition fromPos;	/* First position. */
  int fromx;			/* Horizontal location of first position. */
  XtTextPosition toPos;		/* Second position. */
  int *resWidth;		/* Distance between fromPos and resPos. */
  XtTextPosition *resPos;	/* Actual second position used. */
  int *resHeight;		/* Height required. */
{
    XtTextSink sink = ((TextWidget)w)->text.sink;
    XtTextSource source = ((TextWidget)w)->text.source;

    AsciiSinkData *data;
    register    XtTextPosition index, lastPos;
    register char   c;
    XtTextBlock blk;

    data = (AsciiSinkData *) sink->data;
    /* we may not need this */
    lastPos = GETLASTPOS;
    (*source->Read)(source, fromPos, &blk, toPos - fromPos);
    *resWidth = 0;
    for (index = fromPos; index != toPos && index < lastPos; index++) {
	if (index - blk.firstPos >= blk.length)
	    (*source->Read)(source, index, &blk, toPos - fromPos);
	c = blk.ptr[index - blk.firstPos];
	if (c == LF) {
	    *resWidth += CharWidth(w, fromx + *resWidth, SP);
	    index++;
	    break;
	}
	*resWidth += CharWidth(w, fromx + *resWidth, c);
    }
    *resPos = index;
    *resHeight = data->font->ascent + data->font->descent;
}


static AsciiFindPosition(w, fromPos, fromx, width, stopAtWordBreak, 
			 resPos, resWidth, resHeight)
  Widget w;
  XtTextPosition fromPos; 	/* Starting position. */
  int fromx;			/* Horizontal location of starting position. */
  int width;			/* Desired width. */
  int stopAtWordBreak;		/* Whether the resulting position should be at
				   a word break. */
  XtTextPosition *resPos;	/* Resulting position. */
  int *resWidth;		/* Actual width used. */
  int *resHeight;		/* Height required. */
{
    XtTextSink sink = ((TextWidget)w)->text.sink;
    XtTextSource source = ((TextWidget)w)->text.source;
    AsciiSinkData *data;
    XtTextPosition lastPos, index, whiteSpacePosition;
    int     lastWidth, whiteSpaceWidth;
    Boolean whiteSpaceSeen;
    char    c;
    XtTextBlock blk;
    data = (AsciiSinkData *) sink->data;
    lastPos = GETLASTPOS;

    (*source->Read)(source, fromPos, &blk, bufferSize);
    *resWidth = 0;
    whiteSpaceSeen = FALSE;
    c = 0;
    for (index = fromPos; *resWidth <= width && index < lastPos; index++) {
	lastWidth = *resWidth;
	if (index - blk.firstPos >= blk.length)
	    (*source->Read)(source, index, &blk, bufferSize);
	c = blk.ptr[index - blk.firstPos];
	if (c == LF) {
	    *resWidth += CharWidth(w, fromx + *resWidth, SP);
	    index++;
	    break;
	}
	*resWidth += CharWidth(w, fromx + *resWidth, c);
	if ((c == SP || c == TAB) && *resWidth <= width) {
	    whiteSpaceSeen = TRUE;
	    whiteSpacePosition = index;
	    whiteSpaceWidth = *resWidth;
	}
    }
    if (*resWidth > width && index > fromPos) {
	*resWidth = lastWidth;
	index--;
	if (stopAtWordBreak && whiteSpaceSeen) {
	    index = whiteSpacePosition + 1;
	    *resWidth = whiteSpaceWidth;
	}
    }
    if (index == lastPos && c != LF) index = lastPos + 1;
    *resPos = index;
    *resHeight = data->font->ascent + data->font->descent;
}


static int AsciiResolveToPosition (w, pos, fromx, width,
				   leftPos, rightPos)
  Widget w;
  XtTextPosition pos;
  int fromx,width;
  XtTextPosition *leftPos, *rightPos;
{
    int     resWidth, resHeight;
    XtTextSource source = ((TextWidget)w)->text.source;

    AsciiFindPosition(w, pos, fromx, width, FALSE,
	    leftPos, &resWidth, &resHeight);
    if (*leftPos > GETLASTPOS)
	*leftPos = GETLASTPOS;
    *rightPos = *leftPos;
}


static int AsciiMaxLinesForHeight (w, height)
  Widget w;
  Dimension height;
{
    AsciiSinkData *data;
    XtTextSink sink = ((TextWidget)w)->text.sink;

    data = (AsciiSinkData *) sink->data;
    return(height / (data->font->ascent + data->font->descent));
}


static int AsciiMaxHeightForLines (w, lines)
  Widget w;
  int lines;
{
    AsciiSinkData *data;
    XtTextSink sink = ((TextWidget)w)->text.sink;

    data = (AsciiSinkData *) sink->data;
    return(lines * (data->font->ascent + data->font->descent));
}


static void AsciiSetTabs (w, offset, tab_count, tabs)
  Widget w;			/* for context */
  Position offset;		/* from left, for margin */
  int tab_count;		/* count of entries in tabs */
  Position *tabs;		/* list of character positions */
{
    AsciiSinkData *data = (AsciiSinkData*)((TextWidget)w)->text.sink->data;
    int i;

    if (tab_count > data->tab_count) {
	data->tabs = (Position*)XtRealloc(data->tabs,
				    (unsigned)tab_count * sizeof(Position*));
    }
    
    for (i=0; i < tab_count; i++) {
	data->tabs[i] = offset + tabs[i] * data->em;
    }
    data->tab_count = tab_count;
}

/***** Public routines *****/

XtTextSink XtAsciiSinkCreate (parent, args, num_args)
    Widget	parent;
    ArgList 	args;
    Cardinal 	num_args;
{
    XtTextSink sink;
    AsciiSinkData *data;
    unsigned long valuemask = (GCFont | GCGraphicsExposures |
			       GCForeground | GCBackground | GCFunction);
    XGCValues values;
    long wid;
    XFontStruct *font;

    if (!buf) buf = XtMalloc(bufferSize);

    sink = XtNew(XtTextSinkRec);
    sink->Display = AsciiDisplayText;
    sink->InsertCursor = AsciiInsertCursor;
    sink->ClearToBackground = AsciiClearToBackground;
    sink->FindPosition = AsciiFindPosition;
    sink->FindDistance = AsciiFindDistance;
    sink->Resolve = AsciiResolveToPosition;
    sink->MaxLines = AsciiMaxLinesForHeight;
    sink->MaxHeight = AsciiMaxHeightForLines;
    sink->SetTabs = AsciiSetTabs;
    data = XtNew(AsciiSinkData);
    sink->data = (caddr_t)data;

    XtGetSubresources (parent, (caddr_t)data, XtNtextSink, XtCTextSink, 
		       SinkResources, XtNumber(SinkResources),
		       args, num_args);

    font = data->font;
    values.function = GXcopy;
    values.font = font->fid;
    values.graphics_exposures = (Bool) FALSE;
    values.foreground = data->foreground;
    values.background = parent->core.background_pixel;
    data->normgc = XtGetGC(parent, valuemask, &values);
    values.foreground = parent->core.background_pixel;
    values.background = data->foreground;
    data->invgc = XtGetGC(parent, valuemask, &values);
    values.function = GXxor;
    values.foreground = data->foreground ^ parent->core.background_pixel;
    values.background = 0;
    data->xorgc = XtGetGC(parent, valuemask, &values);


    wid = -1;
    if ((!XGetFontProperty(font, XA_QUAD_WIDTH, &wid)) || wid <= 0) {
	if (font->per_char && font->min_char_or_byte2 <= '0' &&
	    		      font->max_char_or_byte2 >= '0')
	    wid = font->per_char['0' - font->min_char_or_byte2].width;
	else
	    wid = font->max_bounds.width;
    }
    if (wid <= 0)
	data->em = 1;
    else
	data->em = wid;

    data->font = font;
#ifdef SERVERNOTBROKEN
    data->insertCursorOn = CreateInsertCursor(XtScreen(parent));
#else
    /*
     * This is the work around for not being able to do CopyPlane with XOR.
     * However, there is another bug which doesn't let us use the new
     * CreatePixmapFromBitmapData routine to build the pixmap that we will
     * use CopyArea with.
     */
#ifdef SERVERNOTBROKEN2
    data->insertCursorOn =
      XCreatePixmapFromBitmapData (XtDisplay (parent), 
				   RootWindowOfScreen(XtScreen(parent)),
				   insertCursor_bits, insertCursor_width,
				   insertCursor_height, data->foreground,
				   parent->core.background_pixel,
				   parent->core.depth);
#else /* SERVER is BROKEN the second way */
    {
	Screen *screen = XtScreen (parent);
	Display *dpy = XtDisplay (parent);
	Window root = RootWindowOfScreen(screen);
	Pixmap bitmap = XCreateBitmapFromData (dpy, root, insertCursor_bits,
					       insertCursor_width,
					       insertCursor_height);
	Pixmap pixmap = XCreatePixmap (dpy, root, insertCursor_width,
				       insertCursor_height,
				       DefaultDepthOfScreen (screen));
	XGCValues gcv;
	GC gc;

	gcv.function = GXcopy;
	gcv.foreground = data->foreground ^ parent->core.background_pixel;
	gcv.background = 0;
	gcv.graphics_exposures = False;
	gc = XtGetGC (parent, (GCFunction | GCForeground | GCBackground |
			       GCGraphicsExposures), &gcv);
	XCopyPlane (dpy, bitmap, pixmap, gc, 0, 0, insertCursor_width,
		    insertCursor_height, 0, 0, 1);
	XtDestroyGC (gc);
	data->insertCursorOn = pixmap;
    }
#endif /* SERVERNOTBROKEN2 */
#endif /* SERVERNOTBROKEN */
    data->laststate = XtisOff;
    data->tab_count = 0;
    data->tabs = NULL;
    return sink;
}

void XtAsciiSinkDestroy (sink)
    XtTextSink sink;
{
    AsciiSinkData *data;

    data = (AsciiSinkData *) sink->data;
    XtFree((char *) data->tabs);
    XtFree((char *) data);
    XtFree((char *) sink);
}
