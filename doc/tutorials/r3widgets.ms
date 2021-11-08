.ds xT X Toolkit Intrinsics \- C Language Interface
.hw XtMake-Geometry-Request XtQuery-Geometry wid-get sub-class sub-classes
.nr PD 1.25v
.EF '\fBConverting Widgets\fP''\fBX Version 11\fP'
.OF '\fBConverting Widgets\fP''\fBX Version 11\fP'
.EH ''\fB- % -\fP''
.OH ''\fB- % -\fP''
.DA "October 26, 1988"
.TL
Converting Widgets from X11 Release 2 to X11 Release 3
.AU
Ralph R. Swick
.AI
Digital Equipment Corporation
MIT Project Athena
.AB
Many new features were added to the Xt Intrinsics\(dg
for Version 11 Release 3 of the X Window System.
.FS
\(dgSee \fI\*(xT\fP
.sp
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
In the majority of cases, correctly written 
applications will be source compatible with the Release 3 Xt; however
some source changes are necessary to widget implementations to accommodate
the new interfaces.  This document summarizes the changes required to
update a widget that correctly functions under the Release 2 library to
do the same under the Release 3 library.
.AE

.NH 1
Overview
.LP
Release 3 of the X Window System Version 11 includes the first version of
the Xt Intrinsics that have been approved and adopted as part of the core X
standard by the MIT X Consortium.  From the viewpoint of an applications
programmer, the Release 3 library is upward compatible with the library
distributed with X11 Release 2 with two small
exceptions, listed in the last 4 sections of this document.
.LP
The changes listed here are by no means a complete listing of all the
additions that have been made to the Intrinsics in Release 3.  Only
the changes that require source modifications to widgets and
(possibly) to applications are described.
.LP
Some new core fields and some small changes to old methods will require
attention in the source code of any widgets written to the Release 2
specification.  The following sections outline the new core methods, some
obsolete composite methods, and some changes to existing methods.
.LP
The reader should refer to the document \fI\*(xT\fP for the complete
description of the procedures listed in this document; only a summary of
the changes is presented here.
.LP
Of course, many bugs have been fixed in the Release 3 implementation
and it is entirely possible that an application that appeared to work
was actually leaning against such a bug and will now fall over.  The only
instance currently known to us of a fix that might break an application
is described in Section 11.

.NH 1
New \fBCore\fP Fields
.LP
Two new fields have been added to the end of the \fBCoreClassPart\fP
structure to
allow composite widgets to attach event translation bindings to their
descendants and to allow for future updates to \fBCoreClass\fP.
Revised Release 2 widgets can initialize both of these new fields to NULL.  If
the structure initialization followed the template used by the Athena
widgets in R2, the C compiler will do this initialization automatically
and source code modifications are not strictly required.

.SH 2
The display_accelerator Method
.LP
Xt now supports input accelerators.  Accelerators are translation
bindings attached to one widget that invoke actions in a different
widget, usually a parent.  
.LP
The display_accelerator method is called by the \fBXtInstallAccelerators\fP
procedure to notify the widget owning the accelerator table (i.e. the
widget in which the actions will be dispatched) that its accelerators
have been added to the event bindings of some other target widget.
.LP
Widgets that do not define accelerator tables should initialize the
display_accelerator field to \fBXtInheritDisplayAccelerator\fP.

.SH 2
The extension pointer
.LP
In order to allow for future expansion of the \fBCoreClassPart\fP
structure and maintain binary compatibility with then-existing widget
libraries, an extension field has been added after the
display_accelerators field.  This field should be initialized to NULL.

.NH 1
Obsolete \fBComposite\fP Fields
.LP
Two methods have been removed from the \fBCompositeClassPart\fP
structure.  Any widgets that are subclasses of \fBComposite\fP must be
modified to remove the initialization for these two fields before the
widget can be re-compiled.
.LP
The methods \fBmove_focus_to_next\fP and \fBmove_focus_to_prev\fP were
deemed to be insufficient for many desired styles of focus management
and were therefore removed.  If a widget library
desires to implement class
move_focus methods, it should declare a subclass of
\fBCompositeWidgetClass\fP and add the appropriate focus management
methods in the subclass.
.LP
The \fBnum_mapped_children\fP field has been removed from the
\fBCompositePart\fP structure.  This field is no longer used
internally by the Intrinsics and widgets that used it themselves
were probably using it incorrectly.
.LP
A new field, extension, was added to the end of \fBCompositeClassPart\fP
to allow for future growth.  It should have the value NULL.
.LP
Note that \fBConstraint\fP is a subclass of \fBComposite\fP and therefore
these changes apply to all Constraint widgets.

.NH 1
Auditing Data Types
.LP
The data types \fBDimension\fP and \fBPosition\fP are defined for the
fields core.width, core.height, core.border_width and core.x and core.y
respectively.  Many other fields in specific widgets are appropriately
defined as either \fBDimension\fP or \fBPosition\fP.  A common
programming error in Release 2 was to misdeclare some or all of these
fields as \fBint\fP, either in a structure declaration, or more subtly in
the size field in a resource list.\(dg
.FS
\(dgThis error was exacerbated by the Athena Widget documentation and header
files which misdeclared these fields as int.
.FE
.LP
In Release 3, it is critical that fields be consistently declared as
either \fBint\fP, \fBDimension\fP or \fBPosition\fP.  Good compilers,
\fIlint\fP, and other similar tools will catch some programming errors
but cannot check resource lists.  Unpredictable run-time errors will
result from fields that have been misdeclared.
.LP
In addition to auditing all resource lists, all calls to
\fBXtMakeResizeRequest\fP in widgets should be checked to make sure
the return value pointers are of type \fBDimension*\fP and
all uses of \fBXtGetValues\fP in
both widgets and applications should be checked to insure that the
pointers used in the argument lists reflect the correct data type for the
resource being retrieved.

.NH 1
Name Change to Header File
.LP
The header file \fB<X11/Misc.h>\fP exported by the Athena widgets has
been renamed to \fB<X11/XawMisc.h>\fP.  Any widgets or applications that
used it will need to refer to the new name.

.NH 1
Interface Change to Accept_Focus Method
.LP
An additional parameter and a return value have been added to the
accept_focus method.  The new parameter
indicates the X time of the event that caused the procedure to be
invoked, so that the procedure can correctly conform to the
\fIInter-Client Communications Conventions\fP.  The accept_focus
procedure must also return a Boolean value indicating whether or not the
widget actually wanted the input focus.

.NH 1
Data Change to Set_Values Method
.LP
It is frequently the case that a widget will need to make some request of
its parent from within its set_values procedure.  In Release 2, the
widget data structure was not updated to reflect the new resource values
until after all class set_values methods had been called.  In this case,
the parent was unable to perform any processing that depended upon the
new state of the child.
.LP
In Release 3, the widget data structure is updated with the new
resource values before calling the class set_values methods.  A copy is
made of the widget record before the changes are installed.
The definition of the arguments to the set_values method
has been changed so that the ``old'' argument now points to the copy
of the original widget and the ``new'' argument points to the actual
widget record.  Most simple set_values procedures will not need any
changes to work properly with this swap, but procedures which made
geometry requests or other changes that formerly required use of the
``old'' widget must be modified to use the ``new'' argument instead.
.LP
The SetValues procedure must not assume that the current geometry
(i.e. the ``new'' one) is the one they will actually get, as the parent's
GeometryManager has not yet been called.  A reasonable strategy for
widgets that need to perform some set_values computation based upon
their geometry is to use the old geometry and rely solely upon their
\fBresize\fP procedure to notify them of any changes after
\fBset_values\fP has been called.

.NH 1
New Geometry Request Mode
.LP
In order to accommodate some geometry layout semantics that may require a
parent to respond accurately with \fBXtGeometryAlmost\fP but
simultaneously propagate a geometry request up the tree, a new
option has been added to the values permitted in the request_mode field
of an \fBXtGeometryRequest\fP.
.LP
If a Composite widget geometry_manager procedure finds the
\fBXtCWQueryOnly\fP flag set in the request_mode, it is expected to
respond to the query in the normal way \fIbut not to actually make any
changes to its own geometry or the geometry of any of its children\fP.
The Composite widget can expect that the child will repeat the request
without the \fBXtCWQueryOnly\fP flag and may cache any layout state
required to quickly re-compute the new geometry.

.NH 1
Translation Table Syntax
.LP
Several additions have been made to the syntax permitted in translation
tables.  Any translation tables created for the Release 2 Intrinsics
should continue to function without change, but the widget may wish to
take advantage of the new capabilities to fix some obscure bugs.
.LP
In particular, it was impossible in the Release 2 translation syntax to
enter a portable (across keyboards) event binding for most special
characters.
The new
syntax allows, for example, portable key bindings to be made for the ``['' and
``]'' keys independently of whether they occur on the same keycap or on
separate keycaps on two different keyboards.  Other new capabilities in
the translation manager should be backwards compatible with Release 2.

.NH 1
Resource Name and Class Qualifiers
.LP
In Release 2, the name and class passed to \fBXtInitialize\fP were
prepended to the Shell widget name and class to form the full resource
name and class of any resources required by the widget hierarchy.  This
prevented a single process from creating separate widget hierarchies that
behaved as if they were separate applications.
.LP
In Release 3, the "application name" and "application class" are no
longer prepended to the ApplicationShell name and class.  When resources
are retrieved for the widget hierarchy, the instance name and class
of the ApplicationShellWidget created by
XtAppCreateShell (a new routine) become the left-most
resource name and class qualifiers.  The compatibility interface,
\fBXtInitialize\fP, arranges to return an \fBApplicationShellWidget\fP
with the appropriate instance name and class set from the
application_class argument and the command line options.
.LP
The effect of this change on applications written to the Release 2
specifications is that the Shell name/class level of the resource
name/class hierarchy has been removed.  The shell_name argument to
XtInitialize and the name argument to XtCreateApplicationShell are no
longer evaluated.  No source changes to application code should be
required, but resource files (e.g. app-defaults files) may need to be
modified if they
explicitly specify the Shell name and class in any resource
specifications.
.LP
In Release 2, the widget argument to \fBXtGetApplicationResources\fP was
ignored.  In Release 3, it must be specified as the
ApplicationShellWidget instance that identifies the name and class of the
application.  Applications that specified NULL for the widget argument
may not retrieve the resources that were expected.

.NH 1
Bug Fix to \fBXtGetValues\fP
.LP
An implementation bug in \fBXtGetValues\fP in Release 2 caused it to
store any resource shorter than sizeof(long) into the location specified
in the arglist as if the location were always declared as long.
.LP
Many applications (knowingly or unknowingly) relied on this bug to clear
the high-order bytes of fields that were misdeclared.  Other
applications encountered run-time errors when memory was cleared that
should not have been cleared.
.LP
In Release 3, it is critical to audit \fIall\fP calls to
\fBXtGetValues\fP in widgets and in applications and verify that the
correct data type is used in the declarations of the variables named in
the arglists.  The most common errors involved declaring an ``int'' field
for the value of \fBXtNwidth\fP or \fBXtNheight\fP (which are
\fBDimension\fP) or for the value of \fBXtNx\fP and \fBXtNy\fP (which are
both \fBPosition\fP).  Applications that do not call \fBXtGetValues\fP
will probably work correctly with only a re-compile.

.NH 1
Changes to Shell Defaults
.LP
The default value for the \fBXtNinput\fP resource of Shell widgets is now
\fBFalse\fP.  Applications that expect to receive input and that do not
perform their own X focus management may need to set this to \fBTrue\fP
(with \fBXtSetValues\fP) to operate under some window managers.
.LP
\fBXtRealizeWidget\fP now respects the \fBXtNmappedWhenManaged\fP
resource of Shell widgets.  This is unlikely to affect many
applications (except those which may have tried to work around the
automatic MapWindow request for Shells) but it is possible that an
overly-general resource specification in a resource database might
now prevent a hierarchy from being mapped when an application
expected other behavior.

.NH 1
Alternate Input Source and TimeOuts
.LP
In Release 2 it was not possible for an application to query the
Toolkit to discover whether or not there were X events pending
without also executing input or timer callbacks.  In Release 3, the
routines \fBXtPending\fP and \fBXtPeekEvent\fP will only return the
state of any X events and will not dispatch alternate input or
timers.
.LP
An application that relied upon timer events being dispatched by one
of these two routines will have to be modified to use one of the new
routines, \fBXtProcessEvent\fP or \fBXtAppProcessEvent\fP.
