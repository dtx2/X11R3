#ifndef HDWR_SEEN
#define HDWR_SEEN 1
/***********************************************************
		Copyright IBM Corporation 1987,1988

                      All Rights Reserved

Permission to use, copy, modify, and distribute this software and its 
documentation for any purpose and without fee is hereby granted, 
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in 
supporting documentation, and that the name of IBM not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.  

IBM DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
IBM BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.

******************************************************************/
/* $Header: /andrew/X11/R3src/Xbeta/server/ddx/ibm/apa16/RCS/apa16Hdwr.h,v 9.0 88/10/17 00:12:29 erik Exp $ */
/* $Source: /andrew/X11/R3src/Xbeta/server/ddx/ibm/apa16/RCS/apa16Hdwr.h,v $ */
/* "@(#)apa16hdwr.h	3.1 88/09/22 09:30:45" */

#if !defined(lint) && !defined(LOCORE) && defined(RCS_HDRS)
static char *rcsidapa16hdwr = "$Header: /andrew/X11/R3src/Xbeta/server/ddx/ibm/apa16/RCS/apa16Hdwr.h,v 9.0 88/10/17 00:12:29 erik Exp $";
#endif

typedef volatile unsigned short int * apa16MemoryPtr ;

#define	APA16_BASE		(0xf4d80000)
#define SCREEN_ADDR(x,y)	((apa16MemoryPtr) \
				 (APA16_BASE+((y)*(1024/8))+(((x)&0x3ff)>>3)))
#define	SCREEN_SPTR(x,y)	((unsigned short) \

#define	APA16_WIDTH		1024
#define	APA16_HEIGHT		768

extern	int	apa16Qoffset;
extern	int	apa16Qdanger;
extern	unsigned short int	apa16_rop2stype[];

#define CURSOR_X	(*(apa16MemoryPtr)0xf4d9f800)
#define CURSOR_Y	(*(apa16MemoryPtr)0xf4d9f802)

#define	MAXCURSORS		3
#define CURSOR_AND_OFFSET	0
#define CURSOR_XOR_OFFSET	48
#define	CURSOR_WIDTH		48
#define	CURSOR_HEIGHT		64
#define CURSOR_AREA_TOP		784
#define CURSOR_AREA_BOTTOM	( CURSOR_AREA_TOP + CURSOR_HEIGHT )

#define	STAGE_WIDTH		96 /* Allows 64 bit wide on any boundry < 32 */
#define	STAGE_HEIGHT		CURSOR_HEIGHT
#define	STAGE_X_OFFSET		( APA16_WIDTH - STAGE_WIDTH )
#define STAGE_AREA_TOP		CURSOR_AREA_TOP
#define STAGE_AREA_BOTTOM	( STAGE_AREA_TOP + STAGE_HEIGHT )
#define APA16_STAGE_X		STAGE_X_OFFSET
#define APA16_STAGE_Y		STAGE_AREA_TOP

#define ACTIVE_AND_AREA	SCREEN_ADDR( 0, CURSOR_AREA_TOP )
#define ACTIVE_XOR_AREA SCREEN_ADDR( CURSOR_XOR_OFFSET, CURSOR_AREA_TOP )

#define	FONT_TOP	848
#define	FONT_BOTTOM	895	

#define CSR		(*(apa16MemoryPtr)0xf0000d12)
#define	BLACK_ON_WHITE()	(CSR|=0x400)
#define	WHITE_ON_BLACK()	(CSR&=~0x400)
#define	TOGGLE_BACKGRND()	(CSR^=0x400)

#define	MR		(*(apa16MemoryPtr)0xf0000d10)
#define MODE_SHADOW	(*(apa16MemoryPtr)0xf4d9f812)
#define	SCANLINE	(*(apa16MemoryPtr)0xf4d9f808)

#define	MERGE_MODE_MASK	(0xff0f)
#define	MERGE_MODE	(MODE_SHADOW&(~MERGE_MODE_MASK))
#define	SET_MERGE_MODE(mode)	(MR=(MODE_SHADOW&MERGE_MODE_MASK)|(mode))
#define	MERGE_BLACK	(0x20)
#define	MERGE_COPY	(0x90)
#define	MERGE_INVERT	(0xa0)
#define	MERGE_WHITE	(0xb0)
#define	BAD_LOGOP	(0xffff)

#define	BIT_OFFSET_MASK	(0x000f)
#define	BIT_OFFSET	(MODE_SHADOW&(~BIT_OFFSET_MASK))
#define	SET_BIT_OFFSET(off)	(MR=(MODE_SHADOW&BIT_OFFSET_MASK)|(off))

#define SPTR_TO_QPTR(p)	((((unsigned)(p))>>1)&0xffff)
#define QPTR_TO_SPTR(p)	((apa16MemoryPtr)(0xf4d90000|((((unsigned)(p))<<1)&0xffff)))
#define SET_QUEUE_PTR(p)	(QUEUE_PTR=SPTR_TO_QPTR(p))

#define	QUEUE_MARGIN	10
#define	BRANCH_QUEUE_TOP	((1006<<6)|(1023>>4))
#define QPTR_QUEUE_TOP	SPTR_TO_QPTR(QUEUE_TOP)
#define	QPTR_QUEUE_BASE	SPTR_TO_QPTR(QUEUE_BASE)
#define QUEUE_TOP	SCREEN_ADDR(APA16_WIDTH-1,1006)
#define QUEUE_BASE	SCREEN_ADDR(0,897)
#define QUEUE_SIZE	( ( (long int) QUEUE_TOP ) - (long int) QUEUE_BASE )
#define	QUEUE_CNTR	(*(apa16MemoryPtr)0xf4d9f804)
#define QUEUE_PTR	(*(apa16MemoryPtr)0xf4d9f806)
#define INCR_QUEUE_CNTR()	((*(apa16MemoryPtr)0xf0000d14)=0)

#define LOAD_QUEUE(rop,off)	((*(QUEUE_BASE+apa16Qoffset+(off)))=(rop))

#define REG_LOAD(reg,val,off)	\
	LOAD_QUEUE((((reg)<<12)&0xf000)|(val)&0x3ff,off)

#define	CMD_LOAD(cmd,off)	\
		(LOAD_QUEUE(((cmd)|EXEC_MASK),off),INCR_QUEUE_CNTR())

#define	CMD_MASK	0xfff0
#define STYPE_MASK	0x000f
#define EXEC_MASK	0x0800

#define STYPE_BLACK		0x0000
#define STYPE_WHITE		0x000f
#define STYPE_RECT_INVERT	0x0009
#define STYPE_INVERT		0x000a
#define STYPE_NOP		0x0007

#define ROP_RECT_FILL		0xd2f0
#define ROP_RECT_COPY		0xd300
#define ROP_RECT_COPY_L90	0xd310
#define ROP_RECT_COPY_R90	0xd320
#define ROP_RECT_COPY_X180	0xd330
#define ROP_RECT_COPY_Y180	0xd340
#define ROP_VECTOR		0xd350
#define ROP_NULL_VECTOR		0xd360
#define	ROP_BRANCH		0xd3b0
#define	ROP_VIDEO_ON		0xd3e0
#define	ROP_VIDEO_OFF		0xd3f0


#define	CHECK_QUEUE(off,inst) \
	{ \
	  if ((off)>apa16Qdanger) {\
		if (apa16Qoffset-(off)<=QUEUE_MARGIN) {\
		    LOAD_QUEUE(BRANCH_QUEUE_TOP,0);\
		    apa16Qoffset= QUEUE_SIZE/2;\
		}\
		while ((off)>apa16Qdanger) {\
		    apa16Qdanger= apa16Qoffset-(QUEUE_PTR-QPTR_QUEUE_BASE)-\
					QUEUE_MARGIN;\
		    if (apa16Qdanger<=0)	apa16Qdanger+=QUEUE_SIZE/2;\
		    apa16Qdanger=MIN(apa16Qoffset-QUEUE_MARGIN,apa16Qdanger);\
		}\
if ((apa16Qdanger>QUEUE_SIZE/2)||(apa16Qoffset<0)) {\
	ErrorF("queue weirdness!!\n");\
	apa16_queue_check();\
}\
	  }\
	  apa16Qdanger-= (off);\
	  apa16Qoffset-= (off);\
	  (inst);\
	}

#define QUEUE_INIT()	((apa16Qoffset=QUEUE_SIZE/2),\
			    apa16Qdanger= QUEUE_SIZE/2-QUEUE_MARGIN,\
			    SET_QUEUE_PTR(QUEUE_TOP))

#define QUEUE_RESET()	{if (!QUEUE_CNTR) {QUEUE_INIT();}}

#define	QUEUE_WAIT()	{while (QUEUE_CNTR); QUEUE_INIT();}

#define APA16_GET_CMD(cmd,rrop,out) \
	if (((cmd)&CMD_MASK)==ROP_RECT_FILL) {\
	    (out)= ((cmd)&CMD_MASK)|\
		(((rrop)==RROP_WHITE?STYPE_WHITE:\
		    ((rrop)==RROP_BLACK?STYPE_BLACK:\
		    ((rrop)==RROP_INVERT?\
			STYPE_RECT_INVERT:\
			STYPE_NOP)))&STYPE_MASK);\
	}\
	else if ((((cmd)&CMD_MASK)==ROP_VECTOR)||\
		 (((cmd)&CMD_MASK)==ROP_NULL_VECTOR)) {\
	    (out)= ((cmd)&CMD_MASK)|\
		(((rrop)==RROP_WHITE?STYPE_WHITE:\
		    ((rrop)==RROP_BLACK?STYPE_BLACK:\
		    ((rrop)==RROP_INVERT?\
			STYPE_INVERT:\
			STYPE_NOP)))&STYPE_MASK);\
	}\
	else if ((((cmd)&CMD_MASK)==ROP_RECT_COPY)||\
		 (((cmd)&CMD_MASK)==ROP_RECT_COPY_L90)||\
		 (((cmd)&CMD_MASK)==ROP_RECT_COPY_R90)||\
		 (((cmd)&CMD_MASK)==ROP_RECT_COPY_X180)||\
		 (((cmd)&CMD_MASK)==ROP_RECT_COPY_Y180)) {\
	    (out)= ((cmd)&CMD_MASK)|(apa16_rop2stype[rrop&0xf]&STYPE_MASK);\
	}\
	else {\
	    ErrorF("WSGO! Unknown rasterop 0x%x\n",(cmd));\
	}


#define FILL_RECT(cmd,x,y,dx,dy)	\
		CHECK_QUEUE(5,(REG_LOAD(7,x,5),REG_LOAD(6,y,4),\
		 	    REG_LOAD(9,dx,3),REG_LOAD(8,dy,2),\
			    CMD_LOAD((cmd),1)))

#define COPY_RECT(cmd,xd,yd,xs,ys,dx,dy)	\
		CHECK_QUEUE(7,(REG_LOAD(7,xd,7),REG_LOAD(0xA,yd,6),\
		 	    REG_LOAD(5,xs,5),REG_LOAD(4,ys,4),\
		 	    REG_LOAD(9,dx,3),REG_LOAD(0,dy,2),\
		 	    CMD_LOAD((cmd),1)))

#define COPY_CHAR_RECT(cmd,xd,xs,ys,dx,dy)	\
		CHECK_QUEUE(7,(REG_LOAD(7,xd,7),\
		 	    REG_LOAD(5,xs,5),REG_LOAD(4,ys,4),\
		 	    REG_LOAD(9,dx,3),REG_LOAD(0,dy,2),\
		 	    CMD_LOAD((cmd),1)))

#define DRAW_VECTOR(cmd,fx,fy,tx,ty)	\
		CHECK_QUEUE(5,(REG_LOAD(1,fx,5),REG_LOAD(0,fy,4),\
			    REG_LOAD(5,tx,3),REG_LOAD(6,ty,2),\
			    CMD_LOAD((cmd),1)))

#define POLY_VECTOR(cmd,tx,ty) \
		CHECK_QUEUE(3,(REG_LOAD(5,tx,3),REG_LOAD(6,ty,2),\
			    CMD_LOAD((cmd),1)))

#define VIDEO_ON() \
		CHECK_QUEUE(1,(CMD_LOAD(ROP_VIDEO_ON,1)))

#define VIDEO_OFF() \
		CHECK_QUEUE(1,(CMD_LOAD(ROP_VIDEO_OFF,1)))
#endif /* ndef HDWR_SEEN */
