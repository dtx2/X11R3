.nr PD 1.25v
.EF '\fBX Window System Release Notes\fP''\fBX, Version 11, Release 3\fP'
.OF '\fBX Window System Release Notes\fP''\fBX, Version 11, Release 3\fP'
.EH ''\fB- % -\fP''
.OH ''\fB- % -\fP''
.DA "October 28, 1988"
.TL
X Window System, Version 11
.br
Release 3
.br
Release Notes
.AU
Jim Fulton
.AI
X Consortium
MIT Laboratory for Computer Science
.AB
The X Window System is a portable, network-transparent window system 
originally developed at MIT.  It can be used on a wide variety of raster 
display devices, ranging from from simple monochrome frame buffers to deep, 
true color graphics processors.  This document describes contents of 
the third public release of X, Version 11 from MIT and how it has changed
from previous releases.
.AE
.NH 1
Overview
.LP
This is the third release of the X Window System, Version 11 from MIT.
Substantial progress has been made in improving the robustness and general
usability of the sample server, libraries, and applications.
Many bugs have been fixed, support for
several new platforms has been added, and backing store and save-unders have
been implemented.  No incompatible changes have been made to either the
core Protocol or to the \fIXlib\fP programming library.  
.LP
Several changes have been made to the X Toolkit Intrinsics that will
require minor additions to widget class records.  Professionally designed
font families from Adobe Systems, Inc. and
Digital Equipment Corporation, and
from Bitstream, Inc. have replaced the chaotic collection that was 
shipped in the previous releases.
This may require some minor adjustments of user preferences files.
Fonts are no longer named according to the files in which they 
are stored; instead, the server reads a table out of each directory that
specifies in which file to look for each font.  In addition, support for
wildcarding and aliasing font names has been added.
.FS
\fIX Window System\fP is a trademark of MIT.
.sp
Copyright \(co\ 1988 by the Massachusetts Institute of Technology.
.sp
Permission to use, copy, modify, and distribute this
software and its documentation for any purpose and without
fee is hereby granted, provided that the above copyright
notice appear in all copies and that both that copyright
notice and this permission notice appear in supporting
documentation, and that the name of M.I.T. not be used in
advertising or publicity pertaining to distribution of the
software without specific, written prior permission.
M.I.T. makes no representations about the suitability of
this software for any purpose.  It is provided "as is"
without express or implied warranty.  
.sp
This software is not
subject to any license of the American Telephone and Telegraph
Company or of the Regents of the University of California.
.FE
.LP
This release contains two types of software:  that which is supported
by the staff of the X Consortium and which forms the core of the X Window
System, and user-contributed software that is provided as a public service
without direct support.  The core part of the distribution also contains
public implementations of certain utilities that may not be available on all
platforms as well as the tools that are used to build X software.
.LP
The servers, libraries, and applications provided in this distribution are
sample implementations rather than reference implementations.  The X 
Consortium standardizes specifications, not particular renditions in code.
In particular, the document \fIX Window System Protocol, Version 11\fP is the
final authority on what is and is not part of the core X Window System 
protocol.
.LP
Since the last release, the X Consortium has added the following specifications
to the X standard:
.RS .5in
.IP "\fIX Toolkit Intrinsics\fP"
.br
The X Toolkit Intrinsics as specified in the document \fIX Toolkit Intrinsics
\- C Language Interface\fP have been adopted as a non-exclusive, required 
standard for
toolkit development.  Any vendor that ships \fIXlib\fP is now expected to 
provide \fIXt\fP as well.  Vendors are free to develop
and ship alternative toolkits, but they if they ship \fIXlib\fP, they 
should also ship \fIXt\fP.  Applications developers should note that this 
specification does not include a widget set.
.IP "\fIBDF Font File Format\fP"
The Bitmap Distribution Format (BDF) as specified in
the document \fIBitmap Distribution Format Format 2.1\fP has been
adopted as a non-exclusive, required standard for font file interchange.
Any vendor that ships an X Window System server is expected to 
provide a way of translating BDF fonts into the appropriate server-specific
format.
.RE
.LP
To avoid conflict with Version 10 applications, all C language source code is
expected to use the prefix \fIX11/\fP when including header files as well as
use the library name \fIlibX11.a\fP or the loader option \fI-lX11\fP when 
linking.
In addition, UNIX programs are expected to use the command line options
\fI-display host:dpy\fP and \fI-geometry geometryspec\fP in place of the
obsolete \fIhost:dpy\fP or \fI=geometryspec\fP formats.
Applications that ignore these guidelines will no longer
function properly with this release.
.LP
Font support in the sample server has been improved in several ways:
.RS .5in
.IP "\fIfont names different from file names\fP"
.br
A font's name is now determined by the contents of the property named FONT
rather than the name of the file in which it is stored.  A new utility 
named \fImkfontdir\fP is used to scan the fonts in a directory and build a
database listing each font name and the file in which it is stored.  The server
then uses this database when looking for fonts in a given directory.
.IP "\fInew font naming scheme\fP"
.br
A new convention for uniquely naming fonts of any style, resolution, and
size is now used for all of the new core fonts.
.IP "\fIwildcarding in font names\fP"
.br
The sample server now supports
wildcard characters in font names (such as would be given on a command line
or in a resource database).  This makes specifying fonts which use the
new naming scheme much easier.  This behavior is allowed by the Protocol
but is not guarranteed in other servers.
.IP "\fIfont name aliasing\fP"
.br
Font directories may now contain a list of font name synonyms which may be used
as alternate names for existing fonts.  This list is stored in the
file \fIfonts.alias\fP and is read whenever the font database is scanned by
the server.  
.IP "\fInew fonts\fP"
.br
Adobe Systems, Inc. and Digital Equipment Corporation have jointly donated 5
families of fonts (\fICourier\fP, \fITimes\fP, \fIHelvetica\fP, 
\fINew Century Schoolbook\fP, and \fISymbol\fP)
in a variety of sizes, styles and weights.  Bitstream, Inc.
has donated its \fICharter\fP font family in the same sizes, styles 
and weights.
Most of the fonts that were shipped in 
previous releases have been moved to the user-contributed distribution, and
all core clients have been converted to use the new fonts.  Applications
developers and general users are very strongly urged to use the new fonts.
.RE
.LP
This release should build and install on machines running
the following operating systems:  4.3+tahoe, Ultrix, SunOS, HP-UX,
Domain/IX, A/UX, and UNICOS, and AOS 4.3.  See any \fIREADME\fP files in
the appropriate \fIserver/ddx/\fP directories or \fI*.macros\fP files
in \fIutil/imake.includes/\fP for details.  
.FS
\(dd Ultrix and Ultrix-32 are trademarks of Digital Equipment Corporation;
SunOS is a trademark of Sun Microsystems, Inc.; HP-UX is a trademark of the
Hewlett-Packard Company; Domain is a trademark of Apollo Computer;
PostScript is a
trademark of Adobe Systems, Inc.; A/UX is a trademark of Apple Computer;
UNICOS is a trademark of Cray Research; 
AOS is a trademark of the IBM Corporation;
UNIX is a registered trademark of AT&T Bell Laboratories;
Times, Helvetica, and New Century Schoolbook are registered trademarks of
Linotype; and Charter is a registered trademark of Bitstream, Inc.
.FE

