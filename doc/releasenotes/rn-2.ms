.NH 1
Building the Release
.LP
The software in this release is divided into two distributions:
one for the core software that is supported by the staff of the X Consortium, 
and one for everything else.  Great pains have been taken to make the core 
distribution easy to reconfigure, build and install on a wide range of 
platforms.  The user-contributed distribution, on the other hand, has not
been compiled or tested by the staff of the X Consortium and will
require building by hand.  The \fIximake.sh\fP in \fIutil/scripts/\fP may
be useful for creating \fIMakefiles\fP from \fIImakefiles\fP.
.LP
Almost all \fIMakefiles\fP in the core software
are generated automatically by a utility called
\fIimake\fP.  Initial versions of all of the Makefiles are included
for those sites that cannot use \fIimake\fP (they will undoubtedly
require patching for the specific machine).  On many systems, X should build 
correctly right off the tape.  However, it can be reconfigured by simply
setting various parameters in the files \fIsite.def\fP and
\fI*.macros\fP in the directory \fIutil/imake.includes/\fP.
.LP
.NH 2
Installation Summary
.LP
To load and install this release of the X Window System, you will need to:
.\".ll -5
.\".in +5
.ne 4
.IP 1.
Finish reading these Release Notes.
.IP 2.
Create a directory into which you will read the distribution tapes (usually
named something like \fI/usr/local/src/X\fP or \fI/src/R3/\fP).  You will need
roughly 30 megabytes to hold the core software and up to 80 megabytes for
the user-contributed software.
.IP 3.
Unload the core tape into the directory created in step #2.  Since the
user-contributed software must be built by hand, you may wait and load it
in later.  Each of the tapes contain one (very large) UNIX \fItar\fP file 
stored at 1600 bits per inch.  
.IP 4.
Read the file \fIutil/imake.includes/README\fP for instructions on how to
configure the build for your particular site.  Also, make sure that you 
follow the directions in \fIREADME\fP files in \fIserver/ddx/\fP directories 
for which you plan to build servers.  
.IP 5.
If you plan to compile the release on more than one machine and have a 
distributed file system, you may wish to use the
script \fIutil/scripts/lndir.sh\fP to create symbolic link trees on each of
the target machines.  This allows all of the platforms on which you wish
to run X to share a single set of sources.  In either case, the phrase
\fIbuild tree\fP will be used to refer to the directory tree in 
which you are compiling (to distinguish it from the \fIsource tree\fP
which contains the actual files).
.IP 6.
If you are building on a Macintosh II under A/UX 1.0, 
make sure that you have run the \fIR3setup.sh\fP
from the \fIserver/ddx/macII/\fP directory.  This builds a new C preprocessor
from the public domain sources in \fIutil/cpp/\fP and fixes several misplaced
system files.  If you are building on a Sun, make sure that you set the
four \fIOS\fP parameters at the top of the file 
\fIutil/imake.includes/Sun.macros\fP.  These enable workarounds for
bugs in various SunOS compilers.  If you are building on an Apollo, make
sure that you are running rev 9.7.1 or later of the C compiler; otherwise, 
the server will not compile correctly.
.IP 7.
Make sure that you have followed all machine-specific directions and
that \fIimake\fP
has been configured for your machine (see the blocks of #ifdefs at the top 
of \fIutil/imake.includes/Imake.tmpl\fP in the source tree).
.IP 8.
Once you are satisfied with the configuration, you are ready to build the
core distribution.  Look at the \fI\&.macros\fP file in
\fIutil/imake.includes/\fP that applies to your system.  There should be a 
line near the top that sets a \fImake\fP variable named BOOTSTRAPCFLAGS.
If this variable is left blank, you may use the following command to
start the build:
.DS
%  make World >& make.world &
.DE
If it is not blank, you should append that definition to the command line.
This is used by \fIimake\fP to set particular \fIcpp\fP symbols for all
compiles (if you are porting to a different platform, see
\fIutil/imake/imake.c\fP).  Of the systems for which macro files are
provided in the core distribution, only A/UX needs this flag:
.DS
%  make BOOTSTRAPCFLAGS=-DmacII\ \ World\ \ >& make.world &
.DE
Do not call the output file \fImake.log\fP as the \fImake clean\fP done by
\fImake World\fP removes all files of this name.  This will rebuild all of
the \fIMakefiles\fP and execute a \fImake -k all\fP to compile everything 
in the core distribution.  This will take anywhere from 2 to 12 hours, 
depending on your machine. 
.IP 9.
When the \fImake\fP is done, check the log file for any problems.  There should
be no serious errors.  A/UX users may ignore compiler warning about 
enumeration type clashes and Apollo users may ignore optimizer warnings.
.IP 10.
If you are satisfied that everything has built correctly, 
test the various critical programs (servers, \fIxterm\fP, \fIxinit\fP,
etc.) by hand.  You may need to be root to run the server or \fIxterm\fP.
A second workstation or terminal will be useful if you run into problems.
.IP 11.
Make backup copies of your old X header files, binaries, fonts, libraries, etc.
.IP 12.
Go to the top of the build tree and type
.DS
%  make install\ \ >& make.install
.DE
You will either have to do this as root, or have write access to the
appropriate directories (see DIRS_TO_BUILD in the top level \fIImakefile\fP and
\fIutil/imake.includes/Imake.tmpl\fP).  The
\fIxterm\fP program should be installed setuid to root on most systems and
the \fIxload\fP program should be installed setguid to whatever group the
file \fI/dev/kmem\fP belongs to (it is installed setuid to root by default).
.IP 13.
If you would like to install the manual pages, type the following at the
top of the build tree:
.DS
%  make install.man
.DE
.IP 14.
If you would like to create and install lint libraries, type the following
at the top of the build tree:
.DS
%  make install.ln
.DE
.\".in -5
.\".ll +5
.LP
If you are installing X for the first time, you may also need to do 
some of the steps below listed below.
Check the various README files in the \fIserver/ddx\fP directories
for additional instructions.
.\".ll -5
.\".in +5
.IP 15.
Add device drivers or reconfigure your kernel.
.IP 16.
Create additional pseudoterminals.  See your operating system 
script \fI/dev/MAKEDEV\fP and site administrator for details.
.IP 17.
Read the manual page for the new Display Manager \fIxdm\fP.  This program
provides a portable way of running X automatically and has many hooks for
creating a nice interface for novice users.  This supercedes the support in
\fIxterm\fP for running the terminal emulator from \fI/etc/init\fP.
.IP 18.
Make sure that all X11 users have the BINDIR directory (usually
\fI/usr/bin/X11\fP), in their search paths.
.\".in -5
.\".ll +5
.LP
Release 3 of Version 11 of the X Window System should now be ready to use.
.LP
.ne 12
.NH 2
Operating System Requirements
.LP
One of the reasons why X is so popular is that is operating system-independent.
Although this distribution only contains sample implementations for Unix-based
platforms (because that's all we have), support for many non-Unix operating 
systems is available from a wide variety of vendors.  The servers in this 
release have been built on the following systems:
.sp
.ce 7
4.3+tahoe
Ultrix 3.0 FT2 (also compiles under Ultrix 2.0)
SunOS 3.4
HP-UX 6.01
Apollo Domain/IX 9.7 (and 9.6 according to its developers)
IBM AOS 4.3 (according to its developers)
A/UX 1.0
.sp
If you are using versions prior to these, you may run into trouble.
In particular, the server will not run on IBM 4.2A release 2.  The \fIREADME\fP
files in the various \fIserver/ddx/\fR describe particular requirements such 
as compilers, libraries, preprocessors, etc.  As was noted above, 
A/UX 1.0 users will need to build a new version of the C preprocessor and
Apollo users will need the 9.7.1 C compiler.
.LP
You should verify that your networking and interprocess 
communication facilities are working properly before trying to use X.
If programs such as \fItalk\fP and \fIrlogin\fP don't work, X probably
won't either.
.LP
.NH 2
Reading in the Release Tapes
.LP
This release may be obtained electronically from the DARPA Internet, 
the UUNET Project, several consulting firms, 
and various UUCP archive sites.  In addition, 
a set of three 2400 foot, 1600 BPI magnetic tapes is available from the
MIT Software Distribution Center (please do not ask SDC for cartridge tapes
or floppy disks).
.LP
Each tape from MIT contains one large \fItar\fP archive with
source for the software and documentation for part of the release.
If you have a limited amount of disk space, you should load the core tape, 
prune out any servers that you don't need, and generate listings of the 
user-contributed tapes for later retrieval.  
All filenames are given as relative paths (i.e. beginning
with a period instead of a slash) so that the release may be placed anywhere
in your file system.
.LP
.ne 8
Before reading in the tapes, make sure that you have enough disk space.  The
chart below shows the amount of space you will need for each tape:
.sp
.\"    tbl me!
.TS
center tab (/) ;
c c c c .
\fIDistribution/Contents/megabytes/status\fP

core/servers, libraries, utilities/35/required
contrib-1/programs, demos, fonts/36/recommended
contrib-2/contributed toolkits/34/recommended
.TE
.sp
The compiled programs will need roughly half to two thirds as much
space as the source code.  Thus, you will need roughly 60 megabytes to hold
the source and compiled versions of the programs in the core distribution.
.LP
.ne 8
Create a directory into which you will put all of the sources.  In this 
directory, execute the appropriate operating system commands to read in the
core tape.  If your site is set up so that \fItar\fP uses a 1600bpi tape
drive by default, you will probably type something like:
.sp
.DS
%  mkdir /usr/local/src/X
%  cd /usr/local/src/X
%  tar x
.DE
.sp
See your system administrator for help.
.LP
.NH 2
Using Symbolic Links
.LP
This release uses links (symbolic, on machines that support them) in 
several places to avoid duplication of certain files (mostly header files).  
If you are building this release on a system
for which configuration files have not been supplied, you should check
the LN configuration parameter in the appropriate
\fIutil/imake.includes/*.macros\fP file.  If your operating
system does not support soft links, LN should be set either to create hard
links or to copy the source file.
.LP
If you need to move the release to another machine after it has been built,
use \fItar\fP instead of \fIcp\fP or \fIrcp\fP so that you 
preserve dates and links.  This is usually done with a command of the form:
.sp
.DS
%  (chdir /usr/local/src/X; tar cf - .) | rsh othermachine "(chdir /moredisk/X; tar xpBf -)"
.DE
.sp
See your system administrator for help.
.LP
.NH 2
Configuring the Release
.LP
This release makes extensive use of a utility called \fIimake\fP to generate
machine-specific \fIMakefiles\fP from machine-independent \fIImakefiles\fP.
Another utility, called \fImakedepend\fP is used to generate \fIMakefile\fP
dependencies for C language files.  
Sample \fIMakefiles\fP are provided, although you are strongly urged to
use \fIimake\fP and \fImakedepend\fP if you have even the most remote interest
in portability.
.LP
The configuration files for \fIimake\fP are located in the directory
\fIutil/imake.includes\fP.  \fIMakefiles\fP are created from a template file
named \fIImake.tmpl\fP, a machine-specific \fI.macros\fP file, and a
site-specific \fIsite.def\fP file.  With only a few exceptions, configuration
parameters are \fIcpp\fP symbols that may be defined on a per-server basis
or for all servers in a given site.  The template file should \fInot\fP be
modified.
.LP
.ne 10
The file \fIutil/imake.includes/README\fP describes each of the build 
parameters and what value they should have.  The defaults have been chosen
to work properly on a wide range of machines and to be easy to maintain.
Site-specific configurations should be described in the file \fIsite.def\fP
using the following syntax:
.DS
#ifndef \fIBuildParameter\fP
#define \fIBuildParameter\fP \fIsite-specific-value\fP
#endif
.DE
.LP
.NH 2
Compiling the Release
.LP
Once the configuration parameters are set, you should be able to type the
following at the top of the build tree to compile the core software:
.DS
%  make World\ \ >& make.world\ \ &
.DE
Don't redirect the output to \fImake.log\fP as this
particular file is deleted as part of the build process.
This will take anywhere from 2 to 12 hours, depending on the machine used,
and should complete without any significant errors on most machines.
.LP
If you need to restart the build after all of the \fIMakefiles\fP 
and dependencies have been
created, type the following command at the top of the build tree:
.DS
%  make -k >& make.out &
.DE
If you later decide to change any of the configuration parameters, you'll need
to do another full \fImake World\fP.
.LP
.NH 2
Installing the Release
.LP
If everything compiles successfully, you may install the software by 
typing the following as root from the top of the build tree:
.DS
#  make install
.DE
If you would rather not do the installation as root, make the necessary
directories writable by you and do the install from your account.  Then, 
check the ownership and protections on \fIxterm\fP and \fIxload\fP in 
the BINDIR directory (usually \fI/usr/bin/X11/\fP).  \fIXterm\fP must be 
installed setuid to root so that it can set the ownership of its
pseudoterminal
and update \fI/etc/utmp\fP.  \fIXload\fP needs to be setuid to root or
setgid to the group owning the file \fI/dev/kmem\fP so that it can get
the system load average.
.LP
If your \fI/etc/termcap\fP and \fI/usr/lib/terminfo\fP databases don't have 
entries for \fIxterm\fP, sample entries are provided in the directory
\fIclients/xterm/\fP.  System V users will need to compile the \fIterminfo\fP
entry with the \fItic\fP utility.
.LP
If you plan to use the \fIxinit\fP program to run X, you might want to create
a link named \fIX\fP pointing to the appropriate server program (usually
named something like \fIXmachine\fP in the directory \fI/usr/bin/X11/\fP).
However, \fIxinit\fP is not intended for novice users; instead, 
site administrators are expected to either use \fIxdm\fP or
provide user-friendly interfaces.
.LP
If you would like to have manual pages installed, check the
\fIManDirectoryRoot\fP, \fIManDir\fP
and \fILibManDir\fP configuration parameters in \fIutil/imake.includes/\fP 
and type the following at the top of the build tree:
.DS
#  make install.man
.DE
.LP
If you would like to have lint libraries created and installed, type the
following at the top of the build tree:
.DS
#  make install.ln
.DE
.LP
Finally, make sure that all users have the BINDIR (usually \fI/usr/bin/X11/\fP)
in their PATH environment variable.
.LP
.NH 2
Notes on Kernels and Special Files
.LP
On some machines, it may be necessary to rebuild the kernel with a new
device driver, or to at least reconfigure it.  If you have never run X
before and are using a system not listed in these notes, you might need
verify that the CSR addresses in your kernel configuration file match
your hardware.  In addition, you should make sure that the kernel 
autoconfigures the display when booting.
.LP
You may need to create special devices for your display, mouse, or keyboard.
For example, 
.sp
.\"    tbl me!
.TS
center tab (&) ;
l c l .
#  /etc/mknod /dev/bell c 12 2&    &# for bell on Sun
#  MAKEDEV displays&    &# for displays on the RT/PC
.TE
.sp
The protection modes on the display device files should be set so that only 
the server can open them.  If the server is started by \fI/etc/init\fP, the
protections can be root read/write, everyone else no access; otherwise, they 
will have to be read/write for everyone or else your server will have to
be setuid to root.
.LP
On a Digital QVSS (VAXStation II), you should use \fIadb\fP to make sure 
that the kernel variable \fIqv_def_scrn\fP is set to 2 so that the full width
of the VR-260 monitor is used (otherwise there will be an unused black strip 
down the right edge of the screen).  This can be done by changing the
value either in \fI/vmunix\fP directly or in \fI/sys/vaxuba/qv.o\fP and 
relinking and reinstalling the kernel.  You will need to reboot for the new
value to take effect.
.LP
For more information, see the appropriate \fIREADME\fP files and manual pages
in the \fIserver/ddx/\fP directories.
.LP
.NH 2
Testing the Release
.LP
Even if you plan on using \fIxdm\fP to run X all the time, 
you should first run it by hand from another terminal to 
check that everything is installed and working properly.  Error messages
from the X server will then appear on your terminal, rather than being written
to the \fIxdm-errors\fP or to 
\fI/usr/adm/X?msgs\fP (where \fI?\fP is the number of the display).  
.LP
The easiest way to test the server is to go to \fI/usr/bin/X11\fP (or wherever
you have installed the various X programs), and run \fIxinit\fP as follows:
.DS
%  cd /usr/bin/X11
%  xinit
.DE
If all is well, you should see a gray stipple pattern covering the screen,
a cursor shaped like an ``X'' that tracks the pointer, and a terminal
emulator window.  Otherwise, check the following:
.IP 1.
If the gray background doesn't appear at all, check the
permissions on any special device files (usually stored in \fI/dev/\fP)
described in the \fIREADME\fP in the appropriate \fIserver/ddx/\fP 
subdirectories.
.IP 2.
If the background appears, but the cursor is a white square that doesn't
change, make sure that the fonts have been installed (in particular,
the font named \fIcursor.snf\fP in the directory
\fI/usr/lib/X11/fonts/misc/\fP; see the configuration parameter
\fIDefaultFontPath\fP).  Also make sure that there is a file named
\fIfonts.dir\fP in each font directory.  This file is created by the
\fImkfontdir\fP program and is used by the server to find fonts in a
directory.
.IP 3.
If the cursor appears but doesn't track the pointer, make sure that any
special device files (often named something like \fI/dev/mouse\fP)
are installed (see the server's \fIREADME\fP file).  
.IP 4.
If the server starts up and then goes black a few seconds later, the initial
client (usually \fIxterm\fP or \fIxdm\fP) is dying.  Make sure that \fIxterm\fP
is installed setuid to root and that you have created enough pseudoterminals.
If you are running \fIxinit\fP,  and have a file named \fI.xinitrc\fP in your
home directory, make sure that it is executable and that the last program
that it starts is run in the foreground (i.e. that there is no ampersand
at the end of the line).  Otherwise, the \fI.xinitrc\fP will finish
immediately, which \fIxinit\fP assumes means that you are through.
.LP
.ne 8
Once you have the initial window working properly, try running some other
programs from the \fIxterm\fP.  To position a new window with
the \fIuwm\fP window manager, press Button 1 (usually the left most button on
the pointer) when the flashing rectangle appears:
.DS
%  xclock -g 200x200-0+0 &
%  uwm &
%  xlogo &
%  xeyes &
\&\.\.\.
.DE
.LP
X should now be ready to use.  Read the manual pages for the new programs,
look at the new fonts, and have fun.
.LP
.ne 12
.NH 2
Creating Extra Pseudoterminals
.LP
Since each \fIxterm\fP will need a separate pseudoterminal, you should create a
large number of them
(you probably will want at least 32 on a small, multiuser system).
Each pty has two devices, a master and a slave, which are usually
named /dev/tty[pqrstu][0-f] and
/dev/pty[pqrstu][0-f].  If you don't have at least the ``p'' and ``q'' lines
configured (do an ``ls /dev''), you should have your system administrator add
them.  This is often done by running the MAKEDEV script in \fI/dev\fP:
.DS
#  cd /dev
#  ./MAKEDEV pty0
#  ./MAKEDEV pty1
.DE
.LP
.NH 2
Starting X from /etc/rc
.LP
A new utility named \fIxdm\fP is provided in this release for running X servers
from the system startup file \fI/etc/rc\fP.  It provides a portable, attractive
alternative to running X from \fI/etc/init\fP or \fIxinit\fP.  Designed to be
easily tailored to the needs of each specific site, \fIxdm\fP takes care of
keeping the server running, prompting for username and password and
managing the user's session.  The sample configuration currently uses shell 
scripts to provide a fairly simple environment.  This will be an area of
continuing work in future releases.
.LP
The key to \fIxdm\fP's flexibility is its extensive use of resources, allowing
site administrators to quickly and easily test alternative setups.  When
\fIxdm\fP starts up, it reads a configuration file (the default is
\fI/usr/lib/X11/xdm/xdm-config\fP but can be specified with the \fI-config\fP
command line flag) listing the names of the various datafiles, default
parameters, and startup and shutdown programs to be run.  Because it uses the 
standard X Toolkit resource file format, any parameters that may be set
in the \fIxdm-config\fP file may also be specified on the command line using
the standard \fI-xrm\fP option.  
.LP
.ne 10
The default configuration contains the following lines:
.sp
.\"    tbl me!
.TS
center tab (&) ;
l c l .
DisplayManager.servers: &    &/usr/lib/X11/xdm/Xservers
DisplayManager.errorLogFile: &    &/usr/lib/X11/xdm/xdm-errors
DisplayManager*resources: &    &/usr/lib/X11/xdm/Xresources
DisplayManager*startup: &    &/usr/lib/X11/xdm/Xstartup
DisplayManager*session: &    &/usr/lib/X11/xdm/Xsession
DisplayManager*reset: &    &/usr/lib/X11/xdm/Xreset
.TE
.sp
The \fIservers\fP file contains the list of
servers to start.  The \fIerrorLogFile\fP is where output from \fIxdm\fP
is redirected.  The \fIresources\fP file contains default resources for
the \fIxdm\fP login window.  In particular, this is where special key sequences
can be specified (in the \fIxlogin*login.translations\fP resource).  The
\fIstartup\fP file should be a program or executable script that is run
after the user has provided a valid password.  It is a hook for doing
site-specific initialization, logging, etc.  The \fIsession\fP entry is the
name of a session manager program or executable script that is run to start
up the user's environment.  A simple version has been supplied that provides
a simple \fIxterm\fP window and \fIuwm\fP window manager if the user does not
have an executable \fI.xsession\fP file in his or her home directory.  Finally,
the \fIreset\fP program or executable script is run after the user logs out.
It is a hook for cleaning up after the \fIstartup\fP program.
.LP
To run \fIxdm\fP using the default configuration, add the following line to
your system boot file (usually named \fI/etc/rc\fP or \fI/etc/rc.local\fP):
.DS
/usr/bin/X11/xdm &
.DE
Most sites will undoubtedly want to build their own configurations.  We 
recommend that you place any site-specific \fIxdm-config\fP and other
\fIxdm\fP files in a different directory so that they are not overwritten
if somebody ever does a \fImake install\fP.  If you were to store the files
in \fI/usr/local/lib/xdm\fP, the following command could be used to 
start \fIxdm\fP:
.DS
/usr/bin/X11/xdm -config /usr/local/lib/xdm/xdm-config &
.DE
.LP
Many servers set the keyboard to do non-blocking I/O under the assumption that
they are the only programs attempting to read from the keyboard.
Unfortunately, some versions of \fI/etc/getty\fP (A/UX's in particular) will
immediately see a continuous stream of zero-length reads which they interpret 
as end-of-file indicators.  Eventually, \fI/etc/init\fP will disable logins 
on that line until somebody types the following as root:
.DS
#  kill -HUP 1
.DE
Under A/UX, one alternative is to disable logins on the console 
and always
run \fIxdm\fP from \fI/etc/inittab\fP.  However, make sure that you save
a copy of the old \fI/etc/inittab\fP in case something goes wrong and you
have to restore logins from over the network or from single-user mode.
.LP
Another less drastic approach is to set up an account whose shell is the
\fIxdmshell\fP program found in \fIclients/xdm/\fP.  This program is not
installed by default so that site administrators will examine it to see if
it meets their needs.  The \fIxdmshell\fP utility makes sure that it is 
being run from the appropriate type of terminal, starts \fIxdm\fP, waits for
it to finish, and then
resets the console if necessary.  If the \fIxdm\fP resources file
(specified by the \fIDisplayManager*resources\fP entry in the 
\fIxdm-config\fP file) contains a binding to the \fIabort-display\fP action
similar to the following
.DS
xlogin*login.translations: #override  Ctrl<Key>R: abort-display()
.DE
the console can then by restored by pressing the indicated key 
(Control-R in the above example) in the \fIxdm\fP login window.
.LP
.ne 10
The \fIxdmshell\fP program is usually
installed setuid to root but executable only by members of a special group,
of which the account which has \fIxdmshell\fP as its shell is the only member:
.DS
%  grep xdm /etc/passwd
x:aB9i7vhDVa82z:101:51:Account for starting up X:/tmp:/etc/xdmshell
%  grep 51 /etc/group
xdmgrp:*:51:
%  ls -lg /etc/xdmshell
-rws--x---   1 root     xdmgrp     20338 Nov  1 01:32 /etc/xdmshell
.DE
.LP
If the \fIxdm\fP resources have not been configured to have a key bound to
the \fIabort-display()\fP action, there will be no way for general users to
login to the console directly.  Whether or not this is desirable depends on
the particular site.
.LP
.NH 2
Obsolete Support for Starting X from /etc/init
.LP
\fBWarning:  the following is provided for compatibility with older systems
and may not be supported in future releases.\fP
.LP
Ultrix and 4.3bsd use a new, expanded
format of the \fI/etc/ttys\fP configuration file that allows you to specify
a window system and initial program (usually a terminal emulator or session
manager) to be run.  Although it is preferable to use \fIxdm\fP, there is
support in \fIxterm\fP for starting X and an initial \fIxterm\fP window 
from \fI/etc/ttys\fP.
.LP
.ne 8
.NH 3
Creating the ttyv Terminals
.LP
Since most versions of \fI/etc/init\fP require an actual terminal line per
entry in \fI/etc/ttys\fP, you will need to dedicate one pseudoterminal 
for each display.  Although \fIxterm\fP normally
allocates a pty dynamically, the \fI-L\fP option 
may be given to force it to use
the pseudoterminal passed to it from \fI/etc/init\fP.
.LP
.ne 10
By convention, the pseudoterminal pair with the highest minor device 
number is renamed \fI[pt]tyv0\fP, the next highest \fI[pt]tyv1\fP, and so on 
for as many login windows as will be needed.  The highest are chosen instead
of the lowest so that they don't get in the way of the normal low to high
search that most programs do when allocating a pty.  On a small 
system that only
has ``p'' and ``q'' pseudoterminals, the following commands might be used
to set up ``v'' terminals for two displays:
.DS
#  cd /dev
#  mv ttyqf ttyv0 ; mv ptyqf ptyv0
#  mv ttyqe ttyv1 ; mv ptyqe ptyv1
.DE
See your system administrator for help.
.LP
.NH 3
Adding Window System Entries to /etc/ttys
.LP
Once you have renamed the pseudoterminals, you can add entries for them
in \fI/etc/ttys\fP.  Again, this only works on systems that have the new 
4.3bsd format for starting window systems, not on older systems
that use the 4.2bsd small entry format and \fI/etc/ttytype\fP.  If your machine
does not have the new format, rename the ttyv's and ptyv's back and go on to
the next section.
.LP
The \fIXserver\fP manual page gives a more detailed description of 
different ways to set up \fI/etc/ttys\fP
entries (also see your system documentation).  Usually, the ttyv's are
placed at the bottom of the file and look something like this:
.sp
.ce 1
ttyv0 "/usr/bin/X11/xterm -L -geometry 80x24+0+0 -display :0" xterm off window="/usr/bin/X11/X :0"
.sp
Note that unlike X10, the server number argument to the X server command must
be preceeded by a colon.  
Additional comand line options may be specified on either the \fIxterm\fP 
command line
or the \fIX\fP command line.  However, many versions of \fIinit\fP
have fairly small program name buffers, limiting the length of the entry.  
Also, some versions don't allow pound signs within entries, meaning that 
arbitrary numeric color specifications cannot be given.  This is why
\fIxdm\fP was written.
.LP
Once you have added or changed any entries, you need to signal \fIinit\fP to
reread \fI/etc/ttys\fP and restart.  This can be done as root by 
typing the following command:
.DS
#  kill -HUP 1
.DE
This will abort any existing processes on any changed lines before restarting,
so it should only be done by a system administrator.
