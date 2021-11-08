.\"
.\"
.\"  This macro puts the section numbers, labels, and page numbers out
.\"   to the standard output.   aps, crw, rjr.
.\"   WARNING: This macro assumes certain knowledge about the the ms/mu
.\"   packages work (what number and string registers are used, to be exact).
.\"
.de AC
.ft 6
.if !\\$2 .nr PS \\n(PS+2
.if \\$2=1 .nr PS \\n(PS+6
.if \\$2=2 .nr PS \\n(PS+4
.if \\$2=3 .nr PS \\n(PS+2
.NH \\$2
\f6\\$1\fP
.if !\\$2 .nr PS \\n(PS-2
.if \\$2=1 .nr PS \\n(PS-6
.if \\$2=2 .nr PS \\n(PS-4
.if \\$2=3 .nr PS \\n(PS-2
.tm .Bg \\n(NS
.tm \\*(SN \\$1
.tm .Ed \\n%
..

.de AP
.tm .Bg \\n(NS
.tm \\*(SN \\$1
.tm .Ed \\n%
..

.de RU
.br
\l'6.5i'
.sp
..


.de SH
.ti \\n(.iu
.RT
.if \\n(1T .sp
.RT
.ne 3.1
.ft 6
..

.		\" Picture Start
.		\" $1 is height, $2 is width 
.de PS
.nr A1 \\$1
.nr A2 \\$2
.if t .sp .3
.in (\\n(.lu-\\n(A2u)/2u
.ne \\n(A1u
..

.		\" Picture End
.de PE
.in
.if t .sp .6
..
\ 
.LP
.nh
.nr LL 6.5i
.nr HM 1.7i
.nr FM 1.5i
.nr PO 1.0i
.nr PD 0.20i
.LP
\ 
.sp 12
.PP
.TL
\f6\s+8PEX Introduction and Overview\s0\fP
.sp 4
.AU
.ps 12
.nr PS 12
.vs 16
.nr VS 16
\f5Version 3.20
\n(dy-\*(MO-19\n(yr
.sp 1.0i
.nf
.R
.ce 3
\f6Randi J. Rost
Digital Equipment Corporation
Workstation Systems Engineering\fP
.sp 1.0i
.ce 1
.ft 5
Additional Contributors:
.sp
.ta 1.7i 3.1i
	Jeffrey Friedberg	\f7Digital Equipment Corp.\fP
	Marty Hess	\f7Sun Microsystems\fP
.ft 1
.ps 10
.nr PS 10
.vs 12
.nr VS 12
.bp 1
\ 
.sp 32
.LP
.nf
.ce 2
Copyright \(co 1987, 1988, Massachussetts Institute of Technology
All rights reserved.
.LP
.LP
Permission to use, copy, modify, and distribute this document for any purpose
and without fee is hereby granted, provided that the above copyright notice
appear in all copies and that both that copyright notice and this permission
notice are retained, and that the name of M.I.T. not be used in advertising or
publicity pertaining to this document without specific, written prior
permission.  M.I.T. makes no representations about the suitability of this
document or the interface defined in this document for any purpose.  It is
provided "as is" without express or implied warranty.
.bp 1
\ 
.sp 1.0i
\f6\s+9Disclaimer:\s-9\fP
.sp 2
.LP
This document has not been thoroughly reviewed by the PEX Architecture Team.
I have attempted to document at a higher level the current consciousness
that defines the PEX extension.  Due to a lack of a proper review
cycle within the PEX Architecture Team,
this document may still contain material
where differences of interpretation exist.
Nevertheless, I am releasing it at this point because the Architecture
Team believes that the majority of information is correct, and that it
is more important to make this document available to the public immediately
than to hold it up for a thorough Architecture Team review cycle.
It is anticipated that this document will eventually be "blessed" by the PEX
Architecture Team.
The PEX Protocol Specification and Encoding Document currently
comprise the definition of the PEX extension.
It is hoped that over time, all necessary semantics described
herein but absent from the current PEX protocol document will
be added to the protocol definition.
Alternatively, at some point this document
may get Architecture Team approval as part of the definition of the PEX 
extension.  Until then,
this document should not be construed to be
a definition of the PEX extension, but as an aid to understanding it.

.ID 4.0i
Randi J. Rost
Digital Equipment Corporation
Workstation Systems Engineering
29-Apr-1988
.DE
.EH '\s12\f7PEX Introduction and Overview\fP\s0'''
.OH '''\s12\f7PEX Introduction and Overview\fP\s0'
.EF '\s12\f5-%-'PEX Version 3.20'\fP\s0'
.OF '\s12\f5'PEX Version 3.20'-%-\fP\s0'
.bp 1
.AC Introduction 1
.RU
.LP
PEX (a PHIGS/PHIGS+ Extension for X)
is a 3D graphics extension to the X Window System\(dg
that efficiently supports PHIGS and PHIGS+.
PEX is designed to provide a network transparent
3D graphics interface across a range of
workstation products with raster graphics capability.
.LP
This document is an attempt to explain the behavior of the PEX server
extension from the point of view of a client sending protocol requests.
The descriptions in this document complement those found in the \fIPEX
Protocol Specification\fP
.[
PEX protocol spec rost arch
.]
document by presenting a more global view
of the operation of the PEX extension.  Furthermore, the descriptions
in this document are intended to be conceptual in nature; a PEX implementation
is free to implement the functionality in many different ways, as long
as the apparent effect of a request remains the same.
This document
is specifically not intended to be a description of the implementation
architecture of a PEX server extension, nor is it intended to be
a description of how a PHIGS/PHIGS+ library would be implemented
on top of PEX.
.LP
PEX is designed to extend the capabilities of X by providing
3D graphics functionality within the X Window System's
client/server model.
.[
Xlib
.]
.[
X Window System
.]
.FS
.br
.sp
\(dg The X Window System is a trademark of MIT.
.FE
The display is controlled by a process known as the \fIserver\fP.
The client application communicates with the server via a byte stream.
The application and server processes can therefore run on
two separate host machines and communicate over a network connection.
A client-side library of interface routines can then be used to
communicate to the X server's PEX extension.
The byte-stream
protocol for PEX is described in the
\fIPEX Protocol Specification\fP
.[
rost arch team protocol
.]
and the \fIPEX Protocol Encoding Document\fP.
.[
newman encoding
.]
.LP
PEX is designed to efficiently support PHIGS
.[
PHIGS ISO
.]
and the stable functionality enhancements
to PHIGS known as PHIGS+.
.[
ad hoc PHIGS+
.]
PHIGS and other graphics standards products
.[
GKS-3D ISO
.]
play a key role in the computer graphics marketplace.
Applications written using these standards are easily ported
to many different types of hardware.
PEX is intended to serve as a foundation for efficient implementation
of these graphics standards in the X Window System environment.
.LP
This interface is proposed as a standard 3D
graphics extension for the
X Window System.
Since this has many and far-reaching
implications, additional suggestions, thoughts, comments,
and criticisms are welcome.
.bp
.AC "Design Goals" 2
.LP
The section describes the goals that were focused upon during the
design of PEX.
The major goals of the PEX effort can be summarized as follows:
.DS
\(bu  Provide efficient support for PHIGS and stable portions of PHIGS+
\(bu  Gracefully extend the capabilities of X
\(bu  Support a performance range of X server platforms
\(bu  Define the protocol in a timely fashion
.DE
In addition, the goals of this project address the following major
areas of concern:
.DS
\(bu  Performance
\(bu  Extensibility
\(bu  Standardization
\(bu  Simplicity
.DE
.AC "Support for PHIGS and PHIGS+" 3
.LP
PEX should
serve as a foundation upon which to build
PHIGS and other graphics standards products.
It should contain
support for both structure mode (display list) graphics and immediate
mode graphics.
It should also be possible for vendors to extend PEX to support concepts
that are implemented in current workstation products but are not addressed
in the current ANSI/ISO graphics standards.  In particular, strong emphasis
should be placed on including support for the proposed
extension to PHIGS known as PHIGS+.  PHIGS+ functionality that is considered
to be unstable or conflicts with support in the X environment may be
deferred.  An implementation (public or proprietary)
of an application programmer's interface
that conforms to the PHIGS C language binding standard and to the PHIGS+
C language binding standard (when developed) is required in order to
determine how well this goal is met.  Existing PHIGS applications should
run on this interface with no more porting effort than is currently
required to move between conforming PHIGS implementations.
.AC "Gracefully Extend X" 3
.LP
The PEX protocol must be designed to provide an extension that
is compatible with the X protocol.  The X protocol and the PEX protocol
should be as seamless as possible.
In other words, PEX should look like an extension to the X Window
System and should not unnecessarily duplicate functionality contained
in X.  PEX should attempt to maintain the philosophy of X
as much as possible,
extend the philosophy in a manner consistent with existing philosophy
when it is insufficient, and go counter to X philosophy only
when it is necessary to achieve performance and functionality goals.
.LP
In order to allow PHIGS/PHIGS+ clients to access the capabilities
of the X Window System above and beyond the capabilities provided
through the PHIGS/PHIGS+ libraries themselves, 
it should be possible to add extensions to a PHIGS/PHIGS+ library
that expose at least some of the  functionality available in Xlib and
the XToolkit.
.AC "Performance Range of X Platforms" 3
.LP
It should be possible to support PEX across a range of
raster graphics workstation products: from
those that have little or no built-in 3D graphics support
to those that have a great deal of built-in 3D graphics capability.
In order to allow devices with little or no memory for structures to
support a PHIGS client,
PEX should include support for immediate mode graphics and it should
be possible for a PHIGS client library to support client-side traversal
of structures in an efficient manner.
.LP
Specific hardware limitations aside, nothing in the PEX protocol should
preclude an implementation of PEX on any device that supports X.  It should
be possible to support PEX in some fashion on monochrome, grayscale,
pseudo-color,
and true color devices.  Obviously, certain PEX capabilities will suffer
on machines without color, or with limited color capabilities, but these
devices should not be ignored entirely when designing PEX.
.AC "Timely definition of Protocol" 3
.LP
It is important to reach consensus with others in the X community as to
the precise definition of the PEX protocol.  It is highly desirable to
achieve such consensus within one year of the presentation of the
first proposal.
A sample implementation should
be attempted in order to shake out any remaining problems with the PEX
protocol definition.  It is desirable to complete this public implementation
effort in a timely fashion as well.
The sample implementation should be of at least some use to those
attempting implementations on any other device.  That is, as much
portability as can be reasonably attained is a goal in the development
of a public PEX implementation.
.AC "Subgoals" 3
.SH
\fIPerformance\fP
.LP
The interface should allow for an efficient flow of data from the
client down to the hardware level.
The interface should not preclude a high-performance implementation on
any of the target machines.
It is not expected that applications will execute with the same
performance characteristics on all PEX implementations.
Applications written to utilize the
specific high-performance features on a particular device may
perform poorly on devices that support the same features in software.
The PEX definition should provide guidance as to what type of functionality
should be included (minimum requirements) in an implementation,
but should not force implementation of functionality
that will perform unacceptably on a given device.
.SH 
\fIExtensibility\fP
.LP
Since the fundamental purpose of the X Consortium is to encourage research
and development in the realm of the X Window System,
it should be possible to easily extend and evolve the interface
towards more complete and consistent functionality.
It is quite important that the interface be designed in such
a way as to allow easy extension in various directions.
To help achieve this,
PEX should at all times strive to be "policy free".  This means that
it should provide useful 3D graphics functionality but should
attempt to defer policy decisions ("favors") to a higher level.
It should offer fundamental functions that support a variety
of layered environments, including current and future
graphics standards products.
.LP
At this time,
areas within 3D graphics hardware design that are the most dynamic
include support for light sources,
illumination/reflection models, hidden surface techniques, 
color representation, and methods of antialiasing.
Because of this, much more functionality has been considered than is included
in the initial design specification.  It is anticipated that additional
primitives (spheres, other solids)
and attributes (additional hidden surface
techniques, antialiasing algorithms, texture-mapping, additional
lighting/illumination models and light types) can
be added cleanly to PEX, since these extensions have already been given
special consideration in the current design.
.SH
\fIStandardization\fP
.LP
Because X has been ported to many different workstation products,
applications written for X can be used on a wide range of machines.
It is desirable to create a 3D interface that can also be implemented
on a wide range of machines so that 3D applications may achieve similar
portability.
It will be possible to run a PEX application
on any device for which a PEX implementation exists.
Such standardization can also help to minimize some
of the software development, maintenance,
and documentation burdens for customers and third-party software developers.
PEX is not intended to prevent vendors from developing 
additional proprietary 3D extensions to X, however it is hoped that
PEX will be supported by a number of vendors in the X community
so that customers may easily port applications to different vendors'
workstation products.
.SH
\fISimplicity\fP
.LP
The PEX protocol definition should be as simple, clean, and consistent
as possible, within the constraints posed by the other goals.
Ideally, simple things should be simple to understand and simple to do.
The models and abstractions presented in
PEX should be as simple and easy to understand as possible.
Applications writers should not be forced to learn the whole interface
before doing anything; rather, they should be able to learn quickly 
how to do simple things and systematically add to their knowledge
of PEX as necessary.
PEX should not contain functionality
that is of questionable usefulness or functionality that is redundant.
.LP
.bp
.AC "Non-Goals" 2
.LP
The goals section lists what the PEX project is trying to achieve
and the criteria by which design decisions will be made
for PEX.  This section contains a list of considerations that
are expressly \fInot\fP used in the design decision-making process.
This does not imply that these concepts are unimportant or will not be
given any consideration during the design process, but simply that these
are not specific goals of the PEX project.
.AC "PHIGS Mapping" 3
.LP
It is not a goal that the PEX protocol be a one-to-one mapping to
the set of PHIGS/PHIGS+ functions.  Instead, PEX should be thought
of as a sophisticated device handler for a class of devices - the class
of X devices.  PEX should be designed to support PHIGS/PHIGS+ functionality
efficiently in the X Window System, but it is not a requirement that
the PEX protocol "look like" the PHIGS/PHIGS+ interface.  Indeed, other goals
state that functionality already present in core X or planned
for other extensions
(e.g., input) should not be redundantly included in the PEX protocol definition.
Also, because of the network transparent nature of X,
PHIGS/PHIGS+ operations such as file storage and retrieval
are best performed by the client, so should not be included in the
PEX protocol definition.
Since it is desirable to keep the PEX server
extension as small as possible while achieving the goal of supporting
PHIGS/PHIGS+, effort should be made to support many similar PHIGS/PHIGS+
functions with a few generic routines.
It is desirable to retain a one-to-one mapping between PHIGS/PHIGS+ structure
elements and PEX structure elements for efficiency of editing, and between
PHIGS/PHIGS+ output primitives/attributes and PEX
output primitives/attributes
for efficiency of rendering on devices that already support
PHIGS/PHIGS+ output primitives and attributes.
.AC "Total Device Independence" 3
.LP
It is not a goal that the PEX protocol provide total device
independence for applications programmers.  In fact, PHIGS and PHIGS+
provide many query routines that enable applications to inquire
the limitations of the target device.  Such query operations should
be supported by the PEX protocol.
It should be possible for applications to be written in a highly portable
fashion despite these device dependencies.
.AC "Support for Non-3D Applications" 3
.LP
It is not a goal that PEX support functionality other than that found
in current or future graphics standards.  This is not to say that PEX
must contain only functionality that is present in such standards, but
that it is not a goal to support areas other than those supported by
these standards.  Specifically, it is not
a goal for PEX to address support for areas such as image processing,
document preparation, or advanced input.
It is assumed that
further research and development will be done to
determine the best way to provide support for these areas
in the X Window System.
.bp
.AC "X and PEX" 2
.LP
PEX has been designed to extend the capabilities of X
into the realm of 3D graphics.  X was designed to permit
network-transparent access to the input, windowing, and
2D graphics capabilities of
various workstations and display systems.  PEX permits
network-transparent access to 3D graphics capabilities of
such devices.
.LP
PEX employs the same client/server model as X.
An application process that requires access to input, windowing, or graphics
functionality is known as the \fIclient\fP.
The display is controlled by a process known as the \fIserver\fP.
A client application can open a connection and communicate with
the server via a byte stream.  The server can be listening to
several clients simultaneously.
.LP
PEX consists of
three major parts:
.DS
\(bu   The PEX server extension
\(bu   The PEX protocol definition
\(bu   A PEX client interface such as PHIGS/PHIGS+
.DE
.LP
The PEX server extension contains the code responsible for
performing 3D graphics operations on the device.  This extension
utilizes the extension mechanism that is built into X, Version 11.
It is highly
desirable to structure this server extension in a fashion similar
to the X server in order to promote portability.  Key to this
is to include the ideas of both a device-dependent layer and a
device-independent layer.
This will not be as straightforward as with X because some devices
include considerably more hardware support for 3D graphics
than others do.  The strategy
will most likely be to develop a "generic" implementation that runs on
a target machine with limited 3D graphics capabilities.
This server extension then can be ported fairly easily
even to machines with a great deal of 3D hardware support.  Over time,
the server extension
can be modified to take advantage of the increased 3D hardware
support, or have some of its capabilities migrated into microcode.
.LP
The PEX protocol defines the communication between client and
server.  This communication can occur over any reliable byte stream.
Some implementations will utilize sockets, others may use shared memory, and
still others will use various network protocols.
The X11 communications protocol is defined
in the document \fIX Window System Protocol, Version 11\fP.
.[
X window system protocol scheifler
.]
The \fIPEX Protocol Specification\fP
.[
rost arch team protocol
.]
and the \fIPEX Protocol Encoding Document\fP
.[
newman encoding
.]
define the communications protocol for PEX.
The PEX server will interpret
the minor opcode of each packet in the manner described by the
\fIPEX Protocol Specification\fP document.
.LP
The realm of PEX includes geometry in three-dimensional space, and
the realm of X is 2D raster space.  Because of this distinction,
X and PEX do not overlap in functionality.
Although both
might include ideas such as clipping and line styles, X operates
on 2D raster primitives and 2D windows and PEX operates on 3D
geometric primitives in three-dimensional space.
.LP
Figure 1.1 shows the relationship of a client interface library on top of PEX,
Xlib, the X11 and PEX protocols, the X server, and the PEX server extension.
.KS
.PS
boxht = 0.6i; boxwid = 0.8i;
ellipsewid = 1.5i; ellipseht = 0.75i;
Box1: box "PEX Client" "Interface"
Box2: box "Xlib" with .w at Box1.e
Arr1: arrow from (Box1.c.x + Box2.c.x)/2, Box1.s.y - 0.5 to Box1.s
Arr2: arrow from (Box1.c.x + Box2.c.x)/2, Box1.s.y - 0.5 to Box2.s
Box3: box "Core" "X Server" with .n at Arr1.start.x, Arr1.start.y - 1.0i
Box4: box "PEX" "Extension" with .e at Box3.w
Arr3: arrow from Arr1.start to Box3.n
line from Box4.w.x - 0.5, Box4.s.y to Box3.e.x + 0.5, Box4.s.y
"Device Hardware" at Box3.sw.x, Box3.s.y - 0.25
"  X11 Protocol" ljust at Arr3.c
"      + PEX Extension Protocol" ljust at Arr3.c.x, Arr3.c.y - 0.15
.PE
.sp 0.25i
.ce
\f6Figure 1.1: X/PEX Client-Server Relationship\fP
.KE
.LP
The entire PEX system meshes closely with the philosophy and
design of the X Window System.
Applications are
able to access 2D and 3D capabilities of various display
devices in an integrated and consistent fashion.
.bp
.AC "PEX Architecture Overview" 1
.LP
.RU
.LP
This section describes the PEX extension by defining the attributes
and actions of the various PEX resources, describing
the methods used to transfer floating
point and color information to and from these resources, and discussing
the type and format of the commands that can be sent to renderer
and structure resources.
.AC "PEX Overview" 2
.LP
This section contains a brief overview of the PEX system and
attempts to
define the terms that will be used throughout
the remainder of the document.
No attempt will be made to explain the concepts
any further than is necessary to accurately define them.  More detailed
descriptions of each of the concepts presented in this section can be
found in later sections.
This discussion assumes the reader is familiar with terms
defined in the X documents.
The term \fIcore X\fP is used to distinguish X from X
extensions such as PEX.
The \fIcore X server\fP is used to distinguish that part of the server
that supports the core X requests.  The \fIPEX extension\fP is used to
refer to that part of the server that supports PEX requests.
The term \fIserver\fP is used to refer to the
combination of the core X server and the PEX extension.
.AC "PEX Basic Operation" 3
.LP
Client applications can utilize the PEX extension by opening a
connection to the server, inquiring if the PEX extension
has been loaded, and loading it if necessary.  Client applications may
establish multiple connections.
All requests made via a particular connection happen atomically and in order,
regardless of whether they are core X or PEX extension
requests.
.KS
.PS

right


	C1:	ellipse "Client"
		move right 1
	C2:	ellipse "Client"

	MID:	1/2<C1.s,C2.s>

	XS:	box wid .8 ht .6 "Server" \
			with .n at MID + (0,-.7)

		arrow from C1.s to XS.n - (.2,0)
		arrow from C1.s to XS.n - (0,0)
		arrow from C2.s to XS.n + (.2,0)

		box invis "Connections" with .c at 1/2<MID,XS.n> + (0,.1)

.PE

.sp 0.25i
.ce
\f6Figure 2.1:  Client-Server Model\fP
.KE
.LP
Requests may be buffered in a per-connection queue if the
server cannot keep up with incoming requests.
Synchronization of requests across multiple connections
is left up to the client.  Such requests will still happen atomically,
but the order in which they will be handled is indeterminate.
A process in the X server called the \fIcore dispatcher\fP scans
incoming requests and hands all PEX commands over to the \fIPEX dispatcher\fP.
The PEX dispatcher determines what to do with all PEX commands.
.KS
.PS

boxwid = .8
boxht = .6

right

	RE:	box invis "Requests"
		arrow

	CQ:	box "Connection" "Queue"
		arrow

	DCX:	box "Core X" "Dispatcher"
		arrow right 1 "PEX" "Requests"

	DX3:	box "PEX" "Dispatcher"

	CX:	box dotted "Core X" \
			with .n at DCX.s - (0,.5)

	X3:	box dotted "PEX" \
			with .n at DX3.s - (0,.5)
		
		arrow from DCX.s to CX.n
		arrow from DCX.s - (.1,0) to CX.n - (.2,0)
		arrow from DCX.s + (.1,0) to CX.n + (.2,0)

		arrow from DX3.s to X3.n
		arrow from DX3.s - (.1,0) to X3.n - (.2,0)
		arrow from DX3.s + (.1,0) to X3.n + (.2,0)

.PE

.sp 0.25i
.ce
\f6Figure 2.2:  X and PEX dispatchers\fP
.KE
.LP
.AC "PEX Rendering Pipeline" 3
.LP
The process of turning a geometrical description of an object together
with its attributes (colors, normals, etc.) into pixel data suitable
for writing into a pixmap or window is called \fIrendering\fP.
Since the rendering operation can be described as a series of
discrete steps, or stages, it is often described as a \fIpipeline\fP.
The attributes that define the behavior of the pipeline are collectively
referred to as \fIrendering pipeline state\fP or just \fIpipeline state\fP.
To render 3D object descriptions,
client applications can issue sequences of commands called
\fIoutput commands\fP.
Output commands are the
only type of commands that can be presented to a PEX rendering pipeline.
.LP
.AC "PEX Resources" 3
.LP
PEX introduces nine new types of resources.
.ID
\(bu  Lookup Tables
\(bu  Pipeline Contexts
\(bu  Renderers
\(bu  Structures
\(bu  Name Sets
\(bu  Search Contexts
\(bu  PHIGS Workstations
\(bu  Pick Measures
\(bu  PEX Fonts
.DE
.LP
New PEX resources have the properties shared by all X resources.
Each resource has a unique identifier associated with it for naming purposes.
Resources are assigned names (\fIresource ids\fP) on the client side of the
network interface.  Any process on the client side can manipulate any
resource in the server as long as it knows the resource id of the resource.
In general, the lifetime of a resource is bounded by the lifetime of the
connection over which the resource was created (but X provides
a method for overcoming this limitation).  Resources can be created
and freed, and most have attributes that can be modified or queried.
Some resources can be copied.
Each of the PEX resources is introduced briefly here and
will be discussed in more detail in its own section.
.LP
One of the two PEX resources that is capable of transforming geometric
data into raster data is called a \fIrenderer\fP.  Each renderer
contains a built-in rendering pipeline.  All of the
attributes that make up a renderer's pipeline state
are stored in a PEX resource called
a \fIpipeline context\fP.  These attributes are copied to a renderer
in order to provide the initial pipeline state.
When rendering,
some of the pipeline state values may be obtained indirectly from
PEX resources called \fIlookup tables\fP.  There are a number
of types of lookup tables that have been defined by PEX, and each
table consists of zero or more \fItable entries\fP.  Each entry
has an \fIindex\fP by which it can be accessed.  Table entries contain
different types of data for each of the different lookup table types.
\fIName sets\fP are a type of resource that allow for a restricted
type of conditional operation during the rendering process.  For instance,
name sets can be used to determine whether specific output primitives
are visibile or invisible, or whether or not specific primitives should
be highlighted.
.LP
Client applications can build up lists of output commands
that are known as \fIstructures\fP.
Individual structures can be linked
together hierarchically to create \fIstructure networks\fP.
The act of flattening a structure network to provide a stream of
output commands for a rendering pipeline is called \fIstructure traversal\fP.
The first structure in a structure network is
referred to as the \fIroot structure\fP.
Individual commands in structures are known as \fIstructure elements\fP.
Each structure maintains an offset into the structure called an
\fIelement pointer\fP which can be used for editing operations.
An attribute within the structure, called the \fIediting mode\fP,
determines whether elements will be inserted
at the element pointer position, or whether they will replace elements at the
element pointer position.
A resource that is useful for searching a structure network for output
primitives that meet certain constraints is called a \fIsearch context\fP.
.LP
A \fIPHIGS workstation\fP resource contains the capabilities of a renderer
resource.  It too is capable of turning geometric data into raster data.
However, a PHIGS workstation can only obtain output commands from the
structures in its \fIposted structure list\fP, it cannot be passed output
commands directly in the same way that a renderer can.
The PHIGS workstation  maintains a great deal of additional state in order to
efficiently support the PHIGS notion of a "workstation".
PHIGS workstations contain descriptions of devices that can be
used for \fIpicking\fP (selecting a primitive from those that are
displayed).  These descriptions are called \fIpick device descriptors\fP.
In order to perform a picking operation, a \fIpick measure\fP resource
must be created.  The pick measure is initialized using the values
found in a pick device descriptor.
It "measures" output
primitives in order to find those that meet a particular selection
criteria (e.g., closest to a specified point).
.LP
Finally, PEX introduces the \fIPEX font\fP resource, which is a text
font that is capable of fully realizing all of the text and character
attributes defined in the PEX text and annotation text output primitives.

.AC "PEX Execution Semantics" 3
.LP
The X execution semantics state that
all requests are considered to be \fIatomic\fP (i.e.,
individual requests are executed to completion in some serial order)
and that requests received over a single connection are executed 
in the order they are received.
The execution semantics of PEX requests are identical to those of
X requests, therefore all PEX requests are considered atomic.
Although structure traversal and rendering operations
may be slow, they are still considered to be atomic.
.LP
Most server implementations, including the sample server from M.I.T.,
are \fIsingle-threaded\fP, in that the server handles requests one
at a time and processes each one to completion before going on.
PEX operations such as structure traversal and rendering may
take considerable time to complete.
This can lead to unacceptable behavior from a client's point of view,
since a lengthy operation can prevent the server from processing any of its
requests in a timely fashion.
For example, a client that initiates a
structure traversal can
monopolize the server's ability to process requests, effectively preventing
another client from doing simple text editing in another window.
\fIMulti-threaded\fP servers avoid this behavior
by allowing other requests to be processed while lengthy
operations are occurring.
Multi-threaded servers can take advantage of multiple procesors.
Even a uniprocessor system can support a multi-threaded server by using
techniques such as yielding or coroutines.
.LP
If a server is implemented with internal concurrency,
the overall effect must still be to obey the X
execution semantics.
One way to ensure this is to prevent
separate requests from operating on the same resources at the same time.
In this case, a connection will \fIblock\fP 
if a request requires access to
a resource that is already engaged in a lengthy operation.
After the lengthy operation is completed, the 
connection will \fIunblock\fP and the request will be processed.
For instance, if a client starts up a structure traversal
and then reads back the pixels using a core X request,
the "read pixels" operation will not occur until the traversal has completed.
Another example involves a structure traversal request followed
by requests that modify the structures being traversed.
Normally, the editing operations would block until the traversal
was complete.
However, since structure traversal is defined to be a read-only
operation, a conceptual copy of the affected structures can be made
so that the editing can occur simultaneously.  This conceptual copy operation
can be implemented in various ways, including actually making
a physical copy or maintaining a list of pending edits.
.AC "PEX Events" 3
.LP
PEX introduces no new X events.
.AC "PEX Error Handling" 3
.LP
PEX introduces some new error types that are returned in the same way
that errors are returned for X itself.  No additional
error-handling capabilties are provided beyond those already provided by X.
.bp
.AC "PEX Extension Information" 2
.LP
To determine if the PEX extension is present in a specific server,
clients can issue the core X requests \fBListExtensions\fP or 
\fBQueryExtension\fP.  \fBListExtension\fP returns a list of the
names of the extensions that are supported in the server.  If the
PEX extension is present in the server, the null-terminated string "PEX"
will be returned as part of the list of extension names.
A client can explicitly determine whether or not the PEX extension
is present using the core X \fBQueryExtension\fP request and
passing the string "PEX".  This request will return a boolean value
stating whether or not the PEX extension is present.  If it is,
an opcode is returned that distinguishes PEX requests from core X and
other extension requests.  This opcode must then be included as part
of every PEX request that is issued.  The value representing the
first error in the list of errors added by PEX will also be returned.
.LP
The design of the PEX extension is flexible enough to allow different
types of implementations on different types of machines.  The first
PEX request that any client should make is \fBPEXGetExtensionInfo\fP.
This request returns information about the PEX extension in the server,
including the major version and revision numbers, a string representing
the vendor that created or supplied the PEX extension, and an indicator
that specifies whether the PEX server is a complete PEX implementation
or one of the standard PEX subsets.
.PP
Since PEX is a potentially large extension, and since some devices that
support X have severe limitations in the amount of main memory that
is available, two standard PEX subsets have been defined.  Either of
these subsets is capable of supporting a complete, client-side
PHIGS/PHIGS+ library, although perhaps less efficiently than a
complete PEX implementation might.  One permitted subset is called
the "immediate-mode only" subset.  This subset only requires that
the PEX lookup table, name set, pipeline context, renderer, and PEX
font resources be supported.  The second permitted subset is called
the "PHIGS workstation only" subset, which does not require support
for the requests that deal with renderers and pipeline contexts, but
requires support for all remaining PEX resources.
Except for the "execute structure" output command (which may be
treated as a no-op by the immediate mode subset), both subsets require
support for all of the defined output commands.
.PP
Another query request that can be performed by a client at PEX
initialization time is \fBPEXGetEnumemratedTypeInfo\fP.
With PEX, there are various choices for how one might do certain things.
For instance, there are many ways to draw marker primitives, there
are various line styles that might be appropriate to use, and there are
any number of methods that can be used to compute the color of light that
is reflected from a point on an object.  For the cases where there
are several choices for a particular attribute or function, PEX defines
an enumerated type.  Most enumerated types contain a set of defined
or registered values with behavior that is described in the PEX protocol
definition.  A PEX implementation need not always support all the registered
values.  PEX implementors may also choose to support types in addition 
to the registered values.  The \fBPEXGetEnumeratedTypeInfo\fP can be
used to inquire the list of supported values (both registered and proprietary)
for any or all of the defined enumerated types.
.PP
Finally, each PEX implementation may have certain limits to the number of,
or size of, or ability to perform certain things.  Clients can query a PEX
extension to determine these implementation-dependent constants using
the \fBPEXGetImpDepConstants\fP request.  This request allows clients
to determine such things as minimum and maximum supported line widths,
nominal marker size, whether dithering and transparency are supported or
ignored, and so on.
.bp
.AC "PEX Data Types" 2
.LP
Due to potential differences between the floating-point types and
direct color formats that client and server CPU's prefer, certain PEX requests
require that \fIformat information\fP parameters
be passed as part of the request.
These parameters specify the format of floating-point and direct color
values that are passed as part of the request.
These format parameters are essentially used as translators between
the format in which a client sends data and the format in which the
data is stored or processed by the server.  When sending data to
the PEX extension, format parameters are provided so that the
server is capable of interpreting the data that is sent by the client.
When reading data from
the PEX extension, format parameters are provided so that the
server can translate the data into the form desired by the client.
Data values are effectively converted to the specified formats
the instant they are received by the server, or the last moment
before they are sent back to the client.
.LP
The \fIfloating-point format\fP allows client applications to overcome the
possible difference between the
floating-point representation on the client
CPU and the
floating-point representation on the
server CPU.
Whenever floating-point numbers are passed to the server,
they are assumed to be in the format specified by the
floating-point format parameter in the request.
When data is returned to the client as the result
of a query, the server will return floating-point numbers
in the format specified by the floating-point format parameter in
the query request.
.LP
PEX allows color values to be passed either directly
(\fIdirect color values\fP)
or as indices into a color lookup table (\fIindexed color values\fP).
Direct color values are specified as true color values in a specific color
space such as RGB, HSV, HLS, or CIE.
Indexed color values are specified by
providing a 16-bit index into a PEX lookup table resource
that is of type \fIColor\fP.
Since it is not possible to arbitrarily convert direct color values
to indexed color values and vice versa,
a \fIcolor type\fP flag is used to indicate whether color values
are direct or indexed color values.
Conceptually, each color that is passed to or queried from the PEX
extension contains two pieces of information: the color type and the
color value.  In practice, a single request may
contain a single color type and a list of color values,
and each value in the list is assumed to be of the indicated color type.
Therefore, in the context of the PEX protocol, a \fIcolor\fP 
consists of a color value
and either an implicit or explicit color type.  When colors are stored
in structure elements, both the color value and the color type must be
stored in the structure so that the color value can be interpreted when
the element is executed.
Both the color value and the color type will also be returned to the client
whenever such a structure element is queried.
.LP
The "preferred format"
of direct color values varies from one machine to the next and
from one application to the next.
PEX overcomes this potential disparity by allowing
the client to specify the
\fIdirect color format\fP as a parameter to requests that cause
direct color values to be passed
to or from the server.
Whenever direct color values are passed to the server,
they are assumed to be in the format specified by the
direct color format parameter in the request.
When data is returned to the client as the result
of a query, the server will return direct color values
in the format specified by the direct color format parameter in
the query request.
.LP
The server is free to store floating-point values and direct color
values in whatever
representation it finds most convenient.
Clients must realize that
some precision may be lost when values are converted into the server's
internal format, and then back to the format specified by the client.
For instance, color values may be stored
as 8-bit integers within the server.  Some precision will be lost when
color values are converted from floats to 8-bit
integers so they can be stored in the server, and then back to floats
when queried by a client.
.LP
Floating point values and direct color values will be returned in the 
formats specified by a parameter in the query
request, \fInot\fP the formats that
were used when the values were sent to the server.  For example, assume a
server supports both DEC F-Floating and IEEE-754-32
floating-point types, and that
client A prefers DEC F-Floating format
and client B prefers IEEE-754-32 format.
Client A can issue an output command in DEC F-Floating
format and create
a structure element.  Client B can read back the structure element and
indicate that floating-point values should be returned in IEEE-754-32 format.
The server is responsible for converting data to any
type it claims it supports.
.bp
.AC "PEX Output Commands" 2
.LP
A PEX client can cause images to be drawn into an X window or pixmap
by issuing \fIoutput commands\fP.  Output commands can be directed
at a PEX renderer resource for immediate processing via the
\fBPEXRenderOutputCommands\fP request (\fIimmediate mode\fP), or
directed to a PEX structure resource for later execution using the
\fBPEXStoreElements\fP request.  Both requests take as a parameter
a heterogeneous list of output commands of arbitrary length.
The three types of output commands are:
.DS
\(bu  output primitives
\(bu  pipeline attributes
\(bu  miscellaneous
.DE
\fIOutput primitives\fP are commands that can actually cause raster data
to appear in a window or pixmap (e.g., draw polyline, draw marker, draw
triangle strip).
\fIPipeline attributes\fP are commands that modify the behavior of
the rendering pipeline for all primitives, regardless of what type
they are (e.g., set global modeling transformation matrix, set view index,
set pick id),
or commands that modify an attribute
of a particular class of output primitive (e.g., set line color,
set marker type, set surface interior style).
\fIMiscellaneous output commands\fP are neither attributes, nor primitives,
but include such things as "execute structure" and "application data"
that cannot be otherwise classified.
.AC "Output Primitives and Pipeline State" 3
.LP
PEX supports a number of geometric primitives, including
markers, text, lines,
surfaces, and cell arrays.
Polylines and parametric curves are considered to be "lines"
in that they share the pipeline attributes for lines;
fill areas, meshes, and parametric patches
are examples of surfaces.
.AC "Markers" 4
.LP
A \fImarker\fP is a two-dimensional glyph, or symbol, that represents
a location in three-dimensional space.  The marker's position is
specified in modeling coordinates and transformed within the
rendering pipeline like other output primitive coordinates.  Since
a marker has no surface from which to reflect light, the light-source
illumination stage of the rendering pipeline has no effect on markers.
However, a marker's color is affected by the depth-cueing computation
when depth-cueing is enabled.
.LP
The pipeline attributes that affect markers are:
.DS
\(bu  marker color
\(bu  marker type
\(bu  marker scale
\(bu  marker color asf
\(bu  marker type asf
\(bu  marker scale asf
\(bu  marker bundle index
.DE
The \fImarker color\fP is the intrinsic color of the marker.  This color
can be modified by the depth-cueing computation.
The resulting color is then transformed into a color the device is capable of
displaying.
There are two output commands for setting the marker color attribute.
The first allows a client to set the marker color to a particular indexed
color value (provided for strict compatibility with PHIGS).  The second
is more general, and allows the marker color to be set to either an
indexed color value or a direct color value.
The \fPmarker type\fP is simply the glyph that will be used to represent the
marker's position.  A list of the supported marker types can be returned
via the \fBPEXGetEnumeratedTypeInfo\fP request.
The \fImarker scale\fP is used to affect the size of the rendered marker
glyph.  The \fInominal size\fP of a marker is the size that will be used
if the marker scale is set to 1.0.  The marker's displayed size
is computed by multiplying the marker's nominal size by the
marker scale value, and then selecting the nearest available size
for the marker.  It is permissible for implementations to only support
a single size for marker primitives.  An exception to the scaling attribute
is the marker type \fIDot\fP, which is always displayed as the smallest
displayable dot on the workstation.
.LP
Whenever a marker primitive is rendered,
each of the marker attributes can either be obtained directly from the current
pipeline state, or be obtained
indirectly from a \fImarker bundle table\fP.  The source for the attribute
value is determined by the current settings of the marker \fIattribute source
flags\fP, or ASF's.  If an attribute's ASF is set to \fIIndividual\fP, the
attribute value is obtained directly from the current pipeline state.
If an attribute's ASF is set to \fIBundled\fP, the
attribute value is obtained from the current marker bundle table.
The marker bundle table entry to be used
is indicated by the \fImarker bundle index\fP attribute.
\fIMarker color asf, marker type asf\fP, and \fImarker scale asf\fP are the
attributes that control the source for the marker attribute values.
.LP
The treatment of markers during hidden surface computations depends on
the HLHSR mode that is selected and its implementation.  It is likely
that z-buffer algorithms will perform the hidden surface computation
on the 2D realization of marker glyphs, while scan-line algorithms
may determine visibility for markers solely on the basis of the 3D position
of the marker.
Clipping is performed on the marker position.  If the marker
position is barely outside the clipping volume,
no portion of the marker glyph will
be drawn.  Clipping of portions of marker glyphs that extend beyond the
clipping volume is implementation-dependent.
.AC "Text" 4
.LP
PEX supports two types of text primitives: regular text and
annotation text.  Regular text primitives cause text to be drawn
in three dimensions, while annotation text is required to appear parallel
to the view surface.  The two types of text primitives share
certain pipeline attributes, and have other attributes
that are distinct.
.LP
PEX text primitives are similar in many ways to marker primitives.
A \fItext string\fP is a list of characters that are to be drawn at a
particular location in three-dimensional space.
Like markers, text positions are
specified in modeling coordinates and transformed
into screen coordinates within the
rendering pipeline.  Like markers, 
a text string has no surface from which to reflect light, so the light-source
illumination stage of the rendering pipeline has no effect.
Depth-cueing, if enabled, does affect the color of text primitives.
.LP
The settable attributes of text and annotation text strings are:
.ID
\(bu   text font index
\(bu   text precision
\(bu   character expansion
\(bu   character spacing
\(bu   text color
\(bu   character height
\(bu   character up vector
\(bu   text path
\(bu   text alignment
\(bu   annotation text height
\(bu   annotation text up vector
\(bu   annotation text path
\(bu   annotation text alignment
\(bu   annotation text style
\(bu   text font index asf
\(bu   text precision asf
\(bu   character expansion asf
\(bu   character spacing asf
\(bu   text color asf
\(bu   text bundle index
.DE
The \fItext color\fP is the intrinsic color of the text or annotation text
string.  This color
can be modified by the depth-cueing computation.
The resulting color is then transformed into a color the device is capable of
displaying.
There are two output commands for setting the text color attribute.
The first allows a client to set the text color to a particular indexed
color value (provided for strict compatibility with PHIGS).  The second
is more general, and allows the text color to be set to either an
indexed color value or a direct color value.
The \fItext font index\fP specifies which entry in the current \fIfont table\fP
is to be used.  Definitions for character glyphs will be obtained from the
X11 or PEX font whose resource id is stored in the font table entry indicated
by the text font index.
.LP
The remaining attributes are defined exactly as they are for PHIGS.
Examples of the functionality and use of each can be found in the PHIGS
specification document.
.[
PHIGS ISO
.]
.LP
Whenever a text or annotation text primitive is rendered,
each of the attributes can either be obtained directly from the current
pipeline state, or be obtained
indirectly from a \fItext bundle table\fP.  The source for the attribute
value is determined by the current settings of the corresponding attribute
source flags (ASF's).  If an attribute's ASF is set to \fIIndividual\fP, the
attribute value is obtained directly from the current pipeline state.
If an attribute's ASF is set to \fIBundled\fP, the
attribute value is obtained from the current text bundle table.
The text bundle table entry to be used
is indicated by the \fItext bundle index\fP attribute.
\fIText color asf, text font index asf, text precision asf,
character expansion asf\fP,
and \fIcharacter spacing asf\fP are the
attributes that control the source for the text attribute values.
.LP
The treatment of text and annotation text
strings during hidden surface computations depends on
the HLHSR mode that is selected and its implementation.  It is likely
that z-buffer algorithms will perform the hidden surface computation
on the 2D realization of the text string, while scan-line algorithms
may determine visibility for text strings only on the basis of the 3D positions
of each string.
Clipping of text strings is dependent on the text precision attribute.
If the text precision is \fIString\fP, clipping is done in an
implementation-dependent fashion.
If the text precision is \fIChar\fP, clipping is done on at least a
character-by-character basis.
If the text precision is \fIStroke\fP, each character is clipped to
the clipping boundaries.
.AC "Lines" 4
.LP
PEX supports a class of primitives called \fIlines\fP.  Lines are
one-dimensional primitives in the sense that they have length, but
no height or width (although the representation of a line on the display
always has width in order that it can be seen).  Lines consist of a
sequence of vertices that are to be joined together by straight line
segments or a sequence of control points that are used to specify
a curved line.  The three types of lines supported in PEX are
\fIpolylines\fP, \fIparametric polynomial curves\fP, and
\fInon-uniform B-spline curves\fP.  A polyline is specified using
a list of vertices that are to be connected with line segments.
Parametric polynomial curves are specified with a list of control
points and a value that selects the type of basis functions that
are to be used.  Non-uniform B-spline curves are defined with a
list of knot values and a set of control points.
.LP
The settable attributes of lines are:
.ID
\(bu   line type
\(bu   line width
\(bu   line color
\(bu   curve approximation
\(bu   polyline interpolation method
\(bu   line type asf
\(bu   line width asf
\(bu   line color asf
\(bu   curve approximation asf
\(bu   polyline interpolation method asf
\(bu   line bundle index
.DE
The \fIline color\fP is the intrinsic color of the line primitive.
This color can be modified by the depth-cueing computation.
The resulting color is then transformed into a color the device is capable of
displaying.
There are two output commands for setting the line color attribute.
The first allows a client to set the line color to a particular indexed
color value (provided for strict compatibility with PHIGS).  The second
is more general, and allows the line color to be set to either an
indexed color value or a direct color value.  Polyline primitives can
include a color value at each vertex.  The \fIpolyline interpolation
method\fP defines how color values provided at the vertices will be used
when drawing polylines.  If the polyline interpolation method is \fINone\fP,
it is implementation-dependent whether
the color value at the \fIi\fPth vertex will be used to draw the line
connecting the \fIi\fPth and \fI(i+1)\fPth vertices (the color for the
last vertex is unused), or whether the average of the color of each
adjacent pair of points is computed and used.
If the interpolation method is \fIColor\fP,
a linear interpolation is performed between the vertex colors in order
to draw points along each line segment.
The interpolated colors will then be depth-cued and transformed
into device colors.
.LP
The \fIline type\fP defines the visual style of line primitives that
are drawn on the screen.  Possible values for this attribute include
solid, dashed, dotted, etc.  For lines types other than solid, the length
of dots, dashes, and spaces is implementation-dependent.  The \fIline width\fP
attribute is a scale factor that is multiplied by the nominal line width
(usually one pixel) for the display.
The line type and line width attributes are applied to the
2D projection of the line in device coordinates, not to the 3D primitive itself.
The \fIcurve approximation\fP attribute is used to define the fidelity
with which parametric polynomial curves and non-uniform B-spline curves
will be drawn.
.LP
Whenever a line primitive is rendered,
each of the attributes can either be obtained directly from the current
pipeline state, or be obtained
indirectly from a \fIline bundle table\fP.  The source for the attribute
value is determined by the current settings of the corresponding attribute
source flags (ASF's).  If an attribute's ASF is set to \fIIndividual\fP, the
attribute value is obtained directly from the current pipeline state.
If an attribute's ASF is set to \fIBundled\fP, the
attribute value is obtained from the current line bundle table.
The line bundle table entry to be used
is indicated by the \fIline bundle index\fP attribute.
\fILine color asf, line type asf, line width asf, curve approximation asf\fP,
and \fIpolyline interpolation asf\fP are the
attributes that control the source for the line attribute values.
.AC "Surfaces" 4
.LP
PEX supports a class of output primitives called \fIsurfaces\fP.
Surfaces are two-dimensional primitives in the sense that they define
an area, but have no volume.  PEX surface primitives
include \fIfill area\fP, \fIfill area set\fP, \fIindexed polygons\fP,
\fItriangle strips\fP, \fIquadrilateral meshes\fP,
\fIparametric polynomial surfaces\fP, and \fInon-uniform B-spline surfaces\fP.
.LP
A \fIpolygon\fP or a \fIfill area\fP
is a series of points that define the boundary of a planar
surface.  The last point is always implicitly connected to the first point
so that the polygon is closed.
Polygons are not required to be strictly
planar but strange shading artifacts may occur if the polygon is not
planar or nearly so.  Strange shading artifacts may also occur if
a polygon is not planar in color space or nearly so.  A \fPfill area set\fP
is comprised of a number of fill areas, each of
which is considered to define an "island" or a "hole" that is part
of a single, planar, area-defining output primitive.
An \fIindexed polygon\fP contains a list of shared vertices and a set
of pointers to vertices for each polygon.
This primitive is useful for representing
surfaces made up of polygons that share information at the vertices.
The list of vertices is numbered starting from zero.
A \fItriangle strip\fP is specified as a list of \fIn\fP vertices that
represent \fIn-2\fP triangles.  For each value of \fIi\fP less than
or equal to \fIn-2\fP, the \fIi\fPth, \fI(i+1)\fPth, and \fI(i+2)\fPth
vertices define a triangle.
A \fIquadrilateral mesh\fP is specified as an array of \fIm \(mu n\fP
vertices that represent (\fIm-1\ *\ n-1\fP) quadrilaterals.  For each 
each \fIi, j\fP (\fIi\ <\ m\ -\ 1, j\ <\ n\ - 1\fP) a quadrilateral
is formed by the vertices (\fIi,j\fP), (\fIi,j+1\fP), (\fIi+1,j\fP),
and (\fIi+1,j+1\fP).
A \fIparametric polynomial surface\fP is
a spline surface defined by a set of control points and a surface basis type.
A \fInon-uniform B-spline surface\fP is
a B-spline surface specified with two lists of unevenly-spaced knot values
and a set of control points.
A set of \fItrimming loops\fP can be specified along with the data
for a non-uniform B-spline surface in order to specify those areas
of the surface that are not to be rendered.
Together, parametric polynomial surfaces and
non-uniform B-spline surfaces are referred to as \fIparametric surfaces\fP.
Parametric surfaces can be either rational or non-rational.
.LP
The settable attributes of surfaces are:
.ID
\(bu   interior style
\(bu   interior style index
\(bu   surface color
\(bu   reflection attributes
\(bu   reflection model
\(bu   surface interpolation method
\(bu   back face interior style
\(bu   back face interior style index
\(bu   back face surface color
\(bu   back face reflection attributes
\(bu   back face reflection model
\(bu   back face surface interpolation method
\(bu   surface edges
\(bu   surface edge type
\(bu   surface edge width
\(bu   surface edge color
\(bu   surface approximation
\(bu   trim curve approximation
\(bu   culling mode
\(bu   distinguish
\(bu   normalflip
\(bu   pattern size
\(bu   pattern reference point
\(bu   pattern reference vector 1
\(bu   pattern reference vector 2
\(bu   interior style asf
\(bu   interior style index asf
\(bu   surface color asf
\(bu   reflection model asf
\(bu   surface interpolation asf
\(bu   reflection attributes asf
\(bu   back face interior style asf
\(bu   back face interior style index asf
\(bu   back face surface color asf
\(bu   back face reflection model asf
\(bu   back face surface interpolation asf
\(bu   back face reflection attributes asf
\(bu   surface edges asf
\(bu   surface edge type asf
\(bu   surface edge width asf
\(bu   surface edge color asf
\(bu   surface approximation asf
\(bu   trim curve approximation asf
\(bu   edge bundle index
\(bu   interior bundle index
.DE
.LP
\fISurface color\fP is the intrinsic color of the surface.  This color
can be modified by the light source shading and depth-cueing computations.
The resulting color is then transformed into a color the device is capable of
displaying.
Surfaces are the only output primitives affected by the light source
shading computation.
.LP
The \fIinterior style\fP determines how the interior of the surface is to
be rendered.  Choices for the interior style include \fIEmpty\fP (the
interior is not drawn at all),
\fISolid\fP (the interior is filled),
\fIHollow\fP (only the boundary of the interior area is drawn,
and if the surface interior
is clipped, the boundary along the clipped edge is also be drawn),
\fIHatch\fP (use the \fIinterior style index\fP to select
the pattern of parallel and/or crossing lines from the list of
supported hatch style types that is
to be used when drawing the surface interior), and
\fIPattern\fP (use the \fIinterior style index\fP as an index into a
pattern table to obtain the pattern to be used when drawing the
surface interior and use the \fIpattern size\fP, \fPpattern reference point\fP,
and \fIpattern reference vectors\fP to map the pattern onto the surface
interior).  Support for the \fIHatch\fP and \fPPattern\fP styles
is not mandated, and because they were developed for use with displays
with limited color resolution and because of
the difficulty involved in implementing these
styles with the full range of surface types and lighting effects, it
is quite possible that some implementations will not support them.
.LP
The \fIreflection model\fP selects the type of reflection computation
that is to be used.
Ambient, diffuse, and
specular reflection parameters
are used by some types of reflection computations.
Depending on the setting of the surface interior style,
the surface interpolation method, and the data passed along with
a surface primitive, the reflection model computation may be performed
once for the entire surface, once for each vertex, once for each pixel
covered by the surface, or not at all.
The \fIsurface interpolation\fP method
is used to determine how the reflection model is to be used in the
interiors of surfaces.
.LP
Surfaces may obscure any other type of output primitive from view, including
other surfaces.
A surface is clipped if it extends beyond the clipping boundary, and only
the portion within the clipping boundary will be rendered.
.LP
Clients may pass a geometric normal along with some primitives.  If
not provided as part of the primitive, geometric normals are computed
by the PEX server extension.  These geometric normals and the
\fIculling mode\fP attribute may be used to discard portions of primitives
that are back-facing or front-facing with respect to the viewing position.
If the \fIdistinguish\fP flag is set to true, the set of back face surface
attributes will be used to render back-facing portions of primitives.
Otherwise, the standard surface attributes will be used for both front-
and back-facing portions of output primitives.  If the \fInormalflip\fP
flag is true, geometric normals are inverted prior to performing the
shading computations.
.LP
The \fIsurface edges\fP attribute indicates whether surface edges are to
be drawn in addition to the surface interiors.
Edges are considered to be
drawn "on top of" surface interiors, hence surface edges, when enabled,
should be visible no matter what interior style is used.
The surface edges for fill areas, fill area sets, indexed polygons,
triangle strips, and quadrilateral meshes are the just the edges of each
of the constituent polygons.
Edges for the parametric polynomial surface and the non-uniform B-spline
surface are just the edges of the
surface patch.  In addition, depending on the visibility flags passed
along with them, the trimming loops for
non-uniform B-spline surfaces may also be treated as surface edges.
If surface edges are drawn, the \fIsurface edge color\fP,
\fIsurface edge type\fP, and \fIsurface edge width\fP
are used to render them.  These attributes are analogous to the
attributes that affect line primtives.
Surface edge colors may be depth-cued, but
are not affected by light-source computations.  Surface edge type and width,
like line type and width, are raster attributes that are applied after
the surface edge has been transformed into raster coordinates.
Parts of surface edges that are clipped are not visible (no new edge
is generated by the clipping process).
.LP
[A more complete description of parametric surfaces is TBD, for now
see the PHIGS+ specification.]
.AC "Other Output Primitives" 4
.LP
PEX contains some additional output primitives that have not been
discussed in any of the previous categories.
A \fIcell array\f is a parallelogram of equal-sized cells, each of which
is a parallelogram and has a single color.
Since it defines a two-dimensional area, it is similar to a surface
primitive.  Although it does not utilize any of the surface attributes
described in the preceding section, it can obscure other primitives.
Depth-cueing is applied to cell arrays, but the surface interpolation
method and reflection model attributes are not applied to cell arrays.
Because they may be difficult to implement,
an implementation is allowed to simulate a cell array by drawing the
parallelogram that defines the boundary of the cell array.
.LP
For compatibility with PHIGS, PEX provides a generalized drawing primtive
(GDP) output command.  The purpose of the GDP is to allow
implementation-dependent drawing primitives to be added cleanly to PHIGS.
However, due to the fact that the format of the data within a GDP
is implementation-dependent and may contain floating point and direct
color values, it is not anticipated that GDP's will provide
a useful extension mechanism in a heterogeneous network environment.
.LP
.AC "Pipeline Attributes" 3
.LP
The pipeline state values that affect only a certain type of output primitive
were discussed in the previous section.
Pipeline state values that
affect all output primitives in the rendering pipeline regardless of type
include the following:
.ID
\(bu   local transform
\(bu   global transform
\(bu   model clip
\(bu   model clip volume
\(bu   view index
\(bu   light state
\(bu   depth-cue index
\(bu   pick id
\(bu   HLHSR identifier
\(bu   name set
.DE
.LP
The \fIlocal transform\fP and \fIglobal transform\fP are modeling matrices
that are concatenated to create the \fIcomposite modeling matrix\fP.
The composite modeling matrix contains the transformation that is used
to map output primitives from modeling coordinates into world coordinates.
.LP
The \fImodel clip\fP attribute is a flag that indicates whether or not
modeling clipping is to be performed.  If modeling clipping is enabled,
output primitives are clipped to the \fImodel clip volume\fP prior
to being submitted to the viewing and projection transformations.
PEX contains output commands that can modify or reset the modeling
clipping volume.
.LP
The \fIview index\fP is used to indicate the entry in a view table from
which the view orientation matrix, the view mapping matrix, and the
clipping indicators are to be obtained.  The \fIlight state\fP indicates
those lights in the light table that are enabled and those that are disabled.
The \fIdepth-cue index\fP indicates the entry in a depth-cue table
from which the depth-cueing parameters are to be obtained.  The \fIpick id\fP
maintains an identifier which is used to provide further identification
for primitives during picking operations.  The \fIHLHSR identifier\fP
provides implementation-dependent HLHSR information.
.LP
PEX also contains output requests that can be used
to add names to a nameset or to remove
names from a renderer or PHIGS workstations resource's built-in
name set.  Initially, the name set in a PHIGS workstation resource is
empty.  The initial contents of the name set in a renderer
is obtained when
the renderer's pipeline state is initialized from a pipeline context.
The contents of the name set resource specified by the pipeline context
is copied to the renderer's built-in name set.
Filtering operations
such as highlighting and invisibility tests affect all output
primitives, and use the contents of this name set in order to
make the filtering decisions.
.LP
For compatibility with PHIGS, PEX provides a generalized structure element
(GSE) output command.  The purpose of the GSE is to allow
implementation-dependent pipeline
attributess to be added cleanly to PHIGS.
However, due to the fact that the format of the data within a GSE
is implementation-dependent and may contain floating point and direct
color values, it is not anticipated that GSE's will provide
a useful extension mechanism in a heterogeneous network environment.
.AC "Miscellaneous Output Commands" 3
.LP
PEX contains a few additional not-easily-classifiable output commands,
some of which have no effect
on the displayed image when
they are processed by a renderer.
Miscellaneous output requests include execute structure,
label, and application data requests.
.LP
When a \fIlabel\fP output command is passed to a structure, a structure element
containing the label will be created.
Labels can be used during element pointer-positioning operations to
simplify structure editing.
The \fIapplication data\fP request allows the client to store arbitrary
user-defined data in a structure.  When an \fIapplication data\fP request
is passed to a structure, a structure element will be created
containing the specified user data.
Label requests and application data requests are effectively no-ops when
executed by a renderer.
.LP
When executed, the \fIexecute structure\fP output command causes the
current pipeline state to be saved, and the structure elements in the
named structure to be executed.  Following this, the previous pipeline
state will be restored and normal processing of output commands will continue.
.bp
.AC "Lookup Tables" 2
.LP
The PEX \fIlookup table\fP resource provides the capability of matching
index values to data records of various types.  This capability is
used to support the PHIGS functionality present in bundles and various
tables.  Lookup tables provide a level of indirection for various
output primitive attributes and for pipeline attributes that cannot
be stored in structures because they cannot be modified during structure
traversal (e.g., views, depth-cueing information).
.LP
A lookup table consists of a set of \fIlookup table entries\fP, each of
which is a data structure of
the format defined for that type of table.  Each table entry
may contain different data values.  Table
entries do not have to be numbered sequentially starting from zero; rather,
tables may be sparse.  It is possible to have a lookup table for which
only the first, third, sixteenth, and 192nd entries are defined.
.LP
The \fBPEXGetTableInfo\fP request can be used to obtain the following
information for each type of table:
.ID
\(bu   definable entries
\(bu   number of predefined entries
\(bu   predefined minimum
\(bu   predefined maximum
.DE
The \fIdefinable entries\fP attribute indicates the maximum number of
entries that can be maintained in the specified type of table.
The \fInumber of predefined entries\fP attribute contains the number of
entries that are filled in by the server automatically when a table
of this type is created.  \fIPredefined minimum\fP is the index number
for the predefined entry with the lowest index number, and \fIpredefined
maximum\fP is the index number for the predefined entry with the highest
index number.  All indices between the max and min are also guaranteed
to refer to predefined entries.  Predefined entries may be deleted or
overwritten by clients.  It is possible to obtain the predefined entries
in a table without even creating a table of that type using the
\fBPEXGetPredefinedEntries\fP request.  The indices of all of the
\fIdefined entries\fP (whether predefined or defined by a client)
can be queried using the
\fBPEXGetDefinedIndices\fP request.
.LP
An index value for each type of table has been designated as the index
value for the \fIdefault entry\fP for tables of that type.  If a reference
to an undefined entry occurs in that type of table, the default table
entry will be used instead.  If the default entry is also undefined,
the default values for a table of that type will be used.
.LP
The lookup table types that have been defined for PEX include:
.ID
\(bu   line bundle
\(bu   marker bundle
\(bu   text bundle
\(bu   interior bundle
\(bu   edge bundle
\(bu   pattern table
\(bu   color table
\(bu   text font table
\(bu   view table
\(bu   light table
\(bu   depth-cue table
\(bu   RGB approximation table
\(bu   intensity approximation table
.DE
The \fIline bundle\fP table type is used to maintain attributes for
drawing line primitives.
Output primitive attributes for line primitives
are obtained from this type of table if the line ASF values in the pipeline
state are set to \fIBundled\fP.
The \fImarker bundle\fP table type is used to maintain attributes for
drawing marker primitives.  
Output primitive attributes for marker primitives
are obtained from this type of table if the marker ASF values in the pipeline
state are set to \fIBundled\fP.
The \fItext bundle\fP table type is used to maintain attributes for
drawing text and annotation text primitives.  
Output primitive attributes for text and annotation text primitives
are obtained from this type of table if the text ASF values in the pipeline
state are set to \fIBundled\fP.
The \fIinterior bundle\fP table type is used to maintain attributes for
drawing surface primitives.  
Output primitive attributes for surface primitives
are obtained from this type of table if the surface ASF values in the pipeline
state are set to \fIBundled\fP.
The \fIedge bundle\fP table type is used to maintain attributes for
drawing edges of surface primitives.  
Output primitive attributes for edges of surface primitives
are obtained from this type of table if the surface edge
ASF values in the pipeline
state are set to \fIBundled\fP.
.LP
The \fIpattern table\fP table type is used to store patterns for use
when the surface interior style is set to \fIPattern\fP.
The \fIcolor table\fP type contains a set of direct color values
and is used to dereference indexed color values.
The \fItext font table\fP type maintains a list of X or PEX font id's that
may be used when drawing text primitives.
The \fIview table\fP type contains entries that define different views.
The \fIlight table\fP type contains the descriptions of all the light
sources that may affect objects.
The \fIdepth-cue table\fP type has entries that contain the parameters
necessary for performing depth-cueing operations.
The \fIRGB approximation table\fP type has entries that contain the parameters
necessary for performing RGB approximation of pipeline color values.
The \fIintensity approximation table\fP type
has entries that contain the parameters
necessary for performing intensity approximation of pipeline color values.
.bp
.AC "Pipeline Contexts" 2
.LP
The PEX \fIpipeline context\fP resource provides a convenient package for
all of the attributes that are considered to be part of a renderer's pipeline
state.  A pipeline context contains an \fIinstance\fP of
the attributes that define a rendering pipeline.  The attributes
that are contained in a pipeline context are copied to a renderer
resource whenever a \fBPEXBeginRendering\fP request is issued.
This is currently the only use of the pipeline context resource.
.LP
The pipeline context contains the following attributes.  The definition
and use of each attribute can be found in the section
\fIPEX Output Commands\fP.
.LP
.MC 2.0i
.nf
\(bu   marker type
\(bu   marker scale
\(bu   marker color
\(bu   marker type asf
\(bu   marker scale asf
\(bu   marker color asf
\(bu   marker bundle index

\(bu   text font index
\(bu   text precision
\(bu   character expansion
\(bu   character spacing
\(bu   text color
\(bu   character height
\(bu   character up vector
\(bu   text path
\(bu   text alignment
\(bu   annotation text height
\(bu   annotation text up vector
\(bu   annotation text path
\(bu   annotation text alignment
\(bu   annotation text style
\(bu   text font index asf
\(bu   text precision asf
\(bu   character expansion asf
\(bu   character spacing asf
\(bu   text color asf
\(bu   text bundle index

\(bu   line type
\(bu   line width
\(bu   line color
\(bu   curve approximation
\(bu   polyline interpolation
\(bu   line type asf
\(bu   line width asf
\(bu   line color asf
\(bu   curve approximation asf
\(bu   polyline interpolation asf
\(bu   line bundle index

\(bu   interior style
\(bu   interior style index
\(bu   surface color
\(bu   reflection attributes
\(bu   reflection model
\(bu   surface interpolation
\(bu   back face interior style
\(bu   back face interior style index
\(bu   back face surface color
\(bu   back face reflection attributes
\(bu   back face reflection model
\(bu   back face surface interp
\(bu   surface approximation
\(bu   trim curve approximation
\(bu   culling mode
\(bu   distinguish
\(bu   normalflip
\(bu   pattern size
\(bu   pattern reference point
\(bu   pattern reference vector1
\(bu   pattern reference vector2
\(bu   interior style asf
\(bu   interior style index asf
\(bu   surface color asf
\(bu   reflection model asf
\(bu   surface interpolation asf
\(bu   reflection attribute asf
\(bu   back face interior style asf
\(bu   back face interior style index asf
\(bu   back face surface color asf
\(bu   back face reflection model asf
\(bu   back face surface interpolation asf
\(bu   back face reflection attributes asf
\(bu   surface approximation asf
\(bu   trim curve approximation asf
\(bu   interior bundle index

\(bu   surface edges
\(bu   surface edge type
\(bu   surface edge width
\(bu   surface edge color
\(bu   surface edges asf
\(bu   surface edge type asf
\(bu   surface edge width asf
\(bu   surface edge color asf
\(bu   edge bundle index

\(bu   local transform
\(bu   global transform
\(bu   model clip
\(bu   model clip volume
\(bu   view index
\(bu   light state
\(bu   depth-cue index
\(bu   pick id
\(bu   HLHSR identifier
\(bu   name set
.fi
.1C
.AC "Renderers" 2
.LP
PEX deals with three-dimensional (geometric) objects and 
color values that are specified by clients.
\fIRendering\fP is the process that converts geometric information into raster
(2D) coordinates, and converts color values into colors or
intensities the device is capable of displaying.
The \fIrendering pipeline\fP is an abstraction that can be used
to describe the order and the semantics
of the functions that occur during the rendering process.
The rendering pipeline is discussed in more detail in the sections
\fIPEX Geometry Transformations\fP, \fIPEX Color Transformations\fP,
and \fIPEX Rendering Pipeline\fP.
After geometric/color data has been transformed into raster data, it can be
written to an X window or pixmap.
.LP
A \fIrenderer\fP is a resource that has been created in PEX to
encapsulate the functionality necessary to perform rendering.
Output commands sent to a renderer are always executed immediately,
not deferred
like output commands that are sent to a structure resource.  In this way,
renderers support the concept of \fIimmediate mode\fP graphics.
Each renderer resource has the following modifiable attributes:
.ID
\(bu   pipeline context
\(bu   current path\(dg
\(bu   renderer state\(dd
\(bu   marker bundle
\(bu   text bundle
\(bu   line bundle
\(bu   interior bundle
\(bu   edge bundle
\(bu   view table
\(bu   color table
\(bu   depth-cue table
\(bu   light table
\(bu   color approximation table
\(bu   pattern table
\(bu   text font table
\(bu   highlight inclusion
\(bu   highlight exclusion
\(bu   invisibility inclusion
\(bu   invisibility exclusion
\(bu   HLHSR mode
\(bu   NPC subvolume
\(bu   viewport
.DE
.FS
\(dg The \fIcurrent path\fP attribute is modifiable only through the
\fBPEXBeginStructure\fP and \fBPEXEndStructure\fP requests.
.FE
.FS
\(dd The \fIrenderer state\fP attribute is modifiable only through the
\fBPEXBeginRendering\fP and \fBPEXEndRendering\fP requests.
.FE
In addition, each renderer has the following attributes that are not
directly modifiable and cannot be queried:
.ID
\(bu   target drawable
\(bu   pipeline state
\(bu   structure traverser
\(bu   HLHSR buffer
.DE
.LP
Each renderer has a block of state information (the \fIpipeline state\fP)
that defines the
behavior of the rendering pipeline.
A renderer's pipeline state contains all
of the attributes that are stored in a PEX pipeline context,
which includes all of the attribute values that affect output primitives
and the attributes that define the behavior of the
pipeline itself (pipeline attributes).
Whenever a \fBPEXBeginRendering\fP request is issued, the attributes
in the pipeline context specified by the renderer's \fIpipeline context\fP
attribute are copied into the renderer's pipeline state block.  If the
renderer's \fIpipeline context\fP attribute does not contain the resource id of
a valid pipeline context resource, the default values for pipeline context
attributes are copied instead.  The pipeline context's \fIname set\fP attribute
is a special case.  Instead of copying the resource id of the name set
(which is stored in the pipeline context's \fIname set\fP attribute),
the \fIcontents\fP of the specified
name set are copied to the renderer to form the renderer's
initial name set contents.
.LP
Various tables are associated with a renderer.  Output primitive
attributes whose ASF's are set to \fIBundled\fP and various pipeline
attributes (e.g., viewing, depth-cueing information) will be obtained from the
lookup tables that are bound to a renderer.  Output primitive attributes
whose ASF's are set to \fIBundled\fP will obtain their values from the
\fImarker bundle, text bundle, line bundle,  interior bundle,\fP and
\fIedge bundle\fP tables.  Clipping information and viewing and projection
matrices are obtained from the \fIview\fP table.  Indexed colors that
are passed to the renderer are dereferenced using the \fIcolor\fP table.
Depth-cueing information is maintained in the \fIdepth-cue\fP table, and
the definitions for all of the light sources available to the renderer
are stored in the \fIlight\fP table.  The rules and hints for converting
rendering pipeline colors into colors or intensities that the device
is capable of displaying are contained in the \fIcolor approximation\fP table.
Surfaces whose interior style is set to \fIPattern\fP may reference the
table of patterns stored in \fIpattern table\fP.  A list of X/PEX font
resource id's is maintained in the \fItext font\fP table and is accessed
whenever text is drawn.
.LP
Associated with each renderer are \fIhighlighting\fP and \fIinvisibility\fP
filters.
\fIHighlighting\fP is the process of drawing attention to an output
primitive by altering its visual attributes in an implementation-dependent
way.
Primitives that are to be treated as \fIinvisible\fP
are not rendered.
Each filter consists of two name sets: an \fIinclusion set\fP
and an \fIexclusion set\fP.  Another name set is maintained as part
of the renderer's pipeline state.  There are output commands that can
add and remove \fInames\fP from the pipeline's current name set.
In order for a primitive to be highlighted,
the pipeline's name set must contain a name that is also in the
\fIhighlight inclusion\fP name set, and must not contain any names that
are also in the \fIhighlight exclusion\fP name set.
Similarly, in order for a primitive to be treated as invisible,
the pipeline's name set must contain a \fIname\fP that is also in the
\fIinvisibility inclusion\fP name set, and must not contain any names that
are also in the \fIinvisibility exclusion\fP name set.
In other words, to meet the filtering criteria for either highlighting
or invisibility, the intersection of the name
set with inclusion set must not be empty, and
the intersection of the name set with the exclusion set must be the empty set. 
.LP
The \fIrenderer state\fP attribute is used to identify whether
a renderer is currently \fIRendering\fP or \fIIdle\fP.  A
\fBPEXBeginRendering\fP request puts a renderer in the \fIRendering\fP
state.  No output primitives will be processed by a renderer unless it
is \fIRendering\fP.  Output primitives that are sent to a renderer
that is \fIIdle\fP will be ignored.
Hidden surface algorithms require that some form of sorting be performed
on output primitives so that they may be rendered with
hidden surfaces removed.
Each renderer maintains an \fIHLHSR mode\fP that
indicates the hidden line/hidden surface algorithm that is to be used and
an \fIHLHSR buffer\fP that is used in the hidden line/hidden surface
computations.
The HLHSR buffer is initialized when a \fBPEXBeginRendering\fP request is
issued, based on the current value of the HLHSR mode.
Since this is
the only time the HLHSR mode is accessed, subsequent changes to the
HLHSR mode have no effect on the current rendering (i.e., the HLHSR
mode is bound at the time of a \fBPEXBeginRendering\fP).
If the resources necessary to perform the rendering are not available,
an \fIAlloc\fP error may be generated and the renderer will remain \fIIdle\fP.
When a \fBPEXEndRendering\fP request is
made, any output primitives that have not yet been processed are
rendered using the chosen HLHSR algorithm.
.LP
For example, Z-buffers are initialized to negative infinity
when a
\fBPEXBeginRendering\fP request occurs.
(The representation of values in the Z-buffer is implementation-dependent.
No mechanism has been provided in PEX to allow clients to explicitly set or
query Z-buffer values.)
The values in the Z-buffer become undefined when a \fBPEXEndRendering\fP
request is processed.\(dg
PEX requires each renderer to resolve visibility of primitives
independently from other renderers, implying that each render maintains
its own HLHSR buffer.
Therefore, in the case of Z-buffering, Z-buffers are conceptually
associated with renderers, and thus even renderers
that share the same target drawable should have independent Z-buffers.
If it is not possible to support this concept, an
implementation should return an \fIAlloc\fP
error if an attempt is made to begin rendering with a renderer whose
target drawable is already being used by another renderer.\(dd
.FS
\(dg It may be useful to retain these Z-buffer values for use in subsequent
rendering operations.
This functionality has been deferred pending resolution of issues surrounding
double- and multi-buffering proposals.
.FE
.FS
\(dd An implication of this is that concurrency involving multiple renderers
working on a single drawable/Z-buffer pair is not supported in PEX.
However, many opportunities for concurrency still exist within a single
renderer that is doing rendering.
.FE
.KS
.PS

ar_h = 0.3
margin = 0.2

bx_w = 0.8
bx_h = 0.6

di_h = 0.9
di_c = (di_h - bx_h)/2

define diamond X
	box invis wid $1 ht $1 $2
	{
		line from last box.n \
			then to last box.w \
			then to last box.s \
			then to last box.e \
			then to last box.n
	}
X

down


	OC:	box invis "\f6PEXRenderOutputCommands\fP" "request"
		arrow down ar_h + margin
	CS:	diamond( di_h, "call" "structure?" )
		arrow down .5
	RDR:	box ht 1.2i "rendering" "pipeline"
		arrow down ar_h + margin
	DRAW:	box invis "raster" "data"

		" yes" at CS.e + (0,0.1) ljust
		" no" at CS.s + (0,-0.1) ljust

	move to OC.n + (1.5,0)

	TC:	box invis "\f6PEXRenderNetwork\fP" "request"

	ST:	box wid bx_w ht bx_h "structure" "traverser" \
		with .c at (TC.c.x,CS.c.y)

		arrow from TC.s to ST.n
		arrow from CS.e to ST.w

	SS:	box wid bx_w ht bx_h "structures" dashed \
		with .w at ST.e + (0.5,0)

	line -> from ST.e + (0,.1) to SS.w + (0,.1) dashed
	line <- from ST.e - (0,.1) to SS.w - (0,.1) dashed

	line from ST.s down 0.1
	spline down (ST.s.y - RDR.n.y)/2 - 0.1 \
		then left (ST.s.x - RDR.n.x - 0.1) \
		then to RDR.n + (0.1,0.1)
	arrow down 0.1


	box dotted \
		wid (SS.e.x + margin) - (RDR.w.x - margin) \
		ht  (CS.n.y + margin) - (RDR.s.y - margin) \
		with .sw at RDR.sw - (margin,margin)
.PE
.sp 0.25i
.ce
\f6Figure 2.3: A PEX Renderer\fP
.KE
.LP
A renderer is responsible for accepting output commands and
processing them.  A renderer can obtain output commands either
directly from a client application or from a structure.
A \fIstructure traverser\fP is a key component of a renderer.
When a renderer receives a \fBPEXRenderNetwork\fP request,
the specified structure network will be traversed
and the output commands it contains will be processed.
Semantically, the \fIPEXRenderNetwork\fP
requests is the equivalent of an "execute structure" output command
sandwiched between a \fBPEXBeginRendering\fP and a \fBPEXEndRendering\fP
request.
Each time an "execute structure" command is encountered (including
the "execute structure" that causes the traversal to begin),
the pipeline state is saved.  When the called structure has been executed,
this saved pipeline state is restored.  (Note that only the pipeline
state values are saved and restored.  The renderer attributes in the
list above are \fInot\fP saved and restored during a traversal, since
they cannot be modified from within a structure.)
For more on structure traversal, see the \fIPEX Structure Traversal\fP section.
.LP
A renderer's \fIcurrent path\fP attribute provides a bit more help to clients
that decide to do structure traversals on the client side.  The PEX
requests \fBPEXBeginStructure\fP and \fBPEXEndStructure\fP perform
actions identical to those performed when an "execute structure" output
command is executed, namely saving and restoring the pipeline state.
In addition, the current path maintains an element reference path.
Each time a \fBPEXBeginStructure\fP request is executed, an element
reference is added to the renderer's current path.  The element reference
consists of the structure name specified in the request, and an element
offset of zero.  Each subsequent output command issued via a
\fBPEXRenderOutputCommands\fP request will cause the element offset
of the last element reference in the current path to be incremented
by one.  Each time a \fBPEXEndStructure\fP request is executed, the
last element reference (id/offset pair) is removed from the current path.
The current path attribute can be queried by the client at any time to
determine the current position in the client-side structure hierarchy.
.LP
A renderer also contains attributes that define the last two geometry
transformation stages of the rendering pipeline: the NPC subvolume clipping
and the viewport mapping.  The \fINPC subvolume\fP attribute specifies
a cube in the normalized projection coordinate system against which all
primitives are clipped (this clipping cannot be disabled.)  The viewport
mapping contains the rules for mapping the geometry from normalized
projection coordinates into device coordinates.  A special flag has
been added to the viewport specification to allow a shorthand way
to indicate that the viewport should be as large as possible, given
the current size of the drawable.
.bp
.AC "Structures" 2
.LP
A \fIstructure\fP is a receptacle for storing output commands for
later execution.
Output commands that are stored in a structure are referred to
as \fIstructure elements\fP.
Structures can be shared by clients like any other X resource.
Structures can reference other structures.
Such a hierarchy of structures is called a \fIstructure network\fP.
The first structure in a structure network is called the \fIroot structure\fP.
(In fact, since every structure is the root of a subtree,
every structure can be considered to be a root structure.)
.LP
In Figure 2.4, each box \fI(S1,...S7)\fP represents
a structure resource.  Structure S1 and structure S4 represent
the root structures for two structure networks.
.KS
.PS
down
SP:
box dashed ht 2.35 wid 3.5
S1:
box "S1" ht .35i wid .35i with .nw at SP.nw.x+.7i,SP.nw.y-.1i
S2:
box "S2" ht .35i wid .35i with .nw at SP.nw.x+.35i,SP.ne.y-1.0i
S3:
box "S3" ht .35i wid .35i with .sw at SP.nw.x+1.05i,SP.ne.y-1.0i
S4:
box "S4" ht .35i wid .35i with .nw at SP.nw.x+2.5i,SP.nw.y-.1i
S5:
box "S5" ht .35i wid .35i with .nw at SP.nw.x+2.15i,SP.n.y-1.0i
S6:
box "S6" ht .35i wid .35i with .nw at SP.nw.x+2.85i,SP.n.y-1.0i
S7:
box "S7" ht .35i wid .35i with .nw at SP.nw.x+2.5i,SP.n.y-1.9i
A1:
arrow from S1.s to S2.n
arrow from S1.s to S3.n
A2:
arrow from S4.s to S5.n
arrow from S4.s to S6.n
arrow from S5.s to S7.n
arrow from S6.s to S7.n
.PE
.sp 0.25i
.ce
\f6Figure 2.4:  Structure Networks\fP
.KE
.LP
The two modifiable attributes in a structure resource are:
.ID
\(bu  element pointer
\(bu  editing mode
.DE
In addition, the following attributes of a structure can be queried:
.ID
\(bu  number of structure elements
\(bu  length (in units of four bytes)
\(bu  number of times referenced by other structures
.DE
.LP
A structure resource can be created by
issuing a \fBPEXCreateStructure\fP command.  It can later be deleted
with either a \fBPEXFreeStructure\fP command or a \fBPEXDeleteStructures\fP
command.  The only difference is that \fBPEXDeleteStructures\fP takes
a list of structure resources to be deleted, and has the optional
side effect of deleting all other structure elements that reference
any of the deleted structures.
.LP
The position of an element within a structure is its \fIoffset\fP
from the start of the structure.
The first element of a structure has an offset of one.
The last element of a structure has an offset of \fIn\fP, where \fIn\fP
is the total number of elements within the structure.
For example, if there are five structure elements in a structure,
the offsets for those structure elements will be one through
five.  Figure 2.5 contains a structure with five structure
elements
and an element pointer that is set to four. 
Conceptually, a null element exists
\fIbefore\fP the first element and has an offset of zero.
.KS
.PS

down
w = 2.0

SP:	box ht 2.5 wid w

S1:	box dashed "Type: polyline" "Data: num_pts, pts, ..." \
		ht .5i wid w with .nw at SP.nw.x,SP.nw.y

S2:	box dashed "Type: set line color" "Data: color_type, color_value" \
		ht .5i wid w with .nw at SP.nw.x,SP.n.y-.5i

S3:	box dashed "Type: text" "Data: pt, string, ..." \
		ht .5i wid w with .nw at SP.nw.x,SP.nw.y-1.0i

S4:	box dashed "Type: set line type" "Data: line_type" \
		ht .5i wid w with .nw at SP.nw.x,SP.nw.y-1.5i

S5:	box dashed "Type: polyline" "Data: num_pts, pts, ..." \
		ht .5i wid w with .nw at SP.nw.x,SP.nw.y-2.0i

O0:	box invis "offset = 0" \
		with .w at S1.e + (.5,.5)

	arrow from O0.w to S1.e + (0,.5)

O1:	box invis "offset = 1" \
		with .w at S1.e + (.5,0)

	arrow from O1.w to S1.e

O2:	box invis "offset = 2" \
		with .w at S2.e + (.5,0)

	arrow from O2.w to S2.e

O3:	box invis "offset = 3" \
		with .w at S3.e + (.5,0)

	arrow from O3.w to S3.e

O4:	box invis "offset = 4" \
		with .w at S4.e + (.5,0)

	arrow from O4.w to S4.e

O5:	box invis "offset = 5" \
		with .w at S5.e + (.5,0)

	arrow from O5.w to S5.e

OS:	box invis "start of structure" \
		wid 1.15 with .e at S1.nw + (-.5,0)

	arrow from OS.e to S1.nw

OE:	box invis "element ptr = 4" \
		wid 1.15 with .e at S4.w + (-.5,0)

	arrow from OE.e to S4.w

.PE
.sp 0.25i
.ce
\f6Figure 2.5:  Simple Structure with Structure Elements\fP
.KE
.LP
The \fIelement pointer\fP contains an offset into the structure.  This
pointer value is used in many of the requests to fetch, store, or edit
structure elements.
PEX contains a number of requests to modify the position of the element
pointer.  These requests typically require a {\fIwhence, offset\fP}
pair that is similar to that used in the UNIX \fIlseek(2)\fP file
pointer manipulation routine.
The element pointer can be positioned relative to
the beginning of the structure, relative to the current element pointer
position, or relative to the end of the structure.
The specified \fIoffset\fP is always
added to the position indicated by \fIwhence\fP to obtain the absolute offset
to be used.
.LP
A \fIlabel\fP is a special structure element that supports more convenient
editing of a structure.
Since labels are stored as structure elements, they will move with
the other structure elements as elements are inserted or deleted
before them in the structure.
The element pointer can also be moved
to a particular \fIlabel\fP or to the next occurrence of a particular
structure element within the
structure.  When a structure is first created, the element pointer
will have a value of zero.
.LP
An output command can be inserted into the structure
by setting the editing mode of the structure to \fIStructureInsert\fP
(the default),
setting the element pointer to the desired element offset, and
issuing a \fBPEXStoreElements\fP command to the structure.
In insert mode, each output command in the \fBPEXStoreElements\fP request
will cause a structure element to be created and
inserted \fIafter\fP the element specified by the element pointer.  After
each insertion,
the element pointer is updated to point to the newly-inserted element.
In a very common case,
the element pointer points to the last element in a structure and
structure elements are to be added to the end of the strcture.
Each output command sent to this structure
will cause a structure element to be added after the last element, and
the element pointer will then be updated
to point to the newly-inserted structure element.
Therefore, multiple output commands that are sent to a structure when
the editing mode is \fIStructureInsert\fP will result in
an ordered list of structure elements being added to the structure.
.LP
Existing structure elements may be overwritten by setting the
editing mode of a structure to \fIStructureReplace\fP.
In replace mode, existing elements are deleted, and the new elements
specified in a \fBPEXStoreElements\fP request are
created in their place.
The number of elements deleted is equal to \fIn\fP, where \fIn\fP is
the number of elements passed in the \fBPEXStoreElements\fP
request (unless there are fewer then \fIn\fP elements between the
element pointer position and the end of the structure, in which case
only those between the element pointer position and the end of the
structure will be deleted.)
The elements passed in the \fBPEXStoreElements\fP request are then
inserted in place of the deleted elements.
Since it is not possible to set the element pointer to an offset greater
than the last element in the structure, at least one element will always
be overwritten during a replace operation (unless the structure contained
no elements to begin with).
The element pointer will be left
pointing to the element preceding the deleted/inserted range.
.LP
Structure editing commands that copy and delete
a range of structure elements are also supported.
When elements are deleted, the element pointer will be set to the position
immediately \fIpreceding\fP the first element deleted.  The offset of
each element after
those deleted will decrease by one for each element that is deleted.
Deleting element zero is effectively a no-op.  A deletion operation includes
the elements at the boundaries of the deletion range.  After the deletion
occurs, the element pointer is left pointing at the element preceding
the deleted range.  Copy operations ignore the editing mode of the
destination structure - elements are always inserted during copy operations.
After a copy operation, the destination structure's element pointer is left
pointing at the last element that was copied.
Copying elements when the source and destination structure are the same
is also permitted.  The semantics specify that the copy occurs as though
the elements in the copy range are first copied to a temporary area, and
then inserted at the designated spot in the destination structure.
.LP
Various other commands exist for dealing with structures.
These include commands to
delete all references to a structure,
obtain the size of a structure,
and obtain information about the hierarchy of a structure network.
Some of these requests utilize the concept of an element reference path.
A structure element can be referenced with a structure id/element offset pair.
Because structures can call other structures, it is sometimes helpful to know
the organization of the structure network.  In order to uniquely identify
a path through a structure network, an arbitrary length list of
structure id/element offset pairs (an \fIelement reference path\fP) is used.
If structure "A" is the root of a structure network,
an element that lived at element offset 14 in structure "A" would have
an element reference path equal to {"A":14}.  If element 14 in structure
"A" was an execute structure command
that referenced structure "B", element zero
in "B" would have an element reference path of {"A":14, "B":0}.  Since it
is possible that "B" is also referenced by other structures, there may be
alternative paths to the element "B":0.  The \fIdepth\fP of an element
reference path is the number of element references in the path.  A path depth
of zero is used as a shorthand to indicate all unique paths leading to
a particular element.
.bp
.AC "Name Sets" 2
.LP
A \fIname set\fP is a resource that maintains a set of identifiers
(\fInames\fP) in order to allow conditional treatment of output primitives
during rendering, picking, and structure searching operations.
Search contexts and pick measures also have attributes that are name set
resource ids.
Renderer resources and PHIGS workstation resources each have a name set
attribute that is considered to be the \fIcurrent name set\fP.
The contents of the current name set
cannot be directly modified or queried in the PHIGS
workstation resource, however the name set to be used in
a renderer resource can be set via the pipeline context
resource that provides the initial state for the renderer.
Output commands have been defined that cause identifiers to be added
to or removed from this name set.  Thus, as a structure traversal
or series of output commands is executed, the contents of the current name set
may fluctuate.
.LP
Special \fIfilters\fP are built into renderer, search context,
pick measure, and PHIGS workstation
resources.  These filters consist of a pair of name set resource id's,
one of which is considered to be the \fIinclusion set\fP and the
other which is considered to be the \fIexclusion set\fP.  The reason
that both are provided is that sometimes it is easier to specify what
\fIdoes\fP pass the filter test, and other times it is easier to specify
what \fIdoes not\fP pass the filter test.
The filtering criteria is successfully met if, at the time an output
primitive command is executed, the inclusion name set contains at least
one name that is also in the current name set, and the current name set
does not contain any names that are in the exclusion set.  (If a name
is contained in both the inclusion set and the exclusion set, it is
considered to be only in the exclusion set.)
.LP
Conditional operations that are supported with name sets include
highlighting, treating primitives as though they were invisible, and ignoring
certain primitives when searching through a structure network or performing
a picking operation.
As an example of how this might be useful, consider a user that is carefully
crafting a structure network that contains the description of all of the
furniture in a living room.  For whatever reason, the user knows that he
will be interested in highlighting objects that are "wooden" and objects
that are "antique" later on.  Therefore, he selects an identifier (name)
to represent "wooden" and another to represent "antique".  As he constructs
his structure network, he includes a command to add "antique" to the current
name set before each object that is considered "antique", and a command to
remove "antique" from the current name set after each object that is "antique".
Similarly, "wooden" is added to the current name set before each object
that is "wooden" and removed afterwards.  Some objects may be considered
both "antique" and "wooden".
.LP
Now this user can render his scene in different ways by modifying the
highlighting inclusion and exclusion sets and retraversing the structure
network.  If the highlighting inclusion and exclusion sets are empty when
the structure network is traversed, the scene will be drawn as normal.
If the user issues a \fBPEXChangeNameSet\fP request to add "antique" to
the highlighting inclusion set, all of the "antique" objects will be
highlighted the next time the structure network is traversed.  If he
adds "wooden" to the highlighting inclusion set, all "wooden" objects
and all "antique" objects will be highlighted.  If "wooden" is added
to the highlighting exclusion set instead, all objects that are "antique"
but not "wooden" will be highlighted.
.LP
The only attribute in a name set is a list of names.  The set of available
names may be limited by an implementation for performance reasons.  PEX
includes requests to create, copy, free, query, and modify name sets.
Since name sets can be referenced by other PEX resources, they do not
go away when freed until all references to them have been eliminated.
.bp
.AC "Search Contexts" 2
.LP
A \fIsearch context\fP is a resource that supports the process
of spatially searching structure networks.  Search contexts allow
a client to specify a set of search criteria and search through
a structure network for the first output primitive that satisfies
the search criteria.  A search context resource contains the
following attributes:
.ID
\(bu   search position
\(bu   search distance
\(bu   search ceiling
\(bu   start path
\(bu   search inclusion list
\(bu   search exclusion list
.DE
The \fIsearch position\fP
attribute specifies the search reference position in world coordinates.
The \fIsearch distance\fP
attribute specifies a distance from the search reference position.
The distance specifies a search aperture, but the aperture is not
mandated to be either a sphere or a cube.
The \fIstart path\fP attribute
defines the structure network path that is to be used
as the starting point for the search.
The \fIsearch ceiling\fP
attribute defines the ceiling of the search operation.
The search ceiling is an index into the list of element references
contained in the search start path.
Searching stops when the end of the structure specified by
the search ceiling is reached.
If the ceiling is zero, the search operates without
a ceiling.
The \fIsearch inclusion list\fP is a
list of name set resource id's that are to
be used as inclusion sets in the search operation.
The \fIsearch exclusion list\fP is a
list of name set resource id's that are to
be used as exclusion sets in the search operation.
.LP
The \fBPEXSearchNetwork\fP request can be used to perform a search operation
once all of the search criteria attributes have been established.
The search is essentially a traversal of a structure network with comparison
of output primitives
against the search aperture occurring in world coordinates.
For text primitives, the comparison operation is based on the assumption
that the text font index is equal to one.  For annotation text,
the only proximity relationship used is closeness to the annotation
reference point.
To be accepted by the searching filter, the current name set must
have at least one name in common with each of the name sets in the
search inclusion list and no names in common with any of the name
sets in the search exclusion list.
.LP
A successful search occurs only when an output primitive element is found
that satisfies the search filter criteria \fIand\fP is within the
specified distance from the search reference point.
The search begins at the element \fIfollowing\fP
the one indicated by the \fIstart path\fP.
The search is terminated once the end of the structure indicated
by \fIsearch ceiling\fP has been reached.
The structure network path of the
first element that meets the search criteria is returned, or if no
element is found that satisfies the search criteria, a null path will be
returned.
After the search has completed, the \fIstart path\fP attribute
will be set to the path of the element that was found, so that the
next search operation may begin from that point.
.bp
.AC "PHIGS Workstations" 2
.LP
The PHIGS workstation resource encapsulates the PEX functionality
to support the PHIGS concept of a "workstation".
The PHIGS definition
of a "workstation" states that a workstation consists of zero or one
display space.  In X, many display spaces (windows) are possible in
a single workstation display.  The PHIGS workststation resource ties
together a single X drawable, a set of lookup tables, highlighting
and invisibility name sets, and all of the state needed to make
it perform as a PHIGS client thinks a workstation should perform.
.LP
Each PHIGS workstation resource contains the following attributes that are
defined at the time it is created and may be queried,
but may not be modified during the lifetime of the PHIGS workstation:
.ID
\(bu   target drawable
\(bu   line bundle
\(bu   marker bundle
\(bu   text bundle
\(bu   interior bundle
\(bu   edge bundle
\(bu   color table
\(bu   pattern table
\(bu   text font table
\(bu   depth-cue table
\(bu   light table
\(bu   color approximation table
\(bu   highlight inclusion
\(bu   highlight exclusion
\(bu   invisibility inclusion
\(bu   invisibility exclusion
.DE
All of these attributes function in a manner identical to their
counterparts in a renderer resource.
.LP
A PHIGS workstation also contains the following attributes that may be
modified or queried with various PHIGS workstation requests:
.ID
\(bu   posted structure networks
\(bu   requested NPC subvolume
\(bu   current NPC subvolume
\(bu   requested viewport
\(bu   current viewport
\(bu   requested HLHSR mode
\(bu   current HLHSR mode
\(bu   defined views
\(bu   display update
\(bu   pick device descriptors
.DE
The \fIposted structure networks\fP attribute maintains a list of
structure resource id's, together with a priority value for each.
The priority values are relative, and are specified at the time
the structure network is added to the list (\fIposted\fP).
Whenever a structure traversal is requested, each of the structure networks
in this list will be traversed.
If multiple structures are posted that would overlap on the screen, the
structure network with the higher priority is guaranteed to be visible.
If two structures have the same priority and would overlap on the screen,
the structure posted later will have the higher priority.
The request \fBPEXPostStructure\fP can be used to post structure
networks to a PHIGS workstation resource, and the \fBPEXUnpostStructure\fP
request can be used to unpost a structure network.  The
\fBPEXUnpostAllStructures\fP request can be used to reset the posted
structure network list to the null list.  Finally, given a structure
resource id, the
\fBPEXGetWKSPostings\fP request will return a list of all PHIGS workstations
to which the structure has been posted.
.LP
The PHIGS workstation resource has been designed to support the concepts of
\fIrequested\fP changes and \fIcurrent values\fP.  The idea is that
the client can change the viewport, NPC subvolume, and HLHSR mode attributes,
but the changes are not actually applied until the
next structure traversal occurs, or the client issues a
\fBPEXExecuteDeferredActions\fP request.
Thus, there are pairs of attributes
for the NPC subvolume, viewport, and HLHSR mode: the value that has
been requested and the value that is current.
.LP
The list of \fIdefined views\fP is similar to a view table.  However,
a PHIGS workstation must also be able to separate requested
views from current views.  Views also have a priority value associated
with them in PHIGS workstation resources.  Because of these differences
in semantics, the PHIGS workstation resource was designed to have a
built-in view table rather than an associated view lookup table.
When doing a picking
operation, it is necessary to map device coordinates through the inverse
of the viewing/projection transform in order to determine the primitive
that was selected.
The reason for the view priority is that several views can produce a
projection onto the same area of the screen.  Thus, specifying a device
coordinate point for the picking operation can be ambiguous.  If there
is more than one view that projects onto the specified device coordinate
point, it is not clear which set of viewing parameters to use to compute
the inverse transform.  Adding a priority to each of the views removes
the ambiguity.
View priorities can be adjusted with the \fBPEXSetViewPriority\fP request.
\fBPEXMapDCtoWC\fP is provided as an aid for mapping
device coordinate points back into world coordinates.
\fBPEXMapWCtoDC\fP can be used to map
world coordinate points into device coordinates.
Views can be modified with the \fBPEXSetViewRep\fP request, but
are not actually applied until the
next structure traversal occurs, or the client issues a
\fBPEXExecuteDeferredActions\fP request.
.LP
The \fIdisplay update\fP mode is used to indicate how the PHIGS
workstation should attempt to visualize changes that occur.  The defined
types include \fIVisualizeEach\fP (visualize each change as it occurs,
even doing a retraversal of the posted structure network list if necessary),
\fIVisualizeEasy\fP (visualize those changes that can be represented
without doing a retraversal, or even those that require
a retraversal if traversal is very fast), \fIVisualizeNone\fP (apply the
changes but do not do a traversal until it is explicitly requested),
\fIVisualizeWhenever\fP (do a traversal when it is convenient to do so),
or \fISimulateSome\fP (visualize the easy/fast changes and simulate
others, such as drawing a deleted primitive in the background color).
\fIPick device descriptors\fP are defined in the next section.
.LP
A PHIGS workstation contains the following attributes to maintain
the state of the PHIGS workstation resource.
They may be queried, but are not
modifiable by clients.  They are modified as side effects of various requests:
.ID
\(bu   visual state
\(bu   display surface
\(bu   view update
\(bu   wks update
\(bu   HLHSR update
.DE
The \fIvisual state\fP attribute maintains a value that indicates
whether the image is "correct", whether some parts of the image have
been changed but simulated on the display, or whether there are deferred
actions that have yet to be visualized.  The \fIdisplay surface\fP
indicates whether or not the PHIGS workstation has rendered anything
on the associated drawable.  The \fIview update\fP, \fIwks update\fP,
and \fIHLHSR update\fP attributes specify whether or not there are
any requested changes to the list of views, the NPC subvolume or viewport,
or the HLHSR mode respectively.
.LP
Finally, a PHIGS workstation contains the
following attributes that are not
directly modifiable and cannot be queried:
.ID
\(bu   pipeline state
\(bu   structure traverser
\(bu   HLHSR buffer
.DE
.LP
A PHIGS workstation's list of posted structure networks may be retraversed
explicitly with the \fBPEXRedrawAllStructures\fP request.  The
\fBPEXUpdateWorkstation\fP request can be used to perform a traversal if
the visual state is not "correct".  The \fBPEXGetDynamics\fP request
can be used to obtain information about how changes to various
attributes can be visualized.
.LP
PHIGS workstation resources contain a superset
of the capabilities found in renderers.
The fundamental difference between renderers and PHIGS workstation
resources is that renderers attempt to follow the X philosophy of
providing only mechanism and deferring policy decisions to clients,
and, for performance reasons, PHIGS workstation resources have additional
"PHIGS policy" built-in.  The major differences are:
.nr LL 6.0i
.IP "     \(bu"
PHIGS workstation resources maintain the state of the drawable, including
whether anything at all has been drawn on it, and whether the displayed
image is correct with respect to all the resources that were used to 
generate it.  Renderers do not know anything about
the state of the associated drawable, so higher-level concepts like
"picture correctness" are deferred to the client.
.IP "     \(bu"
The drawable, lookup tables, and name sets associated with a PHIGS
workstation are fixed at the time the resource is created and cannot
be changed.  Renderers have similar attributes that, except for
the drawable, can be changed at any time.
.IP "     \(bu"
The PHIGS workstation maintains a list of posted structures, with priorities.
Only the structures in this list may be traversed.
A renderer must be told explicitly what structure network to traverse, but
it may traverse any structure at any time.
.IP "     \(bu"
Output commands cannot be sent directly to a PHIGS workstation resource
for immediate execution as they can to a renderer.
.IP "     \(bu"
Structure traversal using a PHIGS workstation resource has side effects
beyond those of a renderer.  When a traversal is performed using a PHIGS
workstation, the drawable may be cleared to the color stored in entry zero
of the associated color lookup table.  Furthermore, all pending changes
to the PHIGS workstation attributes are made current as a traversal begins.
Finally, the displayed image state is set to "correct" at the end of the
traversal.  None of these actions occur when a traversal is performed
with a renderer resource.
.IP "     \(bu"
Changes to a PHIGS workstation's attributes or to its associated state
(e.g., lookup tables, posted structures, etc.) may cause an implicit
traversal of the list of posted structure networks.  Such changes do
not have any side effects with respect to renderers.
.IP "     \(bu"
Pick measure resources and picking work only with PHIGS workstation
resources, not with renderers.\(dg
.FS
\(dg Picking support for renderers has been considered and is currently
on the list of deferred functionality.  A prototype implementation is
probably required before it can be added to standard PEX.
.FE
.IP "     \(bu"
PHIGS workstations can assign priorities to each view in the view
table and can map points from device coordinates back into world coordinates
based on these view priorities.  Similar functionality is not currently
available for renderer resources. \(dd
.FS
\(dd Mapping points from device coordinates to world coordinates
for renderers has been considered and is currently
on the list of deferred functionality.
.FE
.IP "     \(bu"
PHIGS workstations have a built-in view table.  It is not possible
to associate a PEX view table resource with a PHIGS workstation in
the same way as it can be associated with a renderer.  This
supports the PHIGS concepts of current and requested views.
.nr LL 6.5i
.bp
.AC "Pick Measures" 2
.LP
PHIGS contains the concept of a pick device.
A \fIpick device\fP is a pointing device or a mechanism that can be
used to select a displayed output primitive.  Picking is typically
used in applications to select a primitive so that its attributes
can be modified in some way, or so that it may be scrutinized
more carefully in some fashion.
The purpose of a pick device
is to provide the \fIpick path\fP
(a list of {structure id, element offset, pick id} triples)
for the selected primitive.
The definition of a
pick device was purposely left quite vague in PHIGS, since there are
many mechanisms currently in use for selecting primitives.  This
broad definition of a pick device allows the use of such things as a mouse,
a graphics tablet, a trackball, a light pen, a touch-sensitive screen,
or even a voice recognition unit.  Furthermore, PHIGS does not mandate
that any one pick device must be supported in all implementations.
PHIGS implementors are free to support the pick device(s) they think
are the most useful.
.LP
PEX supports this broad definition of picking through the use of
pick device descriptors in the PHIGS workstation resource, and
pick measure resources.  The existing capabilities
for pointing devices in core X
can be used in conjunction with the PEX pick device descriptors and pick
measures to support PHIGS picking.  The PHIGS concept of a \fItrigger\fP
(an indication that, "yes, this really \fIis\fP what I want to select") can
be built on top of the X event mechanism.
.LP
A PEX implementation may choose to support
several different types of pick devices.  A description of the initial
values for each supported pick device is called a \fIpick device descriptor\fP
and is part of the state of each
PHIGS workstation resource.  A PHIGS workstation resource has exactly
one pick device descriptor for each pick device that is supported.
It is possible to find out the number and type of pick devices that the PEX
extension supports by querying the \fIPickDevice\fP enumerated
type with the \fBPEXGetEnumeratedTypeInfo\fP request.  Once the supported
pick device types are known, it is possible to query the
contents of each pick device descriptor in a PHIGS workstation
with the \fBPEXGetPickDevice\fP request.
The contents of a pick device descriptor can be modified
with the \fBPEXChangePickDevice\fP request.
.LP
A pick descriptor record provides the initial values for a pick measure
resource.
Each pick device descriptor has the following fields:
.ID
\(bu  pick status
\(bu  pick path
\(bu  pick path order
\(bu  pick inclusion
\(bu  pick exclusion
\(bu  pick data record
\(bu  prompt/echo type
\(bu  echo volume
\(bu  echo switch
.DE
A \fIpick measure\fP resource "measures" output primitives
to determine which ones satisfy a specific set of selection criteria.
A pick measure has only two attributes that can be queried,
\fIpick status\fP and \fIpick path\fP.
The pick status attribute is set to \fIOk\fP whenever a primitive meets
the selection criteria, and
is essentially a flag telling whether or not
the information in the pick path is valid.  If the pick path is valid,
it contains a list of structure id, element offset, and pick id
triples that define the structure network path of the primitive
that met the selection criteria.
Other attributes that are
obtained initially from the pick device descriptor
are constant for the lifetime of the pick measure and cannot be queried
or modified.
.LP
The \fIpick path\fP attribute indicates the position within the structure
network from which to search for selected primitives.  Because this path
is updated for each successful picking operation, it is possible to pick
primitives incrementally (i.e., "give me the next picked primitive, then
give me the next picked primitive" etc.).  The \fIpick path order\fP
determines the order in which the element references appear in the
pick path.  They can either be in order from the structure network
root down to the picked primitive (\fITopFirst\fP) or from the picked
primitive back up to the structure network root (\fIBottomFirst\fP).
.LP
The \fIpick inclusion\fP and \fIpick exclusion\fP attributes contain
the resource id's of name sets that are to be used to filter the picking
operation.  A current name set is maintained by the PHIGS workstation
during structure traversal.  Structure elements can cause names to be
added to or removed from this set.  Output primitives are considered
"pickable" or "detectable" if at least one name in the current name set
is also in the pick inclusion name set, and  if none of the names in
the current name set are in the pick exclusion name set.
.LP
The \fIpick data record\fP is copied from the pick device descriptor to the
pick measure when the pick measure is created in order to
provide pick-device-dependent initialization values.
Since the format of this
data record depends on the type of pick device that is in use,
and since there are currently no registered pick
device types, clients will need to consult the PEX implementation
documentation in order to understand the contents of the pick data record.
.LP
The \fPprompt/echo type\fP indicates what should happen when a primitive
is selected.  \fIPrompting\fP is the action that can occur before
the selection trigger is pressed, and \fIechoing\fP is the action that
can occur after the selection trigger is pressed.  The two can be different.
For prompting, a device might cause all output primitives to be drawn in
white as the pointer moves over them.  When the selection trigger occurs,
the selected primitive may be flashed three times in succession to draw
attention to it.
The supported \fIPromptEchoTypes\fP can be queried with
the \fBPEXGetEnumeratedTypeInfo\fP request.
The choices include highlighting only the primitive itself
(\fIEchoPrimitive\fP), highlighting all the primitives in the same
structure that have the same pick id (\fIEchoStructure\fP), and highlighting
the entire structure network that contained the picked primitive
(\fIEchoNetwork\fP).
The \fIecho_volume\fP attribute indicates the region in device coordinates
in which prompting/echoing will occur.  The \fIecho_switch\fP attribute
indicates whether prompting/echoing is to be enabled or disabled.
.LP
Once a pick measure resource is created,
it can be updated (used to perform
picking) by passing it
an input record.  Since the format of this
input record also depends on the type of pick device that is in use,
and since there are currently no registered pick
device types, clients will need to consult the PEX implementation
documentation in order to understand the contents of the input record.
An example input record might be one that supports picking with the
pointing device in core X.  The client could wait for an X
(pointer) button press
event.  When such an event occurred, the current (x,y) location of the pointer
could be bundled into an input record that would be sent to a pick measure
resource with a \fBPEXUpdatePickMeasure\fP request.  The PEX server extension
would then commence to determine the selected primitive.
The pick status would be set to \fIOk\fP
if a primitive satisfied the selection criteria, and the pick path would
be updated
with the picked primitive's structure id, element offset, and pick id.
If no primitive met the selection criteria, the pick measure's pick status
attribute will be set to \fINoPick\fP.  The results of the update operation
can be obtained using the \fBPEXGetPickMeasure\fP request.
.LP
Like structure traversals operations, picking operations can potentially
take a long time.  Nevertheless, picking operations are considered to
be atomic, in that other requests that affect resources required during
a picking operation may happen before or after, but not during, a picking
operation.  Conceptually, a pick measure obtains a copy of all
structure, lookup table, and name set resources that are accessed
during the picking operation.  This allows changes to those resources
to occur during the pick operation.  However, the state used for the
picking operation is effectively "frozen" at the time the picking operation
begins.
.bp
.AC "PEX Fonts" 2
.LP
[Description of PEX fonts is TBD]
.bp
.AC "PEX Functionality Overview" 1
.LP
.RU
.LP
There are two major areas in PEX that require description from a functional
point of view: structure traversal and rendering.  This section defines how
the resources described in previous sections support the functionality
of these two areas.  In addition, more details are provided as to the
effects of changing various resource attributes.
.AC "PEX Structure Traversal" 2
.LP
\fIStructure traversal\fP is the process that turns a hierarchical
structure network into a flattened list of output commands that
will be processed by a rendering pipeline.
Because of differing display capabilities
and limitations,
PEX has been designed to allow flexibility in the way clients
can produce such a flattened list of output commands.
.KS
.PS 4

wire_ht=1.25
wire_wid=4
struct_ht=1.5

my_ht=.6
my_wid=1

	down 

CLIENT:	ellipse ht my_ht wid my_wid "Client"
	move down wire_ht/2 left wire_wid/2
	line dashed right wire_wid
	arrow from CLIENT.s down wire_ht

RDR:	box ht my_ht wid my_wid "Renderer"
	arrow down .4

DRAW:	box ht my_ht wid my_wid "Drawable"

	move to CLIENT.s - (0,wire_ht/4)
	"  Output Commands" ljust


	right

	move to CLIENT.e
	line <->
STR:	box dotted ht my_ht wid my_wid "Structures"

.PE
.sp 0.25i
.ce
\f6Figure 3.1:  Client-Side Traversal\fP
.KE
.LP
The simplest type
of support is shown in Figure 3.1.  The dashed line represents
the client/server network interface boundary.  In this case, a
client is responsible for setting up a renderer resource, connecting
the renderer to a drawable, and maintaining all of its own
structure memory "above the wire".  The client must go through
its own structure memory and issue sequences of output commands
directly to a renderer.  This method of dealing with the PEX
server extension is called \fIclient-side traversal\fP, since all
of the data structures are on the client side, and the PEX extension
only receives a flattened list of output commands.  It is also
sometimes referred to as \fIimmediate mode\fP, since output commands
are sent directly to a renderer for immediate execution.  Output
commands may be freely mixed with other PEX requests (like changes
to lookup tables or changes to name sets) in the data
stream that is sent across the wire, and each request is processed
as soon as it is received.
.LP
In client-side traversal, the client is responsible for maintaining
the state of the renderer's pipeline.  If the client wants to save
the renderer's pipeline state, it must issue a \fBPEXBeginStructure\fP
request.  If it wants to restore the saved pipeline state, it must
issue a \fBPEXEndStructure\fP request.  In this mode, unless all of
the client's output commands will fit into a single
\fBPEXRenderOutputCommands\fP request, it is possible that other
client's requests will be intermixed with the commands to completely
traverse the first client's data structures.
.LP
Another possibility in PEX is to store output commands in structure
resources, and execute an entire structure network
via a \fBPEXRenderNetwork\fP request.
This introduces
some policy decisions that are intended to directly and efficiently
support the concept of PHIGS-style structures "below the wire".
In PEX,
as in PHIGS, attributes are saved when a structure is called and restored
when control is returned from the called structure.  In addition,
the called structure inherits a local modeling matrix that is
the identity matrix, and a global modeling matrix that is the
concatenation of the local and global modeling matrices at the time
the structure was called.  These semantics are exactly those
required by PHIGS, hence the structure resources in PEX can
actually be considered "PHIGS-style" structures.  Storing all
the structures below the wire and traversing them in this fashion
is referred to as \fIserver-side traversal\fP, as shown in Figure 3.2.
.KS
.PS 4

wire_ht=1.25
wire_wid=4
struct_ht=1.5

my_ht=.6
my_wid=1

	down 

CLIENT:	ellipse ht my_ht wid my_wid "Client"
	move down wire_ht/2 left wire_wid/2
	line dashed right wire_wid
	arrow from CLIENT.s down wire_ht

RDR:	box ht my_ht wid my_wid "Renderer"
	arrow down .4

DRAW:	box ht my_ht wid my_wid "Drawable"

	move to CLIENT.s - (0,wire_ht/4)
	"  \fBPEXRenderNetwork\fP request" ljust


	right

	move to RDR.e
	line <->
STR:	box dotted ht my_ht wid my_wid "Structures"

.PE
.sp 0.25i
.ce
\f6Figure 3.2:  Server-Side Traversal\fP
.KE
.LP
A \fIstructure traverser\fP is part of the built-in functionality found in a
PEX renderer.  Clients cannot access a renderer's traverser other than to
tell it to traverse a specific structure network.
When a renderer is created,
the traverser will be in an idle state.
At some point, the renderer
may receive a \fBPEXRenderNetwork\fP request.
The traverser will then lock the drawable
so that any other request made by any client that would affect the drawable
will be blocked.
.LP
The \fBPEXRenderNetwork\fP request, when processed,
initiates a traversal of an entire structure network.
If the traverser is already in the process of doing
a traversal, the traversal will not be initiated
until the traversal in progress has finished.
The \fBPEXRenderNetwork\fP request only initiates
a traversal, it does not
wait for the traversal to complete.
Conceptually, the traverser
obtains its own local copy of the structure network
and any referenced lookup tables or name sets
when the traversal begins.  This
allows changes to structures, lookup tables,
and name sets to be processed while a traversal is occurring.
However, the state used for a traversal is effectively "frozen" at the time
the traversal begins.
An alternate way to express this is that changes to structures, lookup tables,
and name sets that are referenced by the structure network being traversed
become \fIpending changes\fP that will occur eventually.
Inquiries made on these resources will return the result obtained as
if all pending changes were applied.
.LP
After the traverser has obtained its own local copy of the structure
network and all referenced lookup tables and name sets,
the traverser fetches output commands (structure elements)
from the structure network one at a time and
sends them to the rendering pipeline to be processed.
Calls to other structures
cause the traverser to temporarily suspend processing of the current structure
and begin fetching output commands from the specified structure.  When all of
the elements in the called structure have been processed, the traverser
resumes processing the calling structure.
.LP
At the start of a traversal, the equivalent of a \fBPEXBeginRendering\fP
is performed.  At the completion of a traversal, the equivalent of a
\fBPEXEndRendering\fP is performed.  Even though the traversal of
a large structure network may take a long time,
it is important to note that the
\fBPEXRenderNetwork\fP request is considered to be atomic - other requests
that involve the resources necessary to perform a traversal conceptually happen
either before the traversal or after the traversal, but never during the
traversal.
.LP
The \fBPEXRenderNetwork\fP request
does not clear the drawable implicitly before initiating
the traversal.  It is up to the client to clear the drawable prior
to the traversal if it so desires.
While the structure traversal is occurring,
regular request processing may continue.
However, request processing will block if the
request being processed will affect the drawable
in which the traversal is occurring.  Clients can easily arrange to
avoid such blocking by only sending requests that affect
resources other than the drawable on which the traversal is occurring.
.LP
Clients are always permitted to mix other PEX requests with
\fBPEXRenderNetwork\fP requests.  In fact, other output commands
can be issued using the \fBPEXRenderOutputCommands\fP request,
allowing the concept of \fImixed traversal\fP as shown
in Figure 3.3.  This can be useful in an environment where it is
advantageous to maintain the structure network hierarchy on the client
side, but "cache" all of the object geometry in structure resources
in the server.
.KS
.PS 4

wire_ht=1.25
wire_wid=4
struct_ht=1.5

my_ht=.6
my_wid=1

	down 

CLIENT:	ellipse ht my_ht wid my_wid "Client"
	move down wire_ht/2 left wire_wid/2
	line dashed right wire_wid
	arrow from CLIENT.s down wire_ht

RDR:	box ht my_ht wid my_wid "Renderer"
	arrow down .4

DRAW:	box ht my_ht wid my_wid "Drawable"

	move to CLIENT.s - (0,wire_ht/4)
	"  Output Commands + \fBPEXRenderNetwork\fP request" ljust

	right

	move to CLIENT.e
	line <->
	box dotted ht my_ht wid my_wid "Structures"

	move to RDR.e
	line <->
	box dotted ht my_ht wid my_wid "Structures"

.PE
.sp 0.25i
.ce
\f6Figure 3.3:  Mixed Traversal\fP
.KE
.LP
A final option for structure traversal is provided by the PHIGS workstation
resource.  The PHIGS workstation resource embodies the PHIGS abstraction
of a workstation by maintaining all of the attributes as well as
performing all of the side effects that PHIGS expects.  In order to
perform structure traversal with a PHIGS workstation resource,
clients must send output commands
to structure resources to create a structure network, and then post
one or more structure networks to a PHIGS workstation resource.
.LP
Various PEX requests can cause the PHIGS workstation's posted
structure networks to be retraversed.
The most straightforward
way to accomplish a traversal of all of the posted structure networks
is with the \fBPEXRedrawAllStructures\fP request.  When posted, structure
networks are given a priority that is taken into account when determining
the order in which to traverse the list of posted structure networks.
The PHIGS workstation resource contains a "built-in"
renderer resource.  The PHIGS workstation resource determines the order
in which to traverse the list of posted structure networks, then hands
them off one at a time for the built-in renderer to traverse and render
onto a drawable.
This traverser (conceptually)
obtains its own local copy of all of the posted structure networks
and all referenced lookup tables and name sets
when the traversal begins, allowing changes
to be processed while a traversal is occurring.
Figure 3.4 demonstrates structure traversal using
a PHIGS workstation resource.
.KS
.PS 4

wire_ht=1.25
wire_wid=4
struct_ht=1.5

my_ht=.6
my_wid=1

	down 

CLIENT:	ellipse ht my_ht wid my_wid "Client"
	move down wire_ht/2 left wire_wid/2
	line dashed right wire_wid
	arrow from CLIENT.s down wire_ht

WKS:	box ht my_ht wid my_wid "Phigs" "Workstation"
	arrow down .4

RDR:	box dotted ht my_ht wid my_wid "Renderer"
	arrow down .4

DRAW:	box ht my_ht wid my_wid "Drawable"

	move to CLIENT.s - (0,wire_ht/4)
	"  \fBPEXRedrawAllStructures\fP request" ljust


	right

	move to RDR.e
	line <->
STR:	box dotted ht my_ht wid my_wid "Structures"

.PE
.sp 0.25i
.ce
\f6Figure 3.4:  PHIGS Workstation Traversal\fP
.KE
.LP
There are a few more differences between PHIGS workstation traversal
and the other traversal methods.
First, the PHIGS workstation resource maintains
its idea of the state of the attached drawable.  This state includes
whether or not the image on the
drawable is "correct" with respect to the current state of the PHIGS
workstation attributes and the attributes of all resources associated
with the PHIGS workstation.
The \fBPEXUpdateWorkstation\fP request will cause a traversal if and
only if the image on the drawable is not "correct".
In other words, this request will cause a traversal of the list of
posted structure networks if any changes have occurred to the
attributes of the PHIGS workstation, posted structures, lookup tables,
or name sets that would cause the image to change.
Also,
certain changes to the PHIGS workstation are considered to be pending,
and are not actually applied the instant the request to change them is
received.  Changes to the PHIGS workstation's view table entries,
NPC subvolume, viewport, and HLHSR mode are treated in this fashion.
Whenever a structure traversal is initiated on a PHIGS workstation resource,
pending changes of this type are applied before the traversal actually
begins.
In addition, the PHIGS workstation maintains a flag that indicates whether
anything has been drawn on the associated drawable.  Whenever a traversal
is initiated on a PHIGS workstation, this flag is checked to see whether
the drawable is currently \fIEmpty\fP or \fINonEmpty\fP.  If the drawable is
\fINonEmpty\fP, it is cleared to the color stored in entry zero of its
color table before the traversal begins.
Finally, at the completion of the traversal, the state of the image
on the drawable is set to "correct".
.LP
The PHIGS workstation has a display update attribute that, depending on the
implementation, may cause traversals to occur at various other times.
Any traversals that are initiated in this manner
will have the semantics for PHIGS workstation resources
defined in the preceding paragraphs.
If the display update is set to \fIVisualizeNone\fP, changes to posted
structure networks and associated lookup tables will not cause any
traversals to occur; they can only occur if a \fBPEXRedrawAllStructures\fP
or a \fBPEXUpdateWorkstation\fP request is issued.
If the display update is set to \fIVisualizeEach\fP, each change will
cause a traversal to occur.
In all other cases,
the display update attribute is treated as a hint that may be used in an
effort to keep the displayed image reasonably correct without a
large number of complete traversals.
Since it is a hint, it can cause different sequences
of events on different workstations.
If the display update is set to \fIVisualizeEasy\fP, changes that the
PEX extension thinks are "easy" to perform will be performed immediately.
"Easy" changes are defined to be those changes that can be performed
without a regeneration of the displayed image.  Therefore, changes
will never cause traversals when display update is set to \fIVisualizeEasy\fP.
If the display update is set to \fIVisualizeWhenever\fP, changes will be
visualized whenever the server feels like performing a traversal.
If the display update is set to \fISimulateSome\fP, easy changes will be
visualized immediately and other changes will be simulated (e.g., drawing
an "X" through deleted primitives, or drawing them in the background color).
Changes that cannot be visualized immediately or simulated will cause
a traversal to occur.
.bp
.AC "PEX Geometry Transformations" 2
.LP
This section presents an overview of the geometry transformation portion
of the PEX rendering pipeline.
PEX output primitives contain geometrical information such as
coordinates and normals.
Various steps are needed to convert geometry information provided by
the client into locations on a physical display device.
\fIGeometry transformation\fP is the process which computes
a projection of three-dimensional geometry onto a two-dimensional display.
.LP
.AC "Geometry Data" 3
.EQ
delim $$
.EN
.LP
In this discussion, homogeneous coordinates are represented as column
vectors.  Thus, the point represented by $P ~=~ (x,~ y,~ z,~ w)$ will be written
as:
.EQ
P ~=~ left [ ~ pile { " " above x above y above z above w above " "} ~~ right ]
.EN
Geometric transformations of homogeneous points can be expressed in
terms of 4\(mu4 matrices.  For instance, we
could express the identity transformation as:
.EQ
I ~=~ left [ ~
   pile { " " above 1 above 0 above 0 above 0 above " "} ~~
   pile { " " above 0 above 1 above 0 above 0 above " "} ~~
   pile { " " above 0 above 0 above 1 above 0 above " "} ~~
   pile { " " above 0 above 0 above 0 above 1 above " "} ~~
      right ]
.EN
Following mathematical conventions,
coordinate transformations are represented as a matrix (which represents
the transformation to be performed) times a column vector (which represents
the coordinate to be transformed).  To transform the point P by the matrix
M to get the point P\(aa, we would write:
.EQ
P\(aa ~=~
    {
    left [ ~ pile { " " above x\(aa above y\(aa above z\(aa above w\(aa above " "} ~~ right ]
    }
~~=~~    
    {
    left [ ~
    pile { " " above m sub 11 above m sub 21 above m sub 31 above m sub 41 above " "} ~~
    pile { " " above m sub 12 above m sub 22 above m sub 32 above m sub 42 above " "} ~~
    pile { " " above m sub 13 above m sub 23 above m sub 33 above m sub 43 above " "} ~~
    pile { " " above m sub 14 above m sub 24 above m sub 34 above m sub 44 above " "} ~~
    right ]
    }
    ~ 
    {
    left [ ~ pile { " " above x above y above z above w above " "} ~~ right ]
    }
.EN
Subsequent transformations that are to be applied to the point P after
all the transformations in M have been applied are said to be
\fIpostconcatenated\fP to the transformation matrix M.  For instance,
to add to M a transformation that would do a rotation R followed by a
scaling S followed by a translation T, one would compute the transformation:
.EQ
M sub new ~=~ T S R M sub old
.EN
where R, S, and T are 4 \(mu 4 matrices representing the desired
transformations.  If it is desired that the transformations R, S, and T
are to be applied \fIbefore\fP the transformations specified in M have
been applied, the individual matrices are said to be \fIpreconcatenated\fP
and can be thought of as being  applied in the order shown below:
.EQ
M sub new ~=~ M sub old R S T 
.EN
.AC "Computing Normals" 3
.LP
Some computations in the rendering pipeline require normals to
surfaces.  PEX allows normals to be passed as part of the geometry for
some output primitives.  Normals at facet vertices are used in some
light source shading computations.  Normals to the facets themselves
are required in culling operations and some
light source shading computations.  If the facet normal is not provided,
it is computed in the following fashion.
.LP
For a fill area or fill area set, the normal is computed
by finding two points, B and C.  The first vertex in the vertex list is
considered point A.  Point B is the next point in the list that is
non-coincident with A.  Point C is the next point in the list after B
that is non-colinear with A and B.  The normal is computed by taking
${AB} vec \(mu {AC} vec$.  Normals for indexed polygon facets are computed
in a similar fashion, except that the points are computed by referencing
through the vertex indices that define each facet.
For a triangle strip primitive, the normal for each triangle is computed
in the same fashion, using the three vertices of the triangle as points
A, B, and C.  For the purposes of computing the cross product,
the order of the vertices is reversed for every other
triangle.  The vertices of the first triangle in the strip are not reversed.
For each quadrilateral mesh primitive defined with the vertices
$A = (i,j)$, $B = (i,j+1)$, $C = (i+1,j)$, and $D = (i+1, j+1)$, where
\fIi\fP and \fIj\fP are indices into the \fIm \(mu n\fP array of vertices,
the normal is computed using
${AD} vec \(mu {CB} vec$.
For parametric surfaces, the geometric normal is computed by taking
the cross product of the tangent along the \fIu\fP direction (usually
the partial derivative with respect to \fIu\fP), and the tangent along
the \fIv\fP direction (usually the partial derivative with respect to \fIv\fP).
.LP
The client may control the orientation of the computed
normals by controlling the
order of the vertices or control points in the primitives.  If this
is unacceptable, the normals should be computed explicitly by the client
(utilities for doing so can be provided) and then passed to the server.
.AC "Modeling Coordinate System" 3
.LP
All geometry information for output primitives in PEX starts off
in the \fImodeling coordinate system\fP.  The modeling coordinate system
is one that is chosen by the application for convenience.  An architect
may be working on a building design where the unit of measurement is
meters or feet.  The architect may choose to define the origin of this 
coordinate system to be one corner of the building, and describe all the
pertinent coordinates of the design relative to that corner.  Walls,
hallways, and elevators may then all conveniently be parallel to one
of the major axes.
A car designer may specify the origin for his car body to be at the
geometrical center of the car chassis, and define the position of all
of the car's components with respect to that center point.  A person
designing a wheel assembly for the car may choose to define a different
coordinate system that is convenient for specifying the exact geometry
of the wheel assembly.
.LP
If we wish to display a collection of such objects in the same image, we
must map the geometry of each object from the object's modeling coordinate
system into a coordinate system that is common to all objects, the
\fIworld coordinate
system\fP.  This mapping from an object's modeling coordinate
system to the world coordinate system
is referred to as the \fIcomposite modeling transformation\fP, since
it consists of two stages.
The first stage, called the \fIlocal modeling transformation\fP, maps
the modeling coordinate system of a sub-assembly
or sub-object into the modeling coordinate
system of the entire assembly or object.
The second stage is called the \fIglobal
modeling transformation\fP and is used to map the entire assembly's modeling
coordinate system into the world coordinate system.
The combination of the two stages
effectively maps a sub-assembly directly into the world coordinate system.
.LP
The local and global modeling transformations
may involve scaling, translation, and rotation operations.
Various scaling, translation, and rotation operations
can be combined by concatenating the operations into a single 
transformation matrix.  The composite modeling transformation is
defined by
.EQ
T sub comp = T sub global T sub local
.EN
where $T sub global$ represents the global modeling transformation
matrix and $T sub local$ represents the local modeling transformation
matrix.
.LP
The global modeling transformation matrix may be set to a new value
at any time.  The result of setting the global modeling transformation
to a new value $T sub newglobal$ is:
.EQ
T sub comp = T sub newglobal T sub local
.EN
The local modeling transformation matrix may be set to a new value by
specifying a matrix to replace the existing one,
by specifying a transformation matrix whose transformations are to be
applied to coordinates \fIbefore\fP those in the current local transformation
matrix (preconcatenation), or
by specifying a transformation matrix whose transformations are to be
applied to coordinates \fIafter\fP those in the current local transformation
matrix (postconcatenation).
The results of each of these possibilities are:
.LP
Composition type = replace
.EQ I
T sub local = T sub newlocal
.EN
.EQ I
T sub comp = T sub global T sub newlocal
.EN
Composition type = preconcatenate
.EQ I
T sub local = T sub oldlocal T sub newlocal
.EN
.EQ I
T sub comp = T sub global T sub local
.EN
Composition type = postconcatenate
.EQ I
T sub local = T sub newlocal T sub oldlocal
.EN
.EQ I
T sub comp = T sub global T sub local
.EN
.LP
The car example can be used to illustrate how this can be useful.  We
know that a car has a single chassis which has been designed using its
own modeling coordinate system.  The car also has four wheels, each of
which are identical.  We can design one wheel in any convenient modeling
coordinate system we choose, then specify four different local modeling
transformations that cause that single wheel description to be mapped,
or \fIinstanced\fP, in four different positions relative to the car chassis.
The global modeling transformation is then used to map the entire car
assembly (chassis and four wheels) into the coordinate system we have chosen to
represent our "world".  (In practice,
a sub-assembly's local modeling matrix is concatenated to the global
transformation for the assembly to form a composite modeling matrix,
which can then be used to map the sub-assembly
directly into the world coordinate system.)
.LP
It is possible to provide modeling matrices that do perspective, shearing,
or non-uniform scaling, and thus do not preserve angles while transforming
geometry from modeling coordinates into world coordinates.
Since some
light source shading computations depend on surface normals,
this can cause surface normals that have been specified with primitives to be
transformed in such a way as to introduce shading anomalies.
The results of the light source computation for modeling matrices
that are not angle-preserving is implementation-dependent.
.AC "World Coordinate System" 3
.LP
The world coordinate system is a right-handed coordinate system.  This is
the type of 3D coordinate system that is most familiar to scientists and
engineers.  
.LP
Once all objects are mapped into the world coordinate
system, all the interrelationships can be computed.
We can also specify additional information in world coordinates, such
as where lights are located within the scene and the position from which
the scene is to be viewed.  
.AC "Modeling Clipping" 3
.LP
Once all geometry has been transformed into the world coordinate system,
modeling clipping (if enabled) occurs.  The current modeling clipping
volume consists of a list of halfspaces that are combined with boolean
operators to define a clipping volume in world coordinates.  This
volume is then used to clip subsequent
output primitives.
.AC "Viewing Reference Coordinate System" 3
.LP
There is enough information in the world coordinate system to do much
of the work that needs to be done in determining the final representation
of the image.  However, the world coordinate system is not a terribly
convenient system to use for many of the computations that are necessary.
A transformation known as the \fIview orientation transformation\fP is
used to transform coordinates from the world coordinate system into
a more convenient coordinate system for such computations.  This
coordinate system is known variously as the
\fIviewing reference coordinate system\fP, the \fIviewing coordinate system\fP,
or the \fIeye coordinate system\fP.  For consistency with ANSI/ISO standards
terminology, this coordinate system is referred to as the viewing reference
coordinate (VRC) system.  The viewing reference coordinate system is
also a right-handed coordinate system, and is
defined to have the viewing position at the origin, the positive y-axis
increasing in the direction defined as "up", the positive x-axis increasing
to the right and the direction of view along the negative z-axis.
.LP
The viewing position and viewing parameters are used to construct a
transformation that maps world coordinates into
coordinates in the viewing reference coordinate system.
This transformation is also conveniently expressed in terms of a 4 \(mu 4
matrix since it involves only translation and rotation operations.
.LP
There are several methods of computing the matrix used to define the
view orientation
transformation.  One method involves specifying a world coordinate point
that represents the viewing position, a vector representing the direction
of view, and a vector which specifies the "up" direction for the view.
The view mapping matrix is then computed by forming a transformation matrix
which translates the viewing position to the origin, aligns the direction
of view with the negative z-axis, and then aligns the "up" direction with
the positive y-axis.
A second method requires the specification of a
viewing position in world coordinates, and altitude, azimuth, and twist
angles to define the direction and orientation of the view.  The matrix
is computed by concatenating transformations that translate the viewing
position to the origin, aligning the view direction specified by the
altitude and azimuth angles with the negative z-axis, and applying the
twist angle.
.LP
Since the viewing position is required for certain light source shading
computations and for back/front-face culling operations,
and since it is not provided to PEX explicitly, it must
be extracted from the view orientation and view mapping
matrices that are provided.  By definition, the viewing position
is the point in world coordinates that transforms to infinite positive
z in normalized projection coordinates (the homogeneous point
$(0,~ 0,~ 1,~ 0)$).  Therefore, the viewing position can be determined
by transforming the normalized projection coordinate point
$(0,~ 0,~ 1,~ 0)$ through the inverse of the view mapping matrix
and then through the inverse of the view orientation matrix.
For perspective projections, the result is the viewing position
in world coordinates.  For parallel projections, the result is
a direction vector which represents the viewing direction.  The
viewing position lies at infinity along this line.  In the case
where either of the matrices is singular, there is either no solution,
in which case the implementation should choose an implementation-dependent
position, or an infinite number of solutions, in which case the implementation
should choose one of the possible solutions.
.[
PHIGS+ ad hoc
.]
.LP
Depending on the specific algorithm chosen, the viewing reference coordinate
system can be a very useful one in which to do shading
computations.  If standard utilities are used to create the
view orientations transformation matrix, the transformation will
contain only translation and rotation transformation.
All the angles and distances will be preserved since
no shearing or perspective operations have yet been applied.  It is
possible to determine how much light is reflected toward the eye given
the light, object, and viewing positions; what portions of the scene
are in shadow; and so on.  Polygons that are backfacing with respect to
the viewing position can also be eliminated at this stage, saving
shading, clipping, transformation, perspective divide and scan
conversion computations later on.
However,
since the view orientation transformation
is specified with an arbitrary 4 \(mu 4 matrix, it
may contain perspective, shear,
or unequal scaling, and therefore
it may not preserve angles while transforming geometry
from world coordinates to view reference coordinates.
It is recommended (not mandated) that
the results of back-/front-face surface culling and light source shading
computations should be as though they were performed in world coordinates.
However, the manner in which these operations are treated in the case
of a non-angle-preserving view orientation matrix is left to be
implementation-dependent.
.AC "Normalized Projection Coordinate System" 3
.LP
The remainder of the geometry transformation stages of the rendering
pipeline are oriented towards mapping the geometry onto pixels on the
physical display.  Since the world coordinate system is of an arbitrary
orientation and in arbitrary units, and the drawable is of
arbitrary size, the geometry is mapped through an intermediate
coordinate system of pre-determined units and orientation.  This
coordinate system can then be eventually mapped into coordinates on
the physical display.  This intermediate coordinate system is known
as the \fInormalized projection coordinate\fP (NPC) system. 
The normalized projection coordinate system is also defined as a
right-handed coordinate system.
.LP
The \fIview mapping transformation\fP is used to map 
the viewing reference coordinate system into the normalized
projection coordinate system.  This transformation
typically contains a transformation
representing a parallel or perspective projection.  In the case of
a parallel projection, the "region of interest",
or \fIview volume\fP, is a rectangular parallelepiped with sides, front,
and back specified in terms of the viewing reference coordinate system.
For a perspective view, the view volume is a truncated pyramid (frustrum)
whose boundaries are specified in a similar fashion.
The view mapping transformation can be thought of as mapping
the specified view volume into the normalized projection
coordinate system.
.AC "Clipping" 3
.LP
Occasionally, it is necessary (or desirable) to map only a portion of the
unit cube in normalized projection coordinates onto the display screen.
This portion of the unit cube is called a "window" in ANSI/ISO graphics
standards terminology.  We refer to this volume as the \fINPC sub-volume\fP
in PEX so as to avoid confusion with the 
term "window" as defined within the context of the X Window System.
.LP
The NPC sub-volume can be specified by providing the
min and max coordinates for the parallelepiped which represents the
desired sub-volume in normalized projection coordinates.  The NPC
sub-volume must be defined in normalized
projection coordinates with
0 \(<= xmin \(<= xmax \(<= 1,
0 \(<= ymin \(<= ymax \(<= 1, and
0 \(<= zmin \(<= zmax \(<= 1.
.LP
\fIView clipping limits\fP are also
expressed as normalized projection coordinates in the range
0\ \(<=\ xmin\ \(<=\ xmax\ \(<=\ 1,
0\ \(<=\ ymin\ \(<=\ ymax\ \(<=\ 1, and
0\ \(<=\ zmin\ \(<=\ zmax\ \(<=\ 1.
The \fIclipping volume\fP
is found by computing the intersection of the volume defined
by the view clipping limits with the NPC sub-volume.
Since both volumes are rectangular parallelepipeds, the clipping volume
will always be a rectangular parallelepiped.
Geometry (lines, polygons, etc.)
that is outside the clipping volume
is rejected at this point.
Geometry that crosses one of the faces of the clipping volume
is divided where it intersects the clipping volume.  The portion outside
the clipping volume is rejected, and the part within the clipping volume
is passed on
for further computation.  This process is commonly referred to as
\fIclipping\fP.
.LP
The clipping operation is typically defined as a linear interpolation.
Attributes that may vary from one one vertex to the next (such as color or
normals) will be linearly interpolated to compute the value of the attribute
at the intersection point.  The newly-computed attribute values will be
utilized from that point on in the rendering pipeline.
.LP
The result of the clipping process is a series of \fIclipped
normalized device coordinates\fP, or \fIclipped coordinates\fP for short.
.AC "X Window Coordinate System" 3
.LP
Drawables (windows and pixmaps) in X are defined with their
origin at the upper left-hand
corner.  A drawable has a height and width, measured in pixels
on the physical device.  X display devices are assumed to have
an aspect ratio of 1:1.  If the aspect ratio is not 1:1,
the server must make any adjustments necessary
to give the client the appearance of a device with an aspect ratio of 1:1.
Each pixel in the drawable has an address consisting
of an (x,y) coordinate pair.  In PEX, we include the possibility that
each pixel may have a stored z-value as well.  The range of the z-values,
if present, is device-dependent.
The \fIviewport\fP is an area within a drawable that defines where
3D graphics may appear, and is
defined relative to the drawable's origin.  The viewport may actually extend
beyond the bounds of the drawable.
.LP
The \fIX window transformation\fP stage
of the pipeline utilizes the NPC sub-volume limits to map clipped normalized
device coordinates into X window coordinates within the viewport.
The (xmin, ymin, zmin) coordinate of the NPC sub-volume
is mapped to the lower left corner of the viewport.
This is an isotropic transformation, meaning that
x and y values are scaled equally.  If the NPC sub-volume does not
have the same aspect ratio as the viewport, it will be mapped onto
the largest subregion of the viewport which preserves the
NPC sub-volume's aspect ratio.
.AC "Physical Device Coordinates" 3
.LP
The \fIphysical device coordinate transformation\fP is the 
final geometry transformation stage in the pipeline.  Since
windows and pixmaps can exist anywhere within the display system's memory,
the X window-relative coordinates must be translated into display
system memory addresses.
An additional clip is performed at this stage based on the drawable's
clip list.
Pixels outside the window will not be drawn, nor will pixels within any
region of a window that is obscured by another window.
.AC "Summary" 3
.LP
Coordinates are initially defined in the local modeling coordinate system and
are passed through each geometry transformation
stage of the rendering pipeline until they are discarded (as they might
be for clipping or back-facing polygon removal) or they have been transformed
into physical device coordinates.  Normals are also initially 
defined in the local
modeling coordinate system and are passed through the geometrical
transformation stages of the rendering pipeline until they reach the
viewing reference coordinate system.  At this point, they can be
used to determine back-facingness, visibility, shading information, and so on.
Beyond this stage, normals are no longer needed.
.LP
The following diagram illustrates the various geometry transformation
stages that occur in the rendering pipeline.  The boxes represent the
geometry transformation processes that occur in the rendering pipeline and the
phrases between the boxes represent the coordinate spaces that exist
between the transformation processes.
.KS
.PS
.nr PS 8
.ps 8
.B
boxht = 0.50i; boxwid = 1.5i;
lineht = 0.30i
down
"Output Primitives"
move down 0.1i
arrow "  local modeling coordinate system" ljust
box "Composite Modeling" "Transformation"
arrow "  world coordinate system" ljust
box "Modeling" "Clipping"
arrow "  clipped world coordinate system" ljust
box "View Orientation" "Transformation"
arrow "  viewing reference coordinate system" ljust
box "View Mapping" "Transformation"
arrow "  normalized projection coordinate system" ljust
box "Clipping"
arrow "  clipped normalized projection coordinate system" ljust
box "X Window Coordinate" "Mapping"
arrow "  X window coordinate system" ljust
box "Device Coordinate" "Mapping"
arrow "  physical device coordinate system" ljust
move down 0.1i
"Physical Device Coordinates"
.R
.nr PS 10
.ps 10
.PE
.sp .25i
.ce
\f6Figure 3.5:  Geometry Transformation Stages of the Rendering Pipeline\fP
.KE
.bp
.AC "PEX Color Transformations" 2
.LP
This section presents an overview of the color transformation portion
of the PEX rendering pipeline.
The color transformation stages of the rendering
pipeline dereference indexed color values, perform
light source shading and depth-cueing
calculations, and map the resultant colors to those
colors the device is capable of displaying.
.LP
The color transformation model supported by PEX is complicated by
the fact that a wide range of workstations with various color capabilities
must be supported.  PEX supports raster graphics devices including
monochrome systems, 8-bit graphics
systems with color lookup tables, and "true color" 24-bit
systems.  Obviously, applications that depend heavily on color
will suffer when executed on monochrome systems.  However, it should
still be possible for an application to determine the limitations of the
display device and utilize available resources to achieve the best possible
image.
.AC "Color Data" 3
.LP
Color values passed to and from the PEX server extension
are always thought of as color type/color value pairs.
Colors may be \fIDirect\fP (the color
is explicitly specified) or \fIIndexed\fP (the color is specified as an index
into a lookup table resource of type \fIColor\fP).
Each color value has an explicit or
implicit \fIcolor type\fP attribute that specifies whether the color
is direct or indexed.
Color values may be passed as either direct or indexed
values for any of the PEX requests, except when defining
the contents of a lookup table resource of type \fIColor\fP
(which requires direct colors)
and when creating an output command of the form "set {primtype} color index"
(provided for compatibility with PHIGS which requires indexed colors).
Clients may pass direct colors across the wire in any of the direct
color formats supported by the PEX server extension.
For each request that allows color values to be passed,
clients must pass a parameter defining the direct color format to be used.
This parameter allows the PEX server
extension knows the format of the direct color values it is supposed
to send or receive.
.LP
Initially, six different direct color
formats are defined in PEX, and more
can be added later as
necessary.  The first format
is \fIRGBFloat\fP, which requires an array of three floats,
with values in the range [0-1.0] to represent the red, green, and blue
values.
The second defined type is \fIRGBInt8\fP, where 8-bit intensity
values in the range [0-255] for red, green, and blue values are passed,
in order, as unsigned 8-bit values.
The third supported format is \fIRGBInt16\fP, where 16-bit intensity
values in the range [0-65535] for red, green, and blue values are passed,
in order, as unsigned 16-bit integers.
The fourth supported format is \fIHSVFloat\fP, which requires an array of three
floats with values in the range [0-1.0] representing the hue, saturation,
and value as defined by the HSV color cone.  The fifth supported format
is \fIHLSFloat\fP, which also requires an array of three floats.  These three
values all range from [0-1.0] and represent the hue, lightness, and saturation
as defined by the HLS double-ended color cone.  The sixth supported format
is \fICIEFloat\fP which requires an array of three floats that represent a
2D coordinate on the CIE chromaticity diagram plus a luminance value.
.LP
Output primitive color attributes that are specified using indexed color
values provide a level of indirection for applications.  The actual color
will not be bound until the output primitive command is processed
by a renderer.  At that time, the color index is used as an offset into
the associated color lookup table
to determine the actual color value to be used.
Color index values are always stored within the server as 16-bit (or longer)
unsigned integer quantities.
Direct colors that are passed to the server
are converted to a device-specific internal format for use by a renderer or
for storage in a structure.  The device-specific internal color values are
converted to the color format of the client's choice when queries are
performed.  The result that is passed back to the client may contain less
precision than the original color value due to these conversions.
Some devices may suffer a performance penalty
if color values are not provided in the format that is best-suited
to the hardware.
.LP
One of the important assumptions about the color transformations in
the PEX color model is that \fIthe PEX rendering
pipeline supports color internally\fP, even on monochrome systems.
A PEX server implementation may approximate the color information
as a final step to provide a monochrome output, but the
color calculations within the rendering pipeline are performed in
a device-dependent color space.  On most devices, the color space
utilized internally
will likely be RGB, but there is no reason HLS, HSV, CIE, 
and other color models cannot be used.
.AC "Color Dereferencing" 3
.LP
The first color transformation stage in the rendering pipeline is called
\fIcolor dereferencing\fP.  In this stage, \fIinput colors\fP are
converted into \fIintrinsic colors\fP.
Indexed colors that are passed to the pipeline must be dereferenced by
using the index value to obtain a direct color value from a lookup table
resource of type \fIColor\fP.
Direct color values are passed through this stage unmodified.  In either
case, the output from this stage of the pipeline is the \fIintrinsic\fP
color of the object.
.AC "Modeling Clipping" 3
.LP
Geometry and color information are treated together during the
modeling clipping stage.
Once all geometry has been transformed into the world coordinate system,
modeling clipping (if enabled) occurs.
If geometry information is clipped during the modeling clipping stage
of the rendering pipeline, the corresponding color values are clipped
as well.
.AC "Light Source Shading Computation" 3
.LP
After the color type conversion stage, light source information,
the viewing position, and an object's surface characteristics
are combined to perform an operation known as the \fIlight source shading
computation\fP.  The only output primitives that are subjected to light
source shading computations are surfaces.
Markers, text, and line primitives
pass through this stage of the rendering pipeline unmodified.
.LP
There are two major types of information that are used in the light
source shading computation.  The first is \fIillumination information\fP,
which consists of
the type and characteristics of all enabled light sources in the scene.
The second is \fIreflectance information\fP which consists of
the parameters and the equations used to model the reflection of
light that strikes a surface.
The exact equations to be used in modeling the reflection of light are
implementation-dependent.  PEX implementors should always
document the equations that have been implemented in the
release notes documentation.  Some suggested equations
are provided later in this section.
.LP
The coordinate space in which light-source computations occur is also left to
be device-dependent.  Ideally, the effect of the light source computation
will be as if it were performed in world coordinates.  If the viewing
transformation is one that is angle-preserving (such as will always
be the case if utility routines from the PHIGS library are used to compute
the view orientation transformation), the light source computations
may be performed in the viewing reference coordinate system.
In either case, all angles and interrelationships between lights, objects,
and the viewing position can be properly computed.
.EQ
delim $$
.EN
.LP
There are four types of light sources currently supported in PEX.
The simplest type is called an \fIambient light source\fP.
An ambient light is an approximation to the illumination
caused by the reflection of light from all objects in the scene.
Each ambient light causes a constant amount to be added to the intensity
at each point in the scene.  An ambient light is effectively a
non-directional light that affects
the color of all surfaces regardless of their orientation.
An ambient light has only one characteristic: its color.
.LP
The second type of light source is called a \fIWCS vector light source\fP,
or a \fIdirectional
light source\fP, since such a light is assumed to be located at
infinity.  This has the effect of causing all light rays from a vector
light source to
be parallel when they strike an object's surface.  To define a vector
light source, only a direction vector in the world coordinate system
and a light source color are needed.
The direction vector specifies the direction from the vector light source
to any point in the scene.
.LP
A \fIWCS point light source\fP or a \fIpositional light source\fP is a light
that illuminates equally in all directions.
The source of this type of illumination is a particular point in the
world coordinate system.  Attenuation factors can be provided so that the
point light source's intensity will drop off as a function of distance.
Therefore, the properties of a WCS point light
source consist of the light's color, world coordinate position,
and two attenuation coefficients.
.LP
The fourth type of light source is called a \fIWCS spotlight\fP.
This type of light simulates the behavior of a spotlight.  Its position is
defined in the world coordinate system.  A spotlight is defined with
a color, a position, a direction vector, a concentration exponent,
attenuation coefficients, and a spread angle.  The contribution of
a spotlight is zero if the point in question lies outside of the
cone of influence of the spotlight.
.LP
In the equations that follow, we'll use the following notation for the
various light source attributes.  Note that different light source types
use different attributes in the list below:
.KS
.EQ I
L sub "C" ~~=~~ "color of light emitted by the light source (all)"
.EN
.EQ I
L sub "POS" ~~=~~ "world coordinate position of the light source (point, spot)"
.EN
.EQ I
L sub "DIR" ~~=~~ "direction of the light source (vector, spot)"
.EN
.EQ I
L sub "A1" ~~=~~ "first attenuation factor for the light source (point, spot)"
.EN
.EQ I
L sub "A2" ~~=~~ "second attenuation factor for the light source (point, spot)"
.EN
.EQ I
L sub "CONC" ~~=~~ "concentration exponent for the light source (spot)"
.EN
.EQ I
L sub "ANG" ~~=~~ "spread angle for the light source (spot)"
.EN
.KE
.LP
The term \fIreflection attributes\fP
refers to all of the attributes of a surface that
modify the basic illumination/reflection equation.
These attributes will vary from one type of material to another, and
may be modified to simulate different types of surfaces.
The reflection attributes are defined as part of a renderer's current
pipeline state.  The pipeline state actually contains two sets
of reflection attributes.  The \fIback face reflection attributes\fP
are applied to back-facing surface primitives if the \fIdistinguish\fP
attribute is set to \fITrue\fP.  Otherwise, all surface primitives are rendered
using the \fI(front face) reflection attributes\fP pipeline state values.
.LP
Point, vector, and spotlight sources
cause two types of reflection from an
object's surface.  The first type of reflection is known as \fIdiffuse\fP
reflection since the reflection is spread out across the surface of the object.
A dull matte surface exhibits only diffuse reflection:  there are no
sharp highlights, only broad ones.  For diffuse reflection, all the wavelengths
of light from the light source are absorbed, except the wavelengths
representing the color of the object, which are reflected.  Consequently,
diffuse reflection is always perceived as being some component of
the intrinsic color of the object.
The sharp highlights that can be seen on shinier objects are caused by
\fIspecular reflection\fP.
Specular reflections can be seen only when the viewing position is along (or
near) the vector that represents the direction of reflected light if
the surface were a perfect mirror.
The color of these highlights depends on the
type of surface being modeled.  Plastic-type surfaces have specular highlights
that are the color of the light source.  It is as if
there were a thin, clear plastic coating on the surface, and the light was
being reflected from this clear surface, rather than from the object-colored
surface below.
Metallic surfaces, on the other hand, exhibit specular
highlights whose color depends on the 
intrinsic color of the metal,  as well as
the wavelength and incident angle of the incoming light.
.LP
For use in the reflection equations that follow, let us define
the following symbols that represent values obtained either
from the \fIreflection attributes\fP or from the \fIback face
reflection attributes\fP, depending on the \fIdistinguish\fP flag:
.EQ I
K sub a ~~=~~ "ambient reflection coefficient"
.EN
.EQ I
K sub d ~~=~~ "diffuse reflection coefficient"
.EN
.EQ I
K sub s ~~=~~ "specular reflection coefficient"
.EN
.EQ I
S sub "CONC" ~~=~~ "specular concentration"
.EN
.EQ I
S sub "C" ~~=~~ "specular color"
.EN
.EQ I
K sub t ~~=~~ "transmission coefficient"
.EN
The \fIambient coefficient\fP, a value in the range [0.0-1.0],
is used to modify the contribution of the ambient light sources in the
shading equation.
The contribution from each ambient
light source will be multiplied by the ambient coefficient.
The \fIdiffuse\fP and \fIspecular
coefficients\fP are defined and function in the same fashion,
modifying the contribution
of the diffuse and specular reflection components, respectively, for
point, vector, and spotlight sources.  The \fIspecular color\fP is a color
by which the specular component will be multiplied, allowing the specular
highlight to appear as a color other than the color of the light source.
The \fIspecular exponent\fP defines the "sharpness" of the specular
highlight, and thereby the "shininess" of the surface.  The \fPtransmission
coefficient\fP indicates the amount of light transmitted by the surface.
A value of zero indicates an opaque surface and a value of 1.0 indicates
a totally transparent surface.  (The transmission coefficient is not
included in any of the suggested reflectance equations.  How it is used
is left to be implementation-dependent.  It is even permissible for
implementations to ignore this attribute entirely.)
.LP
PEX defines four standard values for the \fIreflection model\fP attribute.
The first, \fINoShading\fP, indicates that none of the lights in the
scene will have any effect.  This reflection model is essentially a
global override for turning all lights off.  The second reflection model,
\fIAmbient\fP, specifies that only contributions from enabled
ambient light sources
will be used during the light source shading computation stage.
\fIDiffuse\fP will sum only the contributions from ambient light sources and
the diffuse components of each enabled point, vector, and spotlight.
The \fISpecular\fP reflection model will sum the contributions from
enabled ambient light sources, as well as diffuse and specular contributions
of enabled point, vector, and spotlights.
.LP
A few other attributes needed for performing the light source shading
computations will be defined as follows:
.EQ I
O sub "IC" ~~=~~ "intrinsic color of object"
.EN
.EQ I
O sub "SC" ~~=~~ "color of object after light source shading computation"
.EN
.EQ I
O sub "POS" ~~=~~ "world coordiate position of object"
.EN
.EQ I
N ~~=~~ "vector normal to object"
.EN
.EQ I
R ~~=~~ "vector representing the direction of maximum highlights"
.EN
.EQ I
V ~~=~~ "vector from object to viewing position"
.EN
.LP
The suggested lighting equations are given below.
In all cases, if a dot product results in a value less than zero, it
will be replaced with the value zero.
Depending on the actual reflection model being used, some of the
components for each type of light may be ignored (e.g., if reflection
model is \fIDiffuse\fP, all specular contributions in the equations
below are ignored).  The discussion that follows assumes
the \fISpecular\fP reflection model is being used, since that method sums
all of the contributions from all of the enabled light sources.
.LP
To compute the shaded color of a point $O sub "SC"$, one would use:
.EQ I
O sub "SC" ~=~ I sub a + I sub v + I sub p + I sub s
.EN
where
$I sub a$ is the intensity contribution from all ambient light sources,
$I sub v$ is the intensity contribution from all vector light sources,
$I sub p$ is the intensity contribution from all point light sources, and
$I sub s$ is the intensity contribution from all spotlights.
(This equation must be duplicated for each color primary.  For instance,
if the light source
shading computations are being carried out in RGB color space, three
equations would be needed: one to compute the red component of $O sub "SC"$,
one to compute the green component, and a third to compute the blue.)
.LP
$I sub a$ can be computed by summing the contributions of
all of the enabled ambient light sources:
.EQ I
I sub a ~=~ K sub a O sub "IC" sum from i=0 to { i<n } L sub "C"
.EN
In this equation, $K sub a$ is the ambient reflection coefficient for the
object (and is obtained from the current pipeline state),
$O sub "IC"$ is the \fIbase\fP,
or intrinsic color of the object (obtained from the \fIsurface color\fP or
\fIbackface surface color\fP attribute in the current pipeline state, or
from the primitive itself if facet or vertex colors are provided),
$n$ is the number of enabled ambient lights
(determined by the \fIlight state\fP attribute in the current pipeline
state and the entries in the current light table),
and $L sub "C"$ is the color of the ambient
light (obtained from the color field in the light table entry that defines
the light).
.LP
$I sub v$ can be computed by summing the diffuse and
specular contributions of all of the enabled vector light sources:
.EQ I
I sub v ~=~ I sub "vd" ~+~ I sub "vs"
.EN
where the diffuse and specular components are defined as:
.EQ I
I sub "vd" ~=~ K sub d O sub "IC" sum from i=0 to i<n L sub "C" (N vec cdot { ( -  L sub "DIR" ) } vec )
.EN
.LP
.EQ I
I sub "vs" ~=~ K sub s S sub "C" sum from i=0 to i<n L sub "C" ( R vec cdot V vec ) sup {S sub conc}
.EN
The diffuse component is just the diffuse coefficient ($K sub d$) times the
intrinsic color of the object ($O sub "IC"$) times the sum of the color of
each enabled vector light ($L sub "C"$) multiplied by the cosine of the
angle between the surface normal ($N vec$) and the negation of the
light source's direction vector
($N vec cdot { ( - L sub "DIR" ) } vec$).
The specular component is the specular coefficient
($K sub s$) times the object's specular color ($S sub "C"$) times
the sum of the color of each enabled light ($L sub "C"$) multiplied by
the cosine of the angle between the direction of maximum highlights from
the light source and a vector from the object to the viewing position raised
to some power ($( R vec cdot V vec ) sup {S sub conc}$).
.LP
$I sub p$ is computed in a similar fashion:
.EQ I
I sub "pd" ~=~ K sub d O sub "IC" sum from i=0 to i<n L sub "C" (N vec cdot { ( - L sub "DIR" ) } vec ) L sub "ATT"
.EN
.LP
.EQ I
I sub "ps" ~=~ K sub s S sub "C" sum from i=0 to i<n L sub "C" ( R vec cdot V vec ) sup {S sub conc} L sub "ATT"
.EN
The terms in this equation are the same as those for vector light sources,
except that ${ ( - L sub "DIR") } vec$ represents a
vector in the direction of the light
source, and it and $R vec$ must be recomputed for each point to be shaded.
The additional attenuation factor $L sub "ATT"$ is defined as:
.EQ I
L sub "ATT" ~=~ 1 over {L sub "A1" + L sub "A2" ( mag (O sub "POS" - L sub "POS") ) }
.EN
The \fImag\fP function computes the magnitude, or distance, between
the two points.
The two attenuation factors are provided for somewhat easier control.  If
$L sub "A2"$ is zero, the attenuation does not depend on the distance
between the light source and the object.  Notice that an error will occur
if both attenuation factors are set to zero.
.LP
Finally, to compute $I sub s$, the contributions from spotlights, the
following equations are recommended:
.EQ I
I sub "sd" ~=~ K sub d O sub "IC" sum from i=0 to i<n L sub "C" (N vec cdot { ( - L sub "DIR" ) } vec ) { ( { O sub L } vec cdot { L sub "DIR" } vec ) } sup {L sub "CONC" } L sub "ATT"
.EN
.LP
.EQ I
I sub "ss" ~=~ K sub s S sub "C" sum from i=0 to i<n L sub "C" ( R vec cdot V vec ) sup {S sub conc} { ( { O sub L } vec cdot { L sub "DIR" } vec ) } sup {L sub "CONC" } L sub "ATT"
.EN
The terms in this equation are the same as those for point light sources,
except that a couple more have been added.  ${O sub L} vec$ is a vector
from the object to the spotlight position ${L sub "POS"}$.  ${L sub "DIR"} vec$
is a vector representing the direction that the spotlight is pointed.
.LP
The \fIinterpolation method\fP is another surface attribute that affects
the final appearance of a shaded object.  This attribute specifies if
interpolation occurs on the interior of a surface, and if so, how that
interpolation is performed.
If the interpolation method is \fINone\fP,
then the intensities computed at the vertices will be averaged to compute a
single intensity that will be used for the interior. 
If the interpolation method is \fIColor\fP, then the shade colors
computed at the vertices will be used as the basis for linear interpolation.
If the interpolation method is \fIDotProduct\fP, then the dot product values
computed at the vertices will be linearly interpolated for the interior, and
the light source
shading computation applied at each pixel in the interior using these
interpolated dot product values.
If the interpolation method is \fINormal\fP, then all normals
computed at the vertices will be linearly interpolated for the interior, and
the light source
shading computation will be
applied at each pixel in the interior using these
interpolated normal values.
.LP
A final surface attribute affecting the overall appearance of an object is 
called the \fIinterior style\fP.
If the interior style is \fIEmpty\fP, the surface interior is not drawn
at all.
If the interior style is \fIHollow\fP,
only the boundaries of the surface are displayed.
If normals exist at the vertices, the
reflection model is evaluated to compute the shaded color at each vertex.
If no vertex normals are present, the facet normal, if present, is used
at each vertex.  If no facet normal is present, a facet normal is computed
in the manner described in Section 3.1.2 (\fIComputing Normals\fP) and 
used at each vertex.
The color values at the vertices are then computed and the values
along each edge are rendered according to the current interpolation method.
If the interior style is \fIsolid\fP, the colors at the vertices
are computed as for \fIHollow\fP, but points within the facet are also
rendered.  
.LP
The list of supported reflection models can easily be extended later to
include more realistic or efficient shading algorithms.  The list of light
types can also be extended to accomodate other light source types.
Moreover, there are various ways of performing the specular reflection
component in addition to the one listed above.
The exact form of the shading equations is device-dependent, however
adherence to the formulas described above is strongly encouraged.
Additional surface 
approximations can also be added as they are shown to be useful.
.AC "Depth-Cueing Computation" 3
.LP
After colors have been transformed into shade colors by the light source
shading computation, they must be transformed into \fIdepth-cued colors\fP
by the \fIdepth-cueing computation\fP.  During this process, colors
are modified on the basis of distance from the viewing position.
An attribute called the \fIdepth-cue color\fP is used to indicate the
color to which objects towards the far depth-cue plane will tend.
\fIDepth-cue scaling factors\fP represent values by which colors will be scaled
at the front and back \fIdepth-cueing planes\fP.
Depth-cueing affects all primitives: curves, markers, text, and surfaces.
.LP
Using the following definitions:
.EQ I
C sub shade ~~=~~ "shade color"
.EN
.EQ I
C sub DC ~~=~~ "depth-cue color"
.EN
.EQ I
C sub "depth-cue" ~~=~~ "computed depth-cued color"
.EN
.EQ I
Z ~~=~~ "distance to object (in NPC)"
.EN
.EQ I
P sub "front" ~~=~~ "distance to front depth-cueing plane (in NPC)"
.EN
.EQ I
P sub "back" ~~=~~ "distance to back depth-cueing plane (in NPC)"
.EN
.EQ I
S sub "front" ~~=~~ "scale factor at front clipping plane"
.EN
.EQ I
S sub "back" ~~=~~ "scale factor at back clipping plane"
.EN
then the exact form of the depth-cueing computation is specified by:
.sp 1
If $Z$ is in front of $P sub "front"$ :
.EQ I
C sub "depth-cue" ~~=~~ C sub "shade" S sub "front" ~ + ~ C sub DC ~ \
( 1 ~ - ~ S sub "front")
.EN
If $Z$ is behind $P sub "back"$ :
.EQ I
C sub "depth-cue" ~~=~~ C sub "shade" S sub "back" ~ + ~ C sub DC ~ \
( 1 ~ - ~ S sub "back")
.EN
If $Z$ is between $P sub "front"$ and $P sub "back"$ :
.sp
.EQ
C sub "depth-cue" ~=~ { C sub "shade" { ~ left [ ~ S sub "back" ~ + ~ { { ( Z ~ - ~ P sub "back") (S sub "front" ~ - ~ S sub "back") } over { P sub "front" ~ - ~ P sub "back" } } ~ right ] ~ } ~ + ~ C sub DC { ~ left [ ~ 1 ~ - ~ S sub "back" ~ - ~ { { (Z ~ - ~ P sub "back") (S sub "front" ~ - ~ S sub "back") } over { P sub "front" ~ - ~ P sub "back" } } ~ right ] ~ } }
.EN
.AC "Clipping" 3
.LP
Once colors have been submitted to the depth-cueing process, they
are transformed from \fIdepth-cued colors\fP to \fIclipped colors\fP
by the process known as \fIclipping\fP.
Color values must be clipped if the geometry they
are associated with extends beyond the
viewing or workstation clipping limits.  A linear interpolation is performed
to compute the color at the point of intersection with the clipping boundary.
If necessary to support tiling 
(scan conversion) computations, normals and other attributes also
will be clipped, and linear interpolation will be performed to compute
new values at the clipped point.
.AC "Color Approximation" 3
.LP
A hardware colormap or software color
compensation stage just prior to sending frame buffer values to 
digital-to-analog converters for display
can provide a larger palette
of colors than the frame buffer memory would otherwise support, as well as
a level of indirection for values in the frame buffer.
These capabilities are quite important for applications in remote sensing,
image enhancement, and colormap animation, and for performing proper
gamma-correction of monitors.
Because of this,
the idea of a hardware colormap or color compensation stage has
been abstracted in X as a \fIcolormap\fP,
and the initialization, allocation and deallocation of colormaps has
been deferred to window managers or to applications.
There are routines in
Xlib that can be used to install
colormaps and allocate and deallocate cells
and planes in colormaps.  Since the colormap can alter
the final appearance of the display so greatly, it is important
to inform the PEX rendering pipeline about the type, length, and
content of the colormap being used, so that appropriate values can
be generated from PEX rendering pipeline
calculations and stored in the frame buffer memory.
.LP
The last stage in the conceptual rendering pipeline before the actual
writing of values to frame buffer memory performs
\fIcolor approximation\fP.
The proper values must be written \fIinto\fP the frame buffer so that
when the values are read \fIfrom\fP
the frame buffer and passed through the colormap and on to the display,
the expected color values appear.
The process of determining
the proper values to be written is
referred to as color approximation.
This stage converts \fIclipped colors\fP into \fIphysical device colors\fP,
which are colors that the device is capable of displaying.
The purpose of this stage is to quantize color information from the rendering
pipeline in a way that
compensates for the contents of a colormap.
It then becomes possible for tiling routines to
write values into the frame buffer only for those
color map entries that have been allocated.
.LP
There are two types of color approximation that are possible.  The
first type, called \fIRGB approximation\fP, converts pipeline color
values into RGB values that can be stored in an RGB frame buffer.
The second type, \fIintensity approximation\fP, converts pipeline color
values into a single intensity value that may be stored in a monochrome
or grayscale frame buffer.
.LP
In order to cause a rendering pipeline to perform RGB approximation,
the resource id of a lookup table of type \fIRGBApprox\fP must be stored
in the color approximation table attribute of a renderer or PHIGS workstation
resource.  For RGB approximation, the pipeline's internal color value 
is converted to an RGB value of some type.  That value is then mapped
into red, green, and blue intensity values and converted to integer values
using the values in the RGB approximation table as follows:
.DS I
red = red intensity from pipeline mapped into the range [0, \fIred_max\fP]
green = green intensity from pipeline mapped into the range [0, \fIgreen_max\fP]
blue = blue intensity from pipeline mapped into the range [0, \fIblue_max\fP]
.DE
Then, a single pixel value is formed by computing:
.DS
pixel = \fIred_mult\fP * red + \fIgreen_mult\fP * green + \fIblue_mult\fP * blue + \fIbase_pixel\fP
.DE
The resulting pixel value is then written into the frame buffer.
The use of addition rather than logical OR for composing pixel values
permits allocations where red, green, and blue values are not allocated
into distinct bitplanes.
\fIDither\fP is treated as a hint to the renderer as to whether
or not some attempt at dithering should be performed.  Whether
or not dithering is supported and the dithering algorithm that
is used are implementation-dependent.
.LP
When an \fIIntensityApprox\fP color approximation lookup table is bound
to a renderer or a PHIGS workstation,
each color that has been computed in the rendering pipeline is
converted to an intensity value in the range [0, 1].
The result is multplied by \fIintensity_max\fP and added to \fIbase_pixel\fP
to produce the pixel value to be written.
\fIDither\fP is treated as a hint to the renderer as to whether
or not some attempt at dithering should be performed.
This type of table may have multiple entries.  The entry to be
used depends on the type of primitive being rendered and the
current color for that primitive.
If the current color type
for the output primitive is \fIDirect\fP, entry zero in the
\fIIntensityApprox\fP table will always be used to convert
pipeline color values into pixels.
If the current color type
for the output primitive is \fIIndexed\fP, the color value will
be used as the index into the
\fIIntensityApprox\fP table to determine the entry of
the table that will be used to convert
pipeline color values into pixels.
.AC "Summary" 3
.LP
Colors are initially defined using the color format and
color type most convenient to the
application.  They
are passed through each color transformation
stage of the rendering pipeline until they are discarded (as they might
be for clipping or back-facing polygon removal) or they have been transformed
into physical device colors.  The format for colors in the rendering
pipeline is device-dependent, but it assumed that all devices perform
the computation in some color space.  Conversion to monochrome, if necessary,
is performed only at the final step.
.LP
Occasionally, applications will desire to ensure that color index values
sent to PEX are the actual numeric values that will be written into the
frame buffer.  Mathematical or logical operations ("tricks")
can then be performed on the colormap to achieve effects such as highlighting
overlapping areas or performing colormap animation.  It is not a goal
of PEX to support all of these types of tricks, since the PEX rendering
pipeline has been designed to support color computations in a true color
space.  However, it is possible to "turn off" the color transformation
stages of the PEX rendering pipeline by performing the following steps:
.ID
1.	Turn all light sources \fIOff\fP
2.	Turn depth-cueing effects \fIOff\fP
3.	Set surface interpolation method to \fINone\fP
4.	Set polyline interpolation method to \fINone\fP
5.	Set the renderer's or PHIGS workstation's color approximation table to
	be a table of type \fIIntensityApprox\fP that contains one value with
	\fIintensity_max\fP equal to the number of cells in the colormap,
	\fIbase_pixel\fP = 0, and dither = \fIOff\fP.
6.	Create a lookup table of type \fIColor\fP that contains \fIintensity_max\fP
	entries and initialize each entry \fIi\fP to have R = G = B = \fIi\fP / \fIintensity_max\fP
.DE
An indexed color value \fIi\fP will then cause a direct color value
R = G = B = \fIi\fP\ /\ \fIintensity_max\fP to be pushed down the pipeline,
and neither the light source shading stage nor the depth cueing stage
will affect the color.  One potential problem arises if the geometry,
and hence the color value, is clipped at any point in the pipeline.
Another potential problem is that the manner in which
pipeline internal color values are converted to intensity values at
the last stage of the pipeline is implementation-dependent.
If the conversion preserves the linearity
(i.e. a value of R = G = B = \fIi\fP\ /\ \fIintensity_max\fP
produces the pixel value \fIi\fP), and if primitives are always fully
within the various clipping volumes, this method will cause the input
index value to be written into the frame buffer.
The X colormap entries can then be manipulated to achieve the desired effect.
.LP
Figure 3.6 illustrates the various color transformation
stages that occur in the rendering pipeline.  The boxes represent the
color transformation processes that occur in the rendering pipeline and the
phrases between the boxes represent the color type that exists
between each pair of transformation processes.
.KS
.PS
.nr PS 8
.ps 8
.B
boxht = 0.50i; boxwid = 1.5i;
lineht = 0.30i
down
arrow "  input color" ljust
box "Color" "Dereferencing"
arrow "  intrinsic color" ljust
box "Modeling" "Clipping"
arrow "  clipped intrinsic color" ljust
box "Light Source Shading" "Computation"
arrow "  shaded color" ljust
box "Depth-Cueing" "Computation"
arrow "  depth-cued color" ljust
box "Clipping"
arrow "  clipped color" ljust
box "Color" "Approximation"
arrow "  physical device color" ljust
.R
.nr PS 10
.ps 10
.PE
.sp 0.25i
.ce
\f6Figure 3.6:  Color Transformation Stages of the Rendering Pipeline\fP
.KE
.bp
.AC "PEX Rendering Pipeline" 2
.LP
The complete PEX rendering pipeline is obtained by merging the geometry
transformation stages with the color transformation stages.  The result
is shown in Figure 3.7.  Each box contains the description of the process
that is occurring at that stage.  The coordinate system that the geometry
is in and the type of colors that are present at each stage are listed beside
each of the arrows.  Functions performed within the rendering pipeline that
are not shown in this diagram are discussed below.
.LP
The description of the rendering pipeline is included only
for purposes of documentation and clarification.
It serves only as a documentation tool for
relating the rendering process to the PEX interface.
In an actual implementation, there are many ways
to organize the rendering and transformation computations to achieve the same
effect as defined here.
This conceptual model of the rendering pipeline is not
intended to be used as a blueprint for hardware or software architectures,
but merely as a documentation tool for describing the eventual outcome
of the rendering process.
.AC "Name Set Testing" 3
.LP
Output primitives can be submitted to the filtering criteria imposed
by highlighting and invisibility filters before entering the rendering
pipeline.  Primitives that are to be treated as invisible are not
processed by the rendering pipeline.  Primitives that are to be
highlighted are submitted after some attribute or attributes of the
pipeline have been modified so that the primitive will be made visually
distinguishable.
.AC "Attribute Dereferencing" 3
.LP
Many of the attributes in the rendering pipeline can be obtained from
tables.  When these attributes are to be used, they must be fetched from
the tables in which they are stored.  Depending on the available hardware
(or lack of it), dereferencing of attributes can occur all at once,
together with color dereferencing, or on an "as needed" basis as rendering
occurs.
.AC "Hidden Line/Hidden Surface Removal" 3
.LP
All geometry must pass through a stage in which primitives that are obscured
from the point
of view are removed.
This process can occur at several stages in the conceptual rendering
pipeline, depending on the algorithm that is used.
For this
reason, its position in the pipeline is not explicitly defined.
.KS
.PS
.nr PS 8
.ps 8
.B
boxht = 0.30; boxwid = 1.2i;
lineht = 0.30i
down
"Output Primitive Geometry & Colors"
move down 0.1i
A1:
arrow
"  local modeling coordinate system" at A1.n.x, A1.n.y - 0.1i ljust
"  input colors" at A1.n.x, A1.n.y - 0.2i ljust
box "Color" "Dereferencing" with .n at A1.s
A2:
arrow
"  local modeling coordinate system" at A2.n.x, A2.n.y - 0.1i ljust
"  intrinsic colors" at A2.n.x, A2.n.y - 0.2i ljust
box "Composite Modeling" "Transformation" with .n at A2.s
A3:
arrow
"  world coordinate system" at A3.n.x, A3.n.y - 0.1i ljust
"  intrinsic colors" at A3.n.x, A3.n.y - 0.2i ljust
box "Modeling" "Clipping" with .n at A3.s
A35:
arrow
"  clipped world coordinate system" at A35.n.x, A35.n.y - 0.1i ljust
"  clipped intrinsic colors" at A35.n.x, A35.n.y - 0.2i ljust
box "View Orientation" "Transformation" with .n at A35.s
A4:
arrow
"  viewing reference coordinate system" at A4.n.x, A4.n.y - 0.1i ljust
"  clipped intrinsic colors" at A4.n.x, A4.n.y - 0.2i ljust
box "Light Source Shading" "Computation" with .n at A4.s
A5:
arrow
"  viewing reference coordinate system" at A5.n.x, A5.n.y - 0.1i ljust
"  shaded colors" at A5.n.x, A5.n.y - 0.2i ljust
box "View Mapping" "Transformation" with .n at A5.s
A6:
arrow
"  normalized projection coordinate system" at A6.n.x, A6.n.y - 0.1i ljust
"  shaded colors" at A6.n.x, A6.n.y - 0.2i ljust
box "Depth-Cueing" "Computation" with .n at A6.s
A7:
arrow
"  normalized projection coordinate system" at A7.n.x, A7.n.y - 0.1i ljust
"  depth-cued colors" at A7.n.x, A7.n.y - 0.2i ljust
box "Clipping" with .n at A7.s
A9:
arrow
"  clipped normalized projection coordinate system" at A9.n.x, A9.n.y - 0.1i ljust
"  clipped colors" at A9.n.x, A9.n.y - 0.2i ljust
box "X Window Coordinate" "Mapping" with .n at A9.s
Aa:
arrow
"  X window coordinate system" at Aa.n.x, Aa.n.y - 0.1i ljust
"  clipped colors" at Aa.n.x, Aa.n.y - 0.2i ljust
box "Device Coordinate" "Mapping" with .n at Aa.s
Ab:
arrow
"  physical device coordinate system" at Ab.n.x, Ab.n.y - 0.1i ljust
"  clipped colors" at Ab.n.x, Ab.n.y - 0.2i ljust
box "Color" "Approximation" with .n at Ab.s
Ac:
arrow
"  physical device coordinate system" at Ac.n.x, Ac.n.y - 0.1i ljust
"  physical device colors" at Ac.n.x, Ac.n.y - 0.2i ljust
move down 0.3i
"Physical Device Coordinates & Physical Device Colors"
.R
.nr PS 10
.ps 10
.PE
.sp 0.25i
.ce
\f6Figure 3.7:  PEX Rendering Pipeline\fP
.KE
.AC "Beyond the Rendering Pipeline" 3
.LP
When data emerges from the
rendering pipeline, raster attributes such as line style and line width
are bound to the resulting
raster primitives.  Following this, the following steps occur:
.AC "Frame Buffer Write" 4
.LP
The computed raster primitives are written into the frame buffer.
It is at this stage that
geometry and color information once more
are treated together.  The device location and the device color
are known at this point, so the proper frame
buffer address can be written with the appropriate color value.
Geometry data is not used beyond this point.
The pixel location to be written is
interpreted by frame buffer memory, and its color value
is stored in the proper memory location in the frame buffer.
.AC "Hardware Colormap" 4
.LP
As data values are
read from the frame buffer memory, they are
submitted to a \fIhardware colormap\fP stage.  This is typically
implemented via a hardware lookup table, but for film recorders
or other devices a software lookup-table may be utilized.
Each value read from the frame buffer is used as input
to the colormap to produce an output value.
If a device does not have the capability to perform this color compensation
stage, the color input equals the color output.
.AC "D/A Conversion" 4
.LP
\fIMapped colors\fP are converted from
digital signals to analog signals and sent to the display system's
monitor for subsequent display.
.bp
.AC "PEX Deferred Functionality" 1
.LP
The following list contains functionality that has been thought about and
discussed, but for one reason or another, put on a list of deferred items.
Some of items in the list represent functionality of uncertain value, some
items require more thought and experimentation before they are included in the
specification, and some were left out for the time being simply to reduce
the initial design/implementation burden.
.nr PD 0.10i
.XP
\fBadvanced input functionality\fP -
Core X has enough input functionality to
accommodate the requirements of most 3D applications.  More study needs to
be given to the issue of providing more sophisticated input functionality
to support 3D graphics applications.
.XP
\fBtraversal scheduling\fP -
In certain environments (such as multiprocessing)
it may become useful to support simultaneous structure
traversal processes with application-defined scheduling.  When this becomes
a real need, we can investigate it further and decide how
best to support it in the PEX interface.
.XP
\fBmultithreaded implementations\fP -
While we have given some thought to avoiding
problems with multithreaded implementations in the current design, we have
not concentrated on designing PEX for a multithreaded environment.
.XP
\fBsystem global clock\fP -
It may be desirable to support the notion of
a system global clock, so that such things as "clocked" traversals
and blinking could be synchronized in all windows simultaneously.
.XP
\fBray tracing\fP -
Ray tracing can be
added in a straightforward fashion as a HLHSR mode at any time.
.XP
\fBconditional structure traversal\fP -
At this
time, the PEX Architecture Team considers the PHIGS+ conditional traversal
functionality to be unstable.  Pending the treatment it receives in
ANSI/ISO meetings, it can be added during the next major revision of PEX.
.XP
\fBbatching of structure edits\fP -
There is currently no way to guarantee
that a collection of structure editing operations will be applied
atomically.  Such "batching" of structure edits might be useful.
.XP
\fBmapping points using renderers\fP -
Only PHIGS workstation resource contain
the capability of mapping points from device coordinates to world coordinates.
It may be useful to add this capability to renderers as well.
.XP
\fBpicking support for client-side traversal\fP -
Pick measures work with PHIGS
workstation resources to provide picking support below the wire.  It might
be useful to have additional support for structure traversal using renderers
and client-side structure traversal.
.XP
\fBgeneral interior style\fP -
The PEX Architecture Team considers
the PHIGS+ general interior style
functionality to be unstable.
Pending the treatment it receives in
ANSI/ISO meetings, it can reconsidered at a later time.
.XP
\fBcolor approximation\fP -
The PEX Architecture Team considers
portions of the PHIGS+ pseudo-color approximation
functionality to be unstable and/or inappropriate for an extension
to the X Window System.  Because of the nature of the X Window
Sytem certain aspects of this problem were treated differently in PEX
than that which was proposed for PHIGS+.
Pending the treatment it receives in
ANSI/ISO meetings, the pseudo-color approximation functionality can
be reconsidered at a later time.
.XP
\fBdouble- or multi-buffering\fP -
There is currently no standard way to
support double-buffering or multi-buffering in the X Window System.
Since this affects X and many of its extensions,
work is being done to address the problem through the X Consortium.
.XP
\fBabort/yield\fP -
There is currently no specific mechanism in PEX or in X
to enable a client to interrupt a lengthy request, such as a structure
traversal operation.  This is a potential problem for X and many
of its extensions,
so work is being done to address the problem through the X Consortium.
.nr PD 0.20i
.bp
.AC "PEX Glossary" 1
.LP
.RU
.LP
This section contains a list of terms and definitions that are used
throughout the PEX document set.  Pertinent terms from core X
have been excerpted from the document \fIX Window System Protocol,
Version 11\fP.
.[
scheifler protocol X Window System
.]
Some definitions have been appropriated from the PHIGS specification.
.[
PHIGS ISO 1987
.]
Additional definitions have been borrowed from the PHIGS+ specification.
.[
PHIGS+ Functional Description, Revision 3.0
.]
An attempt has been made to indicate the origin of each of the terms.
Some of the terms in this list have many meanings in the context
of computer graphics.  This section defines terms only as they are
used in the context of PEX.  The following notation is used to denote
the origins of terms:
.ID
PEX		- Specific or new meaning in PEX that may differ from other common use of the term
X		- From X glossary of terms, PEX usage is consistent with the X usage
PHIGS	- From PHIGS glossary of terms, PEX usage is consistent with the PHIGS usage
PHIGS+	- From PHIGS+ glossary of terms, PEX usage is consistent with the PHIGS+ usage
common	- Common usage, PEX usage is consistent with the commonly-accepted definition
.DE
The list is alphabetized by ignoring case.  Spaces and hyphens
come before any other
characters in the lexical order.
.nr PD 0.05i
.LP
.sp 0.25i
.ps 8
.nr PS 8
.vs 10
.nr VS 10
.XP
\fBabove the wire\fP (X) -
On the client side of the network connection.
.XP
\fBambient light\fP (common) -
An approximation to the illumination caused by the reflection
of light from all objects in the scene.
.XP
\fBambient light source\fP (common) -
An \fIambient light source\fP is
effectively a non-directional light source that affects the
appearance of all surfaces, regardless of their orientation.
.XP
\fBambient reflection\fP (common) -
A reflection from the surface of an object that is due to
ambient light.
.XP
\fBambient reflection coefficient\fP (PHIGS+) -
A pipeline state attribute
that controls the fraction of ambient light reflected from surfaces.
.XP
\fBancestor\fP (PHIGS) -
A structure that references another structure is said to be an \fIancestor\fP
of the called structure.  (This is distinct from the X notion of an
ancestor, which applies to windows in a window hierarchy.)
.XP
\fBannotation text\fP (PHIGS) -
A PEX output primitive that causes text to be drawn such that it is always
parallel to the plane of the display surface.
.XP
\fBannotation text alignment\fP (PHIGS) -
A pipeline state attribute that controls the horizontal and vertical alignment
of annotation text primitives.
.XP
\fBannotation text height\fP (PHIGS) -
A pipeline state attribute that controls the height
of annotation text primitives.
.XP
\fBannotation text path\fP (PHIGS) -
A pipeline state attribute that controls the relative positioning of
successive characters in an annotation text string.
.XP
\fBannotation text style\fP (PHIGS) -
A pipeline state attribute that controls the displayed style
of annotation text primitives.  The value of this attribute is
selected from the list of available styles in the PEX annotation
text style enumerated type.
.XP
\fBannotation text up vector\fP (PHIGS) -
A pipeline state attribute that controls the "up" direction for
annotation text primitives.
.\".XP
.\"\fBantialiasing\fP (common) -
.\"A variety of aliasing artifacts are present in computer-generated
.\"imagery.  These artifacts are the result of a sampling rate (the
.\"discrete pixels on the display) that is insufficient to accurately
.\"represent high frequency components (edges of objects, straight lines,
.\"etc.).  The most common aliasing artifacts are "jaggies", or jagged
.\"edges that appear along edges of objects and lines.  The attempt
.\"to reduce the effects of aliasing is known as antialiasing.
.XP
\fBapplication data\fP (PHIGS) -
A type of output command that contains uninterpreted data from the application.
.XP
\fBapplication program interface (API)\fP (common) -
A specific library with documented calling conventions
to which an application program can be linked in order to create an
executable program.
Standard PHIGS and PHIGS+ language bindings will be used to
provide an application program interface for PEX.
.XP
\fBarea-defining primitive\fP (PHIGS+) -
See \fIsurface\fP.
.XP
\fBaspect source flag (ASF)\fP (PHIGS) -
A flag indicating whether a particular output primitive attribute
is obtained from a lookup table or from the current pipeline state.
Also called \fIattribute source flag\fP.
.XP
\fBattenuation coefficient\fP (PHIGS+) -
An attribute of some types of light sources that indicates how
the intensity of light drops off as a function of distance from the
light source.
.XP
\fBattribute\fP (PEX) -
A particular characteristic of an output primitive, resource, or some
other object in PEX.
An attribute is usually modifiable, either implicitly or explicitly.
.XP
\fBattribute binding\fP (PEX) -
The act of associating an attribute (e.g., an output primitive
attribute) with an entity (e.g., an output primitive) at a
particular point in time.  In the conceptual PEX rendering pipeline,
different attributes get bound at different stages.
.XP
\fBattribute source flag (ASF)\fP (PHIGS) -
See \fIaspect source flag\fP.  (It may be more consistent to use this
term rather than \fIaspect source flag\fP since the term \fIattribute\fP is
used more consistently throughout PEX.)
.XP
\fBB-spline\fP (common) -
A parametric curve or surface that is defined by
knots in parameter space and control points in modeling coordinates.
B-splines may be uniform (equally spaced knots) or non-uniform
(unequally spaced knots).
B-splines may be rational (control points are specified as homogeneous
coordinates) or non-rational (control points are specified as non-homogeneous
coordinates).  The "B" stands for "basis".
.[
spencer thomas
.]
.XP
\fBback face\fP (common) -
A portion of a surface primitive whose geometric normal when
transformed to NPC has a negative Z-axis component.
A separate set of attributes can be applied to back-facing surfaces.
.XP
\fBback face interior style\fP (PHIGS+) -
A pipeline state attribute that controls the interior style
to be used for back-facing surface primitives.
The value of this attribute is
selected from the list of available types in the PEX interior style
enumerated type.
The interior style
to be used for back-facing surface primitives may be obtained
from an interior bundle table instead of directly from the
pipeline state if the 
\fIback face interior style ASF\fP attribute is set to \fIBundled\fP.
.XP
\fBback face interior style index asf\fP (PHIGS+) -
A pipeline state attribute that controls the interior style index
to be used for back-facing surface primitives.
The back face interior style index determines the hatch style to use
if the back face interior style is \fIHatch\fP or the pattern table
entry to use if the back face interior style is \fIPattern\fP.
The interior style index
to be used for back-facing surface primitives may be obtained
from an interior bundle table instead of directly from the
pipeline state if the 
\fIback face interior style ASF\fP attribute is set to \fIBundled\fP.
.XP
\fBback face reflection attributes\fP (PHIGS+) -
A pipeline state attribute that controls the reflection attributes
to be used for back-facing surface primitives.  Reflection attributes
consist of ambient, diffuse, and specular reflection coefficients;
specular color and concentration; and a transmission coefficient.
The reflection attributes
to be used for back-facing surface primitives may be obtained
from an interior bundle table instead of directly from the
pipeline state if the 
\fIback face reflection attributes ASF\fP attribute is set to \fIBundled\fP.
.XP
\fBback face reflection model\fP (PHIGS+) -
A pipeline state attribute that controls the reflection model
to be used for back-facing surface primitives.
The reflection model controls the equation used to model the
reflection of light from an object's surface.
The reflection model
to be used for back-facing surface primitives may be obtained
from an interior bundle table instead of directly from the
pipeline state if the 
\fIback face reflection model ASF\fP attribute is set to \fIBundled\fP.
.XP
\fBback face removal\fP (common) -
See \fIculling\fP.
.XP
\fBback face surface color\fP (PHIGS+) -
A pipeline state attribute that controls the color
to be used for back-facing surface primitives.
The color
to be used for back-facing surface primitives may be obtained
from an interior bundle table instead of directly from the
pipeline state if the 
\fIback face surface color ASF\fP attribute is set to \fIBundled\fP.
.XP
\fBback face surface interpolation\fP (PEX) -
A pipeline state attribute that controls the interpolation method
to be used on the interiors of
back-facing surface primitives.  The interpolation
method defines how light source shading computations are to be carried
out at points between the vertices of a surface.
The surface interpolation method
to be used for back-facing surface primitives may be obtained
from an interior bundle table instead of directly from the
pipeline state if the 
\fIback face surface interpolation ASF\fP attribute is set to \fIBundled\fP.
(Called \fIback face shading\fP in PHIGS+)
.XP
\fBbelow the wire\fP (X) -
On the server side of the network connection.
.XP
\fBblock\fP (PEX) -
A request that is being deferred until the resource it requires
becomes available for use is said to be \fIblocked\fP.
.XP
\fBbundle\fP (PHIGS) -
See \fIlookup table\fP.
.XP
\fBcall structure\fP (common) -
See \fIexecute structure\fP.
.XP
\fBcamera position\fP (common) -
See \fIviewing position\fP.
.XP
\fBcell array\fP (PHIGS) -
A PEX output primitive that is a parallelogram of equal-sized cells, each
of which is a parallelogram and has a single color.
.XP
\fBcharacter expansion\fP (PHIGS) -
A pipeline state
attribute that specifies a horizontal scale factor that is applied
to characters in text primitives
\fIafter\fP they have been scaled to the desired height.
A character expansion value less than 1.0 makes a character narrower
and a character
expansion value greater than 1.0 makes a character wider.
The character expansion factor to be used may be obtained
from a text bundle table instead of directly from the
pipeline state if the 
\fIcharacter expansion ASF\fP attribute is set to \fIBundled\fP.
.XP
\fBcharacter height\fP (PHIGS) -
A pipeline state
attribute that specifies the absolute height of the font's nominal
character in text local coordinates.
.XP
\fBcharacter spacing\fP (PHIGS) -
A pipeline state attribute that specifies the additional spacing between
characters in a text string.
The character spacing is specified as a fraction of the nominal character
height for the font.
The character spacing value to be used may be obtained
from a text bundle table instead of directly from the
pipeline state if the 
\fIcharacter spacing ASF\fP attribute is set to \fIBundled\fP.
.XP
\fBcharacter up vector\fP (PHIGS) -
A pipeline state attribute that specifies the principal "up" direction for
a text string.
.XP
\fBCIE\fP (common) -
The color space in which colors are specified by an (x,y) position
on the CIE chromaticity diagram plus a luminance value.  CIE is an
abbreviation for the Commission Internationale de l'Eclairage.
.XP
\fBclient\fP (X) -
An application program connecting to the X server by some
interprocess communication (IPC) path, such as an ethernet connection or a
shared memory buffer.  More precisely, a client is the IPC path
itself; a program with multiple connections open to the server is viewed as
multiple clients by the protocol.
.XP
\fBclipping\fP (common) -
The process of determining whether a graphics primitive is wholly or
partially outside some defined boundary.
Portions of primitives that are outside the boundary are discarded
and said to be \fIclipped\fP.
In X, clipping refers to the discarding of graphics primitives that
fall outside the boundary of a drawable or are occluded by other
windows.  In PEX, clipping occurs on a 3D volume as part
of the rendering process.
.XP
\fBclipping volume\fP (common) -
The rectangular parallelepiped in normalized projection coordinates
against which all primitives are to be clipped.  The clipping volume
is determined by computing the intersection of the NPC subvolume and the
volume defined by the clipping limits.
.XP
\fBcolor\fP (PEX) -
In the context of the values passed via the PEX protocol, a \fIcolor\fP
is a color type/color value pair.
.XP
\fBcolor approximation\fP (PEX) -
The process of converting rendering pipeline color values into physically
displayable values for a particular device.  Renderers and PHIGS workstations
each have a color approximation attribute that maintains the resource id
of the RGB approximation table or the intensity approximation table to
be used when converting rendering pipeline color values into displayable
color values. (See also \fIRGB approximation table\fP and \fIintensity
approximation table\fP)
.XP
\fBcolor space\fP (common) -
The color coordinate system in which a color value is described.
.XP
\fBcolor table\fP (PEX) -
A type of lookup table resource that maintains a list of direct color
values.  Indexed colors are dereferenced using the color table whose
resource id is maintained as an attribute of a renderer or a
PHIGS workstation resource in order to determine the actual color value to
be used.  This corresponds to the PHIGS "color table" concept.
It is not to be confused with the X colormap resource or the
colormap hardware found in many color display systems.
.XP
\fBcolor type\fP (PEX) -
A flag that indicates whether color values are
specified directly in a particular color space
("direct") or whether color values are
indices that must be deferenced using a color table.
.XP
\fBcolormap\fP (X) -
An X resource that maintains
a set of color values.  A pixel value is used as an index
into a colormap to produce
intensities to be displayed.  On many graphics devices,
a colormap is implemented
in hardware between the frame buffer memory and the digital-to-analog
converters.
.XP
\fBcomposite modeling matrix\fP (PHIGS) -
A pipeline state attribute that contains
the 4 \(mu 4 matrix formed by concatenating the local and global
modeling transformation matrices.  (See also \fImodeling transformation\fP)
.XP
\fBconcentration exponent\fP (PHIGS+)
A measure of the intensity of light emanating from a spotlight
relative to its direction.
.XP
\fBcone of influence\fP (PHIGS+)
A volume that is determined by the position, direction, and spread angle
of a spotlight.  Only primitives inside this cone are influenced
by the light source.
.XP
\fBconnection\fP (X) -
The IPC path between the server and client program.
A client program typically has one
connection to the server over which requests, replies,
and events are sent.
.XP
\fBcoordinate system\fP (common) -
A system of axes and units that is used to define the position
of objects.  PEX and X use several different coordinate systems
in order to define positions and relationships between objects.
.XP
\fBcore X\fP (common) -
A reference to the X Window System itself, excluding any
extensions.
.XP
\fBculling\fP (common) -
The process of removing back-facing surfaces or front-facing
surfaces from the rendering
pipeline.  Culling is controlled by the \fIculling mode\fP
attribute.
.XP
\fBculling mode\fP (PEX) -
A pipeline state attribute that controls
the process of removing back-facing or front-facing surfaces from the rendering
pipeline.  (Called \fIface culling\fP in PHIGS+)
.XP
\fBcurrent HLHSR mode\fP (PHIGS) -
A PHIGS workstation attribute that
defines the HLHSR mode that is currently being used by the PHIGS workstation.
.XP
\fBcurrent NPC subvolume\fP (PEX) -
A PHIGS workstation attribute that
defines the NPC subvolume
that is currently being used by the PHIGS workstation.
(In PHIGS, the NPC subvolume is referred to as the \fIworkstation window\fP)
.XP
\fBcurrent path\fP (PEX) -
A renderer attribute that maintains a list of id/offset pairs for
supporting client-side traversal.
.XP
\fBcurrent viewport\fP (PHIGS) -
A PHIGS workstation attribute that
defines the viewport
that is currently being used by the PHIGS workstation.
.XP
\fBcurve approximation\fP (PHIGS+) -
A pipeline state attribute that
controls the method and the tolerance used to approximate curves.
The value of this attribute is
selected from the list of available methods in the PEX curve approximation
method
enumerated type.
The curve approximation to be used may be obtained
from a line bundle table instead of directly from the
pipeline state if the 
\fIcurve approximation ASF\fP attribute is set to \fIBundled\fP.
.XP
\fBcurve basis\fP (PHIGS+) -
A PEX enumerated type that specifies the basis functions for drawing
parametric polynomial curves.
.XP
\fBcurves\fP (common) -
NURB curves or parametric polynomial curves.
.XP
\fBdefault table entry\fP (PEX) -
Each type of lookup table has a specific index value to denote the
\fIdefault table entry\fP for that type of table.  If references are made
to entries in the table that are undefined, the contents of the default
entry are used instead.
.XP
\fBdefined indices\fP (PEX) -
The indices of all of a lookup table's entries that contain predefined
values or values that have been set by a client.
.XP
\fBdefined views\fP (PEX) -
A PHIGS workstation attribute that contains a list of the views that
have been defined.  Because it must support the concept of "current"
and "requested" views, the PHIGS workstation resource does not have
an explicitly-attached view table, but views are stored in a
built-in view table instead.
.XP
\fBdepth\fP (X) -
The number of bits per pixel in a drawable.
.XP
\fBdepth-cue\fP (common) -
To attenuate the color or intensity of an object based on its distance
from the viewing position.
.XP
\fBdepth-cue index\fP (PHIGS+) -
A pipeline state attribute that selects the entry of the depth-cue
table that is to be used to obtain the attributes for the depth-cueing
stage of the rendering pipeline.
.XP
\fBdepth-cue table\fP (PHIGS+) -
A type of PEX lookup table resource that maintains the attributes
necessary for doing depth-cueing computations.
Renderer resources and PHIGS workstation resources each have an attribute
that maintains the resource id of a depth-cue table.
.XP
\fBdescendant\fP -
A structure that is referenced by
another structure is said to be a \fIdescendant\fP
of the referencing structure.  (This is distinct from the X notion of an
descendant, which applies to windows in a window hierarchy.)
.XP
\fBdevice coordinates (DC)\fP (common) -
The coordinate system used to address locations on a display device.
The device coordinate system is device-dependent, and so can vary from
device to device.
In X, each window and pixmap has its own coordinate system, with the
origin at the upper left-hand corner.  This is called the \fIX window
coordinate system\fP.  A further mapping must be performed in hardware
or software to convert X window coordinates into physical device coordinates
for the display.
For the purposes of PEX, device coordinates
are simply X window coordinates.
.XP
\fBdiffuse reflection\fP (common) -
A reflection from the surface of an object that is spread out
over the surface, and is independent of viewing position.
A dull, matte surface exhibits only diffuse
reflection - light is reflected from the surface (otherwise
you wouldn't be able to see it), but there are no sharp highlights.
.XP
\fBdiffuse reflection coefficient\fP (PHIGS+) -
A pipeline state attribute
that controls the fraction of diffuse light reflected from surfaces.
.XP
\fBdirect color format\fP (PEX) -
A PEX enumerated type that specifies the color
space and the numeric representation that will be used when
passing direct color values.
.XP
\fBdirect color value\fP (PEX) -
A color type/color value pair for which the color type is set (either
implicitly or explicitly) to \fIDirect\fP and the
color value is specified in one of the allowed direct color formats.
Sometimes shortened to \fIdirect color\fP.
Not to be confused with the X \fIDirectColor\fP colormap type.
.XP
\fBDirectColor\fP (X) -
A class of colormap that interprets pixels composed of separate red, green,
and blue components.  These components
are used to separately index into red, green, and blue intensity
arrays to produce red, green, and blue output intensities.  The red, green,
and blue intensity arrays can be modified dynamically by clients.
.XP
\fBdirectional light\fP (PHIGS+) -
See \fIvector light\fP.
.XP
\fBdispatcher\fP (X) -
The portion of the server that communicates with clients and acts upon
any requests that it receives.  The core X dispatcher
passes PEX requests to a PEX dispatcher.
.XP
\fBdisplay screen\fP (X) -
A monitor on which output from an X server can be seen.
.XP
\fBdisplay surface\fP (PHIGS) -
The drawable associated with a PHIGS workstation resource.  The PHIGS
workstation maintains the state of the display surface, knowing at all
times whether or not the display surface is empty, and
whether or not the displayed image is "correct".
.XP
\fBdisplay surface state\fP (PHIGS)
A PHIGS workstation attribute that indicates whether the associated
drawable is \fIEmpty\fP or \fINonEmpty\fP.
.XP
\fBdisplay update\fP (PHIGS) -
A PHIGS workstation attribute that indicates how changes are to be visualized.
The value of this attribute is
selected from the list of available methods in the PEX display update mode
enumerated type.
.XP
\fBdisplay update mode\fP (PHIGS) -
A PEX enumerated type that specifies the mode
that should be used to update the displayed image
when changes are made.
.XP
\fBdisplayed image\fP (PHIGS) -
A PHIGS workstation resource's idea of what is currently displayed on
the associated drawable.
.XP
\fBdistinguish\fP (PEX) -
A pipeline state attribute that indicates whether back-facing surfaces
are to be rendered using the back-face surface attributes or the front-face
surface attributes. (Called \fIface distinguishing mode\fP in PHIGS+)
.XP
\fBdouble-buffering\fP (common) -
The process of using two buffers to achieve smoother
or flicker-free graphics generation.  One buffer is used for
the generation of an image and is never displayed
and the other one is always displayed.  When
the image is complete, the two buffers can be conceptually swapped to display
the generated image.  The process may be repeated to support
smooth animation sequences.  The use of more than two buffers is
called \fImulti-buffering\fP.
.XP
\fBdrawable\fP (X) -
An X window or pixmap.  
Renderers and PHIGS workstation resources each maintain the resource id
of a drawable upon which output primitives are drawn.
.XP
\fBecho switch\fP (PEX) -
An attribute of a pick measure resource
that determines whether or not echoing is to be performed during
the picking operation.
.XP
\fBecho volume\fP (PEX) -
An attribute of a pick measure resource that defines the
region in device coordinates in which prompting/echoing will occur.
.XP
\fBechoing\fP (PHIGS) -
The action that can occur after the selection trigger has occurred
during a picking operation.  Echoing typically involves highlighting
the picked primitive in some visually distinct manner.
.XP
\fBedge\fP (PHIGS) -
See \fIsurface edge\fP.
.XP
\fBedge bundle\fP (PHIGS) -
A type of lookup table resource that maintains a list of surface edge
attribute values.
An edge bundle table resource id is maintained as an
attribute of a renderer and as an attribute of a PHIGS workstation resource.
Depending on the settings of the various surface edge ASF's,
surface edge attributes may be obtained from the edge bundle table rather
than from the renderer's or PHIGS workstation's pipeline state.
.XP
\fBedge bundle index\fP (PHIGS) -
A pipeline state attribute that indicates the entry in the edge bundle
table from which edge attributes may be obtained.
.XP
\fBediting mode\fP (PHIGS) -
An attribute of a structure resource that indicates whether
structure elements are to be created by inserting them at
the element pointer position or whether they will replace
elements at the element pointer position.
.XP
\fBelement\fP (PHIGS) -
See \fIstructure element\fP.
.XP
\fBelement offset\fP (PEX) -
An element's position in the structure resource.  The first element in
the structure has an element offset of one, the second has an offset of
two, and so on.  An element's offset may change as elements are deleted
or inserted in front of it in the structure resource.  (Same as PHIGS
\fIelement position\fP)
.XP
\fBelement pointer\fP (PHIGS) -
An attribute of a structure resource that maintains an offset
into the structure (a pointer to a structure element) where new structure
elements will be either inserted, or replace existing elements, depending on
the editing mode.
.XP
\fBelement position\fP (PHIGS) -
See \fIelement offset\fP.
.XP
\fBelement reference\fP (PHIGS) -
A structure id/element offset pair that is used to uniquely identify
a specific structure element.
.XP
\fBentry\fP (PEX) -
See \fIlookup table entry\fP.
.XP
\fBenumerated type\fP (PEX) -
The set of choices for certain attributes that has a defined numeric
value for each of the standard choices.  Such lists are considered to
be extensible, in that additional choices can be added to the list.
PEX provides a request that allows a client to query the
supported values for all of the enumerated types.
.XP
\fBerror\fP (X) -
Information sent from the server back to the client to inform the client that
a request was malformed or contained erroneous data values.
.XP
\fBevent\fP (X) -
Information provided to the client by the server asynchronously.
Events may be either asynchronously generated from devices, or
generated as side effects of client requests.  Events are grouped into
types; events are never sent to a client by the server unless the
client has specificially asked to be informed of that type of event,
but other clients can force events to be sent to other clients.  Events
are typically reported relative to a window.
.XP
\fBexclusion set\fP (PHIGS) -
A name set resource that maintains a list of names that \fIare not\fP eligible
for a certain operation.
.XP
\fBexecute structure\fP (PHIGS) -
A type of output command that causes a specific structure resource to
be turned into a list of output commands that are submitted to the
rendering pipeline.  Executing a structure involves saving the current
pipeline state, processing all of the structure elements in the
structure, and then restoring the pipeline state.
.XP
\fBexplicit traversal\fP (PHIGS) -
A structure traversal that was caused by an explicit client
request, not by the side effect of some other action.
.XP
\fBextension\fP (X) -
A set of capabilities that has been included in the X server, but is
not part of core X.  PEX is an extension to X; it has a separate protocol,
but PEX requests can be sent over the same wire as X requests.
.XP
\fBeyepoint\fP (common) -
See \fIviewing position\fP.
.XP
\fBeye position\fP (common) -
See \fIviewing position\fP.
.XP
\fBface\fP (common) -
See \fIfacet\fP.
.XP
\fBfacet\fP (common) -
A planar portion of a surface primitive.  Some output primitives
already contain portions that are planar (fill areas, fill areas sets,
triangular strips, quadrilateral meshes, indexed polygons) and others
may be tessellated to produce a list of facets (parametric surfaces,
non-uniform B-spline surfaces).
.XP
\fBfacet color\fP (PHIGS+) -
The color associated with a facet.  Some output primitives allow
facet colors to be passed along with the geometry information.
.XP
\fBfacet normal\fP (PHIGS+) -
The geometric normal associated with a facet.  Some output primitives allow
facet normals to be passed along with the other geometry information.
Facet normals are used in culling and light-source shading operations.
If not provided explicitly, a facet normal may be computed using the vertices
of the facet.
.XP
\fBfill area\fP (PHIGS) -
A PEX output primitive consisting of
a series of vertices that, when connected, define the boundary
of a planar surface.  The last vertex in the list is always
implicitly connected to the first vertex in the list.  Commonly
referred to as a polygon.
.XP
\fBfill area set\fP (PHIGS) -
A PEX output primitive consisting of a list of fill areas which define
contours (islands and/or holes) for a single planar primitive.
Commonly referred to as a "polygon with holes".
.XP
\fBfloating point format\fP (PEX) -
A PEX enumerated type that specifies
the format in which floating point values will be transferred between
the client and the server.
.XP
\fBfont\fP (PEX) -
An array of glyphs (typically characters).  See \fIPEX font\fP and
\fIX font\fP.
.XP
\fBfront face\fP (common) -
A portion of a surface primitive whose geometric normal when
transformed to NPC has a non-negative Z-axis component.
.XP
\fBGDP\fP (PHIGS) -
See \fIgeneralized drawing primitive\fP.
.XP
\fBgeneralized drawing primitive (GDP)\fP (PHIGS)
A PEX output primitive containing data that is to be interpreted
in an implementation-dependent fashion.  GDP's can be used to draw
output primitives other than the ones defined in the standard.\(dg
.XP
\fBgeneralized structure element (GSE)\fP (PHIGS) -
An output command containing data that may be used to access
implementation-dependent pipeline state.  GSE's do not cause
any type of output primitive to be drawn.
GSE's can be used to set attributes
other than the ones defined in the standard.\(dg
.FS
\(dg Due to the potential differences in floating point formats
between clients and servers, and because formats of GDP's and GSE's
are not explicitly defined, it is not expected that implementations
will use this mechanism to add capabilities to PEX.
.FE
.XP
\fBgeometric normal\fP (common) -
A vector that is perpendicular to a surface at a particular point.
.XP
\fBGKS\fP (common) -
Graphical Kernel System.  GKS is an ISO standard 2D graphics
interface.
.XP
\fBGKS-3D\fP (common) -
An ISO-proposed standard to extend GKS to support 3D graphics.
.XP
\fBglobal modeling matrix\fP (PHIGS) -
A pipeline state attribute that contains
the 4 \(mu 4 matrix used to represent the global modeling
transformation.  The local and global modeling matrices
are concatenated to form the composite modeling matrix.
.XP
\fBglyph\fP (common) -
The definition of a simple image, typically of a character or symbol.
.XP
\fBGrayScale\fP (X) -
A class of colormap that interprets pixels composed of a single index
value, which can be used as a single index into an array containing red,
green, and blue intensity values.  For any given index, the red, green,
and blue values will be equal, thus producing shades of gray.
The contents of the
red, green, and blue intensity arrays
may be altered dynamically by clients.
.XP
\fBGSE\fP (PHIGS) -
See \fIgeneralized structure element\fP.
.XP
\fBhalfspace\fP (common) -
A clipping plane divides a three-dimensional coordinate space into two
portions called \fIhalfspaces\fP.
.XP
\fBhatch\fP (PHIGS) -
To render a surface by filling the interior with a pattern of parallel
and/or crossing hatch lines.
.XP
\fBhatch style\fP (PHIGS) -
A PEX enumerated type that specifies
the hatching pattern to use when rendering surface primitives
.XP
\fBhidden surface removal\fP (common) -
See \fIHLHSR\fP.
.XP
\fBhighlight exclusion\fP (PHIGS) -
A renderer and PHIGS workstation attribute that maintains the resource
id of a name set that is to be used to identify
a list of names that \fIare not\fP eligible
for highlighting.
.XP
\fBhighlight inclusion\fP (PHIGS) -
A renderer and PHIGS workstation attribute that maintains the resource
id of a name set that is to be used to identify
a list of names that \fIare\fP eligible
for highlighting.
.XP
\fBhighlighting\fP (PHIGS) -
Modifying the visual appearance of an output primitive in an
implementation-dependent way such that it is visually distinguishable
from other primitives.
.XP
\fBhit-testing\fP (PHIGS)
See \fIpicking\fP.
.XP
\fBHLHSR\fP (common) -
Hidden-line/hidden-surface removal.  Various techniques exist
to try and eliminate surfaces or objects in a 3D image that
are obscured by other objects from the point of view.  This
process is called hidden-surface elimination, hidden-surface
removal, and visible surface computation.  Hidden-line removal
refers to the generation of a wireframe drawing with all
vectors representing obscured surfaces removed.
.XP
\fBHLHSR buffer\fP (PEX) -
That part of a renderer that stores primitives or depth values in
order to perform hidden surface removal computations.  A renderer's
HLHSR buffer is not directly accessible by clients.
.XP
\fBHLHSR identifier\fP (PHIGS) -
A pipeline state attribute that affects the HLHSR stage of the
rendering pipeline in an implementation-dependent manner.
.XP
\fBHLHSR update state\fP (PHIGS) -
A PHIGS workstation attribute that is set to \fIPending\fP or \fINotPending\fP
depending on whether there have been requests to change the PHIGS
workstation's HLHSR mode that have not yet been made current.
.XP
\fBHLHSR mode\fP (PEX) -
A PEX enumerated type that specifies
how hidden line/hidden surface removal operations are to be performed.
Renderer resources and PHIGS workstation resources each have an attribute
that is used to maintain the current HLHSR mode.
.XP
\fBHLS\fP (common) -
The color space in which colors are identified by hue, lightness,
and saturation.
.XP
\fBhomogeneous coordinate\fP (common) -
Three-dimensional coordinates that are represented in 4-space with the
addition of one more coordinate and a scale factor are called
\fIhomogeneous\fP coordinates.  The homogeneous representation of
the three-dimensional point [\fI\ x\ y\ z\fP] is [\fIwx\ wy\ wz\ w\fP], where
\fIw\fP is any nonzero scale factor.
.XP
\fBHSV\fP (common) -
The color space in which colors are identified by hue, saturation,
and value.
.XP
\fBid (X)\fP -
See \fIidentifier\fP.
.XP
\fBidentifier\fP (X) -
A unique value (identifier) associated with a resource
that clients use to refer to that resource.
Identifiers are unique to a client and a specific resource and its type.
Also referred to as a \fIresource id\fP.
.XP
\fBillumination computation\fP (common) -
See \fIlight source shading computation\fP.
.XP
\fBimmediate mode\fP (common) -
A client that passes output commands directly to a renderer for
immediate processing it is said to be operating in \fIimmediate mode\fP.
.XP
\fBimplementation-dependent\fP (common) -
The exact method, style, or value is not defined as part of the standard,
but left to the discretion of the implementor of the standard.
.XP
\fBimplicit regeneration\fP (PHIGS) -
See \fIimplicit traversal\fP.
.XP
\fBimplicit traversal\fP (PHIGS) -
A complete retraversal of a PHIGS workstation's posted structure networks
that wasn't explicitly requested by a client, but occurred due to
the setting of the display update mode and changes that occurred.
The purpose of an implicit traversal is to make the displayed image
"correct".
.XP
\fBinclusion set\fP (PHIGS) -
A name set resource that maintains a list of names that \fIare\fP eligible
for a certain operation.
.XP
\fBindexed color value\fP (PEX) -
A color type/color value pair for which the color type is set (either
implicitly or explicitly) to \fIIndexed\fP and the
color value is specified as
an index into an color lookup table.
Sometimes shortened to \fIindexed color\fP.
.XP
\fBindexed polygons\fP (PEX) -
A PEX output primitive that defines a collection of polygons by
a list of shared vertices and a
list of vertex connection information.
(Called \fIpolyhedron\fP in PHIGS+)
.XP
\fBintensity approximation\fP (PEX) -
The conversion of colors from the pipeline's internal color space
into monochrome intensity values.
.XP
\fBintensity approximation table\fP (PEX) -
A type of lookup table resource that maintains information necessary for
performing intensity approximation.
.XP
\fBinterior bundle\fP (PHIGS) -
A type of lookup table resource that maintains a list of surface
attribute values.
An interior bundle table resource id is maintained as an
attribute of a renderer and as an attribute of a PHIGS workstation resource.
Depending on the settings of the various surface attribute ASF's,
surface attributes may be obtained from the interior bundle table rather
than from the renderer's or PHIGS workstation's pipeline state.
.XP
\fBinterior bundle index\fP (PHIGS) -
A pipeline state attribute that indicates the entry in the interior bundle
table from which surface attributes may be obtained.
.XP
\fBinterior style\fP (PHIGS)
A pipeline state attribute that controls the interior style
to be used for front-facing surface primitives (and for back-facing surface
primitives if \fIdistinguish\fP is \fIFalse\fP).
The value of this attribute is
selected from the list of available types in the PEX interior style
enumerated type.
The interior style
to be used may be obtained
from an interior bundle table instead of directly from the
pipeline state if the
\fIinterior style ASF\fP attribute is set to \fIBundled\fP.
.XP
\fBinterior style index\fP (PHIGS) -
A pipeline state attribute that controls the interior style index
to be used for front-facing surface primitives (and for back-facing surface
primitives if \fIdistinguish\fP is \fIFalse\fP).
The interior style index determines the hatch style to use
if the interior style is \fIHatch\fP or the pattern table
entry to use if the interior style is \fIPattern\fP.
The interior style index
to be used may be obtained
from an interior bundle table instead of directly from the
pipeline state if the
\fIinterior style ASF\fP attribute is set to \fIBundled\fP.
.XP
\fBintrinsic color\fP (common) -
The inherent or fundamental color of an output primitive.  An object's
intrinsic color is transformed into a displayed color by going through
a series of color transformations, including the light source shading
computation, depth-cueing, and color approximation.
.XP
\fBinvisibility exclusion\fP (PHIGS) -
A renderer and PHIGS workstation attribute that maintains the resource
id of a name set that is to be used to identify
a list of names that \fIare not\fP eligible
for invisibility.
.XP
\fBinvisibility inclusion\fP (PHIGS) -
A renderer and PHIGS workstation attribute that maintains the resource
id of a name set that is to be used to identify
a list of names that \fIare\fP eligible
for invisibility.
.XP
\fBIPC\fP (common) -
Interprocess communication.
.XP
\fBisotropic mapping\fP (common) -
a transformation which preserves aspect ratio.
.XP
\fBknot vector\fP (common) -
A non-decreasing sequence of parameter values at which the polynomial
pieces of a parametric curve or surface join.
.XP
\fBlabel\fP (PHIGS) -
An output command containing an identifier that can be used as a
placeholder in structure resources in order to facilitate structure editing.
.XP
\fBlight source\fP (common) -
A mathematical approximation to an object that is capable of emitting light.
In PEX, light source definitions are stored in a light table.  Different
types of light sources that are possible in PEX include ambient, point,
vector, and spotlight sources.
.XP
\fBlight source shading computation\fP (PEX) -
The part of the shading process that involves computing the amount of
light reflected from/transmitted through a surface due to lighting
effects from all enabled light sources.  (Called \fIlighting\fP in PHIGS+)
.XP
\fBlight state\fP (PHIGS+) -
A pipeline state attribute that controls whether lights in
the associated light table are enabled or disabled.
.XP
\fBlight table\fP (PHIGS+) -
A type of lookup table resource that maintains a list of light sources.
A light table resource id is maintained as an
attribute of a renderer and as an attribute of a PHIGS workstation resource.
.XP
\fBlight type\fP (PHIGS+) -
A PEX enumerated type that specifies
the characteristics of a light source.
.XP
\fBlighting\fP (PHIGS+) -
See \fIlight source shading computation\fP.
.XP
\fBline bundle\fP (PHIGS) -
A type of lookup table resource that maintains a list of line and curve
attribute values.
A line bundle table resource id is maintained as an
attribute of a renderer and as an attribute of a PHIGS workstation resource.
Depending on the settings of the various line and curve ASF's,
line and curve attributes may be obtained from the line bundle table rather
than from the renderer's or PHIGS workstation's pipeline state.
.XP
\fBline bundle index\fP (PHIGS) -
A pipeline state attribute that indicates the entry in the line bundle
table from which line and curve attributes may be obtained.
.XP
\fBline color\fP (PHIGS) -
A pipeline state attribute that defines
the color to be used when rendering line and curve primitives.
The line color to be used may be obtained
from a line bundle table instead of directly from the
pipeline state if the
\fIline color ASF\fP attribute is set to \fIBundled\fP.
.XP
\fBline type\fP (PHIGS) -
A pipeline state attribute that
specifies the visual style to be used when drawing line and curve primitives.
Line types include dashed, dotted, solid, etc.
The value of this attribute is
selected from the list of available types in the PEX line type
enumerated type.
The line type to be used may be obtained
from a line bundle table instead of directly from the
pipeline state if the
\fIline type ASF\fP attribute is set to \fIBundled\fP.
.XP
\fBline width\fP (PHIGS) -
A pipeline state attribute that
specifies a multiplier for the implementation-dependent nominal width
of line and curve primitives.
A line width of 1.0 results in a line that is the same width as the nominal
width line for the device.
The line width to be used may be obtained
from a line bundle table instead of directly from the
pipeline state if the
\fIline width ASF\fP attribute is set to \fIBundled\fP.
.XP
\fBlocal modeling matrix\fP (PHIGS) -
A pipeline state attribute that contains the
4 \(mu 4 matrix used to represent the local modeling
transformation.
The local and global modeling matrices
are concatenated to form the composite modeling matrix.
.XP
\fBlock\fP (PEX) -
A resource that is in use by a request is said to be \fIlocked\fP, since
no other request can modify the resource while it is in use.
.XP
\fBlookup table\fP (PEX) -
A PEX resource which allows clients to create arrays of different types
of data structures.  The types of data structures that have
been defined for PEX allow for the efficient support of the PHIGS
concepts of bundles and tables.  Examples of tables include color tables,
line bundle tables, view tables, and light tables.
.XP
\fBlookup table entry\fP (PEX) -
A single data structure or record that is stored in a lookup table is
called a \fIlookup table entry\fP, or just an \fIentry\fP.
.XP
\fBmarker\fP (PHIGS) -
A PEX output primitive consisting of
a two-dimensional glyph, or symbol, that represents a location
in three-dimensional space.  The position of the marker is computed
by the rendering pipeline and the marker glyph is placed at the computed
screen position.
.XP
\fBmarker bundle\fP (PHIGS) -
A type of lookup table resource that maintains a list of marker
attribute values.
A marker bundle table resource id is maintained as an
attribute of a renderer and as an attribute of a PHIGS workstation resource.
Depending on the settings of the various marker ASF's,
marker attributes may be obtained from the marker rather
than from the renderer's or PHIGS workstation's pipeline state.
.XP
\fBmarker bundle index\fP (PHIGS) -
A pipeline state attribute that indicates the entry in the marker bundle
table from which marker attributes may be obtained.
.XP
\fBmarker color\fP (PHIGS) -
A pipeline state attribute that defines
the color to be used when rendering marker primitives.
The marker color to be used may be obtained
from a marker bundle table instead of directly from the
pipeline state if the
\fImarker color ASF\fP attribute is set to \fIBundled\fP.
.XP
\fBmarker type\fP (PHIGS) -
A pipeline state attribute that defines
the symbol that is used to display marker primitives.
The value of this attribute is
selected from the list of available types in the PEX marker type
enumerated type.
The marker type to be used may be obtained
from a marker bundle table instead of directly from the
pipeline state if the
\fImarker type ASF\fP attribute is set to \fIBundled\fP.
.XP
\fBmarker scale factor\fP (PHIGS) -
A pipeline state attribute that
specifies a multiplier for the implementation-dependent nominal marker size.
A marker scale factor of 1.0 results in a marker
that is the same size as the nominal marker size for the device.
The marker scale factor to be used may be obtained
from a marker bundle table instead of directly from the
pipeline state if the
\fImarker scale ASF\fP attribute is set to \fIBundled\fP.
.XP
\fBmodel clip\fP (PHIGS) -
A pipeline state attribute that
indicates whether modeling clipping is enabled or disabled.
.XP
\fBmodel clip operator\fP (PHIGS) -
A PEX enumerated type that specifies
the operator used to combine a list of halfspace with the current
modeling clipping volume in order to define a new modeling clipping volume.
.XP
\fBmodel clip volume\fP (PHIGS) -
A pipeline state attribute that
contains the current modeling clipping volume against
which all primitives are clipped.  The modeling clipping
volume is computed by combining halfspaces defined in modeling coordinates.
.XP
\fBmodeling clipping\fP (PHIGS) -
The process of clipping primitives against the currently-defined
modeling clipping volume.  Modeling clipping is performed prior
to viewing and projection in order to allow effects such as cross-sectioning
of objects.
.XP
\fBmodeling coordinate system\fP (common) -
A device-independent coordinate system used by the application
program to define, construct, and manipulate objects.  This
coordinate system is typically chosen by the application for
convenience in defining an object.  Sometimes referred to as
MCS.
The space defined by this coordinate system is referred to as model space
or object space.
.XP
\fBmodeling transformation\fP (common) -
The process of converting geometry in the modeling coordinate
system into geometry in the world coordinate system.
.XP
\fBmulti-buffering\fP (common) -
An extension of the concept of double-buffering that uses more than
two buffers.
.XP
\fBname\fP (PHIGS) -
An identifier that can be stored in a name set in order to define
primitives that are eligible/ineligible for operations such as
searching and highlighting.
.XP
\fBname set\fP (PEX) -
A PEX resource that maintains a list of names that can be used to
determine the eligibility of primitives
for various operations such as highlighting, invisibility, searching,
and picking.  Pipeline contexts, renderers, search contexts, pick measures
and PHIGS workstations all have one or more attributes that are name set
resource id's.
.XP
\fBnominal value\fP (common) -
The implementation-dependent base value for such attributes as line width
and marker size.
.XP
\fBnon-homogeneous coordinate\fP (common) -
A point in 3-space.  A homogeneous point [\fIx\ y\ z\ w\fP] has a
three-dimensional (non-homogeneous) image [\fIx/w\ y/w\ z/w\fP].
.XP
\fBnon-rational\fP (common) -
In the context of spline curves or surfaces, \fInon-rational\fP splines
are defined by basis functions, each of which is a polynomial (as opposed
to \fIrational\fP splines whose basis functions are expressed as the
ratio of two polynomials.)
Control points for non-rational splines are specified as non-homogeneous
coordinates.
.XP
\fBnon-uniform B-spline curve\fP (common) -
A PEX output primitive that is
a B-spline curve specified with a list of unevenly-spaced knot values
and a set of control points.
Non-uniform B-spline curves can be either rational or non-rational.
.XP
\fBnon-uniform B-spline surface\fP (common) -
A PEX output primitive that is
a B-spline surface specified with two lists of unevenly-spaced knot values
and a set of control points.
Non-uniform B-spline surfaces can be either rational or non-rational.
.XP
\fBnormalflip\fP (PEX) -
A pipeline state attribute that controls whether or not normals
for back-facing surfaces should be inverted prior to performing the
light source shading computations.
.XP
\fBnormalized projection coordinate system (NPC)\fP (PHIGS) -
The device-independent three-dimensional Cartesian coordinate
system in which the composition of images is specified to the
graphics system.
The space defined by this coordinate system is referred to as NPC space.
.XP
\fBNPC subvolume\fP (PEX) -
The volume in the normalized projection coordinate system that
is to be mapped into the viewport.
In PHIGS, this is referred to as the \fIworkstation window\fP.
Renderers and PHIGS workstation resources each have an attribute
that maintains the current NPC subvolume.
.XP
\fBNURB curve\fP (common) -
See \fInon-uniform B-spline curve\fP.
.XP
\fBNURB surface\fP (common) -
See \fInon-uniform B-spline surface\fP.
.XP
\fBNURBS\fP (common) -
Non-uniform rational B-spline curves and surfaces.
.XP
\fBoutput command\fP (PEX) -
Output primitive commands and commands that modify pipeline state
(output primitive attributes and pipeline attributes) are collectively
referred to as \fIoutput commands\fP.
When stored in structure resources,
output commands are called structure elements.
.XP
\fBoutput primitives\fP (PHIGS) -
Commands that cause geometric primitives to be transformed
into raster data.
.XP
\fBoutput primitive attributes\fP (PEX) -
Commands that set attributes
that affect the
appearance of only one type of output primitive (e.g., set marker color,
set line width, etc.).
.XP
\fBpainter's algorithm\fP (common) -
A hidden surface method that involves sorting all primitives based on
their z-values and then rendering them in order from furthest to closest.
The result is that primitives further away will be "painted over" by
primtives that are closer.
.XP
\fBparameter space\fP (common) -
The coordinate space in which knot values are specified for NURB curves
and surfaces.
.XP
\fBparametric curve\fP (common) -
A parametric polynomial curve or a non-uniform B-spline curve.
.XP
\fBparametric polynomial curve\fP (common) -
A PEX output primitive that is
a spline curve defined by a set of control points and a curve basis type.
.XP
\fBparametric polynomial surface\fP (common) -
A PEX output primitive that is
a spline surface defined by a set of control points and a surface basis type.
.XP
\fBparametric surface\fP (common) -
A parametric polynomial surface or a non-uniform B-spline surface.
.XP
\fBpattern\fP (PHIGS) -
A two-dimensional array of colors that is used to render surface
interiors when interior style is set to \fIPattern\fP.
Patterns are stored in pattern tables and referenced via the
interior style index when the interior style is set to \fIPattern\fP.
.XP
\fBpattern reference point\fP (PHIGS) -
A pipeline state attribute that defines the reference point
to be used when drawing patterns on surface primitives.
.XP
\fBpattern reference vector 1\fP (PHIGS) -
A pipeline state attribute that defines one of two reference vectors
to be used when drawing patterns on surface primitives.
.XP
\fBpattern reference vector 2\fP (PHIGS) -
A pipeline state attribute that defines one of two reference vectors
to be used when drawing patterns on surface primitives.
.XP
\fBpattern size\fP (PHIGS) -
A pipeline state attribute that defines the pattern
width vector and pattern height vector
to be used when drawing patterns on surface primitives.
.XP
\fBpattern table\fP (PHIGS) -
A type of lookup table resource that maintains a list of patterns.
A pattern table resource id is maintained as an
attribute of a renderer and as an attribute of a PHIGS workstation resource.
If the interior style attribute is set to \fIPattern\fP, surface
primitives will be drawn using patterns from the associated pattern table.
.XP
\fBpending\fP (PHIGS) -
Changes to a PHIGS workstation's list of defined views, NPC subvolume,
viewport, or HLHSR mode that have been requested but have not yet been
made current are said to be \fIpending\fP.
.XP
\fBPEX encoding document\fP (PEX) -
The document that defines the actual numeric values and numbers of
bytes that are sent over the wire in PEX requests, replies, and errors.
.XP
\fBPEX font\fP (PEX) -
A PEX resource that contains a list of character glyphs.  Unlike X fonts,
PEX fonts are guaranteed to have character glyphs that can be arbitrarily
scaled and rotated.
.XP
\fBPEX protocol\fP (PEX) -
The document that defines the communication between clients and
the PEX extension to the core X server.
.XP
\fBPHIGS+\fP (common) -
A proposal to extend PHIGS to include support for light source
shading, depth-cueing, and additional output primitives such
as parametric curves and surfaces.
.XP
\fBPHIGS\fP (common) -
Programmer's Hierarchical Interactive Graphics System.  PHIGS
is a draft ANSI standard graphics interface and a draft ISO standard.
.XP
\fBPHIGS workstation\fP (PEX) -
A PEX resource which combines other resources into a single entity  that
behaves in a manner similar to the PHIGS abstraction of a "workstation".  A
PHIGS workstation resource has the capabilities of a renderer resource
built into it.
.XP
\fBphysical device coordinate transformation\fP (PEX) -
The process of transforming X window coordinates into
the physical display system memory addresses.
.XP
\fBpick data record\fP (PHIGS) -
A pick-device-dependent data structure that is
used to initialize a pick measure resource when it is created.
.XP
\fBpick device\fP (PHIGS) -
A pointing device or a mechanism that can be
used to select a displayed output primitive.
.XP
\fBpick device descriptor\fP (PEX) -
A description of the initial
values for each supported pick device.
A list of pick device descriptors (one for each supported pick device)
is an attribute of each
PHIGS workstation resource.
.XP
\fBpick device type\fP (PHIGS) -
A PEX enumerated type that defines
the possible types of pick devices.
.XP
\fBpick element reference\fP (PEX) -
A reference to a particular structure element for the purposes of picking.
Each pick element reference contains a structure id, element offset, and
pick id.
.XP
\fBpick exclusion\fP (PHIGS) -
An attribute of a pick measure resource that maintains the resource id
of a name set used to identify a list of names that \fIare not\fP eligible
for picking.
.XP
\fBpick id\fP (PHIGS) -
A pipeline state attribute that maintains
a client-defined value
that can be used to identify primitives selected during picking operations.
(Called \fIpick identifier\fP in PHIGS)
.XP
\fBpick inclusion\fP (PHIGS) -
An attribute of a pick measure resource that maintains the resource id
of a name set used to identify a list of names that \fIare\fP eligible
for picking.
.XP
\fBpick measure\fP (PEX) -
A PEX resource
that "measures" output primitives
to determine which ones satisfy a specific set of selection criteria.
.XP
\fBpick path\fP (PHIGS) -
An attribute of a pick measure resource
that contains a list of pick element references.
The pick path is copied from the pick device descriptor to the
pick measure resource when the pick measure is created and
is used as the initial starting position
for the picking operation.
.XP
\fBpick path order\fP (PHIGS) -
An attribute of a pick measure resource that indicates whether 
pick element references in the
pick path appear in order from the structure network
root down to the picked primitive (\fITopFirst\fP) or from the picked
primitive back up to the structure network root (\fIBottomFirst\fP).
.XP
\fBpick status\fP (PHIGS) -
An attribute of a pick measure resource that indicates whether the last picking
operation was successful or whether no primitives were picked during
the picking operation.
.XP
\fBpicked primitive\fP (PHIGS) -
An attribute of a pick measure resource that contains a list of pick element
references that uniquely identify the structure element selected during
the previous picking operation.
.XP
\fBpicking\fP (PHIGS) -
The act of using some type of pointing device or mechanism to obtain
the structure network location of a displayed output primitive.
.XP
\fBpipeline\fP (common) -
See \fIrendering pipeline\fP.
.XP
\fBpipeline attributes\fP (common) -
Commands that modify the effect of the rendering
pipeline without regard to the types of primitives that are being
rendered (e.g., view index, depth-cue index, global modeling matrix,
light state, etc.).
.XP
\fBpipeline context\fP (PEX) -
A PEX resource that contains the attributes used to define a renderer's
initial pipeline state.  A pipeline context resource id is one of the
attributes of a renderer resource.
.XP
\fBpipeline state\fP (PEX) -
The collection of attributes that define a rendering pipeline at
a specific point in time.  Pipeline state consists of the combination
of the output primitive attributes and the pipeline attributes.
The entire set of pipeline state is saved when structures are executed
and restored when control is returned.
Both renderers and PHIGS Workstation resources have pipeline state.
.XP
\fBpixmap\fP (X) -
A three-dimensional array of bits.  A pixmap is normally
thought of as a two dimensional array of pixels, where each pixel can
be a value from 0 to (2^N)-1, where N is the depth of the
pixmap.  A pixmap can also be thought of as a stack of N bitmaps.
.XP
\fBpolygon\fP (common) -
See \fIfill area\fP and \fIfill area set\fP.
.XP
\fBpoint light\fP (common) -
A type of light source that is defined with a position, two attenuation
coefficients, and a color.  (Called \fIpositional light source\fP in PHIGS+)
.XP
\fBpointer\fP (X) -
The pointing device that is "attached" to the cursor and tracked on the
display screen.
.XP
\fBpointing device\fP (X) -
A device with effective dimensional motion such as a mouse, trackball,
tablet, or joystick.
.XP
\fBpolyline\fP (common) -
A PEX output primitive that consists of
a series of vertices that are connected via line segments.
The last vertex in the list is \fInot\fP connected to the first
vertex in the list.
.XP
\fBpolyline interpolation method\fP (PEX) -
A pipeline state attribute that defines how the values
between the vertices of a polyline segment are to be computed.
The value of this attribute is
selected from the list of available types in the PEX polyline interpolation
enumerated type.
(Called \fIpolyline shading method\fP in PHIGS+)
.XP
\fBpolyline set\fP (PHIGS+) -
A PEX output primitive that consists of one or more polylines.
.XP
\fBpositional light\fP (PHIGS+) -
See \fIpoint light source\fP.
.XP
\fBpost\fP (PHIGS) -
To identify a structure network for display on a particular PHIGS workstation
resource.
.XP
\fBpostconcatenation\fP (PHIGS) -
Transformations that are to be applied to a point \fIafter\fP all of the
transformations in the current composite transformation matrix have been
applied are said to be \fIpostconcatenated\fP to the current composite
transformation matrix.
.XP
\fBposted structure network\fP (PHIGS) -
A structure network that will be
traversed if the PHIGS workstation performs an explicit or
implicit traversal.
.XP
\fBpreconcatenation\fP (PHIGS) -
Transformations that are to be applied to a point \fIbefore\fP any of the
transformations in the current composite transformation matrix have been
applied are said to be \fIpreconcatenated\fP to the current composite
transformation matrix.
.XP
\fBpredefined table entry\fP (PHIGS) -
A lookup table entry that is defined by the PEX server extension
as soon as the lookup table is created.
.XP
\fBprompt/echo type\fP (PHIGS) -
An attribute of a pick measure resource that defines
the visual effects of a picking operation.  Prompting includes the
visual effects that occur before the selection trigger is pressed,
and echoing includes the visual feedback that occurs after a selection
has been made.
The value of this attribute is
selected from the list of available types in the PEX prompt/echo type
enumerated type.
.XP
\fBprompting\fP (PHIGS) -
The visual effects that occur before the selection trigger is pressed
during a picking operation.
.XP
\fBPseudoColor\fP (X) -
A class of colormap that interprets pixels composed of an index
value, which can be used as a single index into an array containing red,
green, and blue intensity values to produce output red, green, and blue
values.  The contents of the
red, green, and blue intensity arrays
may be altered dynamically by clients.
.XP
\fBprojection transformation\fP (common) -
See \fIview mapping transformation\fP.
.XP
\fBprotocol\fP (X) -
The definition of the communication between a client and server.
The X protocol defines the client-server communication
for core X, and
the PEX protocol defines the client-server communication
for the PEX extension.
.XP
\fBquadrilateral mesh\fP (PHIGS+) -
A PEX output primitive
that consists of a two-dimensional array of points that define
a surface composed of adjacent quadrilaterals.
.XP
\fBquery\fP (PEX) -
A request that is issued in order to obtain an attribute value or some
other piece of information from the server is called a \fIquery\fP.
.XP
\fBrational\fP (common) -
In the context of spline curves or surfaces, \fIrational\fP splines
are defined by basis functions, each of which is the ratio of two polynomials.
Control points for rational splines are specified as homogeneous
coordinates.
.XP
\fBreflection\fP (common) -
The mathematical approximation of the amount of light that bounces off
the surface of an object, allowing it to be seen.
.XP
\fBreflection attributes\fP (PHIGS+) -
A pipeline state attribute that controls the reflection attributes
to be used for front-facing surface primitives (and for back-facing surface
primitives if \fIdistinguish\fP is \fIFalse\fP).
Reflection attributes
consist of ambient, diffuse, and specular reflection coefficients;
specular color and concentration; and a transmission coefficient.
The reflection attributes
to be used for surface primitives may be obtained
from an interior bundle table instead of directly from the
pipeline state if the 
\fIreflection attributes ASF\fP attribute is set to \fIBundled\fP.
(Called \fIarea properties\fP in PHIGS+)
.XP
\fBreflection model\fP (PEX) -
A pipeline state attribute that defines
the method that is to be used to perform the light-source shading
computation.  Different sets of equations (reflection models) can
be used to approximate matte surfaces, plastic
surfaces, or metallic surfaces.
The value of this attribute is
selected from the list of available types in the PEX reflection model
enumerated type.
The reflection model
to be used for surface primitives may be obtained
from an interior bundle table instead of directly from the
pipeline state if the 
\fIreflection model ASF\fP attribute is set to \fIBundled\fP.
(Called \fIreflectance equation\fP in PHIGS+)
.XP
\fBrenderer\fP (PEX) -
A PEX resource that contains all of the state necessary to perform rendering.
(A PHIGS workstation resource contains all of the capability of a renderer.)
.XP
\fBrenderer state\fP (PEX) -
A renderer attribute that indicates whether the renderer is currently
\fIIdle\fP or \fIRendering\fP.
.XP
\fBrendering\fP (common) -
The process of converting output primitive commands and
colors into displayable colors and pixel locations.  Core X is
capable of rendering certain 2D output primitives.  The PEX
extension is capable of rendering certain 3D output primtives.  In
the PEX documents, "rendering" refers exclusively to the 3D
rendering operation.
.XP
\fBrendering pipeline\fP (common) -
An abstraction that is used to define the individual steps
that occur during rendering as well as the order in which
these steps occur.  (Not to be confused with a pipeline context resource)
.XP
\fBrendering pipeline state\fP (PEX) -
See \fIpipeline state\fP.
.XP
\fBreply\fP (X) -
Information requested by a client that is sent from the
server.
.XP
\fBrequest\fP (X) -
A command directed to the server by a client.
.XP
\fBrequested HLHSR mode\fP (PHIGS) -
An attribute of a PHIGS workstation that defines the
change to the HLHSR mode
that has been requested but not yet made current.
.XP
\fBrequested NPC subvolume\fP (PHIGS) -
An attribute of a PHIGS workstation that defines the
change to the NPC subvolume
that has been requested but not yet made current.
.XP
\fBrequested viewport\fP (PHIGS) -
An attribute of a PHIGS workstation that defines the
change to the viewport
that has been requested but not yet made current.
.XP
\fBresource\fP (X) -
X windows, pixmaps, cursors, fonts, graphics contexts,
and colormaps are known as "resources".  They all have unique
identifiers associated with them for naming purposes.  The lifetime
of a resource is bounded by the lifetime of the connection over
which the resource was created.  PEX adds several new resources, namely,
lookup tables,
pipeline contexts,
renderers,
structures,
name sets,
search contexts,
PHIGS workstations,
pick measures, and
PEX fonts.
.XP
\fBresource id\fP (X) -
See \fIidentifier\fP.
.XP
\fBRGB\fP (common) -
The color space in which colors are identified by red, green,
and blue intensity values.
.XP
\fBRGB approximation\fP (PEX) -
The conversion of colors from the pipeline's internal color space
into RGB values.
.XP
\fBRGB approximation table\fP (PEX) -
A type of lookup table resource that maintains information necessary for
performing RGB approximation.
.XP
\fBroot structure\fP (PHIGS) -
The first, or base, structure in a structure network.
.XP
\fBscanline algorithm\fP (common) -
An HLHSR method that takes advantage of the coherence from one
scanline to the next by performing the hidden surface removal
operation in scanline order.
.XP
\fBscreen\fP (X) -
See \fIdisplay screen\fP.
.XP
\fBsearch ceiling\fP (PHIGS) -
An attribute of a search context resource
that is an index into the list of element references in the search context's
search path attribute.  Searching stops when the end of the structure
specified by the indicated element reference is reached.  If the index
is zero, the search operates without a ceiling.
.XP
\fBsearch context\fP (PEX) -
A PEX resource that encapsulates the attributes necessary to support
spatial searches of structure networks.
(PHIGS refers to this as an incremental spatial search.)
.XP
\fBsearch distance\fP (PHIGS) -
An attribute of a search context resource that specifies a distance from
the search position attributes.
.XP
\fBsearch exclusion list\fP (PHIGS) -
A search context attribute that maintains a list of name set resource
id's that are to be used to identify
a list of names that \fIare not\fP eligible
for searching operations.
.XP
\fBsearch inclusion list\fP (PHIGS) -
A search context attribute that maintains a list of name set resource
id's that are to be used to identify
a list of names that \fIare\fP eligible
for searching operations.
.XP
\fBsearch position\fP (PHIGS) -
An attribute of a search context resource that specifies
a position in world coordinates.
.XP
\fBsearch start path\fP (PHIGS) -
An attribute of a search context resource that indicates the
point in a structure network at which searching is to begin.
.XP
\fBserver\fP (X) -
The process that provides the basic windowing mechanism.  The server
handles IPC connections from clients, processes graphics requests,
and multiplexes input back to the appropriate clients.  In PEX, the
term usually refers to the combination of the core X server and the
PEX extension to the core X server.
.XP
\fBshading\fP (common) -
The part of rendering that deals with the conversion of color
information into displayable color values.
(PHIGS+ uses this term to indicate the interpolation process for
obtaining color values between the vertices of polylines and surfaces,
see \fIpolyline interpolation method\fP and \fIsurface interpolation method\fP)
.XP
\fBsmooth shading\fP (common) -
Using a surface interpolation method that causes color values to be
interpolated across a surface rather than using a wireframe representation
or using a constant color value for each facet.  (Occasionally, the
term \fIshading\fP is used to refer to smooth shading - PEX attempts
to avoid such usage)
.XP
\fBspecular color\fP (PHIGS+) -
A pipeline state attribute that defines
a color that is used to filter the color of specular highlights.
.XP
\fBspecular concentration\fP (PHIGS+) -
A pipeline state attribute that defines
the "sharpness" of specular highlights
and thereby the "shininess" of the surface.
.XP
\fBspecular reflection\fP (common) -
A mirror-like reflection of varying degree that is caused by
light reflecting from the very top of a surface.  Specular
reflections can be seen only when the viewing position
is along (or near) the vector which represents the direction
of reflected light if the surface were a perfect mirror.
.XP
\fBspecular reflection coefficient\fP (common) -
A pipeline state attribute
that controls the fraction of specular reflection from surfaces.
.XP
\fBspotlight\fP (PHIGS+) -
A type of light source that is defined with a position, a direction,
two attenuation
coefficients, a spread angle
and a color.
.XP
\fBStaticColor\fP (X) -
A class of colormap that interprets pixels composed of a single index
value, which can be used as a single index into arrays containing red, green,
and blue intensity values to produce red, green, and blue output
values.  Unlike PseudoColor, the contents of the
red, green, and blue intensity arrays
cannot be altered by clients.
.XP
\fBStaticGray\fP (X) -
A class of colormap that interprets pixels composed of a single index
value, which can be used as a single index into an array containing red,
green, and blue intensity values.  For any given index, the red, green,
and blue values will be equal, thus the output values are always
shades of gray.  Unlike GrayScale, the contents of the
red, green, and blue intensity arrays
cannot be altered by clients.
.XP
\fBstring\fP (common) -
See \fItext string\fP.
.XP
\fBstructure\fP (PEX) -
A PEX resource
that maintains a list of output commands.
.XP
\fBstructure editing\fP (PHIGS) -
The process of inserting or replacing structure elements in a structure
resource.
.XP
\fBstructure element\fP (PEX) -
An output command that is stored in a structure resource.
.XP
\fBstructure mode\fP (common) -
A client that sends output commands to structure resources
for later display via structure traversal
is said to be operating in \fIstructure mode\fP.
.XP
\fBstructure network\fP (PHIGS) -
A hierarchical collection of structures that is formed by allowing
structures to contain elements that reference
other structures.
.XP
\fBstructure network path\fP (PHIGS) -
A list of element references that uniquely identifies a path through
the hierarchy of a structure network.
.XP
\fBstructure traverser\fP (PEX) -
The part of a renderer that performs structure traversal.
.XP
\fBstructure traversal\fP (PHIGS) -
The process of turning a structure network into a
stream of output commands that can be processed by a renderer.
.XP
\fBsurface\fP (common) -
An output primitive that defines an area, but has
no volume.  Surfaces are typically defined using a
two-dimensional array of control points or by defining a
planar boundary.  Fill areas, fill area sets, indexed polygons,
and parametric surface patches are examples of surfaces.
(PHIGS+ uses the term \fIarea-defining primitive\fP.)
.XP
\fBsurface approximation\fP (PHIGS+) -
A pipeline state attribute that
controls the method and the tolerance used to approximate parametric surfaces.
The value of this attribute is
selected from the list of available methods in the PEX surface approximation
method
enumerated type.
The surface approximation to be used may be obtained
from an interior bundle table instead of directly from the
pipeline state if the 
\fIsurface approximation ASF\fP attribute is set to \fIBundled\fP.
.XP
\fBsurface basis\fP (PHIGS+) -
A PEX enumerated type that specifies the basis functions for drawing
parametric polynomial surfaces.
.XP
\fBsurface color\fP (PHIGS) -
A pipeline state attribute that controls the color
to be used for front-facing surface primitives (and back-facing surface
primtives, if \fIdistinguish\fP is \fIFalse\fP).
The color
to be used for surface primitives may be obtained
from an interior bundle table instead of directly from the
pipeline state if the 
\fIsurface color ASF\fP attribute is set to \fIBundled\fP.
(Called \fIinterior color\fP in PHIGS)
.XP
\fBsurface edge\fP (PHIGS) -
The boundary of a surface primitive.  For polygons, this
is the boundary of the polygon.  For indexed polygons, triangle strips,
and quadrilateral meshes, the surface edge
is the boundary of each polygon.  For parametric polynomial surfaces
and non-uniform B-spline surfaces, the surface edges are the trimming
curves.
.XP
\fBsurface edge color\fP (PHIGS) -
A pipeline state attribute that controls the color
to be used for surface edges.
The color
to be used for surface edges may be obtained
from an edge bundle table instead of directly from the
pipeline state if the 
\fIsurface edge color ASF\fP attribute is set to \fIBundled\fP.
.XP
\fBsurface edge type\fP (PHIGS) -
A pipeline state attribute that
specifies the visual style to be used when drawing surface edges.
Edge types include dashed, dotted, solid, etc.
The value of this attribute is
selected from the list of available types in the PEX surface edge type
enumerated type.
The surface edge type to be used may be obtained
from an edge bundle table instead of directly from the
pipeline state if the
\fIsurface edge type ASF\fP attribute is set to \fIBundled\fP.
.XP
\fBsurface edge width\fP (PHIGS) -
A pipeline state attribute that
specifies a multiplier for the implementation-dependent nominal width
of surface edges.
A surface edge
width of 1.0 results in a line that is the same width as the nominal
width surface edge for the device.
The surface edge width to be used may be obtained
from an edge bundle table instead of directly from the
pipeline state if the
\fIsurface edge width ASF\fP attribute is set to \fIBundled\fP.
.XP
\fBsurface edges\fP (PHIGS) -
A pipeline state attribute that
specifies whether or not surface edges
are to be drawn in addition to surface interiors.
This flag may be obtained
from an edge bundle table instead of directly from the
pipeline state if the
\fIsurface edges ASF\fP attribute is set to \fIBundled\fP.
.XP
\fBsurface interpolation method\fP (PEX) -
A pipeline state attribute that defines how the values
between the vertices of surfaces are to be computed.
The value of this attribute is
selected from the list of available types in the PEX surface interpolation
enumerated type.
(Called \fIshading\fP in PHIGS+)
.XP
\fBtable\fP (PEX) -
See \fIlookup table\fP.
.XP
\fBtable entry\fP (PEX) -
See \fIlookup table entry\fP.
.XP
\fBtable type\fP (PEX) -
An attribute of a lookup table resource that defines the format of the
entries that it maintains.
The table entries within any given table type are all
of the same format, but each entry may contain different data values.
.XP
\fBtarget drawable\fP (PEX) -
The drawable associated with a renderer or a PHIGS workstation upon which
output primitives are rendered.
.XP
\fBtessellate\fP (common) -
The process of approximating a primitive with another type of primitive.
Parametric curves can be approximated with polylines, and
parametric surfaces can be approximated with polygons.
.XP
\fBtext\fP (PHIGS) -
A PEX output primitive that consists of a sequence of characters, a position,
and direction vectors.
.XP
\fBtext alignment\fP (PHIGS) -
A pipeline state attribute that
controls the horizontal and vertical
justification of a text string.
.XP
\fBtext bundle\fP (PHIGS) -
A type of lookup table resource that maintains a list of text
attribute values.
A text bundle table resource id is maintained as an
attribute of a renderer and as an attribute of a PHIGS workstation resource.
Depending on the settings of the various text ASF's,
text attributes may be obtained from the text bundle table rather
than from the renderer's or PHIGS workstation's pipeline state.
.XP
\fBtext bundle index\fP (PHIGS) -
A pipeline state attribute that indicates the entry in the edge bundle
table from which text attributes may be obtained.
.XP
\fBtext color\fP (PHIGS) -
A pipeline state attribute that controls the color
to be used for text primitives.
The color
to be used for text primitives may be obtained
from a text bundle table instead of directly from the
pipeline state if the 
\fItext color ASF\fP attribute is set to \fIBundled\fP.
.XP
\fBtext font index\fP (PHIGS) -
A pipeline state attribute that indicates the entry in the text font
table from which the resource id of a PEX or X font may be obtained.
Characters will be drawn using the glyphs defined in this font.
The text font index
to be used may be obtained
from a text bundle table instead of directly from the
pipeline state if the 
\fItext font index ASF\fP attribute is set to \fIBundled\fP.
(Called \fItext font\fP in PHIGS)
.XP
\fBtext font table\fP (PHIGS) -
A type of lookup table resource that maintains a list of PEX and X font
resource ids.
A text font table resource id is maintained as an
attribute of a renderer and as an attribute of a PHIGS workstation resource.
.XP
\fBtext local coordinate system\fP (PHIGS) -
The coordinate system in which character glyphs have been defined for
a particular font.
.XP
\fBtext path\fP (PHIGS) -
A pipeline state attribute that
controls the relative position of successive
characters of a text string.
.XP
\fBtext precision\fP (PHIGS) -
A pipeline state attribute that
selects the fidelity with which text primitives are transformed and clipped.
The text precision
to be used may be obtained
from a text bundle table instead of directly from the
pipeline state if the 
\fItext precision ASF\fP attribute is set to \fIBundled\fP.
.XP
\fBtext string\fP (PHIGS) -
A list of characters that are to be drawn at a particular location
in three-dimensional space.
.XP
\fBtolerance\fP (PHIGS+) -
A value that is used to provide some control
over the accuracy to which curve and surface approximation calculations
are performed.
.XP
\fBtransparency\fP (common) -
A mathematical approximation of the effect of transmitting light through
a non-opaque surface.  (See \fItransmission coefficient\fP)
.XP
\fBtransmission coefficient\fP (PHIGS+) -
A pipeline state attribute
that controls the fraction of light that is transmitted through a surface.
(Called \fItransparency coefficient\fP in PHIGS+)
.XP
\fBtraverser\fP (PEX) -
See \fIstructure traverser\fP.
.XP
\fBtriangular strip\fP (PHIGS+) -
A PEX output primitive that consists of a series of points which are
used to form a series of connected triangles.
.XP
\fBtrim curve approximation\fP (PHIGS+) -
A pipeline state attribute that
controls the method and the tolerance used to approximate trimming curves.
The value of this attribute is
selected from the list of available methods in the PEX curve approximation
method
enumerated type.
The trimming curve approximation to be used may be obtained
from an interior bundle table instead of directly from the
pipeline state if the 
\fItrim curve approximation ASF\fP attribute is set to \fIBundled\fP.
.XP
\fBtrimmed surface\fP (PHIGS+) -
A parametric surface that is defined together with a set of trimming loops.
.XP
\fBtrimming curve\fP (PHIGS+) -
A parametric curve defined in the parameter space of a parametric surface.
.XP
\fBtrimming loop\fP (PHIGS+) -
A series of trimming curves that explicitly connect in head to tail fashion
and form a closed loop that serves to limit the parameter space over which
a surface will be rendered.
.XP
\fBTrueColor\fP (X) -
A class of colormap that interprets pixels composed of separate red, green,
and blue components.  These components
are used to separately index into red, green, and blue intensity
arrays to produce red, green, and blue output intensities.  Unlike
DirectColor, the red, green, and blue intensity arrays cannot be altered
by clients.
.XP
\fBunlock\fP (PEX) -
When a request is finished using a resource, it is said to \fIunlock\fP
the resource so that other requests may access it.
.XP
\fBunpost\fP (PHIGS) -
The process of removing a structure network from a PHIGS workstation's
list of posted structure networks.
.XP
\fBvector light\fP (common) -
A light source, assumed to be at infinity, that is defined with a
direction vector and a light color.  (Called \fIdirectional light source\fP
in PHIGS+)
.XP
\fBvertex\fP (common) -
A position in some three-dimensional space.  A set of vertices may be
used to describe a higher-order primitive, such as a polyline or
a quadrilateral mesh.
.XP
\fBvertex color\fP (common) -
Color values that are associated with a vertex.  Some output primitives
allow color information to be passed along with vertices and 
used as input to the rendering pipeline.
.XP
\fBvertex normal\fP (common) -
A geometric normal that is associated with a vertex.
Some output primitives
allow normal information to be passed along with vertices and 
used as input to the rendering pipeline.
.XP
\fBview\fP (common) -
The collection of attributes that define the transformations necessary
to transform geometry in world coordinates into device coordinates.
.XP
\fBview index\fP (PHIGS) -
A pipeline state attribute that indicates the entry in the view
table from which viewing and clipping attributes are obtained.
.XP
\fBview mapping matrix\fP (PHIGS) -
The 4 \(mu 4 matrix that is used to represent the view mapping
transformation.
.XP
\fBview mapping transformation\fP (PHIGS) -
The process of mapping geometry in the viewing reference coordinate
system into geometry in the normalized projection coordinate system.
This transformation is also sometimes referred to as the projection
transformation.
.XP
\fBview orientation matrix\fP (PHIGS) -
The 4 \(mu 4 matrix that is used to represent the view orientation
transformation.
.XP
\fBview orientation transformation\fP (PHIGS) -
The process of mapping geometry in the world coordinate system
into geometry in the viewing reference coordinate system.
This transformation is also sometimes referred to as the viewing
transformation.
.XP
\fBview table\fP (PHIGS) -
A type of lookup table resource that maintains a list of viewing
specifications.
A view table resource id is maintained as an
attribute of a renderer and as an attribute of a PHIGS workstation resource.
.XP
\fBview update state\fP (PHIGS) -
A PHIGS workstation attribute that is set to \fIPending\fP or \fINotPending\fP
depending on whether there have been requests to change the PHIGS
workstation's views that have not yet been made current.
.XP
\fBview volume\fP (common) -
The world coordinate volume that will be mapped to the unit cube
in the normalized projection coordinate system.
.XP
\fBviewing position\fP (common) -
The position in space at which the eye or virtual camera is
located.  Also known as the viewpoint, eyepoint, eye
position or camera position.
.XP
\fBviewing reference coordinate system\fP (PHIGS) -
A three-dimensional Cartesian coordinate system used for defining viewing
parameters.  Sometimes referred to as the VRC system.  Also called
eye coordinate system, camera coordinate system, view coordinate system.
The space defined by this coordinate system is referred to as VRC space,
eye space, camera space, or view space.
.XP
\fBviewing transformation\fP (common) -
See \fIview orientation transformation\fP.
.XP
\fBviewpoint\fP (common) -
See \fIviewing position\fP.
.XP
\fBviewport\fP (common) -
That part of a drawable in which output from a renderer or a PHIGS workstation
is to appear.  Renderers and PHIGS workstation resource each maintain
a viewport attribute that is used when mapping geometry into device
coordinates.
.XP
\fBvisible surface computation\fP (common) -
See \fIHLHSR\fP.
.XP
\fBvisual state\fP (PHIGS) -
A PHIGS workstation attribute that reflects the "correctness" of the
displayed image.
.XP
\fBwindow\fP (X) -
An X window.
.XP
\fBwire\fP (X) -
See \fIconnection\fP.
.XP
\fBworkstation update state\fP (PHIGS) -
A PHIGS workstation attribute that is set to \fIPending\fP or \fINotPending\fP
depending on whether there have been requests to change the PHIGS
workstation's NPC subvolume or viewport that have not yet been made current.
.XP
\fBworld coordinate system\fP (common) -
Device-independent Cartesian coordinates used by an application
program to organize modeled objects for display.  All objects
in a scene must be mapped into the world coordinate system so
that interrelationships can be computed and the image can be
generated.  Sometimes referred to as WCS.
The space defined by this coordinate system is referred to as world space.
.XP
\fBX font\fP (PEX) -
An array of glyphs (typically characters).
The protocol does no translation or interpretation of character sets.
The client
simply specifies values that are used to index the glyph array.
A font contains
additional metric information to determine inter-glyph and inter-line
spacing.
X fonts differ from PEX fonts
in that PEX fonts contain glyphs that
are guaranteed to be scaleable and rotatable.
X fonts are typically bitmap text fonts.
.XP
\fBX window coordinate system\fP -
The coordinate system used within an X window.
The X window coordinate system has X horizontal and Y vertical,
with the origin [0, 0] at the upper left of the window.  Coordinates
are discrete, and in terms of pixels.  Each window and pixmap has
its own coordinate system.  For a window, the origin is at the
upper left, inside the border.  For PEX, z values
may also still be present and utilized for z-buffer calculations.
.XP
\fBz-buffer\fP (common) -
A hidden surface removal algorithm that maintains a "depth-" or z-value
for each pixel that is rendered.  Pixel values are only overwritten if the
object being rendered is closer to the viewing position than the
z-value already written for that pixel.
.nr PD 0.20i
.ps 10
.nr PS 10
.vs 12
.nr VS 12
.bp

.LP
.[
$LIST$
.]
