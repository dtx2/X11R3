/*
 *   Copyright (c) 1987, 88 by
 *   PARALLAX GRAPHICS, INCORPORATED, Santa Clara, California.
 *   All rights reserved
 *
 *   This software is furnished on an as-is basis, and may be used and copied
 *   only with the inclusion of the above copyright notice.
 *
 *   The information in this software is subject to change without notice.
 *   No committment is made as to the usability or reliability of this
 *   software.
 *
 *   Parallax Graphics, Inc.
 *   2500 Condensa Street
 *   Santa Clara, California  95051
 */

#ifndef lint
static char *sid_ = "@(#)plxMisc.c	1.35 08/31/88 Parallax Graphics Inc";
#endif

#include	"Xplx.h"

void
plxQueryBestSize(class, pWidth, pHeight)
short *pWidth, *pHeight;
{
	switch(class) {
	case CursorShape:
		*pWidth = 16;
		*pHeight = 16;
		break;
	case TileShape:
	case StippleShape:
		*pWidth = 16;
		*pHeight = 16;
		break;
	}
}

int plx_debug = 0;

short Plx_From_X_Op[] = {
	-2 ,			/* GXclear */
	0 ,			/* GXand */
	1 ,			/* GXandReverse */
	-1 ,			/* GXcopy */
	2 ,			/* GXandInverted */
	-4 ,			/* GXnoop */
	3 ,			/* GXxor */
	4 ,			/* GXor */
	5 ,			/* GXnor */
	6 ,			/* GXequiv */
	7 ,			/* GXinvert */
	8 ,			/* GXorReverse */
	9 ,			/* GXcopyInverted */
	10 ,			/* GXorInverted */
	11 ,			/* GXnand */
	-2 ,			/* GXset */
};

int plxdefaultbswap = 0;

plxboardinit(displayname, displayfile,  microcodename, bswapflag, sbit15flag)
char *displayname;
char *displayfile;
char *microcodename;
{
	int i,j,k;
	int w, h;

	if (px_open(displayname) < 0)
		exit (1);
	px_init();

	if (plxucode(microcodename)) {
		/* we reloaded the microcode */
		px_close();
		if (px_open(displayname) < 0)
			exit (1);
		px_init();
	}

	p_vtstop();
	px_flush();

	/* Model 1280 device setup */

	if (bswapflag != -1) {
		plxdefaultbswap = bswapflag;
	} else {
#if defined(vax) || defined(i386) || defined(ibm032)
		plxdefaultbswap = 0;
#endif
#if defined(sun) || defined(is68k) || defined(motorola131) || defined(hpux)
		plxdefaultbswap = 1;
#endif
	}
	plxbyteswap(plxdefaultbswap);

	if (sbit15flag != -1) {
		if (sbit15flag == 0)
			p_sbit0();
		else
			p_sbit15();
	} else {
#if defined(vax) || defined(i386) || defined(ibm032)
		p_sbit0();
#endif
#if defined(sun) || defined(is68k) || defined(motorola131) || defined(hpux)
		p_sbit15(1);
#endif
	}
	px_flush();

	plxdisplayfile(displayfile, &w, &h);
	px_flush();

	p_zoom(1, 1);
	p_pan(0, 0);
	p_stip16();
	p_mask(0xffff);
	p_tranr(0, 0);
	p_tranw(0, 0);
	p_opaq(0);
	p_rmap(0);
	p_clipd();
	p_floff();
	p_egov();
	p_damvg();

	/* fill empty colormap slots */
	for (i=0;i<256;i++) {
		if ((i < 192) || ((i&1) == 0)) {
			/* regular colormap entry */
			p_clt8(i,
				((i&0x07)<<5),
				((i&0x38)<<2),
				((i&0xc0)   ));
		} else {
			/* video overlay color */
			p_clt8(i, 255, 255, 255);
		}
	}

	p_box(0x55, 0, 0, 2047, 2047);		/* 01010101 pattern */

	/* initiallize Opaq table: Draw only color #0 */
	p_opaq(ONLY0_OPAQ_TABLE);
	p_opaqm(0xff,0);

	/* initiallize Opaq table: Draw only color #1 */
	p_opaq(ONLY1_OPAQ_TABLE);
	p_opaqm(0xff,1);

	/* initiallize Opaq table: bitmap unload */
	p_opaq(UBIT_OPAQ_TABLE);
	p_opaqm(1, 1);

	/* initialize Stencil table: Draw only Stencil color */
	p_opaq(ONLY_STENCIL_COLOR_OPAQ_TABLE);
	p_opaqm(0xff,STENCIL_COLOR);
	p_opaq(0);

	/* initiallize RMap table: bitmap load, fonts, */
	set_lbmrmap(0xff, 0x00);

	{
		unsigned char m[2];

		p_rmap(LBIT_RMAP_TABLE);
#if defined(vax) || defined(i386) || defined(ibm032)
		m[1] = 0xff;
		m[0] = 0x00;
#endif
#if defined(sun) || defined(is68k) || defined(motorola131) || defined(hpux)
		m[0] = 0xff;
		m[1] = 0x00;
#endif
		p_rmapl(254,2,m);
	}

	SetFontMaps(0,0,0,0);
	p_rmap(0);
	/* initiallize ROP1 */
	set_rop1(0);
	px_flush();
}

plxucode(microcodename)
char *microcodename;
{
	static int eis, revmaj, revmin, beenhere = 0;
	FILE *f;
	unsigned short rval1, rval2;
	int didit = 0;
	extern FILE *plxucodefile();

	if (!beenhere) {
		p_show(&rval1);
		p_showr(&rval2);
		if (rval1 == rval2) {
			/* pre 7.02 microcode */
			revmaj = rval1 & 0x001f;
			revmin = 0x99;	/* what else ?? */
			if (revmaj < 6)
				eis = 0;
			else
				eis = 1;
		} else {
			revmaj = (rval2 & 0xff00) >> 8;
			revmin = rval2 & 0x00ff;
			eis = (rval1 & 0x01e0) >> 5;
		}
		beenhere++;
	}

	if ((revmaj <= 6) && (revmin < 5)) {
		fprintf(stderr, "Xplx: microcode too old, sorry, firmware revision %x.%02x\n", revmaj, revmin);
		fprintf(stderr, "Xplx: contact parallax sales office\n");
		exit (1);
	}

	if (!eis) {
		/* force microcode load */
		revmaj = revmin = 0;
	}

	f = plxucodefile(microcodename, revmaj, revmin);
	if (f) {
		plxucodeload(f);
		fclose(f);
		didit = 1; 
	}

	p_show(&rval1);
	p_showr(&rval2);
	if (rval1 == rval2) {
		/* pre 7.02 microcode */
		revmaj = rval1 & 0x001f;
		revmin = 0x99;	/* what else ?? */
		if (revmaj < 6)
			eis = 0;
		else
			eis = 1;
	} else {
		revmaj = (rval2 & 0xff00) >> 8;
		revmin = rval2 & 0x00ff;
		eis = (rval1 & 0x01e0) >> 5;
	}

	if (!eis) {
		fprintf(stderr, "Xplx: microcode not EIS\n");
		fprintf(stderr, "Xplx: contact parallax sales office\n");
		exit(1);
	}

	return (didit);
}

FILE *
plxucodefile(microcodename, orevmaj, orevmin)
char *microcodename;
int orevmaj, orevmin;
{
	FILE *f;
	unsigned short instruction, rval2;
	int n, revmaj, revmin;
	extern char *getenv();

	if ((microcodename == 0) || (*microcodename == '\0'))
		microcodename = getenv("PLXMICROCODE");

	if ((microcodename == 0) || (*microcodename == '\0')) {
		fprintf(stderr, "Xplx: no microcode file specified, trying to continue\n");
		return (NULL);
	}
	if ((f=fopen(microcodename, "r")) == NULL) {
		fprintf(stderr, "Xplx: microcode file: ");
		perror(microcodename);
		return (NULL);
	}
	n = fread(&instruction, sizeof(instruction), 1, f);
	if ((n != 1) || (instruction != 0x3024)) {
		fprintf(stderr, "Xplx: microcode file bad, trying to continue\n");
		fclose(f);
		return (NULL);
	}

	n = fread(&rval2, sizeof(rval2), 1, f);
	if (n != 1) {
		fclose(f);
		return (NULL);
	}
	revmaj = (rval2 & 0xff00) >> 8;
	revmin = rval2 & 0x00ff;
	if ((revmaj == orevmaj) && (revmin == orevmin)) {
		/* same code */
		fclose(f);
		return (NULL);
	}
	printf("Xplx: loading new microcode, firmware revision %x.%02x\n", revmaj, revmin);

	rewind(f);
	return (f);
}

plxucodeload(f)
FILE *f;
{
#define	XFERSIZE	4096
	register int n;
	unsigned short xferbuffer[XFERSIZE];

	pl_ioctl_pio();
	while ((n=fread(xferbuffer, sizeof(unsigned short), XFERSIZE, f)) != 0) {
		p_write(n, xferbuffer);
	}
	pl_ioctl_dma();
#undef XFERSIZE
}

plxdisplayfile(displayfile, width, height)
char *displayfile;
int *width, *height;
{
	FILE *f;
	int rval;
	int interlace;
	int hfp, hsy, hbp, hdd, vfp, vsy, vbp, vdd, xz;

	if ((displayfile == 0) || (*displayfile == '\0'))
		displayfile = getenv("PLXDISPLAY");

	if ((displayfile == 0) || (*displayfile == '\0')) {
		/* It is valid to not specify the display file */
		return;
	}
	if ((f=fopen(displayfile, "r")) == NULL) {
		fprintf(stderr, "Xplx: display file: ");
		perror(displayfile);
		return;
	}

	/*
	 * Display file format, is:
	 *	width height and 10 display format numbers
	 *
	 * 	interlace	- 0 == non interlaced
	 *			  1 == interlaced
	 *
	 *	hfp,hsy,hbp,hdd	- horiz front porch, sync, back porch,
	 *			  horiz display size
	 *	vfp,vsy,vbp,vdd - vert front porch, sync, back porch,
	 *			  vert display size
	 *	xz		- horiz zoom
	 *
	 * These numbers are produced by the plxmonitor
	 * program. (see SW-BASE-DOC).
	 */
	rval = fscanf(f, "%d %d %d %d %d %d %d %d %d %d %d %d",
		width, height,
		&interlace,
		&hfp, &hsy, &hbp, &hdd, &vfp, &vsy, &vbp, &vdd, &xz);
	if (rval != 12) {
		fprintf(stderr, "Xplx: display file: format error\n");
		fclose(f);
		return;
	}
	if (interlace) {
		p_ldph(hfp,hsy,hbp,hdd);
		p_ldpvi(vfp,vsy,vbp,vdd,xz);
	} else {
		p_ldph(hfp,hsy,hbp,hdd);
		p_ldpv(vfp,vsy,vbp,vdd,xz);
	}
	fclose(f);
}

unsigned long plxmaskbits[] = {
	0x00000000,				/* depth = 0 */
	0x00000001,				/* depth = 1 */
	0x00000003,
	0x00000007,
	0x0000000f,
	0x0000001f,
	0x0000003f,
	0x0000007f,
	0x000000ff,				/* depth = 8 */
	0x000001ff,
	0x000003ff,
	0x000007ff,
	0x00000fff,
	0x00001fff,
	0x00003fff,
	0x00007fff,
	0x0000ffff,				/* depth = 16 */
	0x0001ffff,
	0x0003ffff,
	0x0007ffff,
	0x000fffff,
	0x001fffff,
	0x003fffff,
	0x007fffff,
	0x00ffffff,
	0x01ffffff,
	0x03ffffff,
	0x07ffffff,
	0x0fffffff,
	0x1fffffff,
	0x3fffffff,
	0x7fffffff,
	0xffffffff,				/* depth = 32 */
};

/*
 * plxMask()
 *	set the mask based on the pDrawable->depth and GC
 */
void
plxMask(pDrawable, pGC)
register DrawablePtr pDrawable;
GCPtr pGC;
{
	register MapPriv *mp;
	register int mask;

	ifdebug(8) printf("plxMask(), planemask=0x%08x, depth=%d\n",
				pGC->planemask, pDrawable->depth);

	switch (pDrawable->type) {
	case DRAWABLE_WINDOW:
		{
			register WindowPtr pWindow = (WindowPtr)pDrawable;

			mp = (MapPriv *)pWindow->devPrivate;
		}
		break;
	case DRAWABLE_PIXMAP:
		{
			register PixmapPtr pPixmap = (PixmapPtr)pDrawable;

			mp = (MapPriv *)pPixmap->devPrivate;
		}
		break;
	}

	if (pGC) {
		mask = pGC->planemask & plxmaskbits[pDrawable->depth];
	} else {
		mask = plxmaskbits[pDrawable->depth];
	}
	if (mp->video) {
		mask &= 0x0001;
		/* p_damvv(); */
	} else {
		/* p_damvg(); */
	}
	p_damvx();
	p_mask(mask);
}

set_lbmrmap(fore, back)
int fore, back;
{
	static int oback = -1, ofore = -1;

	p_rmap(LBITCOL_RMAP_TABLE);
	if ((fore != ofore) || (back != oback)) {
		unsigned char tbuf[2];

#if defined(vax) || defined(i386) || defined(ibm032)
		tbuf[1] = ofore = fore;
		tbuf[0] = oback = back;
#endif
#if defined(sun) || defined(is68k) || defined(motorola131) || defined(hpux)
		tbuf[0] = ofore = fore;
		tbuf[1] = oback = back;
#endif
		p_rmapl(254, 2, tbuf);
	}
}

/*
 * setup readmap for text rops
 * also used for all stencil stuff.
 */
SetFontMaps(fg, bg, planemask, opaqflag)
int fg, bg, planemask, opaqflag;
{
	static int ofg = -1, obg = -1, oplanemask = -1, oplanemaskopaq = -1;

	ifdebug(14) printf("SetFontMaps(), fg,bg=%d,%d planemask=0x%08x opaqflag=%d\n", fg, bg, planemask, opaqflag);

	p_rmap(FONT_RMAP_TABLE);
	if ((planemask != oplanemask) || (ofg != fg) || (obg != bg)) {
		p_rmapm(planemask, planemask, fg, bg);
		oplanemask = planemask;
		ofg = fg;
		obg = bg;
	}
	if (opaqflag) {
		p_opaq(FONT_OPAQ_TABLE);
		if (planemask != oplanemaskopaq) {
			p_opaqm(planemask, planemask);
			oplanemaskopaq = planemask;
		}
	} else {
		p_opaq(0);
	}
}

set_rop1(op,data)
{
	static int oop = -1,odata = -1;

	if ((oop != op) || (odata != data)) {
		oop=op; 
		odata=data;
		p_srop1(op,ROP1_RMAP_TABLE,data);
	}
}

plxbyteswap(on)
{
	p_bswap(on?plxdefaultbswap:0);
}
