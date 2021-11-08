.NH 1
Contents of this Release
.LP
This release contains roughly 100 megabytes of source code and documentation.
Because no one site will probably need or want all of it,
the release has been broken into 3 pieces of
roughly equal size:
one for the core software, one for various user-contributed toolkits, and one 
for the rest of the user-contributed software.  
.LP
.NH 2
The core distribution
.LP
The core distribution, containing roughly 30 megabytes of software that is 
supported by the staff of the X Consortium, is made up of the directories 
listed below.
If you find a reproducible bug in this section of the release,
please fill out a copy of the form located in
\fIdoc/bugs/bug-report\fP and mail it to \fIxbugs@expo.lcs.mit.edu\fP.
.\".ll -5
.\".in +5 
.IP "\fI./\fP"
.br
The top level directory contains important notices, the general X manual
page, and all of the subdirectories for building the release.
.IP "\fIX11/\fP"
.br
Copies of all public header files are either stored here or linked in during
the \fImake all\fP build phase.  This directory, and the bitmaps in the 
directory below it get copied to the directory specified
by the IncDir configuration parameter (by default, \fI/usr/include/X11\fP)
during the \fImake install\fP phase.  Note that the final directory name should
never be anything other than \fIX11\fP or else all X programs will fail to 
compile.
.IP "\fIX11/bitmaps/\fP"
.br
This directory contains a number of single plane images stored 
in \fIbitmap\fP format
(read and written with the Xlib routines \fIXReadBitmapFile\fP,
\fIXmuReadBitmapDataFromFile\fP, and 
\fIXWriteBitmapFile\fP).  They are often #included in C programs and may be 
used as background tiles with \fIxsetroot\fP utility.  Also, the X Toolkit
allows users to specify the names of files in this directory to be used
as cursors and icons.  For more 
information, see the manual page for \fIbitmap\fP and the
document \fIutil/bm-convert/bm-convert.doc\fP.
.IP "\fIclients/\fP"
.br
This directory is the top of the user program source tree.  There are 
other programs in the \fIdemos/\fP and \fIexamples/\fP trees, but they
are for demonstration purposes only.  A number of other very useful
user-contributed programs can be found under \fIcontrib/clients/\fP.
.IP "\fIclients/bitmap/\fP"
.br
This directory contains 
an editor for creating and modifying single plane bitmap images (such
as the ones stored in \fIX11/bitmaps/\fP) used in defining two color
tiles (for filling areas with stipples) and masks (for clipping and 
specifying cursor images).
All resources can be specified both from the command line and in your
defaults.  Two new programs, \fIbmtoa\fP and \fIatobm\fP have been added
that convert \fIbitmap\fP files to and from simple strings.
.IP "\fIclients/uwm/\fP"
.br
This directory contains one of the older window managers for X.  A number
of bugs have been fixed and several new features have been added.
.IP "\fIclients/x10tox11/\fP"
.br
This directory contains a rewrite of the
X10 to X11 protocol translator.  It masquerades as an X10 server,
converting X10 requests into X11 requests and X11 events into X10 events
(i.e. it is not a libnest implementation).  The \fIx10tox11\fP program
allows X10 programs to be run unmodified on an X11 server, making 
converting from X10 to X11 much easier.  This new version should be
substantially faster and give more accurate results than the last version.
.IP "\fIclients/xbiff/\fP"
.br
This directory contains
a simple little program that displays a picture of a mailbox whose
flag goes up when you get new mail.  It uses the Mailbox widget in the
Athena widget set.
.IP "\fIclients/xcalc/\fP"
.br
This directory contains 
a desktop calculator program that emulates a TI-30, an HP-10C, and
a slide rule.  This program needs to be rewritten.
.IP "\fIclients/xclipboard/\fP"
.br
This directory contains
two programs for manipulating selections:  \fIxclipboard\fP gathers text 
that is sent to the CLIP_BOARD from other clients (see \fIxterm\fP and
the Athena Text widget), and
\fIxcutsel\fP provides a bridge between older clients that can only deal
with cut-buffers and newer clients that use selections.
.IP "\fIclients/xclock/\fP"
.br
This directory contains a simple little program that displays a clock.
It can display the time in either analog or digital format, and uses the 
Clock widget in the Athena widget set.
.IP "\fIclients/xdm/\fP"
.br
This directory contains
the new Display Manager.  It is intended to replace \fIxterm -L\fP and most
uses of \fIxinit\fP.  Site administrators should study the documentation and
sample configurations to tailor \fIxdm\fP to their particular environments.
Several sample configurations are provided underneath the \fIconfig\fP
subdirectory.
.IP "\fIclients/xdpyinfo/\fP"
This directory contains
a utility for printing information about a display's visuals and screens.
.IP "\fIclients/xedit/\fP"
.br
This directory contains
a simple text editor built on top of the Athena Text widget and X Toolkit.
.IP "\fIclients/xev/\fP"
.br
This directory contains
a program for examining the contents of events and how they are generated.
.IP "\fIclients/xfd/\fP"
.br
This directory contains
a utility for displaying the characters in a font.  
.IP "\fIclients/xhost/\fP"
.br
This directory contains
a utility for controlling access to the display on a host by host
basis.
.IP "\fIclients/xinit/\fP"
.br
This directory contains a utility for
starting an X server and an initial client (usually a
terminal emulator) on systems that don't have support for doing so from
\fI/etc/init\fP.  Site administrators are expected to create user-friendly
startup scripts as \fIxinit\fP isn't intended for novice users.
.IP "\fIclients/xkill/\fP"
.br
This directory contains a utility for getting rid of unwanted windows.
.IP "\fIclients/xload/\fP"
.br
This directory contains
a program for monitoring machine load averages.  It displays a histogram of
the most recent load averages and is often used to monitor machines in a
network.  It uses the Load widget from the Athena widget set, which will 
need to be modified if you are porting X to a new operating system.
.IP "\fIclients/xlogo/\fP"
.br
This directory contains a 
program that uses the Logo widget from the Athena widget set to 
display the X Window System logo.
.IP "\fIclients/xlsfonts/\fP"
.br
This directory contains a utility for listing the fonts that are available 
on a given server.  If you request long listings
from a server with many compressed fonts, your display may pause for
a long time.
.IP "\fIclients/xlswins/\fP"
.br
This directory contains a utility for listing windows on the display.  It is
useful for locating windows that might have popped off the screen for some
reason.
.IP "\fIclients/xmag/\fP"
.br
This directory contains a utility for magnifying parts of the display when
debugging graphics.  It may not work properly if more than one type of visual
is displayed on the screen at once.
.IP "\fIclients/xman/\fP"
.br
This directory contains a utility for displaying manual pages.  A small amount
of effort is required to add the appropriate emulation for the local operating
system's \fIman\fP program.
.IP "\fIclients/xmh/\fP"
.br
This directory contains a visual interface to the MH mail handling system.
It makes extensive use the X Toolkit and the Athena widget set.
.IP "\fIclients/xmodmap/\fP"
.br
This directory contains
a utility for displaying changing the keyboard, modifier, or pointer maps.
It reads a scripts of commands and is usually run when you login or first
start up X.
.IP "\fIclients/xpr/\fP"
.br
This directory contains
utilities for printing screen dumps of images obtained with \fIxwd\fP.
.IP "\fIclients/xprop/\fP"
.br
This directory contains
a utility for examining the properties attached to a given window.
.IP "\fIclients/xpseudoroot/\fP"
.br
This directory contains a prototype implementation of pseudo-root windows as
described in the previous version of the \fIInter-Client Communication 
Conventions Manual\fP (ICCCM).  It is provided solely for experimentation and
\fBis guaranteed to change\fP when the final ICCCM specification is adopted.
.IP "\fIclients/xrdb/\fP"
.br
This directory contains
a utility for loading user-specified default resource definitions into the server.
It is usually run when you login or first start up X.
.IP "\fIclients/xrefresh/\fP"
.br
This directory contains
a simple utility for refreshing all or part of your screen.
.IP "\fIclients/xset/\fP"
.br
This directory contains
a utility for setting various personal preferences for 
keyclick, pointer acceleration, etc.  It now supports adding and deleting
entries from the font path.
.IP "\fIclients/xsetroot/\fP"
.br
This directory contains a utility for setting the background of the root 
window.
.IP "\fIclients/xterm/\fP"
.br
This directory contains
a VT102 and Tektronix 4014 terminal emulator.  Many, many bugs have been
fixed since the previous release.  The VT102 mode now uses the X Toolkit
translation
manager (making arbitrary rebinding of keys possible), selections (see
\fIxclipboard\fP and \fIxcutsel\fP), and sets its process group correctly.
.IP "\fIclients/xwd/\fP"
.br
This directory contains
a utility for dumping images of windows.  It is usually used to 
take a snapshop of a window for printing or for later displaying with the
\fIxwud\fP program.  Some servers still have problems with XY format images,
and most run quite slowly.
.IP "\fIclients/xwininfo/\fP"
.br
This directory contains 
a utility for examining the various attributes of a window (such
as size, placement, window manager information, etc.).  It is frequently used
with \fIxprop\fP to examine the window hierarchy.
.IP "\fIclients/xwud/\fP"
.br
This directory contains
a utility for displaying images that were previously creating using
the \fIxwd\fP utility.
.IP "\fIdemos/\fP"
.br
This directory contains several programs that are fun to watch, but are not
of much use beyond that.
.IP "\fIdemos/ico/\fP"
.br
This directory contains
a neat program that rolls a many-sided (you get to choose among several)
object around the screen.
.IP "\fIdemos/maze/\fP"
.br
This directory contains a rather attractive demonstration of maze solving.
.IP "\fIdemos/muncher, demos/plaid/\fP"
.br
These directories contain programs for drawing cute pictures.
.IP "\fIdemos/puzzle/\fP"
.br
This directory contains a ``rearrange the tiles'' puzzle game for X11.
If you have a
color display, try running \fIpuzzle -picture mandrill.cm\fP from the
directory in which it is built.
.IP "\fIdemos/xeyes/\fP"
.br
This directory contains an attentive program inspired by a NeWS demo seen at
SIGGRAPH '88.
.IP "\fIdoc/\fP"
.br
This directory contains all of the documents describing the standard 
distribution.  Manual pages for the various clients can be found in the 
individual program source directories. 
.IP "\fIdoc/HelloWorld/\fP"
.br
This directory contains David Rosenthal's USENIX '88 \fIHello, World\fP 
paper on using toolkits.
.IP "\fIdoc/Protocol/\fP"
.br
This directory contains the official specification of the X Protocol.  
This is the final authority of what is and is not part of the X protocol
and is part of the X standard.
.IP "\fIdoc/Server/\fP"
.br
This directory contains several guides to porting the X server to new
platforms.
.IP "\fIdoc/Xaw/\fP"
.br
This directory contains a guide for the Athena widget set.
.IP "\fIdoc/Xlib/\fP"
.br
This directory contains the source for the Xlib programming guide and 
manual pages.
There is an untested script called \fIdoc/Xlib/Xman/expand.names\fP that will
rename the manual pages to match the names of the routines that they describe.
.IP "\fIdoc/Xt/\fP"
.br
This directory contains the Specification for the X Toolkit Intrinsics.  The
Intrinsics are now part of the X standard; any vendor that ships Xlib is
expected to ship Xt as well.
.IP "\fIdoc/bdf/\fP"
.br
This directory contains the Bitmap Distribution Format for describing fonts.
BDF is now part of the X standard; all server vendors are expected to ship
a program to convert fonts in this format to whatever internal formats they
prefer.
.IP "\fIdoc/bugs/\fP"
.br
This directory contains a template for submittings bugs reports (see 
\fIdoc/bugs/bug-report\fP).  Please use this form when reporting bugs in the
supported distribution to \fIxbugs@expo.lcs.mit.edu\fP.
.IP "\fIdoc/extensions/\fP"
.br
This directory contains documentation for possible extensions to the core
protocol and base libraries.  It currently includes the proposed PEX 
3d graphics extension and an input synthesis extension.
.IP "\fIdoc/fontnames/\fP"
.br
This directory contains the specification for font naming proposal under
evaluation by the X Consortium.  This is the format used in naming the fonts
donated by Adobe Systems, Inc. and Digital Equipment Corporation and by
Bitstream, Inc.
.IP "\fIdoc/releasenotes/\fP"
.br
This directory contains the sources for this document.
.IP "\fIdoc/tutorials/\fP"
.br
This directory contains essays on how to use particularly troublesome parts 
of the X Window System.  Contributions are always welcome.
.IP "\fIexamples/\fP"
.br
This directory contains various example programs that don't belong anywhere
else.
.IP "\fIexamples/CLX/\fP"
.br
This directory contains several samples of how to use the CLX Common Lisp
X interface.
.IP "\fIexamples/Xaw/\fP"
.br
This directory contains several small programs that test out the various
Athena widgets.
.IP "\fIextensions/\fP"
.br
This directory tree contains the source code for several sample server 
extensions:  a spline drawing request (\fIbezier\fP), a package for using
live video in X
(\fIplxvideo\fP), the beginnings of an extension for doing
input recording and synthesis (\fIxtest1\fP),
and a trapezoid drawing request (\fIzoid\fP).
.IP "\fIextensions/include/\fP"
.br
This directory contains header files for the sample extensions.
.IP "\fIextensions/lib/\fP"
.br
This directory contains the client library routines for communicating with
the extension packages.
.IP "\fIextensions/server/\fP"
.br
This directory contains the server routines for implementing the extensions.
.IP "\fIextensions/test/\fP"
.br
This directory contains several example programs for testing the extensions.
.IP "\fIfonts/\fP"
.br
This directory tree contains the sources for various utilities for 
creating and manipulating font files.  The user-contributed distribution
has a variety of translators for converting fonts from various formats to
BDF.
.IP "\fIfonts/bdf/\fP"
.br
This directory tree contains the source for a nice selection of fonts.
Starting with this release, organizing fonts into directories is rather
important given the length of the font names and the ability to have aliased
and wildcarded names for fonts.  As the BDF format is now part of the X
standard, all server vendors are expected to ship compilers to convert
BDF files to the appropriate packed font format for their servers.  By default,
the sample server will have all three of the font directories found here
in the font path.  If the resolution of the main display is less than 88 
dots per inch (as set in the server's \fI.macros\fP file), 
the 75dpi fonts will preceed the 100dpi fonts in the path.
Otherwise, the 100dpi fonts will appear first in the font path.
.IP "\fIfonts/bdf/misc/\fP"
.br
This directory contains the cursor and fixed width fonts from the previous
releases.  It is intended for fonts that should always be in the font
path.
.IP "\fIfonts/bdf/75dpi/\fP"
.br
This directory contains fonts donated by Adobe Systems, Inc. and
Digital Equipment Corporation and by Bitstream, Inc. designed for 
75 dot per inch monitors.  A nice selection
of families, weights, and sizes are provided.  These fonts follow the new
font naming convention; most users will want to learn how to properly use 
wildcards.
.IP "\fIfonts/bdf/100dpi/\fP"
.br
This directory contains versions of the 75dpi fonts for 100 dot per inch
monitors.  Some families at this resolution were not ready in time for this
release.
.IP "\fIfonts/bdftosnf/\fP"
.br
This directory contains the program used to compile fonts in BDF format
into the packed Server Natural Format used by the sample server.
.IP "\fIfonts/mkfontdir/\fP"
.br
This directory contains a new utility for creating the font database that
the server uses to map fontnames to filenames.  Whenever fonts are added
or removed from a directory, this program \fBmust\fP be run in that directory
to rebuild the database.
.IP "\fIlib/\fP"
.br
This directory contains all of the major libraries in the standard
distribution. 
.IP "\fIlib/CLX/\fP"
.br
This directory contains the CLX Common Lisp X package.  This is a native
Common Lisp interface to the X protocol (i.e. it doesn't use Xlib) that 
will be under review 
by the X Consortium for possible inclusion in the X standard.
.IP "\fIlib/X/\fP"
.br
This directory contains the Xlib C language programming library.  
The interface provided by this package
is part of the X standard (i.e. any vendor that ships any C interface
must ship Xlib in order to call its product X).  Vendors are free to change
the internals, but the interface must remain the same.  Several new routines
have been added since the last release to provide access to elements of
opaque data structures.
.IP "\fIlib/X/sysV/\fP"
.br
This directory contains routines used to make Xlib work on System V-based 
machines.
.IP "\fIlib/X/mips/\fP"
.br
This directory contains routines used to make Xlib work on platforms
manufactured by Mips Computer Systems.
.IP "\fIlib/Xaw/\fP"
.br
This directory contains the Athena widget set.  It is the beginning of
a growing collection of user interface objects built on top of the X Toolkit
Intrinsics.  Widgets are used in building higher-level applications, and should
ideally provide as much mechanism and as little policy as possible (that is 
for user interface toolkits, user interface management systems, and
applications to implement).  A much more complete set developed by
Hewlett-Packard is available in the user-contributed distribution
(unfortunately, it has not yet been ported to the R3 standard 
X Toolkit Intrinsics).
.IP "\fIlib/Xmu/\fP"
.br
This directory contains a collection of miscellaneous routines for supporting
the MIT applications.  It is \fInot\fP part of the X standard; vendors are
not required to ship this library.  \fIXmu\fP uses external interfaces to all
libraries and should be portable to wide variety of systems.  It is currently
used by parts of the Athena widget set and by various clients.
.IP "\fIlib/Xt/\fP"
.br
This directory contains a sample implementation of the X Toolkit Intrinsics,
a collection of resource, event, and object managers that provide a mechanism 
for building user interfaces objects called \fIwidgets\fP.  The Intrinsics
have been adopted as part of the X standard.  Any vendor that ships \fIXlib\fP
is expected to ship \fIXt\fP.
.IP "\fIlib/oldX/\fP"
.br
This directory contains X11 implementations of the X10 \fIXDraw\fP (including
spline support) and \fIAssocTable\fP routines.
.IP "\fIrgb/\fP"
.br
This directory contains a sample RGB color database and a program to compile
it.  A better database is sorely needed, but nobody has volunteered one yet.
Gray levels and several new colors have been added.
.IP "\fIserver/\fP"
.br
This directory contains a sample server for the core protocol.  It should
match the X Protocol Specification fairly closely, but the Specification is
the final authority.  Read the documentation in the \fIdoc/Server\fP 
directory, any README files in the \fIserver/ddx/\fP directories, 
and study the code very hard before trying to make any changes.
.IP "\fIserver/ddx/\fP"
.br
This directory contains the device dependant libraries for a number of
different platforms.  Study any README files before 
building or installing any servers.
.IP "\fIserver/ddx/apollo/\fP"
.br
This directory contains full sources for building a server that runs on
monochrome and color Apollo displays.  The README file gives hints and
prerequisites for building this server.
.IP "\fIserver/ddx/cfb/\fP"
.br
This directory contains a sample ``color frame buffer'' library for getting
color ports off the ground.  After a server is running, it should then be 
optimized to use any available hardware support.  
The cfb code is very slow, but very portable.
.IP "\fIserver/ddx/dec/\fP"
.br
This directory contains full sources for building servers for both the 
monochrome (qvss) and color (qdss) displays on the Vaxstation II, 2000, and
3000 series displays.  It also contains routines for controlling and 
interpretting LK201 keyboards.
.IP "\fIserver/ddx/hp/\fP"
.br
This directory contains full sources for building a server that runs on 
the HP 9000/300 series platforms with Topcat displays.
.IP "\fIserver/ddx/ibm/\fP"
.br
This directory contains sources for building a server that runs on the
APA16 and Megapel displays under IBM AOS (but not under AIX).
.IP "\fIserver/ddx/macII/\fP"
.br
This directory contains sources for building a server than runs on the
Apple Macintosh II under A/UX.
.IP "\fIserver/ddx/mfb/\fP"
.br
This directory contains a portable driver for monochrome frame buffer displays.
It is primarily intended for platforms that have no special graphics hardware
and as an initial porting base.  If hardware support is available,
it should be used wherever possible.
.IP "\fIserver/ddx/mi/\fP"
.br
This directory contains a machine independent implementation of the various
graphics operations.  It is frequently used with the mfb and cfb libraries in
porting the server to new platforms and in manipulating in-memory pixmaps.
It is designed to be very portable.
.IP "\fIserver/ddx/ndx/\fP"
.br
This directory contains sources for building a server with no input or output
devices.  It uses the generic monochrome and color frame buffer code and is 
useful for testing the device-independent parts of the server.
.IP "\fIserver/ddx/plx/\fP"
.br
This directory contains sources for building a server that runs on Parallax
video graphics controllers attached to Suns or Vaxes.  It also uses the
Parallax Video Extension in the \fIextensions/server/\fP directory.
.IP "\fIserver/ddx/snf/\fP"
.br
This directory contains the routines for manipulating SNF fonts.
Most servers use the SNF out of convenience, but there
is no requirement that they do so.
.IP "\fIserver/ddx/sun/\fP"
.br
This directory contains full sources for building a server that runs on 
Sun bw2 and various cg displays.  It uses the cfb library instead of a lot of 
device specific routines and is therefore somewhat slower than some of 
the other color servers in the supported distribution.  See the README file
in this directory before building or installing the Sun server.
.IP "\fIserver/dix/\fP"
.br
This directory contains the device independent portions of the sample X server.
It is highly recommended that you not change any code in this directory as it
will prevent you from upgrading to new versions.  If you find problems or have
a proposal for a change that would making porting to new platforms easier,
please send in a bug report as described in Section 3.
.IP "\fIserver/include/\fP"
.br
This directory contains header files that are used throughout the server.
.IP "\fIserver/os/\fP"
.br
This directory contains the various operating system-dependent portions of 
the server.
.IP "\fIserver/os/4.2bsd/\fP"
.br
This directory contains the routines needed to make the sample server run
under operating systems that support the 4.2bsd socket interfaces.
.IP "\fIserver/os/bsdemul/\fP"
.br
This directory contains emulations of commonly-used BSD routines.  It is 
typically used by System V based servers.
.IP "\fIutil/\fP"
.br
This directory contains various programs and scripts for building 
and configuring the release.  The programs in this section should be the 
first ported when bringing up X on a new platform.
.IP "\fIutil/bm-convert/\fP"
.br
This directory contains a filter for converting X10 format bitmap file to
X11 format (see \fIbitmap\fP for additional information).
.IP "\fIutil/checkfn/\fP"
.br
This directory contains a utility for checking for invalid filenames.  It
is typically used before preparing a distribution.
.IP "\fIutil/compress/\fP"
.br
This directory contains the sources for the BSD \fIcompress\fP program.  It
is provided for systems that don't already have it.
.IP "\fIutil/cpp/\fP"
.br
This directory contains sources for a public domain C preprocessor derived
from Martin Minow's DECUS \fIcpp\fP.  It is provided for systems whose
native \fIcpp\fP cannot handle the complexity of the server or the X Toolkit.
Users of A/UX 1.0 will need to build and install as described in the file
\fIserver/ddx/macII/R3setup.sh\fP before attempting to build this release.
.IP "\fIutil/imake/\fP"
.br
This directory contains the most important build utility.  The \fIimake\fP
program is used to generate the appropriate machine dependent Makefiles from
machine independent descriptions called Imakefiles.  It uses the C preprocessor
so that symbolic names and macro functions may be defined.  It has its own
handcrafted Makefile, and a special program for determining the initial
compilation flags.  If you are porting the release to a new machine,
you may need to edit \fIutil/imake/ccflags.c\fP to add any compiler flags that 
your machine will need when compiling \fIimake\fP.  \fIImake\fP will be built
automatically as part of \fImake World\fP.
.IP "\fIutil/imake.includes/\fP"
.br
This directory contains the configuration files used to generate the 
various \fIMakefiles\fP.  The README file describes which parameters can
be set and where.  Server-specific values may be given in the appropriate
.I \.macros 
files and site-wide values that differ from the defaults may be
given in the file \fIsite.def\fP.  The file \fIImake.tmpl\fP should \fBnot\fP
be changed.  If you change any of these parameters, you will need to do a
full rebuild to make them take effect.  If you are porting X to a new
machine, study the files in this directory \fIvery\fP carefully.
.IP "\fIutil/makedepend/\fP"
.br
This directory contains a program for automatically generating dependencies
for Makefiles.  It is used as part of the build process to ensure that the 
right files are recompiled whenever any source files are changed.  If the 
configuration
parameter \fICppSourcesPresent\fP is set to \fIYES\fP, \fImakedepend\fP will
have a copy of the C preprocessor built into it for speed.  This is not 
necessary and is not done in this release.  \fIMakedepend\fP is built
automatically as part of the ``make World'' phase.
.IP "\fIutil/patch/\fP"
.br
This directory contain's Larry Wall's \fIpatch\fP program, an extraordinarily
useful program for patching diffs into source code.  This program is not
built automatically in this release; it is provided simply for
the convenience of people who do not have access to the comp.sources.unix
archives.
.IP "\fIutil/scripts/\fP"
.br
This directory contains various useful scripts for tasks such as
installing software and generating link trees.
.IP "\fIutil/soelim/\fP"
.br
This directory contains a version of the \fIsoelim\fP program for those systems
that lack one.
.\".in -5
.\".ll +5
.LP
.NH 2
The user-contributed distribution
.LP
The user-contributed distribution contains the directories listed below.  Bugs
in this section of the release should be reported to the individual authors,
not to \fIxbugs\fP.
.\".ll -5
.\".in +5 
.IP "\fIcontrib/\fP"
.br
This directory is the top level of the user-contributed distribution.  Its
layout is meant to roughly parallel the core distribution.
.IP "\fIcontrib/clients/\fP"
.br
This directory tree contains a variety of generally useful programs.
.IP "\fIcontrib/clients/alertyorngs/\fP"
.br
This directory contains several utilities for popping up dialog windows 
to tell or ask the user for various information.
.IP "\fIcontrib/clients/gsh/\fP"
.br
This directory contains a graphical shell for novice users.
.IP "\fIcontrib/clients/hpxpr/\fP"
.br
This directory contains a version of \fIxpr\fP that can print on HP Laserjet
printers.
.IP "\fIcontrib/clients/kterm/\fP"
.br
This directory contains a version of \fIxterm\fP that can work with Kanji
fonts.
.IP "\fIcontrib/clients/magic/\fP"
.br
This directory contains X drivers for the \fImagic\fP VLSI design system.
.IP "\fIcontrib/clients/pbm/\fP"
.br
This directory contains Jef Poskanzer's Portable Bitmap Toolkit and a variety
of utilities for converting bitmaps between different formats.
.IP "\fIcontrib/clients/splot/\fP"
.br
This directory contains a plotting package.
.IP "\fIcontrib/clients/spy/\fP"
.br
This directory contains a yet another magnifying glass utility.
.IP "\fIcontrib/clients/texx/\fP"
.br
This directory contains a DVI previewer.
.IP "\fIcontrib/clients/x11startup/\fP"
.br
This directory contains a collection of scripts and default configuration files
that provide a nicer interface than plain \fIxinit\fP.
.IP "\fIcontrib/clients/xbgsun/\fP"
.br
This directory contains a utility for loading a Sun raster image onto the
root window background.
.IP "\fIcontrib/clients/xcalendar/\fP"
.br
This directory contains a program for managing ones calendar.
.IP "\fIcontrib/clients/xdvi/\fP"
.br
This directory contains yet another DVI previewing package.
.IP "\fIcontrib/clients/xdvorak/\fP"
.br
This directory contains a utility for remapping the keyboard to use
Dvorak-style bindings.
.IP "\fIcontrib/clients/xfig/\fP"
.br
This directory contains a drawing package.
.IP "\fIcontrib/clients/xim/\fP"
.br
This directory contains a program for displaying 8 and 24 bit images on 8 plane
displays.
.IP "\fIcontrib/clients/xipr/\fP"
.br
This directory contains a version of \fIxpr\fP for Imagen printers.
.IP "\fIcontrib/clients/xlock/\fP"
.br
This directory contains a program for locking up a display when not in use.
.IP "\fIcontrib/clients/xmessage\fP"
.br
This directory contains another program for displaying messages.  It is 
primarily used for error messages in X startup scripts and for leaving
reminders on other people's displays.
.IP "\fIcontrib/clients/xmore/\fP"
.br
This directory contains a version of the \fImore\fP pagination utility for X.
.IP "\fIcontrib/clients/xperfmon/\fP"
.br
This directory contains a utility for collecting and displaying system
statistics.  It is operating system specific and will require porting to
each particular platform.
.IP "\fIcontrib/clients/xpic/\fP"
.br
This directory contains a \fIpic\fP previewer.
.IP "\fIcontrib/clients/xplaces/\fP"
.br
This directory contains a tool for laying out and recording a user's initial
session.
.IP "\fIcontrib/clients/xpref/\fP"
.br
This directory contains a visual interface to several of the \fIxset\fP
functions.
.IP "\fIcontrib/clients/xshell/\fP"
.br
This directory contains an ancient program for invoking commands with single
keystrokes.
.IP "\fIcontrib/clients/xshowcmap/\fP"
.br
This directory contains a utility for displaying the colors in a colormap.
.IP "\fIcontrib/clients/xstring/\fP"
.br
This directory contains a program for displaying a string on another user's
display.
.IP "\fIcontrib/clients/xtools/\fP"
.br
This directory contains yet another program for laying out a user's initial
session.
.IP "\fIcontrib/clients/xtroff/\fP"
.br
This directory contains a \fItroff\fP previewer. 
.IP "\fIcontrib/clients/xwebster/\fP"
.br
This directory contains a visual interface to the SRI-NIC \fIwebster\fP
dictionary database.
.IP "\fIcontrib/demos/\fP"
.br
This directory tree contains a variety of somewhat amusing demo programs.
.IP "\fIcontrib/demos/paint/\fP"
.br
This directory contains a very simple paint program.
.IP "\fIcontrib/demos/psycho/\fP"
.br
This directory contains a multi-display version of \fIico\fP.
.IP "\fIcontrib/demos/spaceout/\fP"
.br
This directory contains another program for setting the root window background.
.IP "\fIcontrib/demos/worm/\fP"
.br
This directory contains a program for investigating random slither patterns.
.IP "\fIcontrib/demos/xcolors/\fP"
.br
This directory contains a program for displaying the available named colors.
.IP "\fIcontrib/demos/xfish/\fP"
.br
This directory contains another program for setting the root window background.
.IP "\fIcontrib/demos/xgranite/\fP"
.br
This directory contains still another program for setting the root window 
background.
.IP "\fIcontrib/demos/xphoon/\fP"
.br
This directory contains a cute program for displaying the current phase of 
the moon (on the root window background, of course).
.IP "\fIcontrib/demos/xrotmap/\fP"
.br
This directory contains a nasty program for rotating colormaps.
.IP "\fIcontrib/doc/\fP"
.br
This directory tree contains additional documentation.
.IP "\fIdoc/SharedLibs/\fP"
.br
This directory contains a proposed specification from AT&T on how to write
shared versions of the major C language libraries.  It is a proposal from
AT&T and is not part of the X standard.
.IP "\fIcontrib/extensions/\fP"
.br
This directory tree contains extension packages for the core protocol.
.IP "\fIcontrib/extensions/pex/\fP"
.br
This directory contains the current documents and header files for the PEX
proposed 3d graphics extension to X.
.IP "\fIcontrib/fonts/\fP"
.br
This directory tree contains utilities for creating and editing font files 
along with an eclectic collection of fonts.
.IP "\fIcontrib/fonts/bdf/\fP"
.br
This directory tree contains several collections of random fonts.  Most of them
are ugly.
.IP "\fIcontrib/fonts/bdf/bmug/\fP"
.br
This directory contains fonts converted from the Berkeley Mac Users Group
public domain software archives.
.IP "\fIcontrib/fonts/bdf/info-mac/\fP"
.br
This directory contains fonts converted from the INFO-MAC software archives.
.IP "\fIcontrib/fonts/bdf/oldx10/\fP"
.br
This directory contains BDF versions of many of the X10 fonts.
.IP "\fIcontrib/fonts/bdf/oldx11/\fP"
.br
This directory contains the fonts that were shipped in previous releases of
X11.  Most of them are rather ugly.
.IP "\fIcontrib/fonts/utils/\fP"
.br
This directory tree contains a variety of utilities for converting various
font formats to BDF as well as a two versions of a rudimentary tool for
splitting font files apart so that they can be edited.
.IP "\fIcontrib/games/\fP"
.br
This directory tree contains several interesting games that are useful for
getting people accustomed to X.
.IP "\fIcontrib/games/mazewar/\fP"
.br
This directory contains an implementation of the ancient and honorable 
\fImazewar\fP, the grandfather of most networked computer games.
.IP "\fIcontrib/games/qix/\fP"
.br
This directory contains an X version of the video arcade games of the same
name. 
.IP "\fIcontrib/games/xhanoi/\fP"
.br
This directory contains a program for solving the Tower of Hanoi problem.
.IP "\fIcontrib/games/xmille/\fP"
.br
This directory contains a very pretty computer version of the card game
Milles Bournes.
.IP "\fIcontrib/games/xpuzzle/\fP"
.br
This directory contains yet another puzzle program.
.IP "\fIcontrib/games/xsol/\fP"
.br
This directory contains an X version of the card game solitaire.
.IP "\fIcontrib/games/xtrek/\fP"
.br
This directory contains the infamous \fIxtrek\fP.  It requires System V
shared memory interfaces in order to work.
.IP "\fIcontrib/hacks/\fP"
.br
This directory tree contains several programs of questionable usefulness.
.IP "\fIcontrib/hacks/arctest/\fP"
.br
This directory contains a simple program for testing arcs.
.IP "\fIcontrib/hacks/reborder/\fP"
.br
This directory contains a little utility for resetting the borders of windows
after an anti-social window manager dies.
.IP "\fIcontrib/hacks/xbounce/\fP"
.br
This directory contains a program for dribbling windows on the screen.
.IP "\fIcontrib/hacks/xchcursor/\fP"
.br
This directory contains a program for cycling the cursor through different
patterns.
.IP "\fIcontrib/hacks/xsetsize/\fP"
.br
This directory contains a utility for moving, resizing, and iconifying 
windows from the command line.  It is the prime example of a program that
violates the \fIInter-Client Communications Conventions\fP and probably will
not work with most window managers.
.IP "\fIcontrib/server/\fP"
.br
This directory tree contains code and documentation for items that have not
been integrated into the sample server.
.IP "\fIcontrib/server/sgi/\fP"
.br
This directory tree contains patches to the sample server as well as the
machine-specific drivers needed to make the R3 server run on Silicon Graphics
workstations.
.IP "\fIcontrib/server/speedups/\fP"
.br
This directory contains suggested ways of optimizing the sample server.  The
emphasis to date in the server has been on accuracy instead of performance.
There are a great many places where substantial improvements could be made.
.IP "\fIcontrib/server/veryoldxpc/\fP"
.br
This directory contains the results of an old attempt to port a very early
version of the X11 server to the IBM-PC under MS-DOS.  Substantial amounts of
work will be necessary to make it at all useful.  This package will not
be included in future releases unless substantial progress is made on 
bringing it up to date.
.IP "\fIcontrib/toolkits/\fP"
.br
This directory tree contains several native toolkits (i.e. not built on top of
other toolkits).
.IP "\fIcontrib/toolkits/InterViews/\fP"
.br
This directory contains a new version of the InterViews C++ toolkit
from Stanford University.
.IP "\fIcontrib/toolkits/Xr11/\fP"
.br
This directory contains a new version of the X-Ray toolkit from
Hewlett-Packard.
.IP "\fIcontrib/toolkits/andrew/\fP"
.br
This directory contains a new version of the Andrew toolkit from 
Carnegie-Mellon University.
.IP "\fIcontrib/toolkits/clue/\fP"
.br
This directory contains a sample implementation of the Common Lisp User
Environment from Texas Instruments.
.IP "\fIcontrib/widgets/\fP"
.br
This directory tree contains a variety of widgets.  Some of them are based 
on the R2 \fIXt\fP Intrinsics and others are based on the current Intrinsics.
.IP "\fIcontrib/widgets/Dclock/\fP"
.br
This directory contains a digital clock widget based on the R2 Intrinsics.
.IP "\fIcontrib/widgets/Mailwatch/\fP"
.br
This directory contains a better mailbox widget based on the R2 Intrinsics.
.IP "\fIcontrib/widgets/MenuBox/\fP"
.br
This directory contains a prototype of menu widget that will eventually
become part of the Athena widget set.  It is based on the R3 Intrinsics.
.IP "\fIcontrib/widgets/Xhp/\fP"
.br
This directory contains a large, integrated widget set from Hewlett-Packard
and version of the R2 X Toolkit Intrinsics upon which these widgets
are temporarily based.
.IP "\fIcontrib/widgets/Xsw/\fP"
.br
This directory contains a collection widgets from Sony which are also based
on the R2 Intrinsics.
.IP "\fIcontrib/widgets/cpicker/\fP"
.br
This directory contains a widget for selecting and modifying colors
based on the R2 Intrinsics.
.IP "\fIcontrib/widgets/tblwidget/\fP"
.br
This directory contains a geometry manager widget that uses \fItbl\fP-style
formatting commands to layout child windows based on the R2 Intrinsics.
.IP "\fIcontrib/widgets/widgeteditor/\fP"
.br
This directory contains a simple widget editor based on the R2 Intrinsics.
.IP "\fIcontrib/widgets/widgetwrap/\fP"
.br
This directory contains a utility routine for create widgets and setting
their arguments.
.IP "\fIcontrib/widgets/xpalette/\fP"
.br
This directory contains a widget for displaying colors based on the R2 
Intrinsics.
.IP "\fIcontrib/windowmgrs/\fP"
.br
This directory tree contains several of the more popular window managers.
.IP "\fIcontrib/windowmgrs/awm/\fP"
.br
This directory contains the \fIawm\fP window manager.
.IP "\fIcontrib/windowmgrs/rtl/\fP"
.br
This directory contains the \fIrtl\fP tiling window manager.
.IP "\fIcontrib/windowmgrs/twm/\fP"
.br
This directory contains the \fItwm\fP window manager.
.IP "\fIcontrib/windowmgrs/wm/\fP"
.br
This directory contains the old \fIwm\fP window manager.  It is provided for
primarily for historical reasons.
