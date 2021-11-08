/*
 * xdm - display manager daemon
 *
 * $XConsortium: buf.h,v 1.2 88/09/23 14:21:14 keith Exp $
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

# define EOB	-1

# define bufc(b)	((b)->cnt == (b)->bufp ? ((b)->fill ?\
			 (*(b)->fill) (b) : EOB) : (b)->buf[(b)->bufp++])

struct buffer {
	char	*buf;
	int	cnt;
	int	bufp;
	int	size;
	int	(*fill)();
	int	private;
};

extern struct buffer	*fileOpen (), *dataOpen ();
