#include <X11/copyright.h>

/* Copyright 1987 Massachusetts Institute of Technology */

/*
 * xwd.c MIT Project Athena, X Window system window raster image dumper.
 *
 * This program will dump a raster image of the contents of a window into a 
 * file for output on graphics printers or for other uses.
 *
 *  Author:	Tony Della Fera, DEC
 *		17-Jun-85
 * 
 *  Modification history:
 *
 *  11/14/86 Bill Wyatt, Smithsonian Astrophysical Observatory
 *    - Removed Z format option, changing it to an XY option. Monochrome 
 *      windows will always dump in XY format. Color windows will dump
 *      in Z format by default, but can be dumped in XY format with the
 *      -xy option.
 *
 *  11/18/86 Bill Wyatt
 *    - VERSION 6 is same as version 5 for monchrome. For colors, the 
 *      appropriate number of Color structs are dumped after the header,
 *      which has the number of colors (=0 for monochrome) in place of the
 *      V5 padding at the end. Up to 16-bit displays are supported. I
 *      don't yet know how 24- to 32-bit displays will be handled under
 *      the Version 11 protocol.
 *
 *  6/15/87 David Krikorian, MIT Project Athena
 *    - VERSION 7 runs under the X Version 11 servers, while the previous
 *      versions of xwd were are for X Version 10.  This version is based
 *      on xwd version 6, and should eventually have the same color
 *      abilities. (Xwd V7 has yet to be tested on a color machine, so
 *      all color-related code is commented out until color support
 *      becomes practical.)
 */

#ifndef lint
static char *rcsid_xwd_c = "$XConsortium: xwd.c,v 1.41 88/09/20 23:38:19 jim Exp $";
#endif

/*%
 *%    This is the format for commenting out color-related code until
 *%  color can be supported.
%*/

#include <X11/Xos.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <stdio.h>

#define FEEP_VOLUME 0

/* Include routines to do parsing */
#include "dsimple.h"

/* Setable Options */

int format = ZPixmap;
Bool nobdrs = False;
Bool standard_out = True;
Bool debug = False;
long add_pixel_value = 0;

extern int (*_XErrorFunction)();
extern int _XDefaultError();

static long parse_long (s)
    char *s;
{
    char *fmt = "%lu";
    long retval = 0L;
    int thesign = 1;

    if (s && s[0]) {
	if (s[0] == '-') s++, thesign = -1;
	if (s[0] == '0') s++, fmt = "%lo";
	if (s[0] == 'x' || s[0] == 'X') s++, fmt = "%lx";
	(void) sscanf (s, fmt, &retval);
    }
    return (thesign * retval);
}

main(argc, argv)
    int argc;
    char **argv;
{
    register i;
    Window target_win;
    FILE *out_file = stdout;

    INIT_NAME;

    Setup_Display_And_Screen(&argc, argv);

    /* Get window select on command line, if any */
    target_win = Select_Window_Args(&argc, argv);

    for (i = 1; i < argc; i++) {
	if (!strcmp(argv[i], "-nobdrs")) {
	    nobdrs = True;
	    continue;
	}
	if (!strcmp(argv[i], "-debug")) {
	    debug = True;
	    continue;
	}
	if (!strcmp(argv[i], "-help"))
	  usage();
	if (!strcmp(argv[i], "-out")) {
	    if (++i >= argc) usage();
	    if (!(out_file = fopen(argv[i], "w")))
	      Error("Can't open output file as specified.");
	    standard_out = False;
	    continue;
	}
	if (!strcmp(argv[i], "-xy")) {
	    format = XYPixmap;
	    continue;
	}
	if (!strcmp(argv[i], "-add")) {
	    if (++i >= argc) usage();
	    add_pixel_value = parse_long (argv[i]);
	    continue;
	}
	usage();
    }
    
    /*
     * Let the user select the target window.
     */
    if (!target_win)
      target_win = Select_Window(dpy);

    /*
     * Dump it!
     */
    Window_Dump(target_win, out_file);

    fclose(out_file);
}


/*
 * Window_Dump: dump a window to a file which must already be open for
 *              writting.
 */

char *calloc();

#include "X11/XWDFile.h"

Window_Dump(window, out)
     Window window;
     FILE *out;
{
    unsigned long swaptest = 1;
    XColor *colors;
    unsigned buffer_size;
    int win_name_size;
    int header_size;
    int ncolors, i;
    char *win_name;
    XWindowAttributes win_info;
    XImage *image;
    int absx, absy, x, y;
    unsigned width, height;
    int dwidth, dheight;
    int bw;

    XWDFileHeader header;

    
    /*
     * Inform the user not to alter the screen.
     */
    Beep();

    /*
     * Get the parameters of the window being dumped.
     */
    if (debug) outl("xwd: Getting target window information.\n");
    if(!XGetWindowAttributes(dpy, window, &win_info)) 
      Fatal_Error("Can't get target window attributes.");

    absx = win_info.x + (nobdrs ? win_info.border_width : 0);
    absy = win_info.y + (nobdrs ? win_info.border_width : 0);
    width = win_info.width + (nobdrs ? 0 : (2 * win_info.border_width));
    height = win_info.height + (nobdrs ? 0 : (2 * win_info.border_width));
    dwidth = DisplayWidth (dpy, screen);
    dheight = DisplayHeight (dpy, screen);

    /* clip to window */
    if (absx < 0) width += absx, absx = 0;
    if (absy < 0) height += absy, absy = 0;
    if (absx + width > dwidth) width = dwidth - absx;
    if (absy + height > dheight) width = dheight - absy;

    XFetchName(dpy, window, &win_name);
    if (!win_name || !win_name[0])
      win_name = "xwdump";

    /* sizeof(char) is included for the null string terminator. */
    win_name_size = strlen(win_name) + sizeof(char);

    /*
     * Snarf the pixmap with XGetImage.
     */

    bw = nobdrs ? 0 : win_info.border_width;
    x = absx - win_info.x - bw;
    y = absy - win_info.y - bw;
    image = XGetImage (dpy, window, x, y, width, height, AllPlanes, format);
    if (!image) {
	fprintf (stderr, "%s:  unable to get image at %dx%d+%d+%d\n",
		 program_name, width, height, x, y);
	exit (1);
    }

    if (add_pixel_value != 0) XAddPixel (image, add_pixel_value);

    /*
     * Determine the pixmap size.
     */
    buffer_size = Image_Size(image);

    if (debug) outl("xwd: Getting Colors.\n");

    ncolors = Get_XColors(&win_info, &colors);

    /*
     * Inform the user that the image has been retrieved.
     */
    XBell(dpy, FEEP_VOLUME);
    XBell(dpy, FEEP_VOLUME);
    XFlush(dpy);

    /*
     * Calculate header size.
     */
    if (debug) outl("xwd: Calculating header size.\n");
    header_size = sizeof(header) + win_name_size;

    /*
     * Write out header information.
     */
    if (debug) outl("xwd: Constructing and dumping file header.\n");
    header.header_size = (xwdval) header_size;
    header.file_version = (xwdval) XWD_FILE_VERSION;
    header.pixmap_format = (xwdval) format;
    header.pixmap_depth = (xwdval) image->depth;
    header.pixmap_width = (xwdval) image->width;
    header.pixmap_height = (xwdval) image->height;
    header.xoffset = (xwdval) image->xoffset;
    header.byte_order = (xwdval) image->byte_order;
    header.bitmap_unit = (xwdval) image->bitmap_unit;
    header.bitmap_bit_order = (xwdval) image->bitmap_bit_order;
    header.bitmap_pad = (xwdval) image->bitmap_pad;
    header.bits_per_pixel = (xwdval) image->bits_per_pixel;
    header.bytes_per_line = (xwdval) image->bytes_per_line;
    header.visual_class = (xwdval) win_info.visual->class;
    header.red_mask = (xwdval) win_info.visual->red_mask;
    header.green_mask = (xwdval) win_info.visual->green_mask;
    header.blue_mask = (xwdval) win_info.visual->blue_mask;
    header.bits_per_rgb = (xwdval) win_info.visual->bits_per_rgb;
    header.colormap_entries = (xwdval) win_info.visual->map_entries;
    header.ncolors = ncolors;
    header.window_width = (xwdval) win_info.width;
    header.window_height = (xwdval) win_info.height;
    header.window_x = absx;
    header.window_y = absy;
    header.window_bdrwidth = (xwdval) win_info.border_width;

    if (*(char *) &swaptest) {
	_swaplong((char *) &header, sizeof(header));
	for (i = 0; i < ncolors; i++) {
	    _swaplong((char *) &colors[i].pixel, sizeof(long));
	    _swapshort((char *) &colors[i].red, 3 * sizeof(short));
	}
    }

    (void) fwrite((char *)&header, sizeof(header), 1, out);
    (void) fwrite(win_name, win_name_size, 1, out);

    /*
     * Write out the color maps, if any
     */

    if (debug) outl("xwd: Dumping %d colors.\n", ncolors);
    (void) fwrite((char *) colors, sizeof(XColor), ncolors, out);

    /*
     * Write out the buffer.
     */
    if (debug) outl("xwd: Dumping pixmap.  bufsize=%d\n",buffer_size);

    /*
     *    This copying of the bit stream (data) to a file is to be replaced
     *  by an Xlib call which hasn't been written yet.  It is not clear
     *  what other functions of xwd will be taken over by this (as yet)
     *  non-existant X function.
     */
    (void) fwrite(image->data, (int) buffer_size, 1, out);

    /*
     * free the color buffer.
     */

    if(debug && ncolors > 0) outl("xwd: Freeing colors.\n");
    if(ncolors > 0) free(colors);

    /*
     * Free window name string.
     */
    if (debug) outl("xwd: Freeing window name string.\n");
    free(win_name);

    /*
     * Free image
     */
    XDestroyImage(image);
}

/*
 * Report the syntax for calling xwd.
 */
usage()
{
    fprintf (stderr,
"usage: %s [-display host:dpy] [-debug] [-help] %s [-nobdrs] [-out <file>]",
	   program_name, SELECT_USAGE);
    fprintf (stderr, " [-xy] [-add value]\n");
    exit(1);
}


/*
 * Error - Fatal xwd error.
 */
extern int errno;

Error(string)
	char *string;	/* Error description string. */
{
	outl("\nxwd: Error => %s\n", string);
	if (errno != 0) {
		perror("xwd");
		outl("\n");
	}

	exit(1);
}


/*
 * Determine the pixmap size.
 */

int Image_Size(image)
     XImage *image;
{
    if (format != ZPixmap)
      return(image->bytes_per_line * image->height * image->depth);

    return(image->bytes_per_line * image->height);
}


/*
 * Get the XColors of all pixels in image - returns # of colors
 */
int Get_XColors(win_info, colors)
     XWindowAttributes *win_info;
     XColor **colors;
{
    int i, ncolors;

    if (!win_info->colormap)
	return(0);

    if (win_info->visual->class == TrueColor ||
	win_info->visual->class == DirectColor)
	return(0);    /* XXX punt for now */

    ncolors = win_info->visual->map_entries;
    if (!(*colors = (XColor *) malloc (sizeof(XColor) * ncolors)))
      Fatal_Error("Out of memory!");

    for (i=0; i<ncolors; i++)
      (*colors)[i].pixel = i;

    XQueryColors(dpy, win_info->colormap, *colors, ncolors);
    
    return(ncolors);
}

_swapshort (bp, n)
    register char *bp;
    register unsigned n;
{
    register char c;
    register char *ep = bp + n;

    while (bp < ep) {
	c = *bp;
	*bp = *(bp + 1);
	bp++;
	*bp++ = c;
    }
}

_swaplong (bp, n)
    register char *bp;
    register unsigned n;
{
    register char c;
    register char *ep = bp + n;
    register char *sp;

    while (bp < ep) {
	sp = bp + 3;
	c = *sp;
	*sp = *bp;
	*bp++ = c;
	sp = bp + 1;
	c = *sp;
	*sp = *bp;
	*bp++ = c;
	bp += 2;
    }
}
