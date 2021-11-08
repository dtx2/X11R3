.NH 1
Writing Portable X Software
.LP
A favorite saying around here goes:  There's no such thing as portable
software, only software that has been ported.  However, there are few rules
of thumb that go a long way towards making programs written for X easy to
getting running on a wide variety of machines:
.IP 1.
Keep all source filenames to 12 characters or fewer.  This is the maximum
number of characters that older System V file systems allow when using
a source code control system.
.IP 2.
Use \fIImakefiles\fP.  They are the only way to generate 
correct \fIMakefiles\fP.
The \fIximake.sh\fP shell script in \fIutil/scripts/\fP should make it 
relatively easy to create \fIMakefiles\fP outside of the core source tree.
The easiest way to construct an \fIImakefile\fP is to start with one that
does something similar.  The various macros that are used are defined in
the file \fIutil/imake.includes/Imake.rules\fP.
.IP 3.
If you absolutely must use \fIMakefiles\fP instead of \fIImakefiles\fP,
link against \fI-lX11\fP instead of \fI-lX\fP.  If you are using \fIimake\fP,
use the symbolic names $(XAWLIB), $(XMULIB), $(XTOOLLIB), and $(XLIB).
.IP 4.
Reference include files using the syntax \fI<X11/file.h>\fP instead of 
\fI"X11/file.h"\fP, \fI<X/file.h>\fP, or \fI"X/file.h".
.IP 5.
Include \fI<X11/Xos.h>\fP if you need \fItypes.h\fP,
\fIstring.h\fP or \fIstrings.h\fP (then use the routines \fIindex\fP and
\fIrindex\fP instead of \fIstrchr\fP and \fIstrrchr\fP), \fIfile.h\fP, 
and \fItime.h\fP.  
.IP 6.
If you need to put in System V vs. BSD dependencies, use \fI#ifdef SYSV\fP.
.IP 7.
Do not assume that the root window's Visual (returned by the 
\fIDefaultVisual\fP macro) is the only one available.  Some color screens
may use a black and white window for the root or could provide StaticColor
as well as PseudoColor visuals.  Unfortunately, most libraries do not have 
adequate support for locating visuals to use.  In the mean time, use
\fIXGetVisualInfo()\fP.
.IP 8.
Use \fI-display displayname\fP to specify the X server to contact.  Do not
simply assume that if a command line argument has a colon in it that it is
a \fIdisplayname\fP.  If you accept command line abbreviations, make sure that
you also accept the full \fI-display\fP.
.IP 9.
Use \fI-geometry geomspec\fP to specify window geometry.  Do not simply
assume that if a command line argument begins with an equal sign that it is
a window geometry.  If you accept command line abbreviations, make sure that
you also accept the full \fI-geometry\fP.
.IP 10.
Use the \fI.man\fP suffix for program manual page sources.
.IP 11.
If you are interested in contributing software to the MIT public release,
please use a copyright notice that is no more restrictive than the one
shown in the files \fI./COPYRIGHTS\fP and \fIcontrib/COPYRIGHTS\fP.
.LP
We hope you enjoy Release 3.
