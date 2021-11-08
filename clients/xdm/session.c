/*
 * xdm - display manager daemon
 *
 * $XConsortium: session.c,v 1.9 88/11/17 19:13:42 keith Exp $
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
 * session.c
 */

# include "dm.h"
# include <X11/Xlib.h>
# include <signal.h>
# include <X11/Xatom.h>
# include <setjmp.h>

static int	clientPid;

ManageSession (d)
struct display	*d;
{
	struct greet_info	greet;
	struct verify_info	verify;
	int			pid;
	Display			*dpy, *InitGreet ();

	Debug ("ManageSession %s\n", d->name);
	/*
	 * Step 5: Load system default Resources
	 */
	LoadXloginResources (d);
	dpy = InitGreet (d);
	for (;;) {
		/*
		 * Step 6: Greet user, requesting name/password
		 */
		Greet (d, &greet);
		/*
		 * Step 7: Verify user
		 */
		if (Verify (d, &greet, &verify))
			break;
		else
			FailedLogin (d, &greet);
	}
	DeleteXloginResources (d, dpy);
	CloseGreet (d);
	Debug ("Greet loop finished\n");
	/*
	 * Step 8: Run system-wide initialization file
	 */
	if (source (&verify, d->startup) != 0)
		SessionExit (OBEYTERM_DISPLAY);
	/*
	 * Step 9: Start the clients, changing uid/groups
	 *	   setting up environment and running the session
	 */
	if (StartClient (&verify, d, &clientPid)) {
		Debug ("Client Started\n");
		/*
		 * Step 13: Wait for session to end,
		 */
		for (;;) {
			pid = wait (0);
			if (pid == clientPid)
				break;
		}
	} else {
		LogError ("session start failed\n");
	}
	/*
	 * Step 15: run system-wide reset file
	 */
	source (&verify, d->reset);
	SessionExit (OBEYTERM_DISPLAY);
}

LoadXloginResources (d)
struct display	*d;
{
	char	cmd[1024];

	if (d->resources[0] && access (d->resources, 4) == 0) {
		sprintf (cmd, "%s -display %s -load %s",
				d->xrdb, d->name, d->resources);
		Debug ("Loading resource file: %s\n", cmd);
		system (cmd);
	}
}

DeleteXloginResources (d, dpy)
struct display	*d;
Display		*dpy;
{
	XDeleteProperty(dpy, RootWindow (dpy, 0), XA_RESOURCE_MANAGER);
}

static jmp_buf syncJump;

static
syncTimeout ()
{
	longjmp (syncJump, 1);
}

SecureDisplay (d, dpy)
struct display	*d;
Display		*dpy;
{
	Debug ("SecureDisplay\n");
	signal (SIGALRM, syncTimeout);
	if (setjmp (syncJump)) {
		LogError ("WARNING: display %s could not be secured\n",
				d->name);
		SessionExit (ABORT_DISPLAY);
	}
	alarm (d->grabTimeout);
	Debug ("Before XGrabServer\n");
	XGrabServer (dpy);
	if (XGrabKeyboard (dpy, DefaultRootWindow (dpy), True, GrabModeAsync,
			   GrabModeAsync, CurrentTime) != GrabSuccess)
 	{
		alarm (0);
		signal (SIGALRM, SIG_DFL);
		LogError ("WARNING: keyboard on display %s could not be secured\n",
				d->name);
		SessionExit (ABORT_DISPLAY);
	}
	Debug ("XGrabKeyboard succeeded\n");
	alarm (0);
	signal (SIGALRM, SIG_DFL);
	pseudoReset (dpy);
}

UnsecureDisplay (d, dpy)
struct display	*d;
Display		*dpy;
{
	Debug ("Unsecure display %s\n", d->name);
	XUngrabServer (dpy);
	XSync (dpy, 0);
}

SessionExit (status)
{
	exit (status);
}

StartClient (verify, d, pidp)
struct verify_info	*verify;
struct display		*d;
int			*pidp;
{
	char	**f, *home, *getEnv ();
	char	*failsafeArgv[2];
	int	pid;

	Debug ("StartSession %s: ", verify->argv[0]);
	for (f = verify->argv; *f; f++)
		Debug ("%s ", *f);
	Debug ("; ");
	for (f = verify->userEnviron; *f; f++)
		Debug ("%s ", *f);
	Debug ("\n");
	switch (pid = fork ()) {
	case 0:
		CloseOnFork ();
		setpgrp (0, getpid ());
#ifdef NGROUPS

		setgid (verify->groups[0]);
		setgroups (verify->ngroups, verify->groups);
#else
		setgid (verify->gid);
#endif
		setuid (verify->uid);
		home = getEnv (verify->userEnviron, "HOME");
		if (home)
			if (chdir (home) == -1) {
				LogError ("No home directory %s for user %s, using /\n",
					  home, getEnv (verify->userEnviron, "USER"));
				chdir ("/");
			}
		Debug ("executing session %s\n", verify->argv[0]);
		execve (verify->argv[0], verify->argv, verify->userEnviron);
		LogError ("Session execution failed %s\n", verify->argv[0]);
		Debug ("exec failed\n");
		failsafeArgv[0] = d->failsafeClient;
		failsafeArgv[1] = 0;
		execve (failsafeArgv[0], failsafeArgv, verify->userEnviron);
		exit (1);
	case -1:
		Debug ("StartSession, fork failed\n");
		LogError ("can't start session for %d, fork failed\n", d->name);
		return 0;
	default:
		Debug ("StartSession, fork suceeded %d\n", pid);
		*pidp = pid;
		return 1;
	}
}

int
source (verify, file)
struct verify_info	*verify;
char			*file;
{
	char	*args[4];
	int	pid, wpid;
	extern int	errno;
	waitType	result;

	Debug ("source %s\n", file);
	if (file[0] && access (file, 1) == 0) {
		switch (pid = fork ()) {
		case 0:
			CloseOnFork ();
			setpgrp (0, getpid ());
			if (!(args[0] = getEnv (verify->systemEnviron, "SHELL")))
				args[0] = "/bin/sh";
			args[1] = "-c";
			args[2] = file;
			args[3] = 0;
			Debug ("interpreting %s with %s\n", args[2], args[0]);
			execve (args[0], args, verify->systemEnviron);
			LogError ("can't execute system shell %s\n", args[0]);
			exit (1);
		case -1:
			Debug ("fork failed\n");
			LogError ("can't fork to execute %s\n", file);
			return 1;
			break;
		default:
			while ((wpid = wait (&result)) != pid)
				;
			break;
		}
		return waitVal (result);
	}
	return 0;
}
