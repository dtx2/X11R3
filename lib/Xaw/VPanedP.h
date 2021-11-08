/*
* $XConsortium: VPanedP.h,v 1.17 88/09/06 16:42:42 jim Exp $
*/


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

/*
 *  VPaned.h - Private definitions for VPaned widget
 *
 */

#ifndef _VPanedP_h
#define _VPanedP_h

#include <X11/VPaned.h>
#include <X11/Constraint.h>

/*********************************************************************
 *
 * VPaned Widget Private Data
 *
 *********************************************************************/

#define XtInheritSetMinMax	((void (*)())_XtInherit)
#define XtInheritRefigureMode	((void (*)())_XtInherit)

#define BORDERWIDTH          1 /* Size of borders between panes.  */
                               /* %%% Should not be a constant    */

/* New fields for the VPaned widget class record */
typedef struct _VPanedClassPart {
    void  (*SetMinMax)();	/* (Widget) child, (int) min, (int) max */
    void  (*RefigureMode)();	/* (Widget) vpaned, (Boolean) mode */
} VPanedClassPart;

/* Full Class record declaration */
typedef struct _VPanedClassRec {
    CoreClassPart       core_class;
    CompositeClassPart  composite_class;
    ConstraintClassPart constraint_class;
    VPanedClassPart     vpaned_class;
} VPanedClassRec;

extern VPanedClassRec vPanedClassRec;


/* VPaned constraint record */
typedef struct _VPanedConstraintsPart {
    int		position;	/* position location in VPaned */
    Position	dy;		/* Desired Location */
    Position	olddy;		/* The last value of dy. */
    Dimension	min;		/* Minimum height */
    Dimension	max;		/* Maximum height */
    Boolean	allow_resize;	/* TRUE iff child resize requests are ok */
    Boolean	skip_adjust;	/* TRUE iff child's height should not be */
				/* changed without explicit user action. */
    Dimension	dheight;	/* User's desired height */
    Dimension	wp_height;	/* widget's preferred height */
    Widget	grip;		/* The grip for this child */
} VPanedConstraintsPart, *Pane;

typedef struct _VPanedConstraintsRec {
    VPanedConstraintsPart vpaned;
} VPanedConstraintsRec, *VPanedConstraints;

/* New Fields for the VPaned widget record */
typedef struct {
    /* resources */
    Pixel       foreground_pixel;          /* window foreground */
    Position    grip_indent;               /* Location of grips (offset	*/
                                           /*   from right margin)	*/
    Boolean     refiguremode;              /* Whether to refigure changes right now */
    XtTranslations grip_translations;      /* grip translation table */
    Cursor	grip_cursor;               /* inactive grip cursor */
    Cursor	adjust_upper_cursor;       /* active grip cursor: U */
    Cursor	adjust_this_cursor;        /* active grip cursor: T */
    Cursor	adjust_lower_cursor;       /* active grip cursor: D */
    /* private */
    Boolean	recursively_called;        /* for ChangeManaged */
    int         starty;                    /* mouse origin when adjusting */
    Pane        whichadd;                  /* Which pane to add changes to */
    Pane        whichsub;                  /* Which pane to sub changes from */
    GC          normgc;                    /* GC to use when drawing borders */
    GC          invgc;                     /* GC to use when erasing borders */
    GC          flipgc;                    /* GC to use when animating borders */
    int		num_panes;                 /* count of managed panes */
} VPanedPart;

/**************************************************************************
 *
 * Full instance record declaration
 *
 **************************************************************************/

typedef struct _VPanedRec {
    CorePart       core;
    CompositePart  composite;
    ConstraintPart constraint;
    VPanedPart     v_paned;
} VPanedRec;

#endif _VPanedP_h
/* DON'T ADD STUFF AFTER THIS #endif */
