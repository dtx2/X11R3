/***********************************************************
		Copyright IBM Corporation 1988

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

/* $Header: /andrew/X11/R3src/tape/server/ddx/ibm/vga/RCS/vgaData.c,v 6.3 88/10/24 22:21:32 paul Exp $ */
/* $Source: /andrew/X11/R3src/tape/server/ddx/ibm/vga/RCS/vgaData.c,v $ */

#ifndef lint
static char *rcsid = "$Header: /andrew/X11/R3src/tape/server/ddx/ibm/vga/RCS/vgaData.c,v 6.3 88/10/24 22:21:32 paul Exp $";
#endif

#include "X.h"
#include "Xproto.h"
#include "miscstruct.h"
#include "scrnintstr.h"
#include "screen.h"
#include "font.h"
#include "pixmapstr.h"
#include "window.h"
#include "gcstruct.h"
#include "colormapst.h"
#include "cursorstr.h"

#include "mistruct.h"
#include "../../mfb/mfb.h"

#include "OScursor.h"
#include "ibmScreen.h"

#include "ppc.h"
#include "ppcProcs.h"

#include "vgaVideo.h"

#include "vgaProcs.h"

extern void miRecolorCursor() ;
extern void NoopDDA() ;

static PixmapPtr BogusPixmap = (PixmapPtr) 1 ;

VisualRec vgaVisuals[] = {
	{
	0,			/* unsigned long	vid */
	0,			/* short	screen */    
	PseudoColor,		/* short       class */
	0,			/* unsigned long	redMask */
	0,			/* unsigned long	greenMask */
	0,			/* unsigned long	blueMask */
	0,			/* int		offsetRed */
	0,			/* int		offsetGreen */
	0,			/* int		offsetBlue */
	6,			/* short       bitsPerRGBValue */
	1 << VGA_MAXPLANES,	/* short	ColormapEntries */
	VGA_MAXPLANES		/* short	nplanes */
	}
} ;

static unsigned long int vgaDepthVID = 0 ;

DepthRec vgaDepths[] = {
/*	depth		numVid	vids */
    {	1,		0,	NULL	},
    {	VGA_MAXPLANES,	1,	&vgaDepthVID }
} ;

ppcScrnPriv vgaScrnPriv = {
	{
		{
			DRAWABLE_PIXMAP,/* short	type */
			0,		/* ScreenPtr	pScreen */
			VGA_MAXPLANES,	/* int         depth */
			0,		/* unsigned long        serialNumber */
		},		/* DrawableRec drawable */
		MAX_COLUMN + 1,	/* int width */
		MAX_ROW + 1,	/* int height */
		1,		/* int refcnt */
		(MAX_ROW + 1) / 8,	/* int devKind */
		0		/* pointer devPrivate */
	},			/* PixmapRec	pixmap */
	0,			/* Colormap	InstalledColormap */
	vgaBitBlt,		/* void	(* blit)() */
	vgaFillSolid,		/* void	(* solidFill)() */
#ifdef VGA_SPECIFIC_ROUTINES
	vgaTileRect,		/* void	(* tileFill)() */
#else
	ppcTileRect,		/* void	(* tileFill)() */
#endif
	vgaFillStipple,		/* void	(* stipFill)() */
	ppcOpaqueStipple,	/* void	(* opstipFill)() */
	vgaDrawColorImage,	/* void	(* imageFill)() */
	vgaReadColorImage,	/* void	(* imageRead)() */
	vgaBresLine,		/* void	(* lineBres)() */
	vgaHorzLine,		/* void	(* lineHorz)() */
	vgaVertLine,		/* void	(* lineVert)() */
	vgaSetColor,		/* void	(* setColor)() */
	vgaRecolorCursor,	/* void	(* RecolorCursor)() */
	vgaDrawMonoImage,	/* void	(* monoFill)() */
	vgaDrawGlyph,		/* void	(* glyphFill)() */
	0,			/* unsigned long *((* planeRead)()) */
	0,			/* void	(* replicateArea)() */
	0,			/* void	(* DestroyGCPriv)() */
/* High Level Software Cursor Support !! */
	&vgaCursorSemaphore,	/* int	* CursorSemaphore */
	vgaCheckCursor,		/* int	(* CheckCursor)() */
	vgaReplaceCursor	/* void	(* ReplaceCursor)() */
} ;

ScreenRec vgaScreenRec = {
	0,			/* int			myNum */
	0,			/* ATOM id */
	MAX_COLUMN + 1,		/* short		width */
	MAX_ROW	+ 1,		/* short		height */
	((MAX_COLUMN+1)*254 )/800,	/* short		mmWidth */
	((MAX_ROW+1)*254)/800,	/* short		mmHeight */
	sizeof vgaDepths/sizeof (DepthRec),	/* short	numDepths */
	vgaDepths,		/* DepthPtr       	allowedDepths */
	VGA_MAXPLANES,		/* short       	rootDepth */
	0,			/* unsigned long      	rootVisual */
	0,			/* unsigned long	defColormap */
	1,			/* short		minInstalledCmaps */
	1,			/* short		maxInstalledCmaps */
	Always,			/* char                backingStoreSupport */
	NotUseful,		/* char                saveUnderSupport */
	VGA_WHITE_PIXEL,	/* unsigned long	whitePixel */
	VGA_BLACK_PIXEL,	/* unsigned long	blackPixel */
	0,			/* unsigned long	rgf */
	{ 0 },			/* GCPtr	GCperDepth[MAXFORMATS+1] */
	{ 0 },			/* PixmapPtr		PixmapPerDepth[1] */
	(pointer) &vgaScrnPriv,	/* pointer		devPrivate */
	sizeof vgaVisuals/sizeof (VisualRec), /* short       	numVisuals */
	&vgaVisuals[0],		/* VisualPtr		visuals */
/* Random screen procedures */
	vgaScreenClose,		/* Bool (* CloseScreen)() */
	ppcQueryBestSize,	/* void (* QueryBestSize)() */
	ibmSaveScreen,		/* Bool (* SaveScreen)() */
	miGetImage,		/* void (* GetImage)() */
	(unsigned int *(*)()) ppcGetSpans, /* unsigned int  *(* GetSpans)() */
	0,			/* void (* PointerNonInterestBox)() */
/* Window Procedures */
	ppcCreateWindowForXYhardware,	/* Bool (* CreateWindow)() */
	ppcDestroyWindow,	/* Bool (* DestroyWindow)() */
	ppcPositionWindow,	/* Bool (* PositionWindow)() */
	ppcChangeWindowAttributes,	/* Bool (* ChangeWindowAttributes)() */
	mfbMapWindow,		/* Bool (* RealizeWindow)() */
	mfbUnmapWindow,		/* Bool (* UnrealizeWindow)() */
	miValidateTree,		/* int  (* ValidateTree)() */
	miWindowExposures,	/* void (* WindowExposures)() */
/* Pixmap procedures */
	ppcCreatePixmap,	/* PixmapPtr (* CreatePixmap)() */
	mfbDestroyPixmap,	/* Bool (* DestroyPixmap)() */
/* Font procedures */
	mfbRealizeFont,		/* Bool (* RealizeFont)() */
	mfbUnrealizeFont,	/* Bool (* UnrealizeFont)() */
/* Cursor Procedures */
	OS_ConstrainCursor,	/* void (* ConstrainCursor)() */
	OS_CursorLimits,	/* void (* CursorLimits)() */
	vgaDisplayCursor,	/* Bool (* DisplayCursor)() */
	ppcRealizeCursor,	/* Bool (* RealizeCursor)() */
	ppcUnrealizeCursor,	/* Bool (* UnrealizeCursor)() */
	miRecolorCursor,	/* void (* RecolorCursor)() */
	OS_SetCursorPosition,	/* Bool (* SetCursorPosition)() */
/* GC procedures */
	vgaCreateGC,		/* Bool (* CreateGC)() */
/* Colormap procedures */
	NoopDDA,		/* void (* CreateColormap)() */
	NoopDDA,		/* void (* DestroyColormap)() */
	ppcInstallColormap,	/* void (* InstallColormap)() */
	ppcUninstallColormap,	/* void (* UninstallColormap)() */
	ppcListInstalledColormaps,	/* int (* ListInstalledColormaps) () */
	ppcStoreColors,		/* void (* StoreColors)() */
	ppcResolveColor,	/* void (* ResolveColor)() */
/* Region procedures */
	miRegionCreate,		/* RegionPtr (* RegionCreate)() */
	miRegionCopy,		/* void (* RegionCopy)() */
	miRegionDestroy,	/* void (* RegionDestroy)() */
	miIntersect,		/* int (* Intersect)() */
	miUnion,		/* int (* Union)() */
	miSubtract,		/* int (* Subtract)() */
	miInverse,		/* int (* Inverse)() */
	miRegionReset,		/* void (* RegionReset)() */
	miTranslateRegion,	/* void (* TranslateRegion)() */
	miRectIn,		/* int (* RectIn)() */
	miPointInRegion,	/* Bool (* PointInRegion)() */
	miRegionNotEmpty,	/* Bool (* RegionNotEmpty)() */
	miRegionEmpty,		/* void (* RegionEmpty)() */
	miRegionExtents,	/* BoxPtr (*RegionExtents)() */
	miSendGraphicsExpose,	/* void	(*SendGraphicsExpose)() */
/* os layer procedures */
	NoopDDA,		/* void (* BlockHandler)() */
	NoopDDA,		/* void (* WakeupHandler)() */
	(pointer) 0,		/* pointer blockData */
	(pointer) 0		/* pointer wakeupData */
} ;

#define vgaGCInterestValidateMask \
( GCLineStyle | GCLineWidth | GCJoinStyle | GCBackground | GCForeground	\
| GCFunction | GCPlaneMask | GCFillStyle | GC_CALL_VALIDATE_BIT		\
| GCClipXOrigin | GCClipYOrigin | GCClipMask | GCSubwindowMode )

GCInterestRec vgaPrototypeGCInterest = {
	0,			/*  struct _GCInterest	*pNextGCInterest */
	0,			/*  struct _GCInterest	*pLastGCInterest */
	sizeof (GCInterestRec), /*  int		length	 */
	0,			/*  ATOM	owner		 */
	vgaGCInterestValidateMask,	/*  unsigned long ValInterestMask */
	vgaValidateGC,		/*  void	(* ValidateGC) () */
	0,			/*  unsigned long	ChangeInterestMask */
	(int (*)()) 0,		/*  int			(* ChangeGC) ()	 */
	(void (*)()) 0,		/*  void		(* CopyGCSource) () */
	mfbCopyGCDest,		/*  void		(* CopyGCDest) () */
	ppcDestroyGC,		/*  void		(* DestroyGC) () */
	0			/*  pointer		extPriv		 */
} ;

ppcPrivGC vgaPrototypeGCPriv = {
	GXcopy,	/* unsigned char	rop */
	0,	/* unsigned char	ropOpStip */
	0,	/* unsigned char	ropFillArea */
	TRUE,	/* short	fExpose */
	FREE_CC,	/* short	freeCompClip */
	NullPixmap,	/* PixmapPtr	pRotatedTile */
	NullPixmap,	/* PixmapPtr	pRotatedStipple */
	0,	/* RegionPtr	pAbsClientRegion */
	0,	/* RegionPtr	pCompositeClip */
	ppcAreaFill,	/* void 	(* FillArea)() */
	&BogusPixmap,	/* PixmapPtr   *ppPixmap */
		{
		    VGA_ALLPLANES,	/* unsigned long	planemask */
		    1,			/* unsigned long	fgPixel */
		    0,			/* unsigned long	bgPixel */
		    GXcopy,		/* int			alu */
		    FillSolid,		/* int			fillStyle */
		}, /* ppcReducedRrop	colorRrop  */
	-1,	/* short lastDrawableType */
	-1,	/* short lastDrawableDepth */
	0	/* pointer devPriv */
} ;

GC vgaPrototypeGC = {
	&vgaScreenRec,		/*  ScreenPtr	pScreen	 */
	(pointer) &vgaPrototypeGCPriv,	/*  pointer	devPriv	 */
	(pointer) NULL,		/*  pointer	devBackingStore */
	VGA_MAXPLANES,		/*  int         depth	 */
	0,			/*  unsigned long        serialNumber */
	0,			/*  GCInterestPtr	pNextGCInterest */
	0,			/*  GCInterestPtr	pLastGCInterest */
	GXcopy,			/*  int		alu	 */
	VGA_ALLPLANES,		/*  unsigned long	planemask */
	VGA_BLACK_PIXEL,	/*  unsigned long	fgPixel */
	VGA_WHITE_PIXEL,	/*  unsigned long	bgPixel */
	0,			/*  int		lineWidth */
	LineSolid,		/*  int		lineStyle */
	CapButt,		/*  int		capStyle */
	JoinMiter,		/*  int		joinStyle */
	FillSolid,		/*  int		fillStyle */
	EvenOddRule,		/*  int		fillRule */
	ArcPieSlice,		/*  int		arcMode	 */
	0,			/*  PixmapPtr	tile	 */
	0,			/*  PixmapPtr	stipple	 */
	{ 0, 0 },		/*  DDXPointRec	patOrg	 */
	0,			/*  FontPtr	font	 */
	ClipByChildren,		/*  int		subWindowMode */
	TRUE,			/*  Bool	graphicsExposures */
	{ 0, 0 },		/*  DDXPointRec	clipOrg	 */
	NULL,			/*  pointer	clientClip */
	CT_NONE,		/*  int		clientClipType */
	0,			/*  int		dashOffset */
	0,			/*  int		numInDashList */
	0,			/*  unsigned char *dash	 */
	(1<<(GCLastBit+1))-1,	/*  unsigned long	stateChanges */
	{ 0, 0 },		/*  DDXPointRec	lastWinOrg */
	1,			/*  int		miTranslate:1 */
	ppcSolidFS,		/*  void (* FillSpans)() */
	ppcSetSpans,		/*  void (* SetSpans)()	 */
	miPutImage,		/*  void (* PutImage)()	 */
	ppcCopyArea,		/*  RegionPtr (* CopyArea)()	 */
	miCopyPlane,		/*  void (* CopyPlane)() */
	ppcPolyPoint,		/*  void (* PolyPoint)() */
	ppcScrnZeroLine,	/*  void (* Polylines)() */
	miPolySegment,		/*  void (* PolySegment)() */
	miPolyRectangle,	/*  void (* PolyRectangle)() */
	miPolyArc,		/*  void (* PolyArc)()	 */
	miFillPolygon,		/*  void (* FillPolygon)() */
	miPolyFillRect,		/*  void (* PolyFillRect)() */
	miPolyFillArc,		/*  void (* PolyFillArc)() */
	miPolyText8,		/*  int (* PolyText8)()	 */
	miPolyText16,		/*  int (* PolyText16)() */
	miImageText8,		/*  void (* ImageText8)() */
	miImageText16,		/*  void (* ImageText16)() */
	(void (*)()) ppcImageGlyphBlt,	/*  void (* ImageGlyphBlt)() */
	(void (*)()) ppcPolyGlyphBlt,	/*  void (* PolyGlyphBlt)() */
	miPushPixels,		/*  void (* PushPixels)() */
	miMiter,		/*  void (* LineHelper)() */
	mfbChangeClip,		/*  void (* ChangeClip) () */
	mfbDestroyClip,		/*  void (* DestroyClip) () */
	0			/*  void (* CopyClip)()	 */
} ;
