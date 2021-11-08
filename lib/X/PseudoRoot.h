/*
 * $XConsortium: PseudoRoot.h,v 1.6 88/10/22 10:15:23 jim Exp $
 *
 * Copyright 1988 Massachusetts Institute of Technology
 *
 *
 *				 W A R N I N G
 *
 * This is experimental code for implementing pseudo-root windows as specified
 * by the Inter-Client Communications Conventions Manual.  The structures that
 * it provides should be considered private to the MIT implementation of Xlib
 * and are SUBJECT TO CHANGE WITHOUT NOTICE.  They should not be incorporated
 * into any toolkits or applications.  When they change, no effort will be
 * made to provide backwards compatibility.
 *
 * Current questions about the implementation are marked by triple X's.
 *
 * If you think this all sounds severe, you're right.
 */

#ifndef _PSEUDOROOT_H_
#define _PSEUDOROOT_H_

typedef struct {
    Window root;		/* Root window id */
    long width, height;		/* width and height of screen */
    long mwidth, mheight;	/* width and height of screen in millimeters */
    int root_depth;		/* bits per pixel */
    VisualID root_visual;	/* root visual */
    Colormap cmap;		/* default color map */
    unsigned long white_pixel;	/* WhitePixel of pseudoscreen */
    unsigned long black_pixel;	/* BlackPixel of pseudoscreen */
    long max_maps, min_maps;	/* max and min color maps */
    long backing_store;		/* Never, WhenMapped, Always */
    Bool save_unders;		/* are save-unders supported on this screen */
    long root_input_mask;	/* XXX - initial root input mask */
    Atom depths;		/* None or property holding allowed depths */
} PseudoScreen;

typedef struct{
    long depth;			/* this depth (Z) of the depth */
    long nvisuals;		/* XXX - number of Visuals at this depth */
    Atom visuals;		/* None or property holding allowed visuals */
} PseudoDepth;

typedef struct {
    VisualID visualid;		/* visual id of this visual */
    long class;			/* visual class (PseudoColor, etc.) */
    unsigned long red_mask, green_mask, blue_mask;	/* mask values */
    long bits_per_rgb;		/* log base2 of distinct color values */
    long map_entries;		/* color map entries */
} PseudoVisual;

extern Status _XGetPseudoRoot(), _XGetPseudoRootData();

#define PSEUDO_SCREEN_TYPE "SCREEN"	/* XXX - steal generic name */
#define PSEUDO_DEPTHS_TYPE "DEPTHS"	/* XXX - steal generic name */
#define PSEUDO_VISUALS_TYPE "VISUALS"	/* XXX - steal generic name */
#define PSEUDO_SCREEN_FORMAT 32
#define PSEUDO_DEPTHS_FORMAT 32
#define PSEUDO_VISUALS_FORMAT 32




#ifdef NEED_PSEUDOROOT_PROTOCOL
/*
 * Network structures for pseudo-root support; store everything in server
 * natural order (so, if you are using machines of different architectures,
 * the different client will have to swap).  Everything is CARD32 since they
 * are all unsigned anyway and this way we can allow the user to do the 
 * byte swapping.
 *
 * Need <X11/Xmd.h>
 */

/*
 * If your C compiler rearranges fields within structures, you will
 * have to change the definition of protocol structure to be an array 
 * of unsigned longs to which you assign using macros.
 */
typedef struct {
    unsigned long root;
    unsigned long width, height;
    unsigned long mwidth, mheight;
    unsigned long root_depth;
    unsigned long root_visual;
    unsigned long cmap;
    unsigned long white_pixel;
    unsigned long black_pixel;
    unsigned long max_maps, min_maps;
    unsigned long backing_store;
    unsigned long save_unders;
    unsigned long root_input_mask;
    unsigned long depths;
} xPseudoScreen;

typedef struct{
    unsigned long depth;
    unsigned long nvisuals;
    unsigned long visuals;
    unsigned long pad;
} xPseudoDepth;

typedef struct {
    unsigned long visualid;
    unsigned long class;
    unsigned long red_mask, green_mask, blue_mask;
    unsigned long bits_per_rgb;
    unsigned long map_entries;
    unsigned long pad;
} xPseudoVisual;

#define sz_xPseudoScreen 64		/* 16 fields * 4 bytes */
#define sz_xPseudoDepth 16		/* 4 fields * 4 bytes */
#define sz_xPseudoVisual 32		/* 8 fields * 4 bytes */
#endif /* NEED_PSEUDOROOT_PROTOCOL */

#endif /* _PSEUDOROOT_H_ */
