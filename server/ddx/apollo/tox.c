/******************************************************************
Copyright 1987 by Apollo Computer Inc., Chelmsford, Massachusetts.

                        All Rights Reserved

Permission to use, duplicate, change, and distribute this software and
its documentation for any purpose and without fee is granted, provided
that the above copyright notice appear in such copy and that this
copyright notice appear in all supporting documentation, and that the
names of Apollo Computer Inc. or MIT not be used in advertising or publicity
pertaining to distribution of the software without written prior permission.
******************************************************************/

/* tox.c, jah, 10/19/87    */

/* Tell X Server to take back the display and continue */

#include "sys/ins/base.ins.c"
#include "sys/ins/ec2.ins.c"
#include "sys/ins/mutex.ins.c"
#include "sys/ins/sfcb.ins.c"
#include "sys/ins/pgm.ins.c"
#include "sys/ins/ev.ins.c"

#include "Xlib.h"
#include "Xatom.h"
#include "X.h"

#include "switcher.h"

#define FAILURE			-1
#define NULL 0

extern xoid_$t xoid_$nil;

int mytype[2] = {0x37E231FB,0xA000AF6A};

Display *dpy;


void Error(s)
char *s;
{
    printf(s);
    pgm_$exit();
}


Bool Redraw(window)
Window window;                          /* Event window. */
{
    Visual visual;
    XSetWindowAttributes xswa;

    XWindowAttributes winfo;		/* window info. */
    Window w;				/* Refresh window. */
    int status;		/* Routine return status. */

    /*
     * Get info on the target window.
     */
    status = XGetWindowAttributes(dpy, window, &winfo);
    if (status == FAILURE) Error("Refresh -> Can't query target window.");

    /*
     * Create and map a window which covers the target window, then destroy it.
     */
    xswa.background_pixel = BlackPixel(dpy, DefaultScreen(dpy));
    xswa.override_redirect = True;
    visual.visualid = CopyFromParent;
    w = XCreateWindow(dpy, window, 0, 0, 9999, 9999,
	    0, DefaultDepth(dpy, DefaultScreen(dpy)), InputOutput, &visual,
	    CWBackPixel | CWOverrideRedirect, &xswa);
    XMapWindow(dpy, w);
    XFlush(dpy);

    return(False);
}


Display *myOpenDisplay()
{
    char hostname[256];
    char *hnp;

/* note: we can't rely on the DISPLAY environment variable here,
         because it generally won't be set in the environment
         (CPO from a DM key definition) in which this code will
         normally run */

    if (gethostname( hostname, 256 )) return( NULL );
    hnp = hostname;
    /* append ":0" */
    while (*hnp) hnp++;
    if (hnp - hostname + 2 >= 256) return( NULL );
    *hnp++ = ':';
    *hnp++ = '0';
    *hnp++ = 0;

    return( XOpenDisplay( hostname ) );
}



main()
{
    switcher_sfcb *mysfcb;
    status_$t st;
    short ssize;
    char *evn, *evv;

    ssize = sizeof(*mysfcb);
    sfcb_$get( mytype, xoid_$nil, ssize, mysfcb, st );
    if (mysfcb->use_count == 1) {
        ec2_$init( mysfcb->theEc );
        mysfcb->use_count = 2;
    }
    mysfcb->xGo = 0xFF; 
    mutex_$unlock( mysfcb->slock );
    ec2_$advance( mysfcb->theEc, st );

    /* force case-sensitivity */
    evn = "DOWNCASE";
    evv = "F";
    ev_$set_var( *evn, 8, *evv, 1 );

    /* now open a connection to the server so that we can refresh the screen */
    if ((dpy = myOpenDisplay()) == NULL)
	    printf("Couldn't open display!");
    else
        Redraw(RootWindow( dpy, DefaultScreen(dpy) ));

    mysfcb->xGo = 0;

}
