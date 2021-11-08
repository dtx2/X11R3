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

#include <errno.h>
#include <sys/types.h>
#include <stdio.h>		/* for debugging */

#include "X.h"
#include	"gcstruct.h"
#include "windowstr.h"

#include "Ultrix2.0inc.h"
#include "qduser.h"
#include "qdreg.h"
#include	"qd.h"
#include "qdioctl.h"  /* XXX - for bell-style debugging */
#include "tl.h"
#include "tlsg.h"
#include "tltemplabels.h"

#define PIXELSIZE (NPLANES/8)	/* per-pixel image stepping */
# define JUSTGREEN(x) GREEN(x)

extern int errno;
extern int Vaxstar;
extern	int	Nplanes;
extern	int	Nchannels;
extern unsigned int Allplanes;

/*
 *  tlputimage
 *
 *  put a rectangle of Z-mode data into the framebuffer.
 */
int
tlputimage( pwin, pgc, x, y, w, h, pcolor)
    WindowPtr		pwin;
    GCPtr		pgc;
    register int	x, y, w, h;
    register unsigned char *pcolor;
{
    register struct adder * adder = Adder;

    RegionPtr pSaveGCclip = ((QDPrivGCPtr)pgc->devPriv)->pCompositeClip;
    register BoxPtr pclip = pSaveGCclip->rects;	/* step through GC clip */
    register int nclip	  = pSaveGCclip->numRects;	/* number of clips */

    INVALID_SHADOW;	/* XXX */
    SETTRANSLATEPOINT( pwin->absCorner.x, pwin->absCorner.y);
    preptb(pgc->planemask, 0, 0, 1024, 864, umtable[pgc->alu]);
    for ( ; nclip > 0; nclip--, pclip++)
    {
        register int tx, ty, tw, th, iy;
	register unsigned char *tcolor;	/* this stream of image data */
	
	tx = max(pclip->x1 - pwin->absCorner.x,x);
        tw = min(pclip->x2 - pwin->absCorner.x,x+w) - tx;
	ty = max(pclip->y1 - pwin->absCorner.y,y);
        th = min(pclip->y2 - pwin->absCorner.y,y+h) - ty;
	if (tw < 1 || th < 1)
	    continue;		/* nothing to draw in this clip rectangle */
	tcolor = pcolor + ( (ty - y) * w + tx - x );
	ptbspa( tx, ty, tw, th, w, tcolor,
	       DST_INDEX_ENABLE|DST_WRITE_ENABLE|NORMAL ); 
    }
}

tlSaveAreas (pPixmap, prgnSave, xorg, yorg)
    PixmapPtr	  	pPixmap;  	/* Backing pixmap */
    RegionPtr	  	prgnSave; 	/* Region to save (pixmap-relative) */
    int	    	  	xorg;	    	/* X origin of region */
    int	    	  	yorg;	    	/* Y origin of region */
{
    register struct adder *	adder = Adder;	/* must be in a register */
    register u_short		*p;
    register BoxPtr pclip = prgnSave->rects;		/* step through GC clip */
    register int nclip	  = prgnSave->numRects;	/* number of clips */

    INVALID_SHADOW;	/* XXX */
    SETTRANSLATEPOINT( xorg, yorg );
    shutengine(Allplanes);
    write_ID( adder, DST_OCR_A, EXT_NONE|INT_NONE|NO_ID|NO_WAIT); 
    write_ID( adder, LU_FUNCTION_R3, FULL_SRC_RESOLUTION | LF_ONES);

    for ( ; nclip > 0; nclip--, pclip++)
    {
        register int tx, ty, tw, th, iy;
	register unsigned char *tcolor;	/* this stream of image data */
	
	tx = pclip->x1;
        tw = min(pclip->x2, 1024) - tx;
	ty = pclip->y1;
        th = min(pclip->y2, 864) - ty;
	if (tw < 1 || th < 1)
	    continue;		/* nothing to draw in this clip rectangle */
	tcolor = ((QDPixPtr)pPixmap->devPrivate)->data + ( ty * pPixmap->width + tx );
	btpspa (tx, ty, tw, th, pPixmap->width, tcolor,
	       SRC_1_INDEX_ENABLE|NORMAL ); 
    }
    dmafxns.enable();
}

tlRestoreAreas (pPixmap, prgnRestore, xorg, yorg)
    PixmapPtr	  	pPixmap;  	/* Backing pixmap */
    RegionPtr	  	prgnRestore; 	/* Region to restore (screen-relative)*/
    int	    	  	xorg;	    	/* X origin of window */
    int	    	  	yorg;	    	/* Y origin of window */
{
    register struct adder * adder = Adder;

    register BoxPtr pclip = prgnRestore->rects;		/* step through GC clip */
    register int nclip	  = prgnRestore->numRects;	/* number of clips */
    int	pixmapWidth;

    INVALID_SHADOW;	/* XXX */
    SETTRANSLATEPOINT( xorg, yorg );
    preptb(Allplanes, 0, 0, 1024, 864,
    		LF_S | FULL_SRC_RESOLUTION); /* source */

    pixmapWidth = pPixmap->width * (NPLANES/8);
    for ( ; nclip > 0; nclip--, pclip++)
    {
        register int tx, ty, tw, th, iy;
	register unsigned char *tcolor;	/* this stream of image data */
	
	tx = max(pclip->x1 - xorg, 0);
        tw = min(pclip->x2 - xorg, 1024) - tx;
	ty = max(pclip->y1 - yorg, 0);
        th = min(pclip->y2 - yorg, 864) - ty;
	if (tw < 1 || th < 1)
	    continue;		/* nothing to draw in this clip rectangle */
	tcolor = ((QDPixPtr)pPixmap->devPrivate)->data + ( ty * pixmapWidth + tx );
	ptbspa( tx, ty, tw, th, pixmapWidth, tcolor,
	       DST_INDEX_ENABLE|DST_WRITE_ENABLE|NORMAL ); 
    }
}

/* send all red/a-g/a-b formatted data to screen.  no clipping, no trans */
tlspadump( x, y, w, h, ppixel )
int		  x, y, w, h;
unsigned char	* ppixel;
{
    register struct adder *     adder = Adder;

    INVALID_SHADOW;	/* XXX */
    preptb(Allplanes, 0, 0, 1024, 2048,
	   LF_S | FULL_SRC_RESOLUTION); /* source */
    ptbspa( x, y, w, h, w, ppixel, DST_WRITE_ENABLE|NORMAL );
}

/* XXX - is src_1 supposed to be translated? */

void
tlspaca( ppix, pwin, pgc, srcx, srcy, w, h, dstx, dsty )
PixmapPtr	ppix;
WindowPtr	pwin;
GCPtr		pgc;
int		srcx, srcy, w, h, dstx, dsty;
{
    register struct adder *     adder = Adder;

    RegionPtr pSaveGCclip = ((QDPrivGCPtr)pgc->devPriv)->pCompositeClip;
    register BoxPtr pclip = pSaveGCclip->rects;	/* step through GC clip */
    register int nclip	  = pSaveGCclip->numRects;	/* number of clips */
    int		igreen = ppix->width * ppix->height;
    register int	iy;	/* index of current inc past dsty */
    int		pixoffy;	/* y address of offscreen pixmap */
    int		x1clip, y1clip, x2clip, y2clip;
    /* data points to first byte (in red block) */

    INVALID_SHADOW;	/* XXX */

    /* put pixmap offscreen and do bitblt 
       remember to nuke the copy if the original gets drawn to
    */
    if (ppix->width <= 1024 && ppix->height <= DragonPix)
    {
	if (! tlConfirmPixmap( ppix->devPrivate, ppix, &pixoffy ))
	    FatalError( "tlspaca: could not store pixmap off-screen\n");
	tlbitblt( pgc, dstx+pwin->absCorner.x, dsty+pwin->absCorner.y,
		  w, h, srcx, srcy+pixoffy);
	return;
    }

    /*
     * The X toolkit passes in outrageously big rectangles to be blitted,
     * using clipping to restrict the blit to a reasonable size.
     * This is slow on the adder, which generates all the clipped-out
     * pixel addresses, so we soft clip the rectangle.
     */
#ifdef NOTDEF
    x1clip = max( dstx, pSaveGCclip->extents.x1 - pwin->absCorner.x);
    y1clip = max( dsty, pSaveGCclip->extents.y1 - pwin->absCorner.y);
    x2clip = min( dstx+w, pSaveGCclip->extents.x2 - pwin->absCorner.x);
    y2clip = min( dsty+h, pSaveGCclip->extents.y2 - pwin->absCorner.y);

    srcx += x1clip-dstx;
    srcy += y1clip-dsty;
    dstx = x1clip;
    dsty = y1clip;
    w = x2clip-x1clip;
    h = y2clip-y1clip;
#endif

    SETTRANSLATEPOINT( pwin->absCorner.x, pwin->absCorner.y);

    for ( ; nclip > 0; nclip--, pclip++)
    {
	register unsigned char * data = ((QDPixPtr) ppix->devPrivate)->data
					+ srcy * ppix->width + srcx;
	preptb(pgc->planemask, pclip->x1, pclip->y1, pclip->x2, pclip->y2,
		umtable[pgc->alu]);
	/* planes are broken here ! - XXX */
	ptbspa( dstx, dsty, w, h, ppix->width, data,
	       DST_WRITE_ENABLE | DST_INDEX_ENABLE | NORMAL );
    }
}

/* This is extremely gross.  Why is this necessary?  */
static
shutengine(planemask)
unsigned long   planemask;
{
    register unsigned short	*p;
    register struct adder *adder = Adder;

    /*
     * system hangs in following WAITDMADONE, 
     * before first xterm comes up, if these three lines are
     * omitted: WHY?            XX
     */
    Need_dma(8);
    *p++ = JMPT_SETPLANEMASK;
    *p++ = GREEN(planemask);
    *p++ = JMPT_SETFOREBACKCOLOR;
    *p++ = 0xff;
    *p++ = 0x00;
    *p++ = JMPT_SETMASK;
    *p++ = 0xffff;
    *p++ = TEMPLATE_DONE;
    Confirm_dma ();

    dmafxns.flush ( TRUE);      /* spin until all dragon FIFOs are empty */
#ifdef undef
    write_ID(adder, CS_UPDATE_MASK, GREEN(planemask));
    write_ID( adder, FOREGROUND_COLOR, 0xffff );
    write_ID( adder, BACKGROUND_COLOR, 0x0000 );
    write_ID( adder, MASK_1, 0xffff );    /* necessary??? */
#endif
}

/***************************************************************
 * setuptransfer - this does some basic setup for ptb or btp's. *****
 * This is designed to work for stepping or contiguous transfers.    *****
 * It is hoped that in the future this will honor the soft dragon shadow. *
 * Pre Setup:  clear running fifo or dma packets                         *
 * Setup:  setup x, y, w, h in dragon source and destination registers  *
 *         planemasks as given                                         *
 *         initialize dragon fg, bg, and mask registers to sane values *
 * Not Setup:  rasterop_mode                                        ***
 *             -> if hardware-translating then                ******
 *              * hardware translate point              ******
 *             -> if writing-enabled then         ******
 *              * clipping                  ******
 *              * alu                 ******
 *             command register ******
 *******************************/
static
setuptransfer(x, y, w, h, mode)
    int		x, y, w, h, mode;
{
    register struct adder	*adder = Adder;

    poll_status( adder, RASTEROP_COMPLETE );
    poll_status( adder, ADDRESS_COMPLETE );
    poll_status( adder, TX_READY );

    adder->source_1_dx = 1;	/* positive horizontal scanning */
    adder->source_1_dy = 1;	/* positive vertical scanning */
    adder->destination_x = x;
    adder->destination_y = y;
    adder->fast_dest_dx = w;
    adder->fast_dest_dy = 0;
    adder->slow_dest_dx = 0;
    adder->slow_dest_dy = h;

    /* Set up viper registers for PTBZ transfer.
     * Actual data flow is observed to be very different from that
     * described in manual:  bytes from the DMA chip go into the viper SOURCE
     * register rather than FOREGROUND or BACKGROUND.
     */

    adder->rasterop_mode = mode;
}

/* ptbspa - processor to bitmap (aka set pixel array)             *
 *   Pre Inited:  source, planes, masks, r_mode, clip, alu, trans *
 * Dump pixels on the screen.  This either uses the PTBZ command  *
 *   and slow adder polling, or if possible, it does a direct     *
 *   dma-mapped write (for the gpx) or fifo-writes (for the star) *
 */
static
preptb(planemask, x1, y1, x2, y2, alu)
unsigned long   planemask;
short x1, y1, x2, y2, alu;
{
    register unsigned short	*p;
    register struct adder *adder = Adder;

    /*
     * system hangs in following WAITDMADONE, 
     * before first xterm comes up, if these three lines are
     * omitted: WHY?            XX
     */
    Need_dma(15);
    *p++ = JMPT_SETPLANEMASK;
    *p++ = GREEN(planemask);
    *p++ = JMPT_SETFOREBACKCOLOR;
    *p++ = 0xff;
    *p++ = 0x00;
    *p++ = JMPT_SETMASK;
    *p++ = 0xffff;
    *p++ = JMPT_SETCLIP;
    *p++ = x1;
    *p++ = x2;
    *p++ = y1;
    *p++ = y2;
    *p++ = JMPT_SETALU;
    *p++ = alu;
    *p++ = TEMPLATE_DONE;
    Confirm_dma ();
}

static Bool
startptb(w, h, mode)
    unsigned int w, h, mode;
{
    register struct adder *     adder = Adder;
    register unsigned short *p;
    register in_template =
	( w * h <= min(8192,req_buf_size/2) ) ? TRUE : FALSE;
    
    Need_dma( 3 );
    *p++ = JMPT_INITZBLOCKPTOB;	/* because this polls correctly. */
    *p++ = JMPT_SETRASTERMODE;
    *p++ = mode;
    Confirm_dma();
    return( in_template );
}

static
ptbspa( x, y, w, h, realw, ppixel, mode )
    int x, y, w, h;
    int realw;
    unsigned char *ppixel;
    int mode;
{
    register unsigned short *p;
    register struct adder *adder = Adder;

    if ( startptb( w, h, mode) ) /* if in template */
	ptbtemplate( x, y, w, h, realw, w * h, ppixel );
    else
	ptbpackets( x, y, w, h, realw, (w * h + 1) / 2, ppixel );
    Need_dma( 1 );
    *p++ = TEMPLATE_DONE;
    Confirm_dma();
}

static
ptbpackets( x, y, w, h, realw, nwords, ppixel )
    int x, y;
    unsigned w, h, realw;
    register int nwords;
    unsigned char *ppixel;
{
    register unsigned short *p;
    register int cwords;	/* total words in this packet */
    register int scaninc = 0;	/* current offset from start of this scan */
    register int bytesleft;	/* bytes left in this packet (<= cwords*2) */
    register int scanbytes;	/* number of bytes to copy off this scan */
    unsigned char lastbyte;
    register unsigned char *dma_byte;
    extern struct DMAreq *DMArequest;
    extern short sg_fifo_func;

    Need_dma( 6 );
    *p++ = JMPT_PTOB;	/* this does not 'loadd' the command reg. */
    *p++ = x;
    *p++ = y;
    *p++ = w;
    *p++ = h;
    *p++ = TEMPLATE_DONE;
    Confirm_dma();
    dmafxns.flush( TRUE );	/* just to ream out the template packets */
    Adder->command = PTBZ;	/* must be here or a_comp never flushed */
    while (nwords > 0)
    {
	/* We must now assemble the packets.  Internally, the packets must
	 * be accurately copied to the precision of one byte.  The final byte
	 * must be word-padded (the fifo dma engine only recognizes lengths of
	 * shorts -- sorry).  The restrictions are:  packets cannot exceeed
	 * req_buf_size in length, scanlines may be discontiguous, and of
	 * course we eventually hit a tricky final packet length.  The basic
	 * model is to ask for req_buf_size words, fill it up scanline at a
	 * time or in one big chunk, and ask for more when we run out, making
	 * sure to ask for a minimal amount on the last packet.  */
	bytesleft = 2 * (cwords = min( nwords, req_buf_size ));
	General_dma( cwords, FIFO_EMPTY | BYTE_PACK );
	dma_byte = (unsigned char *) dma_word;
	if (!Vaxstar)
	    DMArequest->DMAtype = PTOB;
	else {
	    sg_fifo_type = PTOB;
	    sg_fifo_func = PTB_UNPACK_ENB;
	}
	if (w == realw) {
	    bcopy( ppixel, dma_byte, bytesleft ); /* movec the data */
	    ppixel += bytesleft;
	} else
	    while (bytesleft > 0) {
		scanbytes = min( w - scaninc, bytesleft );
		if (((int) dma_byte) & 1) {
		    bcopy( ppixel + 1, dma_byte + 1, scanbytes - 1 );
		    *((unsigned short *) (dma_byte - 1)) =
			lastbyte + (((unsigned short) *ppixel) << 8);
		} else {
		    if (scanbytes & ((int) 1)) {
			bcopy( ppixel, dma_byte, scanbytes - 1 );
			lastbyte = ppixel[scanbytes - 1];
		    } else
			bcopy( ppixel, dma_byte, scanbytes );
		}
		scaninc = (scaninc + scanbytes) % w;
		bytesleft -= scanbytes;
		dma_byte += scanbytes;
		if (scaninc)
		    ppixel += scanbytes;
		else
		    ppixel += scanbytes + realw - w;
	    }
	if (scanbytes & ((int) 1) && ((int) dma_byte) & 1)
	    *((unsigned short *) (dma_byte - 1)) = lastbyte;
	dma_word += cwords;
	nwords -= cwords;
    }
    dmafxns.flush( TRUE );	/* get rid of ptb packets */
}

ptbtemplate( x, y, w, h, realw, npixels, ppixel )
    register unsigned int x, y, w, h, realw;
    register unsigned int npixels;
    register unsigned char *ppixel;
{
    register unsigned short *p;
    register int scanpixels, cpixels;
    
    Need_dma( npixels + 6 );	/* better be <= req_buf_size */
    *p++ = JMPT_ZBLOCKPTOB;
    *p++ = x;
    *p++ = y;
    *p++ = w;
    *p++ = h;
    *p++ = 0x6000 | (0x1fff & -npixels); /* must be in same packet */
    while (npixels) {
	scanpixels = w;
	while (scanpixels--)
	    *p++ = *ppixel++;
	npixels -= w;
	ppixel += realw - w;
    }
    Confirm_dma();
}

/*
 * Transfer a rectangular full-depth image to the frame buffer.
 * Will not work if the rectangle is more than 8K pixels.
 * Or if the entire packet size is larger than maximum packet size.
 *
 *  the dma buffer has
 *  JMPT_SETVIPER24
 *  rop | FULL_SRC_RESOLUTION
 *  JMPT_INITPTOBZ
 *  x, y, width, height
 *  PTBZ | count
 *  pixvals
 *  DONE
 */
ptbspatl( x, y, width, height, ppix, zblock, step )
    int         x,y;
    register int width;
    register int height;
    register unsigned char *ppix;
    int         zblock;         /* code indicating which block of planes */
    int         step;		/* per-pixel offset to next pixel */
{
    register unsigned short *p;
    register int npix;

    INVALID_SHADOW;	/* XXX */
    SETTRANSLATEPOINT(0, 0);

    npix = width * height;

    Need_dma( 7);
    *p++ = JMPT_SETRGBPLANEMASK;
    *p++ = (zblock == ZRED) ? 0xff : 0;
    *p++ = (zblock == ZGREEN) ? 0xff : 0;
    *p++ = (zblock == ZBLUE) ? 0xff : 0;
    *p++ = JMPT_SETVIPER24;     /* sets foreground and background registers */
    *p++ = umtable[ GXcopy] | FULL_SRC_RESOLUTION;
    *p++ = JMPT_RESETCLIP;
    Confirm_dma ();

    Need_dma( 6 );
    *p++ = JMPT_INITZBLOCKPTOB;
    *p++ = x & 0x3fff;
    *p++ = y & 0x3fff;
    *p++ = width & 0x3fff;
    *p++ = height & 0x3fff;
    /* DGA magic bit pattern for PTB */
    *p++ = 0x6000 |(0x1fff & -npix);
    Confirm_dma ();

    Need_dma( npix );
    while (npix--)              /* unpack pixels into shorts */
        *p++ = *ppix++;
    Confirm_dma ();

    Need_dma( 1 );
    *p++ = TEMPLATE_DONE;
    Confirm_dma ();
}

/*
 *  tlgetspan
 *
 *  written by Brian Kelleher; June 1986
 *  severely hacked by matt.
 *
 *  Get a span of Z-mode data from the framebuffer.
 */
int
tlgetspan( pwin, x, y, w, pcolor)
    WindowPtr		pwin;
    register int	x, y, w;
    register unsigned char *pcolor;
{
    register struct adder *	adder = Adder;	/* must be in a register */

 
    /*
     *  Wait for the DMA buffer to empty, and then for the fifo
     *  maintained by the gate array to empty.
     */
    dmafxns.flush ( TRUE);

    write_ID( adder, CS_UPDATE_MASK, 0xffff);
    write_ID( adder, DST_OCR_A, EXT_NONE|INT_NONE|NO_ID|NO_WAIT); 
    write_ID( adder, LU_FUNCTION_R3, FULL_SRC_RESOLUTION | LF_ONES);
    write_ID( adder, MASK_1, 0xffff);

    adder->source_1_dy = 1;
    adder->slow_dest_dx = 0;
    adder->slow_dest_dy = 1;
    adder->fast_dest_dx = w;
    adder->fast_dest_dy = 0;
    adder->error_1 = 0;
    adder->error_2 = 0;

    adder->rasterop_mode = (NORMAL);	/* no src_index'ing */

    btpzblock( x, y, w, 1, 0xff, pcolor, ZGREEN);
    dmafxns.enable();
}

/*
 *  tlgetimage
 *
 *  Get a rectangle of Z-mode data from the framebuffer.
 */
int
tlgetimage( pwin, x, y, w, h, planemask, pcolor)
    WindowPtr		pwin;
    register int	x, y, w, h;
    unsigned long	planemask;
    register unsigned char *pcolor;
{
    register struct adder *	adder = Adder;	/* must be in a register */
    register u_short		*p;

    INVALID_SHADOW;	/* XXX */
    shutengine(planemask);
    write_ID( adder, DST_OCR_A, EXT_NONE|INT_NONE|NO_ID|NO_WAIT); 
    write_ID( adder, LU_FUNCTION_R3, FULL_SRC_RESOLUTION | LF_ONES);

    x += pwin->absCorner.x;
    y += pwin->absCorner.y;

    setuptransfer(x, y, w, h, NORMAL); /* no source indexing, (negatives) */
    btpzblock( x, y, w, h, GREEN(planemask), pcolor, ZGREEN);
    dmafxns.enable();
}

/*
 *  Get RGB data from the framebuffer.
 */
static
btpzblock( x, y, w, h, planebyte, pcolor, zblock)
    int				x, y;
    register int		w, h;
    register unsigned char	planebyte;
    register unsigned char *	pcolor;
    int				zblock;
{
    register int		width;
    register struct adder *	adder = Adder;	/* must be in a register */

    adder->source_1_x = x;
    adder->source_1_y = y;
    adder->source_1_dx = 1;
    adder->command = (BTPZ | zblock);
    adder->request_enable = RX_READY;
    while (h--)
    {
	for (width = w; width > 0; width--)
	{
	    poll_status( adder, RX_READY);
	    *pcolor = (u_char)adder->id_data & planebyte;
	    pcolor = pcolor + PIXELSIZE;
	}
    }
    poll_status( adder, ADDRESS_COMPLETE); 
}

static
btpspa ( x, y, w, h, realw, ppixel, mode )
    int x, y, w, h;
    int realw;
    unsigned char *ppixel;
    int mode;
{
    register int		width;
    register struct adder *	adder = Adder;	/* must be in a register */
    register unsigned char	*pcolor;

    adder->source_1_x = x;
    adder->source_1_y = y;
    adder->source_1_dx = 1;
    adder->source_1_dy = 1;
    adder->fast_dest_dx = w;
    adder->fast_dest_dy = 0;
    adder->slow_dest_dx = 0;
    adder->slow_dest_dy = h;
    adder->rasterop_mode = mode;
    adder->command = (BTPZ);
    adder->request_enable = RX_READY;
    while (h--)
    {
	pcolor = ppixel;
	for (width = w; width > 0; width--)
	{
	    poll_status( adder, RX_READY);
	    *pcolor = (u_char)adder->id_data;
	    pcolor = pcolor + PIXELSIZE;
	}
	ppixel += realw * PIXELSIZE;
    }
    poll_status( adder, ADDRESS_COMPLETE); 
}

#ifdef notdef
/*
 * this code is incomplete -- it is supposed to
 * transfer data rapidly from dragon to vax.
 */

static
btpspa ( x, y, w, h, realw, ppixel, mode )
    int x, y, w, h;
    int realw;
    unsigned char *ppixel;
    int mode;
{
    (void) startptb( w, h, mode );
    btppackets( x, y, w, h, realw, (w * h + 1) / 2, ppixel);
    Need_dma( 1 );
    *p++ = TEMPLATE_DONE;
    Confirm_dma();
}

static
btppackets( x, y, w, h, realw, nwords, ppixel )
    int unsigned x, y, w, h, realw;
    register int nwords;
    unsigned char *ppixel;
{
    register unsigned short *p;
    register int cwords;	/* total words in this packet */
    register int scaninc = 0;	/* current offset from start of this scan */
    register int bytesleft;	/* bytes left in this packet (<= cwords*2) */
    register int scanbytes;	/* number of bytes to copy off this scan */
    unsigned char lastbyte;
    register unsigned char *dma_byte;
    extern struct DMAreq *DMArequest;
    extern short sg_fifo_func;

    Need_dma( 6 );
    *p++ = JMPT_PTOB;	/* this does not 'loadd' the command reg. */
    *p++ = x;
    *p++ = y;
    *p++ = w;
    *p++ = h;
    *p++ = TEMPLATE_DONE;
    Confirm_dma();
    dmafxns.flush( TRUE );	/* just to ream out the template packets */
    Adder->command = BTPZ;	/* must be here or a_comp never flushed */
    while (nwords > 0)
    {
	/* We must now assemble the packets.  Internally, the packets must
	 * be accurately copied to the precision of one byte.  The final byte
	 * must be word-padded (the fifo dma engine only recognizes lengths of
	 * shorts -- sorry).  The restrictions are:  packets cannot exceeed
	 * req_buf_size in length, scanlines may be discontiguous, and of
	 * course we eventually hit a tricky final packet length.  The basic
	 * model is to ask for req_buf_size words, fill it up scanline at a
	 * time or in one big chunk, and ask for more when we run out, making
	 * sure to ask for a minimal amount on the last packet.  */
	bytesleft = 2 * (cwords = min( nwords, req_buf_size ));
	General_dma( cwords, FIFO_EMPTY | BYTE_PACK );
	dma_byte = (unsigned char *) dma_word;
	if (!Vaxstar)
	    DMArequest->DMAtype = BTOP;
	else {
	    sg_fifo_type = BTOP;
	    sg_fifo_func = BTP_UNPACK_ENB;
	}
	/*	dmafxns.flush( TRUE ); XXX almost... */
	if (w == realw) {
	    bcopy( dma_byte, ppixel, bytesleft ); /* movec the data */
	    ppixel += bytesleft;
	} else
	    while (bytesleft > 0) {
		scanbytes = min( w - scaninc, bytesleft );
		if (((int) dma_byte) & 1) {
		    bcopy( dma_byte + 1, ppixel + 1, scanbytes - 1 );
		    *((unsigned short *) (ppixel - 1)) =
			lastbyte + (*((unsigned short *) dma_byte) << 8);
		} else {
		    if (scanbytes & ((int) 1)) {
			bcopy( dma_byte, ppixel, scanbytes - 1 );
			lastbyte = (unsigned char)
			 (*((unsigned short *) (dma_byte + (scanbytes - 1))));
		    } else
			bcopy( dma_byte, ppixel, scanbytes );
		}
		scaninc = (scaninc + scanbytes) % w;
		bytesleft -= scanbytes;
		dma_byte += scanbytes;
		if (scaninc)
		    ppixel += scanbytes;
		else
		    ppixel += scanbytes + realw - w;
	    }
	if (scanbytes & ((int) 1) && ((int) dma_byte) & 1)
	    *((unsigned short *) (ppixel - 1)) = lastbyte;
	dma_word += cwords;
	nwords -= cwords;
    }
}
#endif

#ifdef undef			/* random bits of old code */
    register int		width;
    
    while (h--)
    {
	for (width = w; width > 0; width--)
	{
	    poll_status( adder, TX_READY);
	    adder->id_data = *ppixel;
	    ppixel += step;
	}
    }
    while (dga->bytcnt_hi & 0xff || dga->bytcnt_lo)
	;
    /* WAITBYTCNTZERO(dga); */
    WAITDMADONE(dga);       /* mode bit 1 is NOT TO BE TOUCHED  until
			       fifo is empty */
} else {			/* either vaxStar or non-contiguous gpx */
    register struct fcc *sgfcc = fcc_cbcsr;
    register short nwords = (w * h + 1) / 2;
    register short cwords;
    extern short *sg_int_flag;
    extern short *change_section;
    while (nwords > 0)
      {
	  cwords = min( nwords, req_buf_size );
	  bcopy( ppixel, SG_Fifo, cwords * 2 ); /* movec, preferably */
	  *(unsigned long *) &sgfcc->cbcsr = (unsigned long) 0L; /* halt */
	  *(unsigned long *) &sgfcc->put = (unsigned long) cwords;
	  /* launch this puppy.. */
	  *(unsigned long *) &sgfcc->cbcsr =
	      (unsigned long)((ENTHRSH<<16)|PTB_UNPACK_ENB); /* & block */
	  /* should wait nicely here -- XXX... */
	  nwords -= req_buf_size;
	  ppixel += cwords * 2;
	  while (!(sgfcc->icsr & ITHRESH)) ; /* wait for thresh int */
	  while (*sg_int_flag != -1) ;
	  while ((sgfcc->fwused) || (*change_section == 1)) ;
      }
}
#endif	/* undef */
