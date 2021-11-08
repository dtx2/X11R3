#ifndef MPEL_PROCS_SEEN
#define	MPEL_PROCS_SEEN	1
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

/* $Header: /andrew/X11/R3src/tape/server/ddx/ibm/mpel/RCS/mpelProcs.h,v 6.1 88/10/25 01:42:28 kbg Exp $ */
/* $Source: /andrew/X11/R3src/tape/server/ddx/ibm/mpel/RCS/mpelProcs.h,v $ */

#ifndef lint
static char *rcsidmpelprocs = "$Header: /andrew/X11/R3src/tape/server/ddx/ibm/mpel/RCS/mpelProcs.h,v 6.1 88/10/25 01:42:28 kbg Exp $";
#endif

extern void mpelColorCursor();
extern Bool mpelDisplayCursor();
extern void mpelShowCursor();
extern void mpelFillStipple();
extern void mpelReadColorImage();
extern void mpelDrawColorImage();
extern void mpelFillSolid();
extern void mpelBitBlt();
extern void mpelResolveColor();
extern void mpelSetColor();
extern void mpelValidateGC();
extern Bool mpelCreateGC();
extern void mpelDestroyGCPriv();
extern Bool mpelScreenInit();
extern Bool mpelScreenClose();
extern void mpelInstallColormap();
extern void mpelUninstallColormap();
extern void mpelTile();
extern void mpelDrawMonoImage();
extern void mpelDrawMonoByteImage();
extern void mpelFillPolygon();
extern void mpelImageGlyphBlt();
extern void mpelZeroSolidLine();
extern void mpelPolySegment();
extern int mpelPolyText8();
extern int mpelPolyText16();
extern void mpelImageText8();
extern void mpelImageText16();
extern Bool mpelRealizeFont();
extern Bool mpelUnrealizeFont();
extern unsigned long int *mpelGetPlane();
extern void mpelPolyArc();
extern void mpelLineVert();
extern void mpelLineHorz();
extern void mpelLineBres();
extern void mpelTilePolygon();
extern void mpelZeroLine();
extern void mpelPolyArc();
extern void mpelPolyPoint();
#endif /* MPEL_PROCS_SEEN */
