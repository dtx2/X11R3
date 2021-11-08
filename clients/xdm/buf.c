/*
 * xdm - display manager daemon
 *
 * $XConsortium: buf.c,v 1.2 88/09/23 14:21:09 keith Exp $
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
 * buf.c
 *
 * buffered i/o for the display manager
 */

# include "buf.h"

extern char	*malloc ();

struct buffer *
dataOpen (d, len)
char	*d;
int	len;
{
	struct buffer	*b;

	b = (struct buffer *) malloc (sizeof (struct buffer));
	b->buf = d;
	b->cnt = len;
	b->bufp = 0;
	b->fill = 0;
	b->size = len;
	b->private = -1;
	return b;
}

fileFill (b)
	struct buffer	*b;
{
	b->cnt = read (b->private, b->buf, b->size);
	b->bufp = 0;
	if (b->cnt <= 0) {
		b->cnt = 0;
		return EOB;
	}
	return b->buf[b->bufp++];
}

# define BUFFER_SIZE	1024

struct buffer *
fileOpen (fd)
int	fd;
{
	struct buffer	*b;

	b = (struct buffer *) malloc (sizeof (struct buffer));
	if (!b)
		LogPanic ("out of memory\n");
	b->buf = malloc (b->size = BUFFER_SIZE);
	if (!b->buf)
		LogPanic ("out of memory\n");
	b->cnt = b->bufp = 0;
	b->private = fd;
	b->fill = fileFill;
	return b;
}

bufClose (b)
	struct buffer	*b;
{
	if (b->private != -1)
		free (b->buf);
	free ((char *) b);
}
