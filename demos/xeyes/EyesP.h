/*
* $XConsortium: EyesP.h,v 1.4 88/09/06 17:55:28 jim Exp $
*/

#ifndef _EyesP_h
#define _EyesP_h

#include "Eyes.h"
#include <X11/CoreP.h>

#define SEG_BUFF_SIZE		128

/* New fields for the eyes widget instance record */
typedef struct {
	 Pixel		puppixel;	/* foreground pixel */
	 Pixel		outline;	/* outline pixel */
	 Pixel		center;		/* inside pixel */
	 GC		outGC;		/* pointer to GraphicsContext */
	 GC		pupGC;		/* pointer to GraphicsContext */
	 GC		centerGC;	/* pointer to GraphicsContext */
/* start of graph stuff */
	 int		backing_store;	/* backing store variety */
	 Boolean	reverse_video;	/* swap fg and bg pixels */
	 Boolean	use_wide_lines;	/* use wide lines instead of fills */
	 Boolean	use_bevel;	/* use bevel join style for arcs */
	 int		update;		/* current timeout index */
	 int		thickness;	/* line thickness */
	 int		odx, ody;	/* old mouse position */
	 XPoint		pupil[2];	/* pupil position */
	 XtIntervalId	interval_id;
   } EyesPart;

/* Full instance record declaration */
typedef struct _EyesRec {
   CorePart core;
   EyesPart eyes;
   } EyesRec;

/* New fields for the Eyes widget class record */
typedef struct {int dummy;} EyesClassPart;

/* Full class record declaration. */
typedef struct _EyesClassRec {
   CoreClassPart core_class;
   EyesClassPart eyes_class;
   } EyesClassRec;

/* Class pointer. */
extern EyesClassRec eyesClassRec;

#endif _EyesP_h
