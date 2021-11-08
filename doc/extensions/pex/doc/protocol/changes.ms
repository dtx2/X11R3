.sp 1.0
.TL
X3D-PEX: V3.10 \(-> V3.20 Changes
.AU
1-October-1988
.LP
This document describes the changes that were made to the PEX
protocol specification since version 3.00.
Section numbers are section numbers for the PEX V3.10 document.
In the text of the V3.20 documents, changes or additions are noted with change
bars in the right margin, and deletions are noted with asterisks.  The
list below is an attempt to describe all changes to the protocol document
(changes in troff formatting commands also generate change bars in some
instances).

.IP "\fBSection 2.2 - Syntax\fP"
.nf
"describes" --> "described"

.IP "\fBSection 2.1.2 - Reply Format\fP"
.nf
Clarification added to definition of length field.

.IP "\fBSection 2.4.6 - Types\fP"
.nf
Deleted BITMASK_ARRAY.
Added definition for CONSTANT_NAME.
Deleted CURVE_FORM.
For ENUM_TYPE, changed \fICurveForm\fP to \fICurveBasis\fP and \fISurfaceForm\fP to \fISurfaceBasis\fP.
Deleted ENUM_TYPE_DESC.
Deleted INDEXED_FACET.
Added PC_BITMASK, defined as CARD32[2].
Changed PICK_PATH to PICK_ELEMENT_REF.
Changed \fISurfaceForm\fP to \fISurfaceBasis\fP for description of SURFACE_BASIS.
Changed CURVE_TYPE to COORD_TYPE in description of TRIM_CURVE.
Reversed order of the fields for VERTEX_INDEX.
In VIEW_REP, reordered fields for packing efficiency and combined clip
	SWITCHes to a single BITMASK.

.IP "\fBSection 2.6.1 - Get Extension Info\fP"
.nf
Changed protocol minor version from "one" to "two".

.IP "\fBSection 2.6.2 - Get Enumerated Type Info\fP"
.nf
Changed LISTofLISTofENUM_TYPE_DESC to LISTofLISTofVALUE to emphasize
	the variability of the returned data.
For \fIPolylineInterpMethod\fP, type \fINone\fP has been redefined to say that if
	color values are defined, it is implementation-dependent whether
	the color for the ith vertex will be used to draw the line between
	the ith and the (i+1)th vertices (if n is the number of vertices,
	the nth color will be ignored) or whether an average is computed
	and used.
For \fICurveBasis\fP, changed the word "form" to "basis" in the first sentence.
For \fISurfaceApproxMethod\fP, changed occurrences of "s or t" to "u or v".
For \fISurfaceBasis\fP, changed the word "form" to "basis" in the first sentence
	and in the first sentence of the description for \fIPiecewiseBezier\fP.

.IP "\fBSection 2.6.3 - Get Implementation-Dependent Constants\fP"
.nf
New request.


.IP "\fBSection 3 - Output Commands\fP"
.nf
Deleted references to \fBPEXStoreElements\fP and \fBPEXFetchStructure\fP, and changed
	\fBPEXFetchElementRange\fP to \fBPEXFetchElements\fP.


.IP "\fBSection 3.2 - Output Command Descriptions\fP"
.nf
Changed the description for "Remove names from name set" (p. 28 in V3.10) to
	match the description for "Add names to name set".  The remove/add
	output commands operate on the name set contained within the
	renderer's "current pipeline state", not on a name set resource.
For Text 3D (p. 29) changed "\fItext_h_alignment\fP, and \fItext_v_alignment\fP" to
	"\fItext_alignment\fP, and".
For Annotation text 3D (p. 30) changed "\fIatext_h_alignment, atext_v_alignment\fP"
	to "\fIatext_alignment\fP".
For Fill area 3D (p. 33) deleted the last sentence of the description, and
	changed "\fIrefl_model\fP" to "\fIreflection_model\fP".
For Fill area set 3D with data (p. 34) separated the facet information into a
	single facet_data (OPT_DATA) and a vertex list (LISTofLISTofVERTEX).\(dg
For Quadrilateral mesh (p. 36) changed LISTofFACET_DATA to LISTofOPT_DATA.
For Indexed polygons (p. 37), the data structures have been changed slightly
	to allow a more efficient implementation.\(dg
For Fill area 3D with data, Fill area set 3D with data, Triangle strip,
	Quadrilateral mesh, and Indexed polygons, changed "\fIvertex_attributes\fP"
	to "\fIvert_attributes\fP".
For Parametric polynomial surface (p. 38), changed BASIS to SURFACE_BASIS


.IP "\fBSection 4 - Lookup Tables\fP"
.nf
More discussion of default entries (and what to do if the client deletes
	them) has been added.
For InteriorBundle table, \fItrim_curve_approx\fP has been changed to CURVE_APPROX.
View table clipping flags changed to a BITMASK rather than left as separate SWITCHes.
Added a pointer to more information for the \fIView\fP, \fILight\fP, and \fIDepthCue\fP tables.
Changed range of allowable table index values for \fIDepthCue\fP, \fIRGBApprox\fP,
	and \fIIntensityApprox\fP tables from "1..n" to "0..n".

.IP "\fBSection 5 - Pipeline Contexts\fP"
.nf
Changed the order of the default values for \fIreflection_attr\fP and \fIbf_reflection_attr\fP
	to correspond to the order of the fields in the REFLECTION_ATTR structure.
Changed default value for \fIdepth_cue_index\fP to zero.
Under ASF attributes, changed "\fIasfs\fP" to "\fIasf_flags\fP".  Under the description
	for \fIasf_flags\fP, noted that the \fIenables\fP part of the ASF_SPECIFIER
	structure is only used when creating, setting, or changing a pipeline context,
	and is ignored when querying a pipeline context (all ASFs are returned).
Deleted the last sentence of the description for the \fIsurface_edges\fP attribute.
Changed all BITMASK_ARRAY's to PC_BITMASK's.


.IP "\fBSection 6.2.1 - Change Renderer\fP"
.nf
"\fIRender_state\fP" changed to "\fIrenderer_state\fP".


.IP "\fBSection 6.5.1 - Render Network\fP"
.nf
Changed STRUCTURE to STRUCTURE_ID.


.IP "\fBSection 7.1.2 - Copy Structure\fP"
.nf
The description should be changed to read as follows:

    This request copies all structure elements in the source structure resource
    \fIsrc_id\fP to the destination structure resource \fIdest_id\fP, after
    first deleting all the elements in the destination structure.  The
    destination structure must already exist as a valid resource.  \fISrc_id\fP's
    element pointer and editing mode values are also copied to \fIdst_id\fP.

.IP "\fBSection 10.1.1 Create PHIGS Workstation\fP"
.nf
Added "light_table: LOOKUP_TABLE_ID" to parameter list.\(dg


.IP "\fBSection 10.1.3 Get PHIGS Workstation Info\fP"
.nf
Added \fIlight_table\fP to attribute list and included description of it.\(dg
Fixed \fIcolor_table\fP and \fIcolor_approx_table\fP descriptions.


.IP "\fBSection 10.3.6 Map Points\fP"
.nf
Renamed to \fBPEXMapDCtoWC\fP.


.IP "\fBSection 10.3.7 Map WC to DC\fP"
.nf
New request.


.IP "\fBSection 11.1 - Picking\fP"
.nf
Changed PICK_PATH to LISTofPICK_ELEMENT_REF\(dg


.IP "\fBSection 11.2 - Pick Measure Resource Management\fP"
.nf
Changed PICK_PATH to LISTofPICK_ELEMENT_REF\(dg


.IP "\fBSection 12.2 - PEX Font Inquiry\fP"
.nf
Changed "\fIall_glyph_exist\fP" to "\fIall_glyphs_exist\fP"


.IP "\fBAppendix A - Definition of Standard PEX Subsets\fP"
.nf
Changed "structure mode only" subset to "PHIGS workstation only" subset.
Added \fBPEXGetImpDepConstants\fP to both subsets.
Changed \fBPEXMapPoints\fP to \fBPEXMapDCtoWC\fP and added \fBPEXMapWCtoDC\fP
	to PHIGS workstation only subset.


.IP "\fBAppendix B - Output Primitive Attributes\fP"
.nf
Changed "\fItransparency\fP" to "\fItransmission coefficient\fP".


.IP "\fBAppendix C - Definition of PEX errors\fP"
.nf
Changed second \fBPEXSetElementPointer\fP to \fBPEXGetElementPointer\fP.
Added \fBPEXGetImpDepConstants\fP and definition for errors.
Changed \fBPEXMapPoints\fP to \fBPEXMapDCtoWC\fP.
Added \fBPEXMapWCtoDC\fP and definitions for possible errors.
Added various other errors and modified protocol request descriptions accordingly.


.IP "\fBAppendix D - Definition of Table Default Values\fP"
.nf
Changed range of allowable values for \fIRGBApprox\fP table from 1..n to 0..n.
Changed range of allowable values for \fIIntensityApprox\fP table from 1..n to 0..n.
Changed default entry for \fIIntensityApprox\fP table from one to zero.
Changed format of \fIView\fP table fields to match that defined in the lookup
	table description section.
.FS
\(dg Caused a non-upwards-compatible change with respect to the V3.10 protocol.
.FE
