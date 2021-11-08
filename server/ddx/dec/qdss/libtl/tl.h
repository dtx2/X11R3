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

/***********************************************************
Copyright 1987 by Digital Equipment Corporation, Maynard, Massachusetts,
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

#include "miscstruct.h"
#include <sys/types.h>

typedef struct	_dft_struct {
	int (*enable)();
	int (*get)();
	int (*flush)();
} dft_struct;

extern u_short *dma_word;
extern u_short *req_buf_max;        /* pointer to word following    */
extern u_short *dma_max;            /* expected end of dma list     */
extern int	req_buf_size;

#define VOID	int
/*
 * 0x1fff (8k bytes) is as much as the gate array can handle for a PTOB
 */
#define MAXDMAPACKET	0x1fff	/* nuke this 	XXX */
#define MAXDGAWORDS	min( 0x1fff/sizeof(short), req_buf_size)
dft_struct dmafxns;

#define MAXDMAWORDS	/* old name XXX */	\
	min( (MAXDMAPACKET/sizeof( short)), req_buf_size)	

#ifdef DEBUG
# define General_dma( nwords, condition )			\
{								\
    if ( (dma_max = dma_word + (nwords)) > req_buf_max) {	\
	/* buffer full; queue it */				\
	dmafxns.flush ( FALSE );				\
	/* set up the request queue entry stuff, and		\
	 * initialize dma_size */				\
	dmafxns.get ( condition );				\
	if ( (nwords) > req_buf_size)				\
	    FatalError( "DMA buffer request too large\n");	\
	dma_max = dma_word + (nwords);				\
    }								\
}

# define Confirm_dma()						\
{								\
    dma_word = p;						\
    if ((unsigned) dma_word > (unsigned) dma_max)		\
	FatalError( "DMA buffer usage exceeds allocation\n");	\
}
#else	/* no tests for DMA screw-ups */
# define	General_dma( nwords, condition )		\
{								\
    if ( (dma_max = dma_word + (nwords)) > req_buf_max) {	\
	dmafxns.flush ( FALSE );				\
	dmafxns.get ( condition );				\
	dma_max = dma_word + (nwords);				\
    }								\
}
# define Confirm_dma()						\
{								\
    dma_word = p;						\
}
#endif

#define Need_dma( nwords ) \
{ \
    General_dma( nwords, COUNT_ZERO | WORD_PACK ); \
    p = dma_word; \
}

#define NCOLOR24SHORTS	3
#define SET24COLOR( pbuf, color)	\
	*(pbuf)++ = (color)     & 0xff; \
	*(pbuf)++ = (color)>> 8 & 0xff; \
	*(pbuf)++ = (color)>>16 & 0xff;
#define NMASK24SHORTS	3
#define SET24PMASK( pbuf, mask)	\
	*(pbuf)++ = (mask)     & 0xff; \
	*(pbuf)++ = (mask)>> 8 & 0xff; \
	*(pbuf)++ = (mask)>>16 & 0xff;
#define NCOLOR8SHORTS	1
#define SET8COLOR( pbuf, color)	\
	*(pbuf)++ = (color) & 0xff;
#define NMASK8SHORTS	1
#define SET8PMASK( pbuf, mask)	\
	*(pbuf)++ = (mask) & 0xff;

#if NPLANES==24

#define MAC_SETFOREBACKCOLOR	JMPT_SETRGBFOREBACKCOLOR
#define MAC_SETVIPER		JMPT_SETVIPER24
#define MAC_SETCOLOR		JMPT_SETRGBCOLOR
#define NCOLORSHORTS 		NCOLOR24SHORTS
#define SETCOLOR( pbuf, color)	SET24COLOR( pbuf, color )
#define NMASKSHORTS 		NMASK24SHORTS
#define SETPMASK( pbuf, color)	SET24PMASK( pbuf, color )

#else  /* NPLANES==8 */

#define MAC_SETFOREBACKCOLOR	JMPT_SETFOREBACKCOLOR
#define MAC_SETVIPER		JMPT_SETVIPER
#define MAC_SETCOLOR		JMPT_SETCOLOR
#define NCOLORSHORTS 		NCOLOR8SHORTS
#define SETCOLOR( pbuf, color)	SET8COLOR( pbuf, color )
#define NMASKSHORTS 		NCOLOR8SHORTS
#define SETPMASK( pbuf, color)	SET8PMASK( pbuf, color )

#endif


#define PLANES24	0xffffff

extern int     tlLastSerial;
extern char *  tlLastOp;         /* points to the last called tl_* routine */

#define GREEN_UPDATE	0x0060
#define GREEN_SCROLL	0x0040
#define RED_UPDATE	0x0030
#define RED_SCROLL	0x0020
#define BLUE_UPDATE	0x0018
#define BLUE_SCROLL	0x0010

#define ZBLOCK_GREEN    0x0             /* for PLANE_ADDRESS */
#define ZBLOCK_RED      0x10
#define ZBLOCK_BLUE     0x20

#define ZGREEN  0
#define ZRED    1
#define ZBLUE   2
#define ZGREY   3

/* ifdef uVAXII/GPX */
extern struct qdmap             Qdss;
extern struct dga *		Dga;
extern struct adder *		Adder;
extern struct duart *		Duart;
extern char *			Template;
extern char *			Memcsr;
extern struct DMAreq_header *	DMAheader;
/* endif uVAXII/GPX */

/* ifdef Vaxstar */
extern struct sgmap 		Sg;
extern struct fcc *             fcc_cbcsr;
extern struct FIFOreq_header *  FIFOheader;
extern int 			FIFOflag;
extern u_short 			sg_fifo_type;
extern u_short *		SG_Fifo;
extern char *			Fcc;
extern char * 			Vdac;
extern char *			Vrback;
extern char *			Fiforam;
extern char * 			Cur;
extern struct color_buf *       Co[2];

#define SG_MAPDEVICE  _IOR(g, 9, struct sgmap) /* map device to user */
#define SG_VIDEOON    _IO(g,23)             /* turn on the video */
#define SG_VIDEOOFF   _IO(g,24)             /* turn off the video */
#define SG_CURSORON   _IO(g,25)             /* turn on the cursor */
#define SG_CURSOROFF  _IO(g,26)             /* turn off the cursor */
#define ADDRESS_COUNTER         0
#define REQUEST_ENABLE          1
#define STATUS                  3
#define ADDRESS                 0x0010  /* ADDRESS_COMPLETE in qdreg.h */
#define PNT(reg)                (0x8000 | reg)
#define HALT_DMA_OPS            (~(0x0700))

#define	Q_BUSWIDTH	16
#define	QD_MINXTILE	Q_BUSWIDTH
#define	QD_MINYTILE	4	/* due to compuation of tileMagic */

/* logic unit functions: */
#define LF_0         	0
#define LF_DSON      	1
#define LF_DNSA      	2
#define LF_DN        	3
#define LF_DSNA      	4
#define LF_SN        	5
#define LF_DSX       	6
#define LF_DSAN      	7
#define LF_DSA       	8
#define LF_DSXN      	9
#define LF_S        		10
#define LF_DNSO     		11
#define LF_D        		12
#define LF_DSNO     		13
#define LF_DSO      		14
#define LF_1        		15

extern char *display;	/* initialized in server/dix/main.c and updated from
/* endif Vaxstar */

extern char *			Redmap;
extern char *			Bluemap;
extern char *			Greenmap;
extern short *			VDIdev_reg;
extern short *			DMAdev_reg;
extern int			Logic_reg [2];
extern int  			Fore_color [2];
extern int			Back_color [2];
extern int			Source [2];
extern int			QILmode [2];
extern int			QDSSplanes [2];
extern int			QDSSall_planes [2];
extern unsigned short *         AdderPtr;


extern short	qdss_id_code;
extern int      fd_qdss;          /* global qdss file descriptor */
extern short    oldwakeflags;     /* select type flags */
extern int	cookie;

/*
 * empty the queue
 */
#define FLUSHDMA(header) while(!(DMA_ISEMPTY(header)));

/*
 * Wait for the byte count to go zero BEFORE waiting for the FIFO to empty
 */
#define WAITBYTCNTZERO(Dga) \
	while ( (Dga)->bytcnt_hi & 0xff && (Dga)->bytcnt_lo);
/*
 * wait for FIFO empty.  "WAITDMADONE" is a bad name; WAITBYTCNTZERO()
 * must also be invoked to ensure that DMA is complete.
 */
#define FIFOCOUNT 0x3f00	/* bits 8-13 */
#define WAITDMADONE(Dga) while(Dga->bytcnt_hi & FIFOCOUNT);

/*
 *  Set the translation point if necessary.
 *  First check the global variable before going to all the
 *  trouble to change things.
 */
extern DDXPointRec _tlTranslate;	/* global state */

#define SETTRANSLATEPOINT( newx, newy) {		\
    if (((newx) != _tlTranslate.x) ||			\
	((newy) != _tlTranslate.y)) {			\
	_tlTranslate.x = (newx);			\
	_tlTranslate.y = (newy);			\
	tlsettranslate(_tlTranslate.x, _tlTranslate.y);	\
    }							\
}

/* nextslot is the state of available space in the buffer */
typedef struct {
    struct DMAreq_header *pDMA;
    char *pBuf;              /* start of buffer */
    int nbyMax;              /* space in an empty buffer */
    char *pNext;             /* next free spot in buffer */
    int nbyFree;
    int ireqMax;             /* pre-computed maximum queue index */
    } NEXTSLOT;

/*
 * mapping from X rop to dragon rop -- X3 update method
 */
extern int umtable[];

/* SETTILEMAGIC
 *	set (short) magic = tilemagic from width and height of pixmap.
 */
#define	SETTILEMAGIC(ppixmap,magic)	\
{	\
    register	W, H;	\
    for (W = 2; ((ppixmap)->width >> W) > 1; W += 1)	\
    {	\
        if (((ppixmap)->width >> W) << W != (ppixmap)->width)	\
        {	\
            FatalError("tlFullTile: bad tile width\n");   /* XXX */	\
        }	\
    }	\
    W -= 2;	\
    for (H = 2; ((ppixmap)->height >> H) > 1; H += 1)	\
    {	\
        if (((ppixmap)->height >> H) << H != (ppixmap)->height)	\
        {	\
            FatalError("tlFullTile: bad tile height\n");   /* XXX */	\
        }	\
    }	\
    H -= 2;	\
    magic = (H << 4 | W) & 0x3fff;	\
}

#define	INVALID_SHADOW	Invalid_Shadow()

