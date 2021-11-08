/*
 * xdm - display manager daemon
 *
 * $XConsortium: error.c,v 1.6 88/11/14 12:29:19 keith Exp $
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
 * error.c
 *
 * Log display manager errors to a file as
 * we generally do not have a terminal to talk to
 */

# include "dm.h"
# include <stdio.h>

InitErrorLog ()
{
	int	i;
	if (errorLogFile[0]) {
		i = creat (errorLogFile, 0666);
		if (i != -1) {
			if (i != 2) {
				dup2 (i, 2);
				close (i);
			}
		} else
			LogError ("Cannot open errorLogFile %s\n", errorLogFile);
	}
}

LogError (fmt, arg1, arg2, arg3, arg4, arg5)
char	*fmt;
int	arg1, arg2, arg3, arg4, arg5;
{
	fprintf (stderr, fmt, arg1, arg2, arg3, arg4, arg5);
	fflush (stderr);
}

LogPanic (fmt, arg1, arg2, arg3, arg4, arg5)
char	*fmt;
int	arg1, arg2, arg3, arg4, arg5;
{
	LogError ("panic: ");
	LogError (fmt, arg1, arg2, arg3, arg4, arg5);
	exit (1);
}

Panic (mesg)
char	*mesg;
{
	int	i;

	i = creat ("/dev/console", 0666);
	write (i, "panic: ", 7);
	write (i, mesg, strlen (mesg));
	exit (1);
}


Debug (fmt, arg1, arg2, arg3, arg4, arg5)
char	*fmt;
int	arg1, arg2, arg3, arg4, arg5;
{
	if (debugLevel > 1) {
		printf (fmt, arg1, arg2, arg3, arg4, arg5);
	}
}
