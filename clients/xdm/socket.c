/*
 * xdm - display manager daemon
 *
 * $XConsortium: socket.c,v 1.6 88/10/20 17:37:36 keith Exp $
 *
 * Copyright 1988 Massachusetts Institute of Technology
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted, provided
 * that the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of M.I.T. not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  M.I.T. makes no representations about the
 * suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 *
 * Author:  Keith Packard, MIT X Consortium
 */

/*
 * socket.c
 *
 * this code is not ready yet, it is included to support the
 * eventual implementation.  The protocol must still be specified
 * and agreed upon before I embody it in code.
 */

# include "dm.h"

#ifdef UDP_SOCKET
# include	<sys/types.h>
# include	<sys/socket.h>
# include	"buf.h"

int	socketFd = -1;

FD_TYPE	WellKnownSocketsMask;
int	WellKnownSocketsMax;


CreateWellKnownSockets ()
{
	struct sockaddr_in	sock_addr;

	if (request_port == 0)
		return;
	Debug ("creating socket %d\n", request_port);
	socketFd = socket (AF_INET, SOCK_DGRAM, 0);
	if (socketFd == -1) {
		LogError ("socket creation failed\n");
		return;
	}
	RegisterCloseOnFork (socketFd);
	sock_addr.sin_family = AF_INET;
	sock_addr.sin_port = htons ((short) request_port);
	sock_addr.sin_addr.s_addr = htonl (INADDR_ANY);
	if (bind (socketFd, &sock_addr, sizeof (sock_addr)) == -1)
		LogError ("error binding socket address %d\n", request_port);
	else {
		WellKnownSocketsMax = socketFd;
		FD_SET (socketFd, &WellKnownSocketsMask);
	}
}

AnyWellKnownSockets ()
{
	return socketFd != -1;
}

WaitForSomething ()
{
	FD_TYPE	reads;
	int	nready;

	Debug ("WaitForSomething\n");
	if (socketFd) {
		reads = WellKnownSocketsMask;
		nready = select (WellKnownSocketsMax + 1, &reads, 0, 0, 0);
		if (nready > 0 && FD_ISSET (socketFd, &reads))
			ProcessRequestSocket (socketFd);
	} else
		pause ();
}

/*
 * respond to a request on the UDP socket.  The protocol here
 * is under development, see the file "protocol" for some
 * uninspired ideas
 */

ProcessRequestSocket (fd)
int	fd;
{
}

/*
 * send a control message to a managed server.
 */

serverMessage (d, text)
struct display	*d;
char		*text;
{
}
#endif
