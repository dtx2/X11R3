
		X Display Manager Control Protocol

	The X display manager (xdm) is designed to manage a collection of
local or remote displays.  The remote displays communicate with xdm
using the protocol described herein.  This protocol does not represent
any commitment on the part of MIT, nor is it expected to ever be
standardized in this form.  If you have comments and suggestions,
please send them to keith@expo.lcs.mit.edu.  The server semantics
for the magic authorized connection are also subject to change, if
you see any holes in them, please let me know.

	Xdmcp is designed to accept multiple copies of any message,
in any order, and should work using UDP.

	The message format is fixed, although some messages may
use portions of the format for different things:

	2	CARD16		Version number
	2	i		length of CMD
	i	STRING8		CMD
	2	j		length of DISPLAY
	j	STRING8		DISPLAY
	2	k		length of AUTH_NAME
	k	STRING8		AUTH_NAME
	2	l		length of AUTH_DATA
	l	STRING8		AUTH_DATA


CMD is an ISO Latin-1 string (case ignored) from the set:

	query			poll for availible managers
	willing			response to poll
	manage			service request
	unmanage		request for discontinuation of service
	unmanged		positive acknowledgement of unmanage
	protocol error		negative acknowledgement of any request

The manage command is followed by an optional list of display-type
parameters seperated from "manage" and each other by spaces:

	transient		request for single session (default)
	permanent		request for multiple sessions

	secure			unmanage must have correct auth data
	insecure		unmanage always suceeds

DISPLAY is a list of uninterpreted octects which can be passed to the
XOpenDisplay function to connect to the requesting display.

AUTH_NAME and AUTH_DATA are used by xdm to authenticate unmange requests and
are passed in the server connection setup dialog for the display to
authenticate the display manager.

The X server must recognise the display manager connection by checking
AUTH_NAME and AUTH_DATA; this connection must be handled specially:  when
the connection is made, an implicit grab-server is executed.  When the
connection is broken, the server must reset itself, disconnecting all
clients.  At this time, it can also request another session.

Any requesting display must have no other connected clients when xdm
requests its connection.  Either the other connections must be closed down
when this connection is made, or the display should disable connections
before sending the "manage" command and only accept authorized connections.

The query command is sent by the display to discover availible display
managers.  Xdm will always respond to the query with a "willing" response.

The manage command is sent by the display to request a session.

unmanage can be sent from anywhere and disables the given display.  If the
display is of type 'secure', the authorization name/data in the message must
match the name/data sent by the display in the manage request.

unmanaged is sent by xdm when an unmanage request is received and the
display can be unmanaged, or is already unmanaged.  All fields except the
command field are filled with the cooresponding field from the unmanage
packet.

protocol error is sent by xdm when any incomprehensible message is received,
or when any request fails.  All fields except the command field are filled
with the cooresponding field from the unmanage packet.

