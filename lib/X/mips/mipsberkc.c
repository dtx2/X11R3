/* $XConsortium: mipsberkc.c,v 1.2 88/09/06 16:06:35 jim Exp $ */

#ifndef lint
static char *sccsid = "@(#)mips_vfork.c	1.11	3/02/88";
#endif lint
/*
 * Copyright 1988 by Mips Computer Systems, Sunnyvale, California.
 */




/*  Need a bcopy than can copy backwards if necessary  */
void bcopy (src, dst, length)
     char *src, *dst;
     int length;
{
  if (src < dst && src + length > dst)
    {src = src + length - 1;
     dst = dst + length - 1;
     for (; length > 0; length--, dst--, src--) *dst = *src;
   }
  else if (src != dst)
    for (; length > 0; length--, dst++, src++) *dst = *src;
}
