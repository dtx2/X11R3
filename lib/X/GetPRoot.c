/*
 * $XConsortium: GetPRoot.c,v 1.8 88/10/22 10:00:55 rws Exp $
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

#include "copyright.h"

#include <stdio.h>
#include "Xlibint.h"
#include <X11/Xos.h>
#include "Xatom.h"
#define NEED_PSEUDOROOT_PROTOCOL
#include "PseudoRoot.h"


/*
 * routines to convert protocol data structures (arrays of unsigned longs)
 * to the client data structures specified in the ICCCM.
 */

static Status PseudoScreenProtocolToClient (p, c, n)
    register xPseudoScreen *p;
    register PseudoScreen *c;
    register int n;
{
    if (!c || !p) {
	return False;
    }

    for (; n > 0; n--, c++, p++) {
	/*
	 * If your C compiler rearranges fields within structures, you will
	 * have to change the definition of protocol structure to be an array 
	 * of unsigned longs to which you assign using macros.
	 */
	c->root = p->root;
	c->width = p->width;
	c->height = p->height;
	c->mwidth = p->mwidth;
	c->mheight = p->mheight;
	c->root_depth = p->root_depth;
	c->root_visual = p->root_visual;
	c->cmap = p->cmap;
	c->white_pixel = p->white_pixel;
	c->black_pixel = p->black_pixel;
	c->max_maps = p->max_maps;
	c->min_maps = p->min_maps;
	c->backing_store = p->backing_store;
	c->save_unders = p->save_unders;
	c->root_input_mask = p->root_input_mask;
	c->depths = p->depths;
    }

    return True;
}

static Status PseudoDepthProtocolToClient (p, c, n)
    register xPseudoDepth *p;
    register PseudoDepth *c;
    register int n;
{
    if (!c || !p) {
	return False;
    }

    for (; n > 0; n--, c++, p++) {
	/*
	 * If your C compiler rearranges fields within structures, you will
	 * have to change the definition of protocol structure to be an array 
	 * of unsigned longs to which you assign using macros.
	 */
	c->depth = p->depth;
	c->nvisuals = p->nvisuals;
	c->visuals = p->visuals;
    }

    return True;
}

static Status PseudoVisualProtocolToClient (p, c, n)
    register xPseudoVisual *p;
    register PseudoVisual *c;
    register int n;
{
    if (!c || !p) {
	return False;
    }

    for (; n > 0; n--, c++, p++) {
	/*
	 * If your C compiler rearranges fields within structures, you will
	 * have to change the definition of protocol structure to be an array 
	 * of unsigned longs to which you assign using macros.
	 */
	c->visualid = p->visualid;
	c->class = p->class;
	c->bits_per_rgb = p->bits_per_rgb;
	c->map_entries = p->map_entries;
	c->red_mask = p->red_mask;
	c->green_mask = p->green_mask;
	c->blue_mask = p->blue_mask;
    }

    return True;
}


/*
 * routines to retrieve the protocol data structures from properties on the
 * screen's root window; the final implementation will probably want to make
 * them atomic by adding the appropriate GrabServer calls
 */

static Status GetPseudoScreen (dpy, w, propName, ps)
    Display *dpy;
    Window w;
    char *propName;
    PseudoScreen *ps;
{
    Atom screenProp, screenType, actual_type;
    xPseudoScreen *xps = NULL;
    int actual_format;
    unsigned long nitems, bytesafter;
    Status status;

    /*
     * get atoms for the screen type and indicated property in that order
     * so as to minimize unused atoms in the server
     */
    if ((screenType = XInternAtom (dpy, PSEUDO_SCREEN_TYPE, True)) == None ||
	(screenProp = XInternAtom (dpy, propName, True)) == None) {
	return False;
    }

    if (XGetWindowProperty (dpy, w, screenProp, 0L,
			    (long) (SIZEOF(xPseudoScreen) >> 2L), False,
			    screenType, &actual_type, &actual_format,
			    &nitems, &bytesafter, (unsigned char **) &xps) !=
	Success) {
	return False;
    }

    if (actual_type != screenType || actual_format != PSEUDO_SCREEN_FORMAT) {
	status = False;
    } else {
	status = PseudoScreenProtocolToClient (xps, ps, 1);
    }
    free ((char *) xps);
    return status;
}

static Status GetPseudoDepths (dpy, w, depthsProp, depthsType,
			       ndepthsPtr, pdPtr)
    Display *dpy;
    Window w;
    Atom depthsProp, depthsType;
    int *ndepthsPtr;
    PseudoDepth **pdPtr;
{
    xPseudoDepth *xpd = NULL;
    PseudoDepth *pd = NULL;
    Atom actual_type;
    int actual_format;
    unsigned long nitems, bytesafter;
    Status status;
    int ndepths;

    /*
     * XXX - this routine should probably grab the server to be atomic; the
     * spec doesn't provide a field indicating how many depths there are, so
     * we have to go figure it out ourselves.  That is fine, except that it
     * isn't symmetric with the way visuals are counted within PseudoDepths.
     */

    if (XGetWindowProperty (dpy, w, depthsProp, 0L, 0L, False, depthsType,
			    &actual_type, &actual_format, &nitems, &bytesafter,
			    (unsigned char **) &xpd) != Success) {
	return False;
    }

    if (xpd) {
	free ((char *) xpd);
	xpd = NULL;
    }

    if (actual_type != depthsType || actual_format != PSEUDO_DEPTHS_FORMAT) {
	return False;
    }

    ndepths = bytesafter / SIZEOF(xPseudoDepth);

    if (XGetWindowProperty (dpy, w, depthsProp, 0L, 
			    (long) (ndepths * SIZEOF(xPseudoDepth)) >> 2L,
			    False, depthsType, &actual_type, &actual_format,
			    &nitems, &bytesafter, (unsigned char **) &xpd) !=
	Success) {
	return False;
    }

    if (actual_type != depthsType || actual_format != PSEUDO_DEPTHS_FORMAT) {
	status = False;
    } else {
	pd = (PseudoDepth *) Xcalloc (ndepths, sizeof(PseudoDepth));
	if (pd) {
	    status = PseudoDepthProtocolToClient (xpd, pd, ndepths);
	} else {
	    status = False;
	}
    }
    free ((char *) xpd);
    if (status) {
	*ndepthsPtr = ndepths;
	*pdPtr = pd;
    }
    return status;
}

static Status GetPseudoVisuals (dpy, w, visualsProp, visualsType,
				nvisualsPtr, pvPtr)
    Display *dpy;
    Window w;
    Atom visualsProp, visualsType;
    int *nvisualsPtr;
    PseudoVisual **pvPtr;
{
    xPseudoVisual *xpv = NULL;
    PseudoVisual *pv = NULL;
    Atom actual_type;
    int actual_format;
    unsigned long nitems, bytesafter;
    Status status;
    int nvisuals;

    /*
     * XXX - this routine should probably grab the server to be atomic, or it
     * should believe the PseudoDepth's notion of how many visuals there are;
     * except that the PseudoScreen doesn't record how many depths there are.
     */

    if (XGetWindowProperty (dpy, w, visualsProp, 0L, 0L, False, visualsType,
			    &actual_type, &actual_format, &nitems, &bytesafter,
			    (unsigned char **) &xpv) != Success) {
	return False;
    }

    if (xpv) {
	free ((char *) xpv);
	xpv = NULL;
    }

    if (actual_type != visualsType || actual_format != PSEUDO_DEPTHS_FORMAT) {
	return False;
    }

    nvisuals = bytesafter / SIZEOF(xPseudoVisual);

    if (XGetWindowProperty (dpy, w, visualsProp, 0L, 
			    (long) (nvisuals * SIZEOF(xPseudoVisual)) >> 2L,
			    False, visualsType, &actual_type, &actual_format,
			    &nitems, &bytesafter, (unsigned char **) &xpv) !=
	Success) {
	return False;
    }

    if (actual_type != visualsType || actual_format != PSEUDO_VISUALS_FORMAT) {
	status = False;
    } else {
	pv = (PseudoVisual *) Xcalloc (nvisuals, sizeof(PseudoVisual));
	if (pv) {
	    status = PseudoVisualProtocolToClient (xpv, pv, nvisuals);
	} else {
	    status = False;
	}
    }
    free ((char *) xpv);
    if (status) {
	*nvisualsPtr = nvisuals;
	*pvPtr = pv;
    }
    return status;
}



/*
 * semi-public interfaces.  Again, these routines are subject to change without
 * notice and should not be incorporated into any toolkit or application.  They
 * exist solely for prototyping the ICCCM functionality.
 *
 *     _XGetPseudoRootData        fetch appropriate data from the properties
 *     _XGetPseudoRoot            replace screen information
 *
 */

Status _XGetPseudoRootData (dpy, w, propName, ps,
			    depthsPtr, visualsListPtr, ndepthsPtr)
    Display *dpy;			/* connection to server */
    Window w;				/* window from which to get info */
    char *propName;			/* prop containing info */
    PseudoScreen *ps;			/* where to fill in info */
    PseudoDepth **depthsPtr;		/* new depths */
    PseudoVisual ***visualsListPtr;	/* new visuals for each depth */
    int *ndepthsPtr;			/* number of new depths */
{
    Atom depthsType = None;
    int ndepths = 0;
    PseudoDepth *depths = NULL;
    PseudoVisual **visualsList = NULL;
    

    /*
     * fill in the pseudo screen
     */
    if (!GetPseudoScreen (dpy, w, propName, ps)) {
	return False;
    }

    /*
     * now, check to see if we need to get the depths structure
     */
    if (ps->depths != None) {
	Atom visualsType = None;	/* if necessary */
	int i;				/* iterator, temp variable */

	depthsType = XInternAtom (dpy, PSEUDO_DEPTHS_TYPE, True);
	if (depthsType == None) {
	    return False;
	}

	if (!GetPseudoDepths (dpy, w, ps->depths, depthsType,
			      &ndepths, &depths)) {
	    return False;
	}

	/*
	 * get new visual info for depths that want it
	 */
	visualsList = (PseudoVisual **) Xmalloc (ndepths *
						 sizeof (PseudoVisual **));
	if (!visualsList) {
	    free ((char *) depths);
	    return False;
	}

	/* get the visual atom in case */
	visualsType = XInternAtom (dpy, PSEUDO_VISUALS_TYPE, False);
	if (visualsType == None) {
	    free ((char *) visualsList);
	    free ((char *) depths);
	    return False;
	}

	for (i = 0; i < ndepths; i++) {
	    if (depths[i].visuals == None) {
		visualsList[i] = NULL;
	    } else {
		int nvisuals = 0;
		PseudoVisual *visuals = NULL;

		/* get the visuals for this depth */
		if (!GetPseudoVisuals (dpy, w, depths[i].visuals, visualsType,
				       &nvisuals, &visuals)) {
		    while (--i > 0) {
			if (depths[i].visuals)
			  free ((char *) visualsList[i]);
		    }
		    free ((char *) visualsList);
		    free ((char *) depths);
		    return False;
		}
		visualsList[i] = visuals;
		if (depths[i].nvisuals != nvisuals) {
		    /*
		     * XXX - should this just be a failure instead?
		     */
		    depths[i].nvisuals = nvisuals;
		}
	    }
	}
    }

    *depthsPtr = depths;
    *visualsListPtr = visualsList;
    *ndepthsPtr = ndepths;
    return True;
}


Status _XGetPseudoRoot (dpy, propName)
    Display *dpy;			/* connection with *real* root */
    char *propName;			/* property name with info on it */
{
    Window w;				/* window from which to get info */
    PseudoScreen pscreen;		/* struct for getting data from prop */
    int ndepths;			/* result of fetching */
    PseudoDepth *depths = NULL;		/* for array of depth infos */
    PseudoVisual **visualsList = NULL;	/* one per depth */
    int i, j;				/* iterator, temp variable */
    Screen *sp;				/* for bashing display info */
    int screenno;			/* screen number used */
    Depth *newDepths = NULL;		/* new array */
    Status retval = False;		/* be pessimistic */
    int root_depth = 0;			/* depth of root visual */
    Visual *root_visual = NULL;		/* for setting screen field */
    int nd;				/* count of depths */
    Depth *d;				/* temp variable */

/*
 * The following grossness is to cope with an schain botch compiler error
 * in native 4.3bsd which has been fixed in 4.3+tahoe.  This is one of the
 * few times where a goto might be better....
 * Effective arglist is (retval,newDepths,depths,ndepths,visualsList)
 */
#define   FREE_RETURN()	\
    {									\
	if (newDepths) {						\
	    Depth *d;							\
									\
	    for (i = 0, d = newDepths; i < ndepths; i++, d++) {		\
		if (d->visuals) free ((char *) d->visuals);		\
	    }								\
	    free ((char *) newDepths);					\
	}								\
	if (depths) free ((char *) depths);				\
	if (visualsList) free ((char *) visualsList);			\
	return retval;							\
    }
/*enddef*/

    /*
     * This implementation builds shadow versions of the Depths and Visuals
     * data structures so that it can always back out and fail gracefully.
     */
    screenno = DefaultScreen (dpy);
    w = RootWindow (dpy, screenno);
    sp = ScreenOfDisplay (dpy, screenno);

    /*
     * fetch the relevant data off of the root window
     */
    if (!_XGetPseudoRootData (dpy, w, propName, &pscreen,
			      &depths, &visualsList, &ndepths)) {
	return False;
    }


    /*
     * create new Depths and Visuals lists if needed
     */
    if (pscreen.depths != None) {
	/* the following will be used to replace dpy->depths */
	newDepths = (Depth *) Xcalloc (ndepths, sizeof(Depth));
	if (!newDepths) {
	    return False;
	}

	/* run down list of depths building visual arrays */
	for (i = 0; i < ndepths; i++) {
	    Depth *d = newDepths + i;
	    PseudoDepth *pd = depths + i;

	    d->depth = pd->depth;
	    d->nvisuals = pd->nvisuals;
	    d->visuals = NULL;
	    /*
	     * if there is no PseudoVisual, then copy the real Visuals
	     */
	    if (pd->visuals == None) {
		/* scan list of real depths look for a match */
		for (j = 0; j < sp->ndepths; j++) {
		    Depth *realD = sp->depths + j;

		    if (realD->depth == d->depth) {
			d->nvisuals = realD->nvisuals;
			d->visuals = (Visual *) Xmalloc (d->nvisuals *
							 sizeof(Visual));
			if (!d->visuals) 
			    FREE_RETURN();
			bcopy ((char *) realD->visuals,
			       (char *) d->visuals,
			       d->nvisuals * sizeof(Visual));
		    }			/* end if - got real depth */
		}			/* end for - iter over real depths */
		if (!d->visuals) {
		    /* XXX isn't this really an error? */
		    d->nvisuals = 0;
		}
	    } else {
		/* 
		 * if there are PseudoVisuals then convert them
		 */
		d->visuals = (Visual *) Xmalloc (d->nvisuals * sizeof(Visual));
		if (!d->visuals) 
		    FREE_RETURN();
		for (j = 0; j < d->nvisuals; j++) {
		    Visual *v = d->visuals + j;
		    PseudoVisual *pv = visualsList[i] + j;

		    v->ext_data = NULL;
		    v->visualid = pv->visualid;
		    v->class = pv->class;
		    v->red_mask = pv->red_mask;
		    v->green_mask = pv->green_mask;
		    v->blue_mask = pv->blue_mask;
		    v->bits_per_rgb = pv->bits_per_rgb;
		    v->map_entries = pv->map_entries;
		}			/* end for - copy pseudo to real */
	    }				/* end if - make visuals for depth */
	}				/* end for - iter over pseudodepths */
    }					/* end if - if pseudodepths */
	
    /*
     * we now have a new Depth array with visuals hung off it; check to make
     * sure that it is okay; also, find root_depth
     */

    {
	if (pscreen.depths == None) {
	    nd = sp->ndepths;
	    d = sp->depths;
	} else {
	    nd = ndepths;
	    d = newDepths;
	}

	/* walk depths lists looking for root_visual */
	for (i = 0; i < nd; i++, d++) {
	    Visual *v = d->visuals;

	    for (j = 0; j < d->nvisuals; j++) {
		if (v[j].visualid == pscreen.root_visual) {
		    root_depth = d->depth;
		    root_visual = &v[j];
		    i = nd;	/* break */
		    break;
		}
	    }
	}
	if (root_depth == 0) {
	    FREE_RETURN();
	}
    }


    /*
     * we think everything is okay, so bash the appropriate structures
     */

    /* don't need to change sp->ext_data */
    /* don't need to change sp->display */

    /*
     * XXX - if the program managing the pseudo root doesn't catch 
     * ConfigureNotify events and reset the width and height fields,
     * applications will get very confused.
     */
    sp->root = pscreen.root;
    sp->width = pscreen.width;
    sp->height = pscreen.height;
    sp->mwidth = pscreen.mwidth;
    sp->mheight = pscreen.mheight;

    if (pscreen.depths != None) {
	Depth *d;

	for (i = 0, d = sp->depths; i < sp->ndepths; i++, d++) {
	    if (d->visuals) free ((char *) d->visuals);
	}
	free ((char *) sp->depths);
	sp->ndepths = ndepths;
	sp->depths = newDepths;
	newDepths = NULL;		/* so that we don't free it */
    }

    sp->root_depth = root_depth;
    sp->root_visual = root_visual;

    sp->cmap = pscreen.cmap;
    sp->white_pixel = pscreen.white_pixel;
    sp->black_pixel = pscreen.black_pixel;
    sp->max_maps = pscreen.max_maps;
    sp->min_maps = pscreen.min_maps;
    {
	XGCValues gcv;
	gcv.foreground = pscreen.black_pixel;
	gcv.background = pscreen.white_pixel;
	XChangeGC (dpy, sp->default_gc, GCForeground|GCBackground, &gcv);
    }

    sp->backing_store = pscreen.backing_store;
    sp->save_unders = pscreen.save_unders;
    {
	/*
	 * XXX - spec says to get this from pscreen.root_input_mask but that
	 * doesn't account for other applications that select on the root; if
	 * we don't query then nested applications that care will get confused.
	 */
	XWindowAttributes attr;
	if (XGetWindowAttributes (dpy, sp->root, &attr)) {
	    sp->root_input_mask = attr.all_event_masks;
	}
    }

    retval = True;

    FREE_RETURN();

#undef FREE_RETURN
}
