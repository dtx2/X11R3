/*
 * xdm - display manager daemon
 *
 * $XConsortium: dm.c,v 1.10 88/11/17 17:04:50 keith Exp $
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
 * display manager
 */

# include	<stdio.h>
# include	<sys/signal.h>
# include	"dm.h"
# include	"buf.h"

main (argc, argv)
int	argc;
char	**argv;
{
	int	CleanChildren (), TerminateAll (), RescanServers ();

	/*
	 * Step 1 - load configuration parameters
	 */
	InitResources (argc, argv);
	LoadDMResources ();
	if (debugLevel == 0 && daemonMode)
		BecomeDaemon ();
	InitErrorLog ();
	StorePid ();
	signal (SIGTERM, TerminateAll);
	signal (SIGINT, TerminateAll);
	/*
	 * Step 2 - Read /etc/Xservers and set up
	 *	    the socket.
	 *
	 *	    Keep a sub-daemon running
	 *	    for each entry
	 */
	ScanServers ();
	signal (SIGHUP, RescanServers);
#ifdef UDP_SOCKET
	CreateWellKnownSockets ();
	signal (SIGCHLD, CleanChildren);

	while (AnyWellKnownSockets () || AnyDisplaysLeft ()) {
		Debug ("WaitForSomething\n");
		WaitForSomething ();
	}
#else
	while (AnyDisplaysLeft ())
		WaitForChild ();
#endif
	Debug ("Nothing left to do, exiting\n");
}


ScanServers ()
{
	struct buffer	*serversFile;
	struct display	*d;
	int		fd;
	static DisplayType	acceptableTypes[] =
		{ { Local, Permanent, Secure },
		  { Local, Transient, Secure },
		  { Local, Permanent, Insecure },
		  { Local, Transient, Insecure },
		  { Foreign, Permanent, Secure },
		  { Foreign, Transient, Secure },
		};

	if (servers[0] == '/') {
		fd = open (servers, 0);
		if (fd == -1) {
			LogError ("cannot access servers file %s\n", servers);
			return;
		}
		serversFile = fileOpen (fd);
	} else {
		fd = -1;
		serversFile = dataOpen (servers, strlen (servers));
	}
	if (serversFile == NULL)
		return;
	while (ReadDisplay (serversFile, acceptableTypes,
 			    sizeof (acceptableTypes) / sizeof (acceptableTypes[0]),
		 	    (char *) 0) != EOB)
		;
	StartDisplays ();
	bufClose (serversFile);
	if (fd != -1)
		close (fd);
}

static
MarkDisplay (d)
struct display	*d;
{
	d->state = MissingEntry;
}

RescanServers ()
{
	Debug ("Caught SIGHUP, rescanning servers\n");
	ForEachDisplay (MarkDisplay);
	ScanServers ();
#ifdef SYSV
	signal (SIGHUP, RescanServers);
#endif
}

/*
 * catch a SIGTERM, kill all displays and exit
 */

TerminateAll ()
{
	void	TerminateDisplay ();

	ForEachDisplay (TerminateDisplay);
}

CleanChildren ()
{
#ifdef SYSV
	signal (SIGCHLD, CleanChildren);
#endif
	Debug ("CleanChildren\n");
	WaitForChild ();
}

/*
 * notice that a child has died and may need another
 * sub-daemon started
 */

WaitForChild ()
{
	int		pid;
	struct display	*d;
	waitType	status;

	pid = wait (&status);
	Debug ("pid: %d\n", pid);
	d = FindDisplayByPid (pid);
	if (d) {
		d->status = notRunning;
		switch (waitVal (status)) {
		case UNMANAGE_DISPLAY:
			Debug ("Display exited with UNMANAGE_DISPLAY\n");
			RemoveDisplay (d);
			break;
		case OBEYSESS_DISPLAY:
			Debug ("Display exited with OBEYSESS_DISPLAY\n");
			if (d->displayType.lifetime == Permanent)
				StartDisplay (d);
			else
				RemoveDisplay (d);
			break;
		default:
			Debug ("Display exited with unknown status %d\n", waitVal(status));
			StartDisplay (d);
			break;
		case REMANAGE_DISPLAY:
			Debug ("Display exited with REMANAGE_DISPLAY\n");
			StartDisplay (d);
			break;
		}
	}
}

static
CheckDisplayStatus (d)
struct display	*d;
{
	switch (d->state) {
	case MissingEntry:
		TerminateDisplay (d);
		break;
	case NewEntry:
		StartDisplay (d);
		break;
	case OldEntry:
		break;
	}
}

StartDisplays ()
{
	ForEachDisplay (CheckDisplayStatus);
}

StartDisplay (d)
struct display	*d;
{
	int	pid;

	Debug ("StartDisplay %s\n", d->name);
	switch (pid = fork ()) {
	case 0:
		signal (SIGCHLD, SIG_DFL);
		signal (SIGTERM, SIG_IGN);
		signal (SIGHUP, SIG_DFL);
		CloseOnFork ();
		ManageDisplay (d);
		exit (REMANAGE_DISPLAY);
	case -1:
		break;
	default:
		Debug ("pid: %d\n", pid);
		d->pid = pid;
		d->status = running;
		break;
	}
}

void
TerminateDisplay (d)
struct display	*d;
{
	DisplayStatus	status;
	int		pid;

	status = d->status;
	pid = d->pid;
	RemoveDisplay (d);
	if (status == running) {
		if (pid < 2)
			abort ();
		kill (pid, SIGTERM);
	}
}

static FD_TYPE	CloseMask;
static int	max;

RegisterCloseOnFork (fd)
int	fd;
{
	FD_SET (fd, &CloseMask);
	if (fd > max)
		max = fd;
}

ClearCloseOnFork (fd)
int	fd;
{
	FD_CLR (fd, &CloseMask);
	if (fd == max) {
		while (--fd >= 0)
			if (FD_ISSET (fd, &CloseMask))
				break;
		max = fd;
	}
}

CloseOnFork ()
{
	int	fd;

	for (fd = 0; fd < max; fd++)
		if (FD_ISSET (fd, &CloseMask))
			close (fd);
	FD_ZERO (&CloseMask);
	max = 0;
}

StorePid ()
{
	FILE	*f;

	if (pidFile[0] != '\0') {
		f = fopen (pidFile, "w");
		if (!f) {
			LogError ("process-id file %s cannot be opened\n",
				  pidFile);
		} else {
			fprintf (f, "%d\n", getpid ());
			fclose (f);
		}
	}
}
