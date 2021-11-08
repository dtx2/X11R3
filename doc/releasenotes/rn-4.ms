.ne 6
.NH 1
What's New in this Release
.LP
The primary focus of this release has been to fix bugs in the server and in
the base programming libraries.  In addition, several key features have been
added so that this release can provide a clean, sturdy foundation for 
developing applications.
.NH 2
Changes to the core distribution
.LP
The following additions, deletions, and modifications have been made to the
software in the core distribution.  Users of the old fonts will need to edit
their resources files.  Widget writers will need to following the instructions
in the document \fIConverting Widgets from X11 Release 2 to X11 Release 3\fP.
.\".ll -5
.\".in +5
.IP "many, many bugs fixed"
.br
A large number of bugs have been fixed in the server, the libraries, 
and the
clients.  Many parts of the sample server that were broken 
in the previous release have been
fixed.
In particular, ink metrics in fonts have been fixed; client errors in computing
character sizes will now be easy to spot.
.IP "build configuration simplified"
.br
The configuration templates in \fIutil/imake.includes\fP have been restructured
to make per-server and per-site changes easier to make.  A \fIREADME\fP 
file in that
directory now describes each of the parameters that may be set and
site-specific changes have been isolated in a new file named \fIsite.def\fP.
The template file \fIImake.tmpl\fP should never need to be modified.
.IP "new servers"
.br
New support has been added to the sample server for the following platforms:
Parallax video graphics processor for Suns and Vaxes, and 
Apple Macintosh II under A/UX.
.IP "new fonts"
.br
Adobe Systems, Inc. and Digital Equipment Corporation have jointly donated 
BDF font files for the \fICourier\fP, \fITimes\fP, \fIHelvetica\fP, 
\fINew Century Schoolbook\fP,
and \fISymbol\fP families at 8, 10, 12, 14, 18, and 24 point sizes, 
Roman and Italic or Oblique styles,
medium and bold weights, 
for 75 dot per inch monitors.  Bitstream, Inc. has donated BDF font files for 
its \fICharter\fP family at the
same sizes, styles, and weights for both 75 and 100 dpi monitors.
Both donations use the ISO Latin-1 character set.
All of the core clients have been changed
to use the new fonts.  The fonts that were shipped with previous releases
have been moved to the user-contributed distribution (except for 6x10, 6x12,
6x13, 8x13, 8x13bold, and 9x15).
.IP "font name wildcarding"
.br
The sample server now accepts the same wildcard characters in the 
\fBOpenFont\fP request as in 
the \fBListFonts\fP request.  This is very useful for refering to fonts with
very long names, such as those in the new font sets.  Users are encouraged
to carefully select wildcarded names so that they will work properly on 
monitors with different resolutions (for example, when specifying a size,
use a wild card for the pixel size, but specify the point size).  Note that
this behavior is allowed by the Protocol, but is not required of other
servers.
.IP "font names separated from file names"
.br
Font names are no longer tied to the names of the files in which they are 
stored.  Instead, the name of the font is taken from the FONT property in
the font data.  A script for generating the appropriate font name from the
other properties in a font file may be found in \fIutil/scripts/fontname.sh\fP.
The sample server no longer scans font directories, but rather
looks in a database created by the \fImkfontdir\fP program.
.IP "font name aliases support"
.br
The sample server now supports multiple names for a single font.  These are
stored in a file named \fIfonts.alias\fP in each font directory and may refer
to any font.  This is very useful for providing collections of symbolic
names for fonts.
.IP "BDF font format now a standard"
.br
The X Consortium has adopted the BDF font format as a non-exclusive standard.
Server vendors are now expected to provide a program for converting BDF
files to whatever format is used by their server.  The sample server font
compiler has been renamed \fIbdftosnf\fP and has been fixed to properly
generate ink metrics for terminal emulator fonts.
.IP "\fIbdftosnf\fP and \fIshowsnf\fP installed"
.br
The \fIbdftosnf\fP and \fIshowsnf\fP utilities are now installed 
for public use.
.IP "definition of wide arcs changed"
.br
The definition of wide arcs has been fixed to be a brush centered on and
normal to the path described by the arc.  The old definition used to draw
arcs of non-uniform width.
.IP "backing store and save-unders supported"
.br
The sample server now supports backing store and save-unders.  The backing
store hooks which were unused in the previous release have changed; server
developers should study the new interfaces.
.IP "X Toolkit Intrinsics Standard Adopted"
.br
The X Toolkit Intrinsics (frequently refered to as \fIXt\fP) have been 
reviewed by the X Consortium and are now part of the X standard.  Any vendor 
that ships \fIXlib\fP is expected to ship \fIXt\fP.  Some interface and 
widget class
record changes have been made since the previous release.  Sources for
the document \fIConverting Widgets from X11 Release 2 to X11 Release 3\fP
may be found in 
\fIdoc/tutorials/r3widgets.ms\fP.  Applications that used the proper
interfaces and data types should simply recompile and work.  However,
application writers should also read the conversion tutorial as certain
programming errors that might not have been visible in previous releases
will become noticable in this release.  Note that the \fIXt\fP standard does
not include a widget set.
.IP "Athena widget enhancements"
.br
A new List widget has been added the Athena widget set.  A prototype menu
widget that will eventually be added to this set is available in the 
user-contributed distribution in the directory \fIcontrib/widgets/MenuBox/\fP.
The Athena VendorShell widget now supports specifying icon pixmaps as
resources. The text widget now supports selections.
.IP "new Miscellaneous Utilities library"
.br
A library named \fIXmu\fP containing miscellaneous utilities to support 
the MIT clients has been added.  It currently has a variety of X Toolkit
converters as well as routines for displaying \fIXlib\fP-like XError messages,
drawing rounded rectangles, converting ISO Latin-1 strings to lower case,
and reading bitmap files.  It is not part of the X standard and is expected
to be a repository for generally useful routines.  It only uses external
interfaces to the standard libraries and should be portable to 
vendor-supplied platforms.
.IP "\fIXlib\fP support for 64bit architectures"
.br
The internals of \fIXlib\fP have been reworked to support 64 bit machine
architectures and Cray supercomputers running UNICOS in particular.  Extension
writers should look at the new internal interfaces for sending and receiving 
data.
.IP "\fIXlib\fP Property packing semantics clarified"
.br
The \fIXGetWindowProperty\fP and \fIXChangeProperty\fP now take care of
converting between host data objects (e.g. unsigned char, unsigned short, and 
unsigned long) and network data objects (e.g. CARD8, CARD16, and CARD32).  
Applications which store data in or retreive data from properties should 
also use arrays of host data objects instead of structures.
.IP "new routines in \fIXlib\fP"
.br
The following routines have been added to \fIXlib\fP for accessing data 
from opaque 
structures:  \fIXMaxRequestSize\fP, \fIXResourceManagerString\fP,
\fIXDisplayMotionBufferSize\fP, \fIXDisplayKeycodes\fP, and
\fIXVisualIDFromVisual\fP.
.IP "new Display Manager \fIxdm\fP"
.br
A new, portable system for running X and starting user sessions has been
added to this release.  This Display Manager, called \fIxdm\fP, provides a
convenient way for sites to run X automatically.  Site administrators may 
easily tailor it provide an wide variety of environments.  This new utility
superceeds \fIxterm -L\fP as well as many uses of \fIxinit\fP.  Future releases
will concentrate on providing even nicer user interfaces.
.IP "new utilities"
.br
Several new utility programs have been provided:  \fIxdpyinfo\fP for printing
out information about a particular display, \fIxev\fP for printing out
the contents of various events, \fIxlswins\fP for listing the windows on a 
display, \fIxkill\fP for getting rid of unwanted windows, \fIxmag\fP for
enlarging parts of the screen, \fIxclipboard\fP for collecting text selections,
and \fIxcutsel\fP for converting between cut buffers and selections.
.IP "xman now supported"
.br
The \fIxman\fP program is now part of the core distribution.  
Many of the bugs which causes poor performance on some servers have been
fixed, making this program one of the prettier demonstrations of the 
new fonts.
.IP "new demos"
.br
Two new demos have been added:  \fIxeyes\fP and \fImaze\fP.
.IP "new features in \fIxterm\fP"
.br
The os-specific portion of \fIxterm\fP has been substantially rewritten and
now works properly on both BSD and System V derived versions of UNIX.  The
full translation manager rebinding facilities are supported for both
keyboard and VT102-mode pointer input.  This includes arbitrary binding of 
strings to keys (including keys that bind other keys!), making, inserting,
and transmitting selections, and popping up of menus.
Reverse video has been substantially improved.
Several bugs in Tektronix graphics input mode have been fixed.  Function keys
can now be set to generate Sun keyboard escape sequences instead of ANSI
escape sequences.  Default settings for various terminal modes are now
supported.
.IP "rewrite of \fIx10tox11\fP"
.br
The \fIx10tox11\fP protocol translator for running X10 binary programs on X11
displays has been completely rewritten.  It should be substantially faster
and give better results for a larger number of programs.
.IP "font path support in \fIxset\fP"
.br
The \fIxset\fP utility now supports adding to and deleting from the
font directory list.  A new keyword \fIrehash\fP has been added to the \fIfp\fP
option to force the server to reread any changed font directories (this
is useful after running \fImkfontdir\fP in a directory).
.IP "pointer map support in \fIxmodmap\fP"
.br
The \fIxmodmap\fP utility has been enhanced to support changing and displaying
the pointer map.  A new option for displaying the keyboard map has replaced
the old \fIxprkbd\fP program.
.IP "new coversion utilities for bitmap files"
.br
Two new programs, \fIbmtoa\fP and \fIatobm\fP, have been provided for 
converting \fIbitmap\fP to and from
plain text.  They are very useful for generating pictures for including in 
documents and for creating bitmaps from arbitrary arrays of characters.
The \fIpbm\fP package in the user-contributed distribution contains a much
wider range of tools.
.IP "BSD \fIcompress\fP sources supplied"
.br
The sources for the BSD \fIcompress\fP program are now included for those 
sites that do not already have this program.  Starting with this release,
some documentation and bitmap images in the user-contributed 
distribution have been compressed to save space.
.IP "public domain \fIcpp\fP supplied"
.br
The sources for a C preprocessor based on Martin Minow's public domain
DECUS \fIcpp\fP are now included for sites whose native \fIcpp\fP cannot
handle the server and the X Toolkit.  This \fIcpp\fP must be built and
installed by A/UX 1.0 users.
.IP "substitute \fIsoelim\fP"
.br
A version of the \fIsoelim\fP utility for removing \fB.so filename\fP 
directives from troff sources has been provided for systems that do not have
one.
.IP "sample copyright notice \fI./COPYRIGHTS\fP"
.br
The file \fICOPYRIGHTS\fP in the top level directory contains a sample
copyright notice recommended for people who are interested in contributing
software to the public releases.
.IP "X Standards \fI./STANDARDS\fP"
.br
The file \fISTANDARDS\fP in the top level directory contains a description
of what is and is not part of the X Consortium's standard for the X Window
System.
.NH 2
Changes to the user-contributed distribution
.LP
The user-contributed distribution is now arranged in a hierarchy similar to
the core distribution.  Generally useful programs are in \fIclients\fP,
visually interesting programs are in \fIdemos\fP, amusements are in
\fIgames\fP, server and client extensions are in \fIextensions\fP,
server code that has not been integrated into the sample server is in 
\fIserver\fP,
font conversion utilities and BDF files are in \fIfonts\fP,
native libraries for building applications are in \fItoolkits\fP, X Toolkit
widget sets are in \fIwidgets\fP, and several popular window managers are in 
\fIwindowmgr\fP.
.IP "old XMenu library removed"
.br
The old \fIXMenu\fP library has been removed from this release.  Applications
in need of menus should use a toolkit.  A prototype X Toolkit menu widget
is available in the \fIcontrib/widgets/MenuBox/\fP directory.
.IP "new clients"
.br
A variety of new programs have been added to the \fIclients\fP subdirectory:
a plotting package (\fIsplot\fP), a magnifying application (\fIspy\fP), 
more previewers (\fIxdvi\fP, \fIxpic\fP, and \fIxtroff\fP), programs for
querying the user (\fIalteryorngs\fP, \fIxmessage\fP, and \fIxstring\fP), 
a utility for
looking up words from the SRI-NIC dictionary server (\fIxwebster\fP),
versions of \fIxpr\fP for other devices (\fIhpxpr\fP and \fIxipr\fP), 
several programs for configuring your session (\fIx11startup\fP, \fIxbgsun\fP,
\fIxdvorak\fP, \fIxplaces\fP, \fIxpref\fP, and \fIxtools\fP), several
tools for displaying images (\fIpbm\fP and \fIxim\fP), and a drawing package
(\fIxfig\fP).
.IP "new demos"
.br
Several striking programs have been added to the \fIdemos\fP directory:
a multi-screen version of \fIico\fP (\fIpsycho\fP),
a utility for creating interstellar backgrounds (\fIspaceout\fP),
a random slither program (\fIworm\fP),
a program for displaying the phase of the moon (\fIxphoon\fP), and
a nasty program for demonstrating what ``going technicolor'' really
means (\fIxrotmap\fP).
.IP "fonts"
.br
A random collection of public domain fonts from the Berkeley Mac Users Group 
and the INFO-MAC archives have been contributed.  BDF versions of 
many of the X10 fonts have been included as well as the old X11 fonts.
.IP "games"
.br
Several games have been contributed since the last release:  the ancient and
honorable \fImazewar\fP, the ubiquitous tower of \fIxhanoi\fP, two cards games
(\fIxmille\fP and \fIxsol\fP), yet another \fIxpuzzle\fP, 
a version of the video arcade game \fIqix\fP,
and the infamous
\fIxtrek\fP.
.IP "toolkits"
.br
New versions of \fIInterViews\fP, \fIXr11\fP, and \fIandrew\fP along with a
prototype implementation of \fICLUE Common Lisp User Interface Environment\fP
are available in the \fItoolkits\fP directory.
.IP "widgets"
.br
The \fIHP\fP and \fISony\fP widget sets, along with a collection of random, 
interesting widgets may be found in the \fIwidgets\fP subdirectory.
Since the HP and Sony widgets are temporarily based on the R2 Intrinsics, 
a version of
\fIXt\fP compatible with the previous release is provided in the the 
\fIcontrib/widgets/Xhp/Xt/\fP subdirectory.
.IP "windowmgrs"
.br
Relatively recent versions of the \fIawm\fP, \fIrtl\fP, \fItwm\fP and \fIwm\fP
window managers are available in the \fIwindowmgrs\fP subdirectory.
Their interoperability with R3 core clients has not been tested.
