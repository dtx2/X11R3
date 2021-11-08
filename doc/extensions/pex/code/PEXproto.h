/*
 *      $Header: PEXproto.h,v 1.44 88/09/27 15:36:18 todd Exp $
 */

/* Definitions for the PEX used by server and c bindings */

/*
 * This packet-construction scheme makes the following assumptions:
 *
 * 1. The compiler is able
 * to generate code which addresses one- and two-byte quantities.
 * In the worst case, this would be done with bit-fields.  If bit-fields
 * are used it may be necessary to reorder the request fields in this file,
 * depending on the order in which the machine assigns bit fields to
 * machine words.  There may also be a problem with sign extension,
 * as K+R specify that bitfields are always unsigned.
 *
 * 2. 2- and 4-byte fields in packet structures must be ordered by hand
 * such that they are naturally-aligned, so that no compiler will ever
 * insert padding bytes.
 *
 * 3. All packets are hand-padded to a multiple of 4 bytes, for
 * the same reason.
 */

#ifndef PEXPROTO_H
#define PEXPROTO_H
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

/* In the following typedefs, comments appear that say "LISTof Foo(numItems)",
 * "CLIST of Foo()", and "SINGLE Foo()".   These are used when the protocol 
 * specifies that a request or reply contains a variable length list of 
 * (possibly variable types of) objects.
 *
 * A LISTof list is one for which we have already been given the length.
 * The items in the list are of type "Foo". The number of items in the list
 * appears parenthetically after the type.  ("numItems" in our example.)
 * Any other information needed to parse the list is also passed in the
 * parentheses. (E.g., "tableType" in a list of table entries.)
 *
 * A CLISTof list is the same, except that the first 4 bytes of the list
 * indicate the number of items in the list.  The length may need to be
 * byte-swapped.
 *
 * A SINGLE item of an indeterminate length is indicated in the same
 * manner.  (E.g., a "SINGLE TableEntry()". )
 *
 * (Where the protocol had  "LISTofVALUE",  we now have, for example, "SINGLE 
 * EditingContextValues", because this isn't a list of fixed size objects,
 * but one object that may have different fields present.) 
 *
 * I am working on a program to automatically generate byte swappers and 
 * length checkers from these header files.
 *
 * Todd Newman 3/88
 */
#include "PEXprotostr.h"

/****************************************************************
 *  		REPLIES 					*
 ****************************************************************/
typedef struct {
    BYTE	type;	/* X_Reply */
    CARD8	what;
    CARD16	sequenceNumber	B16;
    CARD32	length B32;	/* not 0 */
    CARD16	majorVersion B16;
    CARD16	minorVersion B16;
    CARD32	release B32;
    CARD32	lengthName B32;
    CARD32	subsetInfo B32;
    BYTE	pad[8];
    /* list of CARD8 follows -- Don't swap */
    } pexGetExtensionInfoReply;


typedef struct {
    BYTE	type;	/* X_Reply */
    CARD8	what;
    CARD16	sequenceNumber	B16;
    CARD32	length B32;	/* NOT 0; this is an extra-large reply */
    CARD32	numLists B32;
    BYTE	pad[20];		/* lists of lists begin afterwards */
    /* LISTof CListofEnumTypeDesc(numLists) */
    } pexGetEnumeratedTypeInfoReply;

typedef struct {
    BYTE	type;	/* X_Reply */
    CARD8	what;
    CARD16	sequenceNumber	B16;
    CARD32	length B32;	/* 0 */
    CARD16	tableType B16;
    CARD16	definableEntries B16;
    CARD16	numPredefined B16;
    INT16	predefinedMin B16;
    INT16	predefinedMax B16;
    BYTE	pad[14];
    } pexGetTableInfoReply;

typedef struct {
    BYTE	type;	/* X_Reply */
    CARD8	what;
    CARD16	sequenceNumber	B16;
    CARD32	length B32;	/* not 0 */
    CARD16	tableType B16;
    CARD16	unused B16;
    CARD32	numEntries B32;
    BYTE	pad[16];
    /* LISTof TableEntry(numEntries, tableType) */
    } pexGetPredefinedEntriesReply;

typedef struct {
    BYTE	type;	/* X_Reply */
    CARD8	what;
    CARD16	sequenceNumber	B16;
    CARD32	length B32;	/* not 0 */
    CARD32	numIndices B32;
    BYTE	pad[20];
    /* LISTof pexTableIndex(numIndices) */
    } pexGetDefinedIndicesReply;

typedef struct {
    BYTE	type;	/* X_Reply */
    CARD8	what;
    CARD16	sequenceNumber	B16;
    CARD32	length B32;	/* not 0 */
    CARD16	status B16;
    CARD16	tableType B16;
    BYTE	pad[20];
    /* SINGLE TableEntry(tableType)  */
    } pexGetTableEntryReply;

typedef struct {
    BYTE	type;	/* X_Reply */
    CARD8	what;
    CARD16	sequenceNumber	B16;
    CARD32	length B32;	/* not 0 */
    CARD16	tableType B16;
    CARD16	unused B16;
    CARD32	numEntries B32;
    BYTE	pad[16];
    /* LISTof TableEntry(numEntries, tableType) */
    } pexGetTableEntriesReply;

typedef struct {
    BYTE	type;	/* X_Reply */
    CARD8	what;
    CARD16	sequenceNumber	B16;
    CARD32	length B32;	/* not 0 */
    CARD32	numValues B32;
    CARD32	itemMask[2];
    BYTE	pad[12];
    /* SINGLE PipelineContextAttributes(itemMask)  */
    } pexGetPipelineContextReply;


typedef struct {
    BYTE	type;	/* X_Reply */
    CARD8	what;
    CARD16	sequenceNumber	B16;
    CARD32	length B32;	/* not 0 */
    CARD32	itemMask B32; 
    BYTE	pad[20];
    /* SINGLE RendererAttributes(itemMask) */
    } pexGetRendererAttributesReply;

typedef struct {
    BYTE	type;	/* X_Reply */
    CARD8	what;
    CARD16	sequenceNumber	B16;
    CARD32	length B32;	/* 0 */
    CARD16	editMode B16;
    CARD16	pad1	B16;
    CARD32	elementPtr B32;
    CARD32	numElements B32;
    CARD32	lengthStructure B32;
    CARD32	numRefs B32;
    BYTE	pad[4];
    } pexGetStructureInfoReply;

typedef struct {
    BYTE	type;	/* X_Reply */
    CARD8	what;
    CARD16	sequenceNumber	B16;
    CARD32	length B32;	/* not 0 */
    CARD32	numInfo B32;
    BYTE	pad[20];
    /* LISTof pexElementInfo(numInfo) */
    } pexGetElementInfoReply;

typedef struct {
    BYTE		type;	/* X_Reply */
    CARD8		what;
    CARD16		sequenceNumber	B16;
    CARD32		length B32;	/* not 0 */
    pexStructure	sid B32;
    CARD16		which B16;
    CARD16		pad1 B16;
    CARD32		numStructures B32;
    BYTE		pad[12];
    /* LISTof StructureID(numStructures)  */
    } pexGetStructuresInNetworkReply;

typedef struct {
    BYTE		type;	/* X_Reply */
    CARD8		what;
    CARD16		sequenceNumber	B16;
    CARD32		length B32;	/* not 0 */
    pexStructure	sid B32;
    CARD32		pathOrder B32;
    CARD32		pathDepth B32;
    CARD32		numPaths;
    BYTE		pad[8];
    /* LISTof CListofElementRef(numPaths) */
    } pexGetAncestorsReply;

typedef pexGetAncestorsReply pexGetDescendantsReply;

typedef struct {
    BYTE		type;	/* X_Reply */
    CARD8		what;
    CARD16		sequenceNumber	B16;
    CARD32		length B32;	/* not 0 */
    pexStructure	sid B32;
    CARD32		elementPtr B32;
    CARD32		editingMode B32;
    BYTE		pad[8];
    /* SINGLE EditingContextValues(length) */
    } pexGetEditingContextReply;

typedef struct {
    BYTE	type;	/* X_Reply */
    CARD8	what;
    CARD16	sequenceNumber	B16;
    CARD32	length B32;	/* 0 */
    CARD16	status B16;
    CARD16	unused B16;
    CARD32	foundOffset B32;
    BYTE	pad[16];
    } pexElementSearchReply;


typedef struct {
    BYTE	type;	/* X_Reply */
    CARD8	what;
    CARD16	sequenceNumber	B16;
    CARD32	length B32;	/* not 0 */
    CARD32	numElements B32;
    BYTE	pad[20];
    /* LISTof OutputCommand(numElements) */
    } pexFetchElementsReply;

typedef struct {
    BYTE	type;	/* X_Reply */
    CARD8	what;
    CARD16	sequenceNumber	B16;
    CARD32	length B32;	/* not 0 */
    CARD32	numNames B32;
    BYTE	pad[20];
    /* LISTof Name(numNames) */
    } pexGetNameSetReply;


typedef struct {
    BYTE	type;	/* X_Reply */
    CARD8	itemMask;
    CARD16	sequenceNumber	B16;
    CARD32	length B32;	/* not 0 */
    CARD32	searchContext B32;
    CARD32	numItems B32;
    pexCoord3D	searchPos;
    FLOAT	searchDist B32;
    /* SINGLE SearchContext(itemMask) */
    } pexGetSearchContextReply;

typedef struct {
    BYTE	type;	/* X_Reply */
    CARD8	what;
    CARD16	sequenceNumber	B16;
    CARD32	length B32;	/* not 0 */
    CARD32	searchContext B32;
    CARD32	numItems B32;
    BYTE	pad[16];
    /* LISTof ElementRef(numItems) */
    } pexSearchNetworkReply;


typedef struct {
    BYTE	type;	/* X_Reply */
    CARD8	what;
    CARD16	sequenceNumber	B16;
    CARD32	length B32;	/* not 0 */
    CARD32	wks B32;
    CARD32	itemMask B32;
    CARD32	numValues B32;
    BYTE	pad[12];
    /* SINGLE WksInfo(itemMask) */
    } pexGetWksInfoReply;


typedef struct {
    BYTE		type;	/* X_Reply */
    CARD8		what;
    CARD16		sequenceNumber	B16;
    CARD32		length B32;	/* 0 */
    pexDynamicType	viewRep;
    pexDynamicType	lineBundle;
    pexDynamicType	markerBundle;
    pexDynamicType	textBundle;
    pexDynamicType	interiorBundle;
    pexDynamicType	edgeBundle;
    pexDynamicType	patternTable;
    pexDynamicType	colorTable;
    pexDynamicType	wksTransform;
    pexDynamicType	highlightFilter;
    pexDynamicType	invisibilityFilter;
    pexDynamicType	HlhsrMode;
    pexDynamicType	structureModify;
    pexDynamicType	postStructure;
    pexDynamicType	deleteStructure;
    pexDynamicType	referenceModify;
    CARD8		pad[8];
    } pexGetDynamicsReply;

typedef struct {
    BYTE	type;	/* X_Reply */
    CARD8	what;
    CARD16	sequenceNumber	B16;
    CARD32	length B32;	/* 0 */
    CARD16	viewUpdate B16;	/* Pending, NotPending */ 
    CARD16	pad1 B16;
    BYTE	pad[20];
    /* SINGLE ViewRep() 	requested */
    /* SINGLE ViewRep() 	current */
    } pexGetViewRepReply;

typedef struct {
    BYTE	type;	/* X_Reply */
    CARD8	what;
    CARD16	sequenceNumber	B16;
    CARD32	length B32;	/* not 0 */
    CARD16	viewIndex B16;
    CARD16	pad1 B16;
    CARD32	numPoints B32;
    BYTE	pad[16];
    /* LISTof Coord3D(numPoints) */
    } pexMapPointsReply;

typedef struct {
    BYTE	type;	/* X_Reply */
    CARD8	what;
    CARD16	sequenceNumber	B16;
    CARD32	length B32;	/* not 0 */
    BYTE	pad[24];
    /* LISTof pexPhigsWksID(numIDs) */
    } pexGetWksPostingsReply;

typedef struct {
    BYTE	type;	/* X_Reply */
    CARD8	what;
    CARD16	sequenceNumber	B16;
    CARD32	length B32;	/* not 0 */
    CARD32	itemMask B32;
    CARD32	numItems B32;
    BYTE	pad[16];
    /* SINGLE PickDeviceAttributes(itemMask) */
    } pexGetPickDeviceReply;

typedef struct {
    BYTE	type;	/* X_Reply */
    CARD8	what;
    CARD16	sequenceNumber	B16;
    CARD32	length B32;	/* not 0 */
    CARD32	itemMask B32;
    CARD32	numItems B32;
    BYTE	pad[16];
    /* SINGLE pexPickMeasureAttributes(itemMask) */
    } pexGetPickMeasureReply;

typedef struct {
    BYTE	type;	/* X_Reply */
    CARD8	what;
    CARD16	sequenceNumber	B16;
    CARD32	length B32;	/* not 0 */
    CARD32	lengthFontInfo B32;
    CARD32	numCharInfo B32;
    CARD8	pad[16];
    /* SINGLE FontInfo() */
    /* LISTof PexCharInfo(numCharInfo) */
    } pexQueryFontReply;

typedef struct {
    BYTE	type;	/* X_Reply */
    CARD8	what;
    CARD16	sequenceNumber	B16;
    CARD32	length B32;	/* not 0 */
    CARD32	numStrings B32;
    BYTE	pad[20];
    /* LISTof String(numStrings) */
    } pexListFontsReply;

typedef struct {
    BYTE	type;	/* X_Reply */
    CARD8	what;
    CARD16	sequenceNumber	B16;
    CARD32	length B32;	/* not 0 */
    CARD32	numStrings B32;
    BYTE	pad[20];
    /* LISTof String(numStrings) */
    /* CLISTof PexFontData() */
    } pexListFontsWithInfoReply;

typedef struct {
    BYTE	type;	/* X_Reply */
    CARD8	what;
    CARD16	sequenceNumber	B16;
    CARD32	length B32;	/* 1 */
    CARD32	overallAscent B32;
    CARD32	overallDescent B32;
    CARD32	overallLeft B32;
    CARD32	overallRight B32;
    pexCoord2D	concatPoint;
    /* CARD32	overallWidth */
    } pexQueryTextExtentsReply;

typedef struct {
    BYTE	type;	/* X_Reply */
    CARD8	what;
    CARD16	sequenceNumber	B16;
    CARD32	length B32;	/* not 0 */
    CARD32	numValues B32;
    BYTE	pad[20];
    /* LISTof VALUE(numValues) */
    } pexGetImpDepConstantsReply;

/****************************************************************
 *  		REQUESTS 					*
 ****************************************************************/
/* Request structure */

typedef struct {
    CARD8	reqType;
    CARD8	opcode;            /* meaning depends on request type */
    CARD16	length B16;        
				/* length in 4 bytes quantities */
                                /* of whole request, including this header */
} pexReq;

/*****************************************************************
 *  structures that follow request.
 *****************************************************************/

/* ResourceReq is used for any request which has a resource ID
   (or Atom or Time) as its one and only argument.  */

typedef struct {
    CARD8	reqType;
    CARD8	opcode;
    CARD16	length B16;
    CARD32	id B32;  /* a Structure, Renderer, Font, Pixmap, etc. */
    } pexResourceReq;


/*****************************************************************
 *  Specific Requests 
 *****************************************************************/


typedef struct {
    CARD8	reqType;
    CARD8	opcode;
    CARD16	length B16;
    CARD16	clientProtocolMajor B16;
    CARD16	clientProtocolMinor B16;
} pexGetExtensionInfoReq;

typedef struct {
    CARD8	reqType;
    CARD8 	opcode;
    CARD16 	length B16;
    Drawable 	drawable B32;
    pexBitmask	itemMask B32;
    CARD32	numEnums B32;
} pexGetEnumeratedTypeInfoReq;


typedef struct {
    CARD8		reqType;
    CARD8 		opcode;
    CARD16 		length B16;
    Drawable		drawableExample B32;
    pexLookupTable	lut B32;
    pexTableType	tableType B16;
    CARD16		used B16;
} pexCreateLookupTableReq;


typedef struct {
    CARD8		reqType;
    CARD8		opcode;
    CARD16		length B16;
    pexLookupTable	src B32;
    pexLookupTable	dst B32;
} pexCopyLookupTableReq;

typedef pexResourceReq pexFreeLookupTableReq;

typedef struct {
    CARD8 		reqType;
    CARD8 		opcode;
    CARD16 		length B16;
    Drawable		drawableExample B32;
    pexTableType	tableType B16;
    CARD16		used B16;
} pexGetTableInfoReq;


typedef struct {
    CARD8 		reqType;
    CARD8 		opcode;
    CARD16 		length B16;
    pexFormat		format;
    Drawable		drawableExample B32;
    pexTableType	tableType B16;
    pexTableIndex	start B16;
    CARD16		count B16;
    CARD16		used B16;
} pexGetPredefinedEntriesReq;

typedef pexResourceReq pexGetDefinedIndicesReq;

typedef struct {
    CARD8 		reqType;
    CARD8 		opcode;
    CARD16 		length B16;
    pexFormat		format;
    pexLookupTable	lut B32;
    pexTableIndex	index B16;
    CARD16		pad B16;
} pexGetTableEntryReq;

typedef struct {
    CARD8 		reqType;
    CARD8 		opcode;
    CARD16 		length B16;
    pexFormat		format;
    pexLookupTable	lut B32;
    pexTableIndex	start B16;
    CARD16		count B16;
} pexGetTableEntriesReq;


typedef struct {
    CARD8 		reqType;
    CARD8 		opcode;
    CARD16 		length B16;
    pexFormat		format;
    pexLookupTable	lut B32;
    pexTableIndex	start B16;
    CARD16		count B16;
} pexSetTableEntriesReq;


typedef struct {
    CARD8 		reqType;
    CARD8 		opcode;
    CARD16 		length B16;
    pexLookupTable	lut B32;
    pexTableIndex	start B16;
    CARD16		count B16;
} pexDeleteTableEntriesReq;


typedef struct {
    CARD8 	reqType;
    CARD8 	opcode;
    CARD16 	length B16;
    pexFormat	format;
    pexPC	pc B32;
    CARD32	itemMask[2];	/* pexBitmask Array */
    CARD32	numItems B32;
} pexCreatePipelineContextReq;


typedef struct {
    CARD8 	reqType;
    CARD8 	opcode;
    CARD16 	length B16;
    pexPC	src B32;
    pexPC	dst B32;
    CARD32	itemMask[2];	/* pexBitmask Array */
} pexCopyPipelineContextReq;

typedef pexResourceReq  pexFreePipelineContextReq;

typedef struct {
    CARD8 	reqType;
    CARD8 	opcode;
    CARD16 	length B16;
    pexFormat	format;
    pexPC	pc B32;
    CARD32	itemMask[2];	/* pexBitmask Array */
} pexGetPipelineContextReq;

typedef struct {
    CARD8 	reqType;
    CARD8 	opcode;
    CARD16 	length B16;
    pexFormat	format;
    pexPC	pc B32;
    CARD32	itemMask[2];	/* pexBitmask Array */
    CARD32	numItems B32;
} pexChangePipelineContextReq;

typedef struct {
    CARD8 	reqType;
    CARD8 	opcode;
    CARD16 	length B16;
    pexFormat	format;
    pexRenderer	rdr B32;
    Drawable	drawable B32;
    pexBitmask	itemMask B32;
    CARD32	numItems B32;
} pexCreateRendererReq;

typedef pexResourceReq pexFreeRendererReq;

typedef struct {
    CARD8 	reqType;
    CARD8 	opcode;
    CARD16 	length B16;
    pexFormat	format;
    pexRenderer	rdr B32;
    pexBitmask	itemMask B32;
    CARD32	numItems B32;
} pexChangeRendererReq;

typedef struct {
    CARD8 	reqType;
    CARD8 	opcode;
    CARD16 	length B16;
    pexFormat	format;
    pexRenderer	rdr B32;
    pexBitmask	itemMask B32;
} pexGetRendererAttributesReq;

typedef struct {
    CARD8 	reqType;
    CARD8 	opcode;
    CARD16 	length B16;
    pexRenderer	rdr B32;
    Drawable	drawable B32;
} pexBeginRenderingReq;

typedef pexResourceReq pexEndRenderingReq;

typedef struct {
    CARD8 		reqType;
    CARD8 		opcode;
    CARD16 		length B16;
    pexRenderer		rdr B32;
    CARD32		sid B32;
} pexBeginStructureReq;

typedef pexResourceReq pexEndStructureReq;

typedef struct {
    CARD8 	reqType;
    CARD8 	opcode;
    CARD16 	length B16;
    pexFormat	format;
    pexRenderer	rdr B32;
    CARD32	numCommands B32;
    /* LISTof OutputCommand(numCommands) */
} pexRenderOutputCommandsReq;
/* individual output commands may be found in the section "Output Commands" */


typedef struct {
    CARD8 		reqType;
    CARD8 		opcode;
    CARD16 		length B16;
    pexRenderer		rdr B32;
    Drawable		drawable B32;
    pexStructure	sid B32;
} pexRenderNetworkReq;

typedef pexResourceReq pexCreateStructureReq;

typedef struct {
    CARD8 		reqType;
    CARD8 		opcode;
    CARD16 		length B16;
    pexStructure	src B32;
    pexStructure	dst B32;
} pexCopyStructureReq;

typedef pexResourceReq pexFreeStructureReq;

typedef struct {
    CARD8 	reqType;
    CARD8 	opcode;
    CARD16 	length B16;
    CARD16	refFlag B16;
    CARD16	pad B16;
    CARD32	numStructures B32;
    /* LISTof pexStructure(numStructures) */
} pexDeleteStructuresReq;

typedef struct {
    CARD8 		reqType;
    CARD8 		opcode;
    CARD16 		length B16;
    pexStructure	sid B32;
} pexGetStructureInfoReq;

typedef struct {
    CARD8 		reqType;
    CARD8 		opcode;
    CARD16 		length B16;
    pexStructure	sid B32;
    pexElementRange	range;
} pexGetElementInfoReq;

typedef struct {
    CARD8 		reqType;
    CARD8 		opcode;
    CARD16 		length B16;
    pexStructure	sid B32;
    CARD16		which B16;
    CARD16		pad B16;
} pexGetStructuresInNetworkReq;

typedef struct {
    CARD8 		reqType;
    CARD8 		opcode;
    CARD16 		length B16;
    pexStructure	sid B32;
    CARD16		pathOrder B16;
    CARD16		pad B16;
    CARD32		pathDepth B32;
} pexGetAncestorsReq;

typedef pexGetAncestorsReq pexGetDescendantsReq;

typedef struct {
    CARD8 		reqType;
    CARD8 		opcode;
    CARD16 		length B16;
    pexFormat		format;
    pexStructure	sid B32;
    pexElementRange	range;
} pexFetchElementsReq;

typedef struct {
    CARD8 		reqType;
    CARD8 		opcode;
    CARD16 		length B16;
    pexStructure	sid B32;
    CARD16		mode B16;
    CARD16		pad B16;
} pexSetEditingModeReq;

typedef struct {
    CARD8 		reqType;
    CARD8 		opcode;
    CARD16 		length B16;
    pexStructure	sid B32;
    pexElementPos	position;
} pexSetElementPointerReq;

typedef struct {
    CARD8 		reqType;
    CARD8 		opcode;
    CARD16 		length B16;
    pexStructure	sid B32;
    CARD32		label;
    INT32		offset;
} pexSetElementPointerAtLabelReq;

typedef struct {
    CARD8 		reqType;
    CARD8 		opcode;
    CARD16 		length B16;
    pexStructure	sid B32;
    pexElementPos	position;
    CARD32		direction B32;
    CARD32		numIncls B32;
    CARD32		numExcls B32;
} pexElementSearchReq;

typedef struct {
    CARD8 		reqType;
    CARD8 		opcode;
    CARD16 		length B16;
    pexFormat		format;
    pexStructure	sid B32;
    CARD32		numCommands B32;
} pexStoreElementsReq;

typedef struct {
    CARD8 		reqType;
    CARD8 		opcode;
    CARD16 		length B16;
    pexStructure	sid B32;
    pexElementRange	range;
} pexDeleteElementsReq;

typedef struct {
    CARD8 		reqType;
    CARD8 		opcode;
    CARD16 		length B16;
    pexStructure	sid B32;
    pexElementPos	position;
    CARD32		label B32;
} pexDeleteElementsToLabelReq;

typedef struct {
    CARD8 		reqType;
    CARD8 		opcode;
    CARD16 		length B16;
    pexStructure	sid B32;
    CARD32		label1 B32;
    CARD32		label2 B32;
} pexDeleteBetweenLabelsReq;

typedef struct {
    CARD8 		reqType;
    CARD8 		opcode;
    CARD16 		length B16;
    pexStructure	src B32;
    pexElementRange	srcRange;
    pexStructure	dst B32;
    pexElementPos	dstPosition;
} pexCopyElementsReq;

typedef pexResourceReq pexCreateNameSetReq;

typedef struct {
    CARD8 	reqType;
    CARD8 	opcode;
    CARD16 	length B16;
    pexNameSet	src B32;
    pexNameSet	dst B32;
} pexCopyNameSetReq;

typedef struct {
    CARD8 		reqType;
    CARD8 		opcode;
    CARD16 		length B16;
    pexStructure	old B32;
    pexStructure	new B32;
} pexChangeStructureRefsReq;

typedef pexResourceReq pexFreeNameSetReq;

typedef pexResourceReq pexGetNameSetReq;

typedef struct {
    CARD8 	reqType;
    CARD8 	opcode;
    CARD16 	length B16;
    pexNameSet	ns B32;
    CARD16	action B16;
    CARD16	pad B16;
    /* LISTof Name(numNames) */
} pexChangeNameSetReq;

typedef struct {
    CARD8 	reqType;
    CARD8 	opcode;
    CARD16 	length B16;
    pexFormat	format;
    pexSC	sc B32;
    pexBitmask	itemMask B32;
    CARD32	numValues B32;
} pexCreateSearchContextReq;

typedef struct {
    CARD8 	reqType;
    CARD8 	opcode;
    CARD16 	length B16;
    pexSC	src B32;
    pexSC	dst B32;
    pexBitmask	itemMask B32;
} pexCopySearchContextReq;

typedef pexResourceReq pexFreeSearchContextReq;

typedef struct {
    CARD8 	reqType;
    CARD8 	opcode;
    CARD16 	length B16;
    pexFormat	format;
    pexSC	sc B32;
    pexBitmask	itemMask B32;
} pexGetSearchContextReq;

typedef struct {
    CARD8 	reqType;
    CARD8 	opcode;
    CARD16 	length B16;
    pexFormat	format;
    pexSC	sc B32;
    pexBitmask	itemMask B32;
    CARD32	numValues B32;
} pexChangeSearchContextReq;

typedef pexResourceReq pexSearchNetworkReq;

typedef struct {
    CARD8 		reqType;
    CARD8 		opcode;
    CARD16 		length B16;
    pexPhigsWks		wks B32;
    Drawable		drawable B32;
    pexLookupTable	lineBundle B32;
    pexLookupTable	markerBundle B32;
    pexLookupTable	textBundle B32;
    pexLookupTable	interiorBundle B32;
    pexLookupTable	edgeBundle B32;
    pexLookupTable	colorBundle B32;
    pexLookupTable	patternBundle B32;
    pexLookupTable	textFontBundle B32;
    pexLookupTable	depthCueBundle B32;
    pexLookupTable	colorApproxBundle B32;
    pexNameSet		highlightIncl B32;
    pexNameSet		highlightExcl B32;
    pexNameSet		invisIncl B32;
    pexNameSet		invisExcl B32;
} pexCreatePhigsWksReq;

typedef pexResourceReq pexFreePhigsWksReq;

typedef struct {
    CARD8 	reqType;
    CARD8 	opcode;
    CARD16 	length B16;
    pexFormat	format;
    pexPhigsWks	wks B32;
    pexBitmask	itemMask B32;
} pexGetWksInfoReq;

typedef struct {
    CARD8 	reqType;
    CARD8 	opcode;
    CARD16 	length B16;
    Drawable	drawable B32;
} pexGetDynamicsReq;

typedef struct {
    CARD8 		reqType;
    CARD8 		opcode;
    CARD16 		length B16;
    pexFormat		format;
    pexPhigsWks		wks B32;
    pexTableIndex	index B16;
    CARD16		pad B16;
} pexGetViewRepReq;

typedef pexResourceReq pexRedrawAllStructuresReq;	

typedef pexResourceReq pexUpdateWorkstationReq;

typedef pexResourceReq pexExecuteDeferredActionsReq;

typedef struct {
    CARD8 		reqType;
    CARD8 		opcode;
    CARD16 		length B16;
    pexPhigsWks		wks B32;
    pexTableIndex	index1 B16;
    pexTableIndex	index2 B16;
    CARD16		priority B16;
    CARD16		pad B16;
} pexSetViewPriorityReq;

typedef struct {
    CARD8 		reqType;
    CARD8 		opcode;
    CARD16 		length B16;
    pexPhigsWks		wks B32;
    pexEnumTypeIndex	displayUpdate B16;
    CARD16		pad B16;
} pexSetDisplayUpdateModeReq;

typedef struct {
    CARD8 	reqType;
    CARD8 	opcode;
    CARD16 	length B16;
    pexFormat	format;
    pexPhigsWks	wks B32;
    CARD32	numPoints B32;
} pexMapPointsReq;

typedef struct {
    CARD8 	reqType;
    CARD8 	opcode;
    CARD16 	length B16;
    pexFormat	format;
    pexPhigsWks	wks B32;
    pexViewRep	viewRep;
} pexSetViewRepReq;

typedef struct {
    CARD8 		reqType;
    CARD8 		opcode;
    CARD16 		length B16;
    pexFormat		format;
    pexPhigsWks		wks B32;
    pexNpcSubvolume	npcSubvolume;
} pexSetWksWindowReq;

typedef struct {
    CARD8 	reqType;
    CARD8 	opcode;
    CARD16 	length B16;
    pexFormat	format;
    pexPhigsWks	wks B32;
    pexViewport	viewport;
} pexSetWksViewportReq;

typedef struct {
    CARD8 		reqType;
    CARD8 		opcode;
    CARD16 		length B16;
    pexPhigsWks		wks B32;
    pexEnumTypeIndex	mode B16;
    CARD16		pad B16;
} pexSetHlhsrModeReq;

typedef struct {
    CARD8 		reqType;
    CARD8 		opcode;
    CARD16 		length B16;
    pexPhigsWks		wks B32;
    pexStructure	sid B32;
    CARD32		priority B32;
} pexPostStructureReq;

typedef struct {
    CARD8 		reqType;
    CARD8 		opcode;
    CARD16 		length B16;
    pexPhigsWks		wks B32;
    pexStructure	sid B32;
} pexUnpostStructureReq;

typedef pexResourceReq pexUnpostAllStructuresReq;

typedef pexResourceReq pexGetWksPostingsReq;

typedef struct {
    CARD8 		reqType;
    CARD8 		opcode;
    CARD16 		length B16;
    pexFormat		format;
    pexPhigsWks		wks B32;
    pexEnumTypeIndex	devType B16;
    CARD16		pad B16;
    pexBitmask		itemMask B32;
} pexGetPickDeviceReq;

typedef struct {
    CARD8 		reqType;
    CARD8 		opcode;
    CARD16 		length B16;
    CARD16		format B16;
    CARD16		pad1 B16;
    pexPhigsWks		wks B32;
    pexEnumTypeIndex	devType B16;
    CARD16		pad B16;
    pexBitmask		itemMask B32;
    CARD32		numItems;
} pexChangePickDeviceReq;

typedef struct {
    CARD8 		reqType;
    CARD8 		opcode;
    CARD16 		length B16;
    pexPhigsWks		wks B32;
    pexPickMeasure	pm;
    pexEnumTypeIndex	devType B16;
    CARD16      	pad B16;
} pexCreatePickMeasureReq;

typedef pexResourceReq pexFreePickMeasureReq;

typedef struct {
    CARD8 		reqType;
    CARD8 		opcode;
    CARD16 		length B16;
    pexPickMeasure	pm B32;
    pexBitmask		itemMask B32;
} pexGetPickMeasureReq;

typedef struct {
    CARD8 		reqType;
    CARD8 		opcode;
    CARD16 		length B16;
    pexPickMeasure	pm B32;
    CARD32		numBytes B32;
} pexUpdatePickMeasureReq;

typedef struct {
    CARD8 	reqType;
    CARD8 	opcode;
    CARD16 	length B16;
    pexFont	font B32;
    CARD32	numBytes B32;
    /* list of CARD8 -- don't swap */
} pexOpenFontReq;

typedef pexResourceReq pexCloseFontReq;
typedef struct {
    CARD8 	reqType;
    CARD8 	opcode;
    CARD16 	length B16;
    pexFormat	format;
    pexFont	font B32;
} pexQueryFontReq;

typedef struct {
    CARD8 	reqType;
    CARD8 	opcode;
    CARD16 	length B16;
    CARD16	maxNames B16;
    CARD16	numChars B16;
    /* list of CARD8 -- don't swap */
} pexListFontsReq;

typedef struct {
    CARD8 	reqType;
    CARD8 	opcode;
    CARD16 	length B16;
    CARD16	fpFormat B16;
    CARD16	maxNames B16;
    CARD16	numChars B16;
    CARD16	pad B16;
    /* LISTof CARD8(numChars)  */
} pexListFontsWithInfoReq;

typedef struct {
    CARD8 	reqType;
    CARD8 	opcode;
    CARD16 	length B16;
    CARD16	fpFormat B16;
    CARD16	pad B16;
    pexFont	font B32;
    pexPC	pc B32;
    CARD32	numChars B32;
}  pexQueryTextExtentsReq;

typedef struct {
    CARD8	reqType;
    CARD8	opcode;
    CARD16	length B16;
    CARD16	fpFormat B16;
    CARD16	pad B16;
    CARD32	drawable B32;
    CARD32	numNames B32;
    /* LISTof PexImpDepConstantNames (numNames)  */
} pexGetImpDepConstantsReq;

/*****************************************************************
 * Output Commands 
 *****************************************************************/

typedef struct {
    pexElementInfo	head;
    pexEnumTypeIndex	markerType B16;
    CARD16		pad B16;
} pexMarkerType;

typedef struct {
    pexElementInfo	head;
    FLOAT		scale B32;
} pexMarkerScale;

typedef struct {
    pexElementInfo	head;
    pexTableIndex		color B16;
    CARD16		pad B16;
} pexMarkerColorIndex;

typedef struct {
    pexElementInfo	head;
    pexColorSpecifier	color;
} pexMarkerColor;

typedef struct {
    pexElementInfo	head;
    pexTableIndex		index B16;
    CARD16		pad B16;
} pexMarkerBundleIndex;

typedef pexMarkerBundleIndex pexTextFontIndex;
typedef pexMarkerColorIndex pexTextColorIndex;
typedef pexMarkerBundleIndex pexAnnotationTextStyle;
typedef pexMarkerBundleIndex pexTextBundleIndex;
typedef pexMarkerBundleIndex pexLineBundleIndex;
typedef pexMarkerBundleIndex pexSurfaceInteriorStyleIndex;
typedef pexMarkerBundleIndex pexBfSurfaceInteriorStyleIndex;
typedef pexMarkerBundleIndex pexInteriorBundleIndex;
typedef pexMarkerColorIndex pexSurfaceEdgeColorIndex;
typedef pexMarkerBundleIndex pexEdgeBundleIndex;
typedef pexMarkerBundleIndex pexViewIndex;
typedef pexMarkerBundleIndex pexDepthCueIndex;

typedef struct {
    pexElementInfo	head;
    CARD16		precision;
    CARD16		pad B16;
} pexTextPrecision;

typedef struct {
    pexElementInfo	head;
    FLOAT		expansion B32;
} pexCharExpansion;

typedef struct {
    pexElementInfo	head;
    FLOAT		spacing B32;
} pexCharSpacing;

typedef struct {
    pexElementInfo	head;
    pexColorSpecifier	color;
} pexTextColor;

typedef struct {
    pexElementInfo	head;
    FLOAT		height B32;
} pexTextHeight;
typedef pexTextHeight pexAnnotationTextHeight;

typedef struct {
    pexElementInfo	head;
    pexVector2D		up;
} pexTextUpVector;
typedef pexTextUpVector pexAnnotationTextUpVector;

typedef struct {
    pexElementInfo	head;
    CARD16		path B16;
    CARD16		pad B16;
} pexTextPath;
typedef pexTextPath pexAnnotationTextPath;

typedef struct {
    pexElementInfo		head;
    pexTextAlignmentData	alignment;
} pexTextAlignment;

typedef pexTextAlignment pexAnnotationTextAlignment;

typedef struct {
    pexElementInfo	head;
    pexEnumTypeIndex	lineType;
    CARD16		pad B16;
} pexLineType;

typedef struct {
    pexElementInfo	head;
    FLOAT		width B32;
} pexLineWidth;

typedef struct {
    pexElementInfo	head;
    pexTableIndex	color B16;
    CARD16		pad B16;
} pexLineColorIndex;

typedef struct {
    pexElementInfo	head;
    pexColorSpecifier	color;
} pexLineColor;

typedef struct {
    pexElementInfo	head;
    pexCurveApprox	approx;
} pexCurveApproximation;

typedef struct {
    pexElementInfo	head;
    pexEnumTypeIndex	polylineInterp B16;
    CARD16		pad B16;
} pexPolylineInterp;

typedef struct {
    pexElementInfo	head;
    pexEnumTypeIndex	interiorStyle B16;
    CARD16		pad B16;
} pexSurfaceInteriorStyle;

typedef pexSurfaceInteriorStyle pexBfSurfaceInteriorStyle;

typedef struct {
    pexElementInfo	head;
    pexTableIndex	color B16;
    CARD16		pad B16;
} pexSurfaceColorIndex;
typedef pexSurfaceColorIndex pexBfSurfaceColorIndex;

typedef struct {
    pexElementInfo	head;
    pexColorSpecifier	color;
} pexSurfaceColor;
typedef pexSurfaceColor pexBfSurfaceColor;

typedef struct {
    pexElementInfo	head;
    pexReflectionAttr	reflectionAttr;
} pexSurfaceReflectionAttr;
typedef pexSurfaceReflectionAttr pexBfSurfaceReflectionAttr;

typedef struct {
    pexElementInfo	head;
    pexEnumTypeIndex	reflectionModel B16;
    CARD16		pad B16;
} pexSurfaceReflectionModel;

typedef pexSurfaceReflectionModel pexBfSurfaceReflectionModel;

typedef struct {
    pexElementInfo	head;
    pexEnumTypeIndex	surfaceInterp B16;
    CARD16		pad B16;
} pexSurfaceInterpMethod;

typedef pexSurfaceInterpMethod pexBfSurfaceInterpMethod;

typedef struct {
    pexElementInfo	head;
    pexSurfaceApprox	approx;
} pexSurfaceApproximation;

typedef struct {
    pexElementInfo	head;
    pexCurveApprox	approx;
} pexTrimCurveApproximation;

typedef struct {
    pexElementInfo	head;
    pexCullMode		cullMode B16;
    CARD16		unused B16;
} pexFacetCullingMode;

typedef struct {
    pexElementInfo	head;
    CARD16		distinguish B16;
    CARD16		unused B16;
} pexFacetDistinguishFlag;

typedef struct {
    pexElementInfo	head;
    CARD16		normalFlip B16;
    CARD16		pad B16;
} pexNormalReorientationMode;

typedef struct {
    pexElementInfo	head;
    pexCoord2D		size;
} pexPatternSize;

typedef struct {
    pexElementInfo	head;
    pexCoord2D		point;
} pexPatternReferencePoint;

typedef struct {
    pexElementInfo	head;
    pexCoord3D		refPt;
    pexVector3D		vector1;
    pexVector3D		vector2;
} pexPatternAttributes;

typedef struct {
    pexElementInfo	head;
    pexSwitch		onoff;
    CARD8		pad1;
    CARD16		pad B16;
} pexSurfaceEdgeFlag;

typedef struct {
    pexElementInfo	head;
    pexEnumTypeIndex	edgeType B16;
    CARD16		pad B16;
} pexSurfaceEdgeType;

typedef struct {
    pexElementInfo	head;
    FLOAT		width B32;
} pexSurfaceEdgeWidth;

typedef struct {
    pexElementInfo	head;
    pexColorSpecifier	color;
} pexSurfaceEdgeColor;

typedef struct {
    pexElementInfo	head;
    pexAsfAttribute	attribute B32;
    pexAsfValue		source B32;
} pexSetIndividualAsf;

typedef struct {
    pexElementInfo	head;
    pexComposition	compType;
    CARD16		pad B16;
    pexMatrix		matrix;
} pexLocalTransform;

typedef struct {
    pexElementInfo	head;
    pexComposition	compType;
    CARD16		pad B16;
    pexMatrix3X3	matrix3X3;
} pexLocalTransform2D;

typedef struct {
    pexElementInfo	head;
    pexMatrix		matrix;
} pexGlobalTransform;

typedef struct {
    pexElementInfo	head;
    pexMatrix3X3	matrix3X3;
} pexGlobalTransform2D;

typedef struct {
    pexElementInfo	head;
    pexSwitch		onoff;
    CARD8		pad1;
    CARD16		pad  B16;
} pexModelClip;

typedef struct {
    pexElementInfo	head;
    pexEnumTypeIndex	operator B16;
    CARD16		numHalfSpaces B16;
    /* LISTof HalfSpace(numHalfSpaces) */
} pexSetModelClipVolume3D;

typedef struct {
    pexElementInfo	head;
    pexEnumTypeIndex	operator B16;
    CARD16		numHalfSpaces B16;
    /* LISTof HalfSpace2D(numHalfSpaces) */
} pexSetModelClipVolume2D;

typedef struct {
    pexElementInfo	head;
} pexRestoreModelClipVolume;

typedef struct {
    pexElementInfo	head;
    pexBitmask		enable B32;
    pexBitmask		disable B32;
} pexLightSourceState;

typedef struct {
    pexElementInfo	head;
    CARD32		pickId B32;
} pexPickID;

typedef struct {
    pexElementInfo	head;
    CARD32		hlhsrID B32;
} pexHlhsrIdentifier;

typedef struct {
    pexElementInfo	head;
    /* LISTof Name(head.length) */
} pexAddNameToNameSet;

typedef pexAddNameToNameSet pexRemoveNamesFromNameSet;

typedef struct {
    pexElementInfo	head;
    CARD32		id B32;
} pexExecuteStructure;

typedef struct {
    pexElementInfo	head;
    CARD32		label B32;
} pexLabelInfo;

typedef struct {
    pexElementInfo	head;
    CARD16		numElements B16;
    CARD16		unused B16;
    /* list of CARD8 -- don't swap */
} pexApplicationData;
    
typedef struct {
    pexElementInfo	head;
    CARD32		id B32;
    CARD16		numElements B16;
    CARD16		unused B16;
/* list of CARD8 -- don't swap */
} pexGSE;

typedef struct {
pexElementInfo	head;
/* LISTof Coord3DLength(head.length) */
} pexMarker;	

typedef struct {
pexElementInfo	head;
/* LISTof Coord2DLength(head.length) */
} pexMarker2D;	

typedef struct {
pexElementInfo	head;
pexCoord3D		origin;
pexVector3D		vector1;
pexVector3D		vector2;
pexString		string;
} pexText;

typedef struct {
pexElementInfo	head;
pexCoord2D		origin;
pexString		string;
} pexText2D;

typedef struct {
pexElementInfo	head;
pexCoord3D		origin;
pexCoord3D		offset;
pexString		string;
} pexAnnotationText;

typedef struct {
pexElementInfo	head;
pexCoord2D		origin;
pexCoord2D		offset;
pexString		string;
} pexAnnotationText2D;

typedef struct {
pexElementInfo	head;
/* LISTof Coord3DLength(head.length)  */
} pexPolyline;

typedef struct {
pexElementInfo	head;
/* LISTof Coord2DLength(head.length) */
} pexPolyline2D;

typedef struct {
pexElementInfo	head;
pexColorType	colorType B16;
CARD16		vertexAttribs B16;
CARD32		numLists B32;
    /* LISTof ClistofPexVertex(numLists, vertexAttribs, colorType) */
} pexPolylineSet;

typedef struct {
    pexElementInfo	head;
    pexEnumTypeIndex	curveBasis B16;
    CARD16		curveOrder B16;
    pexCoordType	coordType B16;
    CARD16		pad B16;
    CARD32		numPoints B32;
    /* LISTof Coord(numPoints, coordType) */
} pexParametricCurve;

typedef struct {
    pexElementInfo	head;
    CARD16		curveOrder B16;
    pexCoordType	coordType B16;
    FLOAT		tMin B32;
    FLOAT		tMax B32;
    CARD32		numKnots B32;
    CARD32		numPoints B32;
    /* LISTof Float(numKnots) */
    /* LISTof Coord(numPoints, coordType) */
} pexNonUniformBSplineCurve;

typedef struct {
    pexElementInfo	head;
    CARD16		shape B16;
    CARD16		ignoreEdges B16;
    /* LISTof Coord3D(head.length) */
} pexFillArea;

typedef struct {
    pexElementInfo	head;
    CARD16		shape B16;
    CARD16		ignoreEdges B16;
    /* LISTof Coord2D(numVertices) */
} pexFillArea2D;

typedef struct {
    pexElementInfo	head;
    CARD16		shape B16;
    CARD16		ignoreEdges B16;
    pexColorType	colorType B16;
    CARD16		facetAttribs B16;
    CARD16		vertexAttribs B16;
    CARD16		pad B16;
    /* SINGLE Facet(facetAttribs, vertexAttribs, colorType) */
} pexFillAreaWithData;

typedef struct {
    pexElementInfo	head;
    CARD16		shape B16;
    CARD16		ignoreEdges B16;
    CARD32		numLists B32;
    /* LISTof ClistofCoord3D(numLists) */
} pexFillAreaSet;

typedef struct {
    pexElementInfo	head;
    CARD16		shape B16;
    CARD16		ignoreEdges B16;
    CARD32		numLists B32;
    /* LISTof CListofCoord2D(numLists) */
} pexFillAreaSet2D;


typedef struct {
    pexElementInfo	head;
    CARD16		shape B16;
    CARD16		ignoreEdges B16;
    pexColorType	colorType B16;
    CARD16		facetAttribs B16;
    CARD16		vertexAttribs B16;
    CARD16		pad B16;
    CARD32		numFacets B32;
    /* LISTof Facet(numFacets, facetAttribs, vertexAttribs, colorType) */
} pexFillAreaSetWithData;

typedef struct {
    pexElementInfo	head;
    pexColorType	colorType B16;
    CARD16		facetAttribs B16;
    CARD16		vertexAttribs B16;
    CARD16		pad B16;
    CARD32		numVertices B32;
    /* actually, length of OptData is numVert - 2 */
    /* LISTof OptData(numVertices, facetAttribs, vertexAttribs, colorType) */
    /* LISTof Vertex(numVertices, vertexAttribs, colorType) */
} pexTriangleStrip;

typedef struct {
    pexElementInfo	head;
    pexColorType	colorType B16;
    CARD16		mPts B16;
    CARD16		nPts B16;
    CARD16		facetAttribs B16;
    CARD16		vertexAttribs B16;
    CARD16		pad B16;
    /* actually, there are mPts*nPts/4 opt data entries */
    /* LISTof OptData(mPts, nPts, facetAttribs, vertexAttribs, colorType) */
    /* LISTof Vertex(mPts, nPts, vertexAttribs, colorType) */
} pexQuadrilateralMesh;

typedef struct {
    pexElementInfo	head;
    CARD16		shape B16;
    pexColorType	colorType B16;
    CARD16		facetAttribs B16;
    CARD16		vertexAttribs B16;
    CARD16		edgeAttribs B16;
    CARD16		numFacets B16;
    CARD16		numEdges B16;
    CARD16		numVertices B16;
/*  LISTof FacetCount(numFacets) */
/*  LISTof FacetOptData(numFacets, facetAttribs, colorType) */
/*  LISTof Edges(numEdges, edgeAttribs) */
/*  LISTof Vertex(numVertices, vertexAttribs, colorType) */
} pexIndexedPolygon;

typedef struct {
    pexElementInfo	head;
    pexEnumTypeIndex 	surfForm B16;
    pexEnumTypeIndex	surfType B16;
    CARD16		surfUOrder B16;
    CARD16		surfVOrder B16;
    CARD32		numCoord B32;
    /* LISTof Coord(surfType, numCoord) */
} pexParametricSurface;

typedef struct {
    pexElementInfo	head;
    pexCoordType 	type B16;
    CARD16		uOrder B16;
    CARD16		vOrder B16;
    CARD16		pad B16;
    FLOAT		uMin B32;
    FLOAT		uMax B32;
    FLOAT		vMin B32;
    FLOAT		vMax B32;
    CARD32		numUknots B32;
    CARD32		numVknots B32;
    CARD32		numPoints B32;
    CARD32		numLists B32;
    /* LISTof Float(numUknots) */
    /* LISTof Float(numVKnots) */
    /* LISTof Coord(numPoints, surfaceType) */
    /* LISTofLISTof TrimCurve(numLists) */
} pexNonUniformBSplineSurface;

typedef struct {
    pexElementInfo	head;
    pexCoord3D		point1;
    pexCoord3D		point2;
    pexCoord3D		point3;
    CARD32		dx B32;
    CARD32		dy B32;
    /* LISTof Card16(dx, dy) */
} pexCellArray;

typedef struct {
    pexElementInfo	head;
    pexColorType	colorType B16;
    CARD16		pad B16;
    pexCoord2D		point1;
    pexCoord2D		point2;
    CARD32		dx B32;
    CARD32		dy B32;
    /* LISTof Color(dx, dy) */
} pexCellArray2D;

typedef struct {
    pexElementInfo	head;
    pexColorType	colorType B16;
    CARD16		pad B16;
    pexCoord3D		point1;
    pexCoord3D		point2;
    pexCoord3D		point3;
    CARD32		dx B32;
    CARD32		dy B32;
    /* LISTof Color(dx, dy) */
} pexExtendedCellArray;

typedef struct {
    pexElementInfo	head;
    CARD32		gdpId B32;
    CARD32		numPoints B32;
    CARD32		numBytes B32;
    /* LISTof Coord3D(numPoints) */
    /* list of CARD8 -- don't swap */
} pexGDP;

typedef struct {
    pexElementInfo	head;
    CARD32		gdpId B32;
    CARD32		numPoints B32;
    CARD32		numBytes B32;
    /* LISTof Coord2D(numPoints) */
    /* list of CARD8 -- don't swap */
} pexGDP2D;
#endif /* PEXPROTO_H */
