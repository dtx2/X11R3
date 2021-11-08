/*
 *      $Header: PEX.h,v 1.31 88/08/08 16:49:46 todd Exp $
 */

/* Definitions for PEX graphics extension likely to be used by applications */

#ifndef PEX_H
#define PEX_H

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
#define PEX_PROTOCOL      3              /* current protocol version */
#define PEX_PROTOCOL_REVISION 0          /* current minor version */


/* Asf Attributes */

/* Masks for setting Asf's */
#define MarkerTypeAsf		(1L<<0)
#define MarkerScaleAsf		(1L<<1)
#define MarkerColorAsf		(1L<<2)
#define TextFontIndexAsf	(1L<<3)
#define TextPrecAsf		(1L<<4)
#define CharExpansionAsf	(1L<<5)
#define CharSpacingAsf		(1L<<6)
#define TextColorAsf		(1L<<7)
#define LineTypeAsf		(1L<<8)
#define LineWidthAsf		(1L<<9)
#define LineColorAsf		(1L<<10)
#define CurveApproxAsf		(1L<<11)
#define PolylineInterpAsf	(1L<<12)
#define InteriorStyleAsf	(1L<<13)
#define InteriorStyleIndexAsf	(1L<<14)
#define SurfaceColorAsf		(1L<<15)
#define SurfaceInterpAsf	(1L<<16)
#define ReflectionModelAsf	(1L<<17)
#define ReflectionAttrAsf	(1L<<18)
#define BfInteriorStyleAsf	(1L<<19)
#define BfInteriorStyleIndexAsf (1L<<20)
#define BfSurfaceColorAsf	(1L<<21)
#define BfSurfaceInterpAsf	(1L<<22)
#define BfReflectionModelAsf	(1L<<23)
#define BfReflectionAttrAsf	(1L<<24)
#define SurfaceApproxAsf	(1L<<25)
#define TrimCurveApproxAsf	(1L<<26)
#define SurfaceEdgesAsf		(1L<<27)
#define SurfaceEdgeTypeAsf	(1L<<28)
#define SurfaceEdgeWidthAsf	(1L<<29)
#define SurfaceEdgeColorAsf	(1L<<30)
#define MaxAsfShift	31

/* Asf Values */
#define Bundled		0
#define Individual	1


/* color types  */
#define Indexed		0
#define Direct		1

/* Composition */
#define PreConcatenate		0
#define PostConcatenate		1
#define Replace			2

/* Cull mode */
/* 0 None */
#define BackFaces	1
#define	FrontFaces	2

/* Curve Type  and Surface Type */
#define Rational	0
#define NonRational	1

/* Display State */
#define Empty		0
#define NotEmpty	1

/* dynamic types */
#define IMM	0
#define IRG	1
#define CBS	2

/* Edit Mode */
#define StructureInsert		0
#define StructureReplace	1 

/* Whence values */
#define	Beginning	0
#define	Current		1
#define	End		2

#define ETIndex		1
#define ETMnemonic	2

/* Enum Types */
#define ETMarkerType		 1
#define ETATextStyle		 2
#define ETInteriorStyle		 3
#define ETHatchStyle		 4
#define ETLineType		 5
#define ETSurfaceEdgeType	 6
#define ETPickDeviceType	 7
#define ETPolylineInterpMethod	 8
#define ETCurveApproxMethod	 9
#define ETCurveBasis		10
#define ETReflectionModel	11
#define ETSurfaceInterpMethod	12
#define ETSurfaceApproxMethod	13
#define ETSurfaceBasis		14
#define ETModelClipOperator	15
#define ETLightType		16
#define ETDirectColorFormat	17
#define ETFloatFormat		18
#define ETHlhsrMode		19
#define ETPromptEchoType	20
#define ETDisplayUpdateMode	21

/* renderer state */
#define Idle 0
#define Rendering 1

#define Off	0
#define On	1

/* Shape */
/* Complex, Nonconvex, Convex, are defined  as 0, 1, 2 in X.h */
#define Unknown		3

/* Table Type */
#define LineBundle	 1
#define MarkerBundle	 2
#define TextBundle	 3
#define InteriorBundle	 4
#define EdgeBundle	 5
#define Pattern		 6
#define TextFont	 7
#define Color		 8
#define View		 9
#define Light		10
#define DepthCue	11
#define RgbApprox	12
#define IntensityApprox	13
#define maxTableType	14

/*  status in GetTableEntry */
#define Default		0
#define Defined		1

/* constants for Horizontal, Vertical alignment and Path */
#define Normal		0
#define Left		1
#define Right		2
#define	Center		3
#define Up		0
#define	Down		3
#define Top		1
#define Cap		2
#define Half		3
#define Base		4
#define Bottom		5

/* Text precision */
#define	String	0
#define Char	1
#define Stroke 	2

/* Update state */
#define NotPending	0
#define Pending		1

#define Correct		0
#define	Deferred	1
#define Simulated	2

/* Geometric attributes */
#define GAColor		0x0001
#define GANormal	0x0002
#define GAEdges		0x0004

#define NoPick	0
#define Ok	1

#define NoEcho	0
#define Echo	1

#define DeleteRefs      0
#define LeaveRefs       1
#define Orphans         0
#define All             1
#define TopFirst        0
#define BottomFirst     1
#define Forward         0
#define Backward        1
#define Add             0
#define Remove          1
/* 	Replace		2	already defined */
#define Higher          0
#define Lower           1

/* Enumerated Type Descriptors */

/* Marker Type */
#define MarkerDot		1
#define MarkerCross		2
#define MarkerAsterisk		3
#define MarkerCircle		4
#define MarkerX			5
/* ATextStyle */
#define ATextNotConnected	1
#define ATextConnected		2
/* InteriorStyle */
#define InteriorStyleHollow	1
#define InteriorStyleSolid	2
#define	InteriorStylePattern	3
#define	InteriorStyleHatch	4
#define InteriorStyleEmpty	5
/* HatchStyle */
/* LineType */
#define LineTypeSolid		1
#define LineTypeDashed		2
#define LineTypeDotted		3
#define LineTypeDashDot		4
/* SurfaceEdgeType */
#define SurfaceEdgeSolid	1
#define SurfaceEdgeDashed	2
#define SurfaceEdgeDotted	3
#define SurfaceEdgeDashDot	4
/* PickDeviceType */
/* PolylineInterpMethod */
#define PolylineInterpNone	1
#define PolylineInterpColor	2
/* Curve(and Surface)ApproxMethod */
#define ApproxConstant				1
#define ApproxConstantBetweenKnots		2
#define ApproxWcsMetric				3
#define ApproxNpcMetric				4
#define CurveApproxWcsChordalDev		5
#define CurveApproxNpcChordalDev		6
#define SurfaceApproxWcsPlanarDev		5
#define SurfaceApproxNpcPlanarDev		6
/* CurveBasis */
#define CurveBasisUniformBSpline	1
#define CurveBasisPiecewiseBezier	2
/* ReflectionModel */
#define ReflectionNoShading	1
#define ReflectionAmbient	2
#define ReflectionDiffuse	3
#define ReflectionSpecular	4
/* SurfaceInterpMethod */
#define SurfaceInterpNone	1
#define SurfaceInterpColor	2
#define SurfaceInterpDotProduct	3
#define SurfaceInterpNormal	4
/* SurfaceBasis */
#define SurfaceBasisUniformBSpline	1
#define SurfaceBasisPiecewiseBezier 	2
/* ModelClipOperator */
#define ModelClipReplace	1
#define ModelClipIntersection	2
/* LightType */
#define LightAmbient		1
#define LightWcsVector		2
#define LightWcsPoint		3
#define LightWcsSpot		4
/* DirectColorFormat */
#define RgbFloat	1
#define CieFloat	2
#define HsvFloat	3
#define HlsFloat	4
#define RgbInt8		5
#define RgbInt16	6
#define maxColorType	7
/* FloatFormat */
#define Ieee_754_32	1
#define DEC_F_Floating	2
/* HlhsrMode */
#define HlhsrOff		1
#define HlhsrZBuffer		2
#define HlhsrPainters		3
#define HlhsrScanline		4
#define HlhsrHiddenLineOnly	5
/* PromptEchoType */
#define EchoPrimitive	1
#define EchoStructure	2
#define EchoNetwork	3
/* DisplayUpdateMethod */
#define VisualizeEach		1
#define VisualizeEasy		2
#define VisualizeNone		3
#define SimulateSome		4
#define VisualizeWhenever	5

#define Owner                   (1<<0)
#define Version                 (1<<1)
#define Type                    (1<<2)

#define PEXClipXY 1
#define PEXClipBack 2
#define PEXClipFront 4
#define PEXClippingOn 7


/* Implementation Dependent Constant Names */
#define IDChromaticityLuminance		1
#define IDChromaticityX			2
#define IDChromaticityY			3
#define IDDitheringSupported		4
#define IDMaxEdgeWidth			5
#define IDMaxLineWidth			6
#define IDMaxMarkerSize			7
#define IDMaxModelClipPlanes		8
#define IDMaxNameSetNames		9
#define IDMaxNonAmbientLights		10
#define IDMaxNURBOrder			11
#define IDMaxTrimCurveOrder		12
#define IDMaxUPPOrder			13
#define IDMinEdgeWidth			14
#define IDMinLineWidth			15
#define IDMinMarkerSize			16
#define IDNominalEdgeWidth		17
#define IDNominalLineWidth		18
#define IDNominalMarkerSize		19
#define IDNumSupportedEdgeWidths	20
#define IDNumSupportedLineWidths	21
#define IDNumSupportedMarkerSizes	22
#define IDRgbBestApproximation		23
#define IDTransparencySupported		24

/* Constants for IDRgbBestApproximation */
#define RgbApproxAnyValues	0
#define RgbApproxPowersOf2	1

#define PCAllValues		0
#define	PCMarkerType		1
#define	PCMarkerScale		2
#define	PCMarkerColor		3
#define PCMarkerBundleIndex	4
#define PCTextFont		5
#define PCTextPrecision		6
#define	PCCharExpansion		7
#define	PCCharSpacing		8
#define	PCTextColor		9
#define	PCCharHeight		10
#define	PCCharUpVector		11
#define PCTextPath		12
#define PCTextAlignment		13
#define	PCAtextHeight		14
#define	PCAtextUpVector		15
#define PCAtextPath		16
#define PCAtextAlignment	17
#define	PCAtextStyle		18
#define PCTextBundleIndex	19
#define	PCLineType		20
#define	PCLineWidth		21
#define	PCLineColor		22
#define	PCCurveApprox		23
#define	PCPolylineInterp	24
#define PCLineBundleIndex	25
#define	PCInteriorStyle		26
#define PCInteriorStyleIndex	27
#define	PCSurfaceColor		28
#define	PCReflectionAttr	29
#define	PCReflectionModel	30
#define	PCSurfaceInterp		31
#define	PCBfInteriorStyle	32
#define PCBfInteriorStyleIndex	33
#define	PCBfSurfaceColor	34
#define	PCBfReflectionAttr	35
#define	PCBfReflectionModel	36
#define	PCBfSurfaceInterp	37
#define	PCSurfaceApprox		38
#define PCTrimCurveApprox	39
#define PCCullingMode		40
#define PCDistinguish		41
#define PCNormalFlip		42
#define	PCPatternSize		43
#define	PCPatternRefPt		44
#define	PCPatternRefVec1	45
#define	PCPatternRefVec2	46
#define PCInteriorBundleIndex	47
#define PCSurfaceEdgeFlag	48
#define	PCSurfaceEdgeType	49
#define	PCSurfaceEdgeWidth	50
#define	PCSurfaceEdgeColor	51
#define PCEdgeBundleIndex	52
#define	PCLocalTransform	53
#define	PCGlobalTransform	54
#define PCModelClip		55
#define PCModelClipVolume	56
#define PCViewIndex		57
#define PCLightState		58
#define PCDepthCueIndex		59
#define PCPickId		60
#define PCHlhsrIdentifier	61
#define PCNameSet		62
#define PCAsfValues		63
#define MaxPCIndex	63	/* XXX |||  should be 64 */

#define RDPipelineContext	(1L<<0)
#define RDCurrentPath		(1L<<1)
#define RDMarkerBundle		(1L<<2)
#define RDTextBundle		(1L<<3)
#define RDLineBundle		(1L<<4)
#define RDInteriorBundle	(1L<<5)
#define RDEdgeBundle		(1L<<6)
#define RDViewTable		(1L<<7)
#define RDColorTable		(1L<<8)
#define RDDepthCueTable		(1L<<9)
#define RDLightTable		(1L<<10)
#define RDColorApproxTable	(1L<<11)
#define RDPatternTable		(1L<<12)
#define RDTextFontTable		(1L<<13)
#define RDHighlightIncl		(1L<<14)
#define RDHighlightExcl		(1L<<15)
#define RDInvisibilityIncl	(1L<<16)
#define RDInvisibilityExcl	(1L<<17)
#define RDRendererState		(1L<<18)
#define RDHlhsrMode		(1L<<19)
#define RDNpcSubvolume		(1L<<20)
#define RDViewport		(1L<<21)
#define MaxRDShift	22

/*
 * defines for output commands
 */

#define OCAll			0L
#define	OCMarkerType		1L
#define	OCMarkerScale		2L
#define	OCMarkerColorIndex	3L
#define	OCMarkerColor		4L
#define OCMarkerBundleIndex	5L
#define OCTextFontIndex		6L
#define OCTextPrecision		7L
#define	OCCharExpansion		8L
#define	OCCharSpacing		9L
#define	OCTextColorIndex	10L
#define	OCTextColor		11L
#define	OCCharHeight		12L
#define	OCCharUpVector		13L
#define OCTextPath		14L
#define OCTextAlignment		15L
#define	OCAtextHeight		16L
#define	OCAtextUpVector		17L
#define OCAtextPath		18L
#define OCAtextAlignment	19L
#define	OCAtextStyle		20L
#define OCTextBundleIndex	21L
#define	OCLineType		22L
#define	OCLineWidth		23L
#define	OCLineColorIndex	24L
#define	OCLineColor		25L
#define	OCCurveApprox		26L
#define	OCPolylineInterp	27L
#define OCLineBundleIndex	28L
#define	OCInteriorStyle		29L
#define OCInteriorStyleIndex	30L
#define	OCSurfaceColorIndex	31L
#define	OCSurfaceColor		32L
#define	OCReflectionAttr	33L
#define	OCReflectionModel	34L
#define	OCSurfaceInterp		35L
#define	OCBfInteriorStyle	36L
#define OCBfInteriorStyleIndex	37L
#define	OCBfSurfaceColorIndex	38L
#define	OCBfSurfaceColor	39L
#define	OCBfReflectionAttr	40L
#define	OCBfReflectionModel	41L
#define	OCBfSurfaceInterp	42L
#define	OCSurfaceApprox		43L
#define OCTrimCurveApprox	44L
#define OCCullingMode		45L
#define OCDistinguish		46L
#define OCNormalFlip		47L
#define	OCPatternSize		48L
#define	OCPatternRefPt		49L
#define	OCPatternAttr		50L
#define OCInteriorBundleIndex	51L
#define OCSurfaceEdgeFlag	52L
#define	OCSurfaceEdgeType	53L
#define	OCSurfaceEdgeWidth	54L
#define	OCSurfaceEdgeColorIndex	55L
#define	OCSurfaceEdgeColor	56L
#define OCEdgeBundleIndex	57L
#define OCAsfValues		58L
#define	OCLocalTransform	59L
#define	OCLocalTransform2D	60L
#define	OCGlobalTransform	61L
#define	OCGlobalTransform2D	62L
#define OCModelClip		63L
#define OCModelClipVolume	64L
#define OCModelClipVolume2D	65L
#define OCRestoreModelClip	66L
#define OCViewIndex		67L
#define OCLightState		68L
#define OCDepthCueIndex		69L
#define OCPickId		70L
#define OCHLHSRIdentifier	71L
#define OCAddToNameSet		72L
#define OCRemoveFromNameSet	73L
#define OCExecuteStructure	74L
#define OCLabel			75L
#define OCApplicationData	76L
#define OCGse			77L
#define OCMarkers		78L
#define OCMarkers2D		79L
#define OCText			80L
#define OCText2D		81L
#define OCAnnotationText	82L
#define OCAnnotationText2D	83L
#define OCPolyline		84L
#define OCPolyline2D		85L
#define OCPolylineSet		86L
#define OCParametricCurve	87L
#define OCNurbCurve		88L
#define OCFillArea		89L
#define OCFillArea2D		90L
#define OCExtFillArea		91L
#define OCFillAreaSet		92L
#define OCFillAreaSet2D		93L
#define OCExtFillAreaSet	94L
#define OCTriangleStrip		95L
#define OCQuadrilateralMesh	96L
#define OCIndexedPolygons	97L
#define OCParametricSurface	98L
#define OCNurbSurface		99L
#define OCCellArray		100L
#define OCCellArray2D		101L
#define OCExtCellArray		102L
#define OCGdp			103L
#define OCGdp2D			104L
#define MaxPEXOC		105

#define ElementType		(1L<<0)
#define ElementSize		(1L<<1)
#define ElementData		(1L<<2)

#define SCPosition              (1L<<0)
#define SCDistance              (1L<<1)
#define SCCeiling               (1L<<2)
#define SCStartPath             (1L<<3)
#define SCInclList              (1L<<4)
#define SCExclList              (1L<<5)

#define NotFound	1
#define Found		2

/* Phigs Workstation Attributes */
#define PWDisplayUpdate		(1L<<0)
#define PWVisualState		(1L<<1)
#define PWDisplaySurface	(1L<<2)
#define PWViewUpdate		(1L<<3)
#define PWDefinedViews		(1L<<4)
#define PWWksUpdate		(1L<<5)
#define PWReqNpcSubvolume	(1L<<6)
#define PWCurNpcSubvolume	(1L<<7)
#define PWReqWksViewport	(1L<<8)
#define PWCurWksViewport	(1L<<9)
#define PWHlhsrUpdate		(1L<<10)
#define PWReqHlhsrMode		(1L<<11)
#define PWCurHlhsrMode		(1L<<12)
#define PWDrawable		(1L<<13)
#define PWLineBundle		(1L<<14)
#define PWMarkerBundle		(1L<<15)
#define PWTextBundle		(1L<<16)
#define PWInteriorBundle	(1L<<17)
#define PWEdgeBundle		(1L<<18)
#define PWColorTable		(1L<<19)
#define PWPatternTable		(1L<<20)
#define PWTextFontTable		(1L<<21)
#define PWDepthCueTable		(1L<<22)
#define PWLightTable		(1L<<23)
#define PWColorApproxTable	(1L<<24)
#define PWHighlightIncl		(1L<<25)
#define PWHighlightExcl		(1L<<26)
#define PWInvisibilityIncl	(1L<<27)
#define PWInvisibilityExcl	(1L<<28)
#define PWPostedStructures	(1L<<29)

/* Dynamics */
#define PWDViewRep		(1L<<0)
#define PWDLineBundle		(1L<<1)
#define PWDMarkerBundle		(1L<<2)
#define PWDTextBundle		(1L<<3)
#define PWDInteriorBundle	(1L<<4)
#define PWDEdgeBundle		(1L<<5)
#define PWDPatternTable		(1L<<6)
#define PWDColorTable		(1L<<7)
#define PWDWksTransform		(1L<<8)
#define PWDHighlightFilter	(1L<<9)
#define PWDInvisibilityFilter	(1L<<10)
#define PWDHlhsrMode		(1L<<11)
#define PWDStructureModify	(1L<<12)
#define PWDPostStructure	(1L<<13)
#define PWDUnpostStructure	(1L<<14)
#define PWDDeleteStructure	(1L<<15)
#define PWDReferenceModify	(1L<<16)

#define PickStatus              (1L<<0)
#define PickPath                (1L<<1)
#define PickPathOrder           (1L<<2)
#define PickIncl            	(1L<<3)
#define PickExcl            	(1L<<4)
#define PickDataRec             (1L<<5)
#define PickPromptEchoType      (1L<<6)
#define PickEchoVolume          (1L<<7)
#define PickEchoSwitch          (1L<<8)

#define PMStatus                (1L<<0)
#define PMPath                  (1L<<1)

/* Errors */
#define DirectColorFormatError		 0
#define EditingContextError		 1
#define FloatingPointFormatError	 2
#define LabelError			 3
#define LookupTableError		 4
#define NameSetError			 5
#define PathError			 6
#define PEXfontError			 7
#define PhigsWKSError			 8
#define PickMeasureError		 9
#define PipelineContextError		10
#define RendererError			11
#define SearchContextError		12
#define StructureError			13
#define MaxPEXError			13 /* should be 14 ||| XXX */
/* Requests */

#define PEX_GetExtensionInfo		 1
#define PEX_GetEnumeratedTypeInfo	 2
#define PEX_GetImpDepConstants		 3
#define PEX_CreateLookupTable		 4
#define PEX_CopyLookupTable		 5
#define PEX_FreeLookupTable		 6
#define PEX_GetTableInfo		 7
#define PEX_GetPredefinedEntries	 8
#define PEX_GetDefinedIndices		 9
#define PEX_GetTableEntry		10
#define PEX_GetTableEntries		11
#define PEX_SetTableEntries		12
#define PEX_DeleteTableEntries		13
#define PEX_CreatePipelineContext	14
#define PEX_CopyPipelineContext		15
#define PEX_FreePipelineContext		16
#define PEX_GetPipelineContext		17
#define PEX_ChangePipelineContext	18
#define PEX_CreateRenderer		19
#define PEX_FreeRenderer		20
#define PEX_ChangeRenderer		21
#define PEX_GetRendererAttributes	22
#define PEX_BeginRendering		23
#define PEX_EndRendering		24
#define PEX_BeginStructure		25
#define PEX_EndStructure		26
#define PEX_RenderOutputCommands	27
#define PEX_RenderNetwork		28
#define PEX_CreateStructure		29
#define PEX_CopyStructure		30
#define PEX_FreeStructure		31
#define PEX_DeleteStructures		32
#define PEX_GetStructureInfo		33
#define PEX_GetElementInfo		34
#define PEX_GetStructuresInNetwork	35
#define PEX_GetAncestors		36
#define PEX_GetDescendants		37
#define PEX_FetchElements		38
#define PEX_SetEditingMode		39	
#define PEX_SetElementPointer		40
#define PEX_SetElementPointerAtLabel	41
#define PEX_ElementSearch		42
#define PEX_StoreElements		43
#define PEX_DeleteElements		44
#define PEX_DeleteElementsToLabel	45
#define PEX_DeleteBetweenLabels		46
#define PEX_CopyElements		47
#define PEX_ChangeStructureRefs		48
#define PEX_CreateNameSet		49
#define PEX_CopyNameSet			50
#define PEX_FreeNameSet			51
#define PEX_GetNameSet			52
#define PEX_ChangeNameSet		53
#define PEX_CreateSearchContext		54
#define PEX_CopySearchContext		55
#define PEX_FreeSearchContext		56
#define PEX_GetSearchContext		57
#define PEX_ChangeSearchContext		58
#define PEX_SearchNetwork		59
#define PEX_CreatePhigsWKS		60
#define PEX_FreePhigsWKS		61
#define PEX_GetWKSInfo			62
#define PEX_GetDynamics			63
#define PEX_GetViewRep			64
#define PEX_RedrawAllStructures		65
#define PEX_UpdateWorkstation		66
#define PEX_ExecuteDeferredActions	67
#define PEX_SetViewPriority		68
#define PEX_SetDisplayUpdateMode	69
#define PEX_MapDCtoWC			70
#define PEX_MapWCtoDC			71
#define PEX_SetViewRep			72
#define PEX_SetWKSWindow		73
#define PEX_SetWKSViewport		74
#define PEX_SetHLHSRMode		75
#define PEX_PostStructure		76
#define PEX_UnpostStructure		77
#define PEX_UnpostAllStructures		78
#define PEX_GetWKSPostings		79
#define PEX_GetPickDevice		80
#define PEX_ChangePickDevice		81
#define PEX_CreatePickMeasure		82
#define PEX_FreePickMeasure		83
#define PEX_GetPickMeasure		84
#define PEX_UpdatePickMeasure		85
#define PEX_OpenFont			86
#define PEX_CloseFont			87
#define PEX_QueryFont			88
#define PEX_ListFonts			89
#define PEX_ListFontsWithInfo		90
#define PEX_QueryTextExtents 		91
#define MaxPEXCommand			92
#endif /* PEX.h */

