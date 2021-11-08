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
/* $Header: /andrew/X11/R3src/Xbeta/server/ddx/ibm/apa16/RCS/apa16Decls.h,v 9.0 88/10/17 00:12:09 erik Exp $ */
/* $Source: /andrew/X11/R3src/Xbeta/server/ddx/ibm/apa16/RCS/apa16Decls.h,v $ */
/* "@(#)apa16decls.h	3.1 88/09/22 09:30:27" */

/* apa16bitblt.c */
extern	RegionPtr	apa16CopyArea();
extern	int		apa16DoBitblt();

/* apa16cursor.c */
extern	int		apa16CursorInit();
extern	Bool		apa16RealizeCursor();
extern	Bool		apa16UnrealizeCursor();
extern	int		apa16DisplayCursor();
extern	void		apa16HideCursor();

/* apa16fillsp.c */
extern	void		apa16SolidFS();
extern	void		apa16StippleFS();

/* apa16font.c */
extern	Bool		afRealizeFont();
extern	Bool		afUnrealizeFont();

/* apa16gc.c */
extern	Bool		apa16CreateGC();
extern	void		apa16ValidateGC();
extern	void		apa16DestroyGC();

/* apa16hdwr.c */
extern	int		apa16_qoffset;
extern	unsigned short int apa16_rop2stype[];

/* apa16imggblt.c */
extern	void		apa16ImageGlyphBlt();

/* apa16io.c */
extern	Bool		apa16Probe();
extern	Bool		apa16ScreenInit();
extern	Bool		apa16ScreenClose();

/* apa16line.c */
extern	void		apa16LineSS();
extern	void		apa16PolySegment();

/* apa16plygblt.c */
extern	void		apa16PolyGlyphBlt();

/* apa16pntwin.c */
extern	void		apa16PaintWindowSolid();

/* apa16pnta.c */
extern	void		apa16SolidFillArea();
extern	void		apa16StippleFillArea();

/* apa16reparea.c */
extern	void		apa16replicateArea();

/* apa16text.c */
extern	void		apa16ImageText8();
extern	void		apa16ImageText16();
extern	int		apa16PolyText8();
extern	int		apa16PolyText16();

/* apa16tile.c */
extern	void		apa16TileArea32();

/* apa16window.c */
extern	Bool		apa16CreateWindow();
extern	void		apa16CopyWindow();
extern	Bool		apa16ChangeWindowAttributes();
