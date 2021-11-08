/*
 * xdm - display manager daemon
 *
 * $XConsortium: displaylist.c,v 1.5 88/10/20 17:36:39 keith Exp $
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
 * a simple linked list of known displays
 */

# include "dm.h"

struct display	*displays;

AnyDisplaysLeft ()
{
	return displays != (struct display *) 0;
}

ForEachDisplay (f)
	void	(*f)();
{
	struct display	*d, *next;

	for (d = displays; d; d = next) {
		next = d->next;
		(*f) (d);
	}
}

struct display *
FindDisplayByName (name)
char	*name;
{
	struct display	*d;

	for (d = displays; d; d = d->next)
		if (!strcmp (name, d->name))
			return d;
	return 0;
}

struct display *
FindDisplayByPid (pid)
int	pid;
{
	struct display	*d;

	for (d = displays; d; d = d->next)
		if (pid == d->pid)
			return d;
	return 0;
}

RemoveDisplay (old)
struct display	*old;
{
	struct display	*d, *p;

	p = 0;
	for (d = displays; d; d = d->next)
		if (d == old) {
			if (p)
				p->next = d->next;
			else
				displays = d->next;
			free (d);
			break;
		}
}

struct display *
NewDisplay (name)
char	*name;
{
	struct display	*d;

	d = (struct display *) malloc (sizeof (struct display));
	if (!d)
		LogPanic ("out of memory\n");
	d->next = displays;
	d->name = strcpy (malloc (strlen (name) + 1), name);
	if (!d->name)
		LogPanic ("out of memory\n");
	d->argv = 0;
	d->status = notRunning;
	d->pid = -1;
	displays = d;
	LoadDisplayResources (d);
	return d;
}
