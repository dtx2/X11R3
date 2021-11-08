/*
 * $XConsortium: SetPRoot.c,v 1.2 88/09/06 14:26:49 jim Exp $
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
#include "Xlibint.h"
#define NEED_PSEUDOROOT_PROTOCOL
#include "PseudoRoot.h"


/*
 * routines to convert client data structures specified in the ICCCM to 
 * protocol data structures (arrays of unsigned longs)
 */

static Status PseudoScreenClientToProtocol (c, p, n)
    register PseudoScreen *c;
    register xPseudoScreen *p;
    register int n;
{
    if (!c || !p) return False;

    for (; n > 0; n--, c++, p++) {
	/*
	 * If your C compiler rearranges fields within structures, you will
	 * have to change the definition of protocol structure to be an array 
	 * of unsigned longs to which you assign using macros.
	 */
	p->root = c->root;
	p->width = c->width;
	p->height = c->height;
	p->mwidth = c->mwidth;
	p->mheight = c->mheight;
	p->root_depth = c->root_depth;
	p->root_visual = c->root_visual;
	p->cmap = c->cmap;
	p->white_pixel = c->white_pixel;
	p->black_pixel = c->black_pixel;
	p->max_maps = c->max_maps;
	p->min_maps = c->min_maps;
	p->backing_store = c->backing_store;
	p->save_unders = c->save_unders;
	p->root_input_mask = c->root_input_mask;
	p->depths = c->depths;
    }

    return True;
}


static Status PseudoDepthClientToProtocol (c, p, n)
    register PseudoDepth *c;
    register xPseudoDepth *p;
    register int n;
{
    if (!c || !p) return False;

    for (; n > 0; n--, c++, p++) {
	/*
	 * If your C compiler rearranges fields within structures, you will
	 * have to change the definition of protocol structure to be an array 
	 * of unsigned longs to which you assign using macros.
	 */
	p->depth = c->depth;
	p->nvisuals = c->nvisuals;
	p->visuals = c->visuals;
    }

    return True;
}

static Status PseudoVisualClientToProtocol (c, p, n)
    register PseudoVisual *c;
    register xPseudoVisual *p;
    register int n;
{
    if (!c || !p) return False;

    for (; n > 0; n--, c++, p++) {
	/*
	 * If your C compiler rearranges fields within structures, you will
	 * have to change the definition of protocol structure to be an array 
	 * of unsigned longs to which you assign using macros.
	 */
	p->visualid = c->visualid;
	p->class = c->class;
	p->bits_per_rgb = c->bits_per_rgb;
	p->map_entries = c->map_entries;
	p->red_mask = c->red_mask;
	p->green_mask = c->green_mask;
	p->blue_mask = c->blue_mask;
    }

    return True;
}


/*
 * semi-public interfaces.  Again, these routines are subject to change without
 * notice and should not be incorporated into any toolkit or application.  They
 * exist solely for prototyping the ICCCM functionality.
 *
 *     _XSetPseudoScreen          store the screen data
 *     _XSetPseudoDepths          store the depths information
 *     _XSetPseudoVisuals         store the visuals information
 *
 */

Status _XSetPseudoScreen (dpy, w, propid, proptype, ps)
    Display *dpy;
    Window w;
    Atom propid, proptype;
    PseudoScreen *ps;
{
    xPseudoScreen xps;

    if (!PseudoScreenClientToProtocol (ps, &xps, 1)) 
      return False;

    XChangeProperty (dpy, w, propid, proptype, PSEUDO_SCREEN_FORMAT,
		     PropModeReplace, (unsigned char *) &xps,
		     (SIZEOF(xPseudoScreen) >> 2));
    return True;
}

Status _XSetPseudoDepths (dpy, w, propid, proptype, pd, n)
    Display *dpy;
    Window w;
    Atom propid, proptype;
    PseudoDepth *pd;
    int n;
{
    xPseudoDepth *xpd;

    xpd = (xPseudoDepth *) malloc (n * sizeof(xPseudoDepth));
    if (!xpd) 
      return False;

    if (!PseudoDepthClientToProtocol (pd, xpd, n)) {
	free ((char *) xpd);
	return False;
    }

    XChangeProperty (dpy, w, propid, proptype, PSEUDO_DEPTHS_FORMAT,
		     PropModeReplace, (unsigned char *) xpd,
		     (n * (SIZEOF(xPseudoDepth) >> 2)));
    free ((char *) xpd);
    return True;
}

Status _XSetPseudoVisuals (dpy, w, propid, proptype, pv, n)
    Display *dpy;
    Window w;
    Atom propid, proptype;
    PseudoVisual *pv;
    int n;
{
    xPseudoVisual *xpv;

    xpv = (xPseudoVisual *) malloc (n * sizeof(xPseudoVisual));
    if (!xpv) 
      return False;

    if (!PseudoVisualClientToProtocol (pv, xpv, n)) {
	free ((char *) xpv);
	return False;
    }

    XChangeProperty (dpy, w, propid, proptype, PSEUDO_DEPTHS_FORMAT,
		     PropModeReplace, (unsigned char *) xpv,
		     (n * (SIZEOF(xPseudoVisual) >> 2)));
    free ((char *) xpv);
    return True;
}




