/*-
 * mibstore.h --
 *	Header file for users of the MI backing-store scheme.
 *
 * Copyright (c) 1987 by the Regents of the University of California
 *
 * Permission to use, copy, modify, and distribute this
 * software and its documentation for any purpose and without
 * fee is hereby granted, provided that the above copyright
 * notice appear in all copies.  The University of California
 * makes no representations about the suitability of this
 * software for any purpose.  It is provided "as is" without
 * express or implied warranty.
 *
 *	"$XConsortium: mibstore.h,v 1.3 88/10/21 18:01:18 keith Exp $ SPRITE (Berkeley)"
 */

#ifndef _MIBSTORE_H
#define _MIBSTORE_H

/*
 * Contents of window devBackingStore field.
 */
typedef struct {
    GCPtr   	  pgcBlt;   	    /* GC for drawing onto screen */
    PixmapPtr	  pBackingPixmap;   /* Pixmap for saved areas */
    RegionPtr	  pSavedRegion;	    /* Valid area in pBackingPixmap */
    Bool    	  viewable; 	    /* Tracks pWin->viewable so pSavedRegion may
				     * be initialized correctly when the window
				     * is first mapped */
    int    	  status;    	    /* StatusNoPixmap, etc. */
    PixmapPtr backgroundTile;
    unsigned long backgroundPixel;

    void    	  (*SaveAreas)();   /* Device-dependent function to actually
				     * save the obscured areas */
    void    	  (*RestoreAreas)();/* Device-dependent function to actually
				     * restore exposed areas */
    void    	  (*SetClipmaskRgn)();/* Device-dependent function to set
				     * BackingStore GC clipmask region */
} MIBackingStoreRec, *MIBackingStorePtr;

#define StatusNoPixmap	1	/* pixmap has not been created */
#define StatusVirtual	2	/* pixmap is virtual, tiled with background */
#define StatusVDirty	3	/* pixmap is virtual, visiblt has contents */
#define StatusExists	4	/* pixmap is created, no valid contents */
#define StatusContents	5	/* pixmap is created, has valid contents */

#define MIBS_FILLSPANS        	(1L<<0) 
#define MIBS_SETSPANS       	(1L<<1) 
#define MIBS_PUTIMAGE       	(1L<<2) 
#define MIBS_COPYAREA       	(1L<<3) 
#define MIBS_COPYPLANE      	(1L<<4) 
#define MIBS_POLYPOINT      	(1L<<5) 
#define MIBS_POLYLINES      	(1L<<6) 
#define MIBS_POLYSEGMENT        (1L<<7) 
#define MIBS_POLYRECTANGLE      (1L<<8) 
#define MIBS_POLYARC            (1L<<9) 
#define MIBS_FILLPOLYGON        (1L<<10)
#define MIBS_POLYFILLRECT       (1L<<11)
#define MIBS_POLYFILLARC        (1L<<12)
#define MIBS_POLYTEXT8      	(1L<<13)
#define MIBS_POLYTEXT16     	(1L<<14)
#define MIBS_IMAGETEXT8     	(1L<<15)
#define MIBS_IMAGETEXT16        (1L<<16)
#define MIBS_IMAGEGLYPHBLT      (1L<<17)
#define MIBS_POLYGLYPHBLT       (1L<<18)
#define MIBS_PUSHPIXELS     	(1L<<19)
#define MIBS_ALLPROCS       	((1L<<20)-1)

extern void miInitBackingStore();
extern void miFreeBackingStore();
extern void miValidateBackingStore();
extern void miBSGetImage();
extern void miBSGetSpans();

#endif _MIBSTORE_H
