/*
 * xdm - display manager daemon
 *
 * $XConsortium: display.c,v 1.10 88/11/17 17:04:43 keith Exp $
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
 * display.c
 *
 * this part manages a single display
 */

# include	"dm.h"
# include	<signal.h>
# include	<X11/Xlib.h>
# include	<setjmp.h>
# include	<errno.h>

#ifdef SYSV
#define killpg(pgrp, sig) kill(-(pgrp), sig)
#endif /* SYSV */

extern int	errno;

static jmp_buf	terminated;

static CatchTerm (), someoneDied (), abortOpen (), StartServer ();
static WaitForServer (), TerminateServer (), HupServer (), StartSession ();

extern unsigned sleep ();
extern void exit ();

static
CatchTerm ()
{
	Debug ("display manager caught SIGTERM\n");
	longjmp (terminated, 1);
}

static
CatchHup ()
{
	Debug ("display manager caught SIGHUP\n");
	abort ();
}

static int	someoneDead;

static
someoneDied ()
{
	Debug ("someone died\n");
	someoneDead = 1;
}

ManageDisplay (d)
struct display	*d;
{
	int		pid;
	waitType	status;
	int		serverPid;
	int		sessionPid;

	Debug ("manage display %s\n", d->name);
	if (setjmp (terminated)) {
		Debug ("processing SIGTERM\n");
		if (sessionPid < 2)
			abort ();
		(void) killpg (sessionPid, SIGTERM);
		TerminateServer (d, serverPid);
		exit (OBEYSESS_DISPLAY);
	}
	(void) signal (SIGTERM, CatchTerm);
	(void) signal (SIGHUP, CatchHup);
	(void) signal (SIGCHLD, someoneDied);
	(void) signal (SIGPIPE, SIG_IGN);
	/*
	 * Step 4: Start server control program
	 */
	if (!StartServer (d, &serverPid)) {
		Debug ("aborting display %s\n", d->name);
		exit (1);
	}
	(void) signal (SIGCHLD, SIG_DFL);
	/*
	 * keep a session running on this display
	 */
	StartSession (d, &sessionPid);		
	for (;;) {
		pid = wait (&status);
		Debug ("manager wait pid is %d status %x\n", pid, status);
		if (pid == -1) {
			if (errno == ECHILD)
				exit (OBEYSESS_DISPLAY);
			else
				continue;
		}
		if (pid == sessionPid) {
			Debug ("session died %s\n", d->name);
			switch (waitVal (status)) {
			case DISABLE_DISPLAY:
				Debug ("Session exited with DISABLE_DISPLAY\n");
				TerminateServer (d, serverPid);
				exit (UNMANAGE_DISPLAY);
			case ABORT_DISPLAY:
				Debug ("Session exited with ABORT_DISPLAY\n");
				TerminateServer (d, serverPid);
				exit (REMANAGE_DISPLAY);
			case OBEYTERM_DISPLAY:
				Debug ("Session exited with OBEYTERM_DISPLAY\n");
				if (d->terminateServer ||
 				    d->displayType.lifetime == Transient)
 				{
					TerminateServer (d, serverPid);
					exit (OBEYSESS_DISPLAY);
				}
			case RESTART_DISPLAY:
			default:
				break;
			}
			if (!HupServer (d, serverPid)) {
				Debug ("hup failed %s", d->name);
				TerminateServer (d, serverPid);
				exit (OBEYSESS_DISPLAY);
			} else {
				Debug ("restarting session %d\n", d->name);
				StartSession (d, &sessionPid);
			}
		} else if (pid == serverPid) {
			Debug ("Server died, terminating %s\n", d->name);
			if (sessionPid < 2)
				abort ();
			(void) killpg (sessionPid, SIGTERM);
			exit (OBEYSESS_DISPLAY);
		}
	}
	/* NOTREACHED */
}

Display	*dpy;

static
StartServer (d, pidp)
struct display	*d;
int		*pidp;
{
	char	**f;

	Debug ("StartServer ");
	if (d->displayType.location == Local) {
		for (f = d->argv; *f; f++)
			Debug ("'%s' ", *f);
		Debug ("\n");
		switch (*pidp = fork ()) {
		case 0:
			setpgrp (0, getpid ());
			(void) signal (SIGCHLD, SIG_DFL);
			(void) signal (SIGTERM, SIG_DFL);
			CloseOnFork ();
			(void) execv (d->argv[0], d->argv);
			LogError ("server %s cannot be executed",
					d->argv[0]);
			sleep ((unsigned) d->openDelay);
			exit (ABORT_DISPLAY);
		case -1:
			LogError ("fork failed, sleeping\n");
			return 0;
		default:
			if (d->displayType.location == Local)
				sleep ((unsigned) d->openDelay);
			break;
		}
	} else {
#ifdef UDP_SOCKET
		serverMessage (d, START);
#endif
	}
	Debug ("Server Started %d\n", *pidp);
	return WaitForServer (d, *pidp);
}

static jmp_buf	closeAbort;

static
abortClose (signal)
{
	longjmp (closeAbort, 1);
}

static
CloseDisplay (d, serverPid)
struct display	*d;
{
	(void) signal (SIGALRM, abortClose);
	(void) alarm ((unsigned) d->openTimeout);
	if (!setjmp (closeAbort)) {
		XCloseDisplay (dpy);
		(void) alarm (0);
		(void) signal (SIGALRM, SIG_DFL);
		dpy = 0;
	} else {
		Debug ("hung in close, aborting\n");
		LogError ("Hung in XCloseDisplay(%s), aborting\n", d->name);
		(void) signal (SIGALRM, SIG_DFL);
		TerminateServer (d, serverPid);
		exit (OBEYSESS_DISPLAY);
	}
}

/*
 * this code is complicated by some TCP failings.  On
 * many systems, the connect will occasionally hang forever,
 * this trouble is avoided by setting up a timeout to longjmp
 * out of the connect (possibly leaving piles of garbage around
 * inside Xlib) and give up, terminating the server.
 */

static jmp_buf	openAbort;

static
abortOpen ()
{
	longjmp (openAbort, 1);
}

static
WaitForServer (d, serverPid)
struct display	*d;
int		serverPid;
{
	int	i;
	int	pid;

	Debug ("WaitForServer, delay: %d repeat: %d\n", d->openDelay,
		d->openRepeat);
	/*
	 * kludge to avoid race condition in TCP.  Without this,
	 * sun os3.4 crashes...
	 */
	if (d->displayType.location == Local)
		sleep ((unsigned) d->openDelay);
	for (i = 0; i < d->openRepeat > 0 ? d->openRepeat : 1; i++) {
		(void) signal (SIGALRM, abortOpen);
		(void) alarm ((unsigned) d->openTimeout);
		if (!setjmp (openAbort)) {
			Debug ("Before XOpenDisplay(%s)\n", d->name);
			dpy = XOpenDisplay (d->name);
			(void) alarm ((unsigned) 0);
			(void) signal (SIGALRM, SIG_DFL);
			Debug ("After XOpenDisplay(%s)\n", d->name);
			if (dpy) {
				RegisterCloseOnFork (ConnectionNumber (dpy));
				return 1;
			}
			if (someoneDead) {
				pid = wait ((waitType *) 0);
				if (pid == serverPid) {
					Debug ("server died\n");
					return 0;
				}
			}
			Debug ("waiting for server to start\n");
			sleep ((unsigned) d->openDelay);
		} else {
			Debug ("hung in open, aborting\n");
			LogError ("Hung in XOpenDisplay(%s), aborting\n", d->name);
			(void) signal (SIGALRM, SIG_DFL);
			break;
		}
	}
	Debug ("giving up on server\n");
	LogError ("server open failed for %s, giving up\n", d->name);
	pid = 0;
	if (someoneDead)
		pid = wait ((waitType *) 0);
	if (pid != serverPid)
		TerminateServer (d, serverPid);
	return 0;
}

/*
 * terminate the server.  This TERMs the server and waits for it to die.
 * If the kill fails, xdm KILLs it several times.  If the server refuses
 * to exit, it is disabled.
 */

static jmp_buf terminateAbortWait;

static
terminateTimeout ()
{
	longjmp (terminateAbortWait, 1);
}

static
TerminateServer (d, serverPid)
struct display	*d;
int		serverPid;
{
	int	pid;
	int	i;

	Debug ("terminating server %s, pid %d\n", d->name, serverPid);
	if (d->displayType.location == Local) {
		if (dpy) {
			ClearCloseOnFork (ConnectionNumber (dpy));
			CloseDisplay (d, serverPid);
			dpy = 0;
		}
		(void) signal (SIGALRM, terminateTimeout);
		for (i = 0; i < d->openRepeat; i++) {
			Debug ("killing server %d\n", i);
			if (serverPid < 2)
				abort ();
			if (killpg (serverPid, i == 0 ? SIGTERM : SIGKILL) == -1) {
				if (errno == ESRCH)
					break;
				else if (errno == EPERM) {
					LogError ("terminate server: permission denied\n");
					exit (UNMANAGE_DISPLAY);
				}
			}
			if (!setjmp (terminateAbortWait)) {
				(void) alarm (d->openTimeout);
				pid = wait ((waitType *) 0);
				(void) alarm (0);
				if (pid == serverPid)
					break;
			}
		}
		if (i == d->openRepeat) {
			Debug ("The server that would not die...\n");
			exit (UNMANAGE_DISPLAY);
		}
		(void) signal (SIGALRM, SIG_DFL);
	} else {
#ifdef UDP_SOCKET
		Debug ("sending TERMINATE message through network\n");
		if (!serverMessage (d, TERMINATE) && dpy)
			pseudoReset (dpy);
#else
		if (dpy)
			pseudoReset (dpy);
#endif
		if (dpy) {
			ClearCloseOnFork (ConnectionNumber (dpy));
			CloseDisplay (d, serverPid);
		}
	}
	Debug ("Server terminated\n");
}

static
HupServer (d, serverPid)
struct display	*d;
int		serverPid;
{
	Debug ("hupping server %s, pid %d\n", d->name, serverPid);
	if (d->displayType.location == Local) {
		Debug ("sending HUP signal\n");
		if (dpy) {
			ClearCloseOnFork (ConnectionNumber (dpy));
			CloseDisplay (d, serverPid);
		}
		if (serverPid < 2)
			abort ();
		if (killpg (serverPid, SIGHUP) == -1) {
			if (errno == ESRCH) {
				Debug ("server already dead\n");
				exit (OBEYSESS_DISPLAY);
			}
			else if (errno == EPERM) {
				Debug ("permission denied\n");
				LogError ("reset server: permission denied\n");
				exit (UNMANAGE_DISPLAY);
			}
		}
	} else {
#ifdef UDP_SOCKET
		Debug ("sending RESTART message over network\n");
		if (!serverMessage (d, RESTART) && dpy)
#else
			pseudoReset (dpy);
#endif
		if (dpy) {
			ClearCloseOnFork (ConnectionNumber (dpy));
			CloseDisplay (d, serverPid);
		}
	}
	return WaitForServer (d, serverPid);
}

static
StartSession (d, pidp)
struct display	*d;
int		*pidp;
{
	int	pid;

	Debug ("StartSession %s\n", d->name);
	switch (pid = fork ()) {
	case 0:
		setpgrp (0, getpid ());
		(void) signal (SIGTERM, SIG_DFL);
		(void) signal (SIGCHLD, SIG_DFL);
		CloseOnFork ();
		ManageSession (d);
		exit (0);
	case -1:
		break;
	default:
		Debug ("pid: %d\n", pid);
		*pidp = pid;
		break;
	}
}
