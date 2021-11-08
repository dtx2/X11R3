/*
 * Copyright 1988 Mips Computer Systems
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted, provided
 * that the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of Mips not be used in advertising or  
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  Mips makes no representations about the  
 * suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 */

/* $XConsortium: mipssysvc.c,v 1.3 88/09/06 16:06:41 jim Exp $ */

#ifndef lint
static char *sccsid = "@(#)mips_vfork.c	1.11	3/02/88";
#endif lint

#include <errno.h>
#include <limits.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/uio.h>

#define TRUE 1
#define FALSE 0

char *malloc ();

int vfork ()
{
  int pid;

  pid = fork ();
  return pid;
}



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



int readv (d, iov, iovcnt)
     int d;
     struct iovec *iov;
     int iovcnt;
{
  int i, isum, cnt, mc, inext, j;
  struct iovec *tiov;
  char *mybuf;

  if (iovcnt <= 0 || iovcnt > 16) {
    errno = EINVAL;
    return (-1);
  }

  isum = 0;
  for (i = 0, tiov = iov; i < iovcnt; i++, tiov++) {
    if (tiov->iov_len < 0 || INT_MAX - tiov->iov_len < isum) {
      errno = EINVAL;
      return (-1);
    }
    isum += tiov->iov_len;
  }

/*  We do not check whether part of the iov points outside the process's
    allocated data space, and therefore cannot return EFAULT.  */

  mybuf = malloc (isum+8);
  while (-1 == (cnt = read (d, mybuf, isum)) && errno == EINTR);
  if (cnt <= 0) return cnt;
  mc = 0;
  for (i = 0, tiov = iov; i < iovcnt; i++, tiov++) {
    inext = tiov->iov_len;
    if (inext > cnt - mc) inext = cnt - mc;
    for (j = 0; j < inext; j++) 
      (*(tiov->iov_base + j)) = (*(mybuf + mc + j));
    mc += inext;
  }
  free (mybuf);
  return (cnt);
}


int writev (d, iov, iovcnt)
     int d;
     struct iovec *iov;
     int iovcnt;
{
  int i, isum, cnt, mc, j;
  struct iovec *tiov;
  char *mybuf;

  if (iovcnt <= 0 || iovcnt > 16) {
    errno = EINVAL;
    return (-1);
  }

  isum = 0;
  for (i = 0, tiov = iov; i < iovcnt; i++, tiov++) {
    if (tiov->iov_len < 0 || INT_MAX - tiov->iov_len < isum) {
      errno = EINVAL;
      return (-1);
    }
    isum += tiov->iov_len;
  }

  mybuf = malloc (isum+4);
  mc = 0;
  for (i = 0, tiov = iov; i < iovcnt; i++, tiov++) {
    for (j = 0; j < tiov->iov_len; j++) 
      (*(mybuf + mc + j)) = (*(tiov->iov_base + j));
    mc += tiov->iov_len;
  }
  while (-1 == (cnt = write (d, mybuf, mc)) && errno == EINTR);
  free (mybuf);
  return (cnt);
}



int ReadFromServer (d, buf, nbytes)
     int d;
     char *buf;
     int nbytes;
{
  int nn;

  while (-1 == (nn = read (d, buf, nbytes)) && errno == EINTR);
  return nn;
}


int WriteToServer (d, buf, nbytes)
     int d;
     char *buf;
     int nbytes;
{
  int nn;
  void (*save_io)(), (*save_alarm)();

  save_io = sigset(SIGIO, SIG_HOLD);
  save_alarm = sigset(SIGALRM, SIG_HOLD);
  while (-1 == (nn = write (d, buf, nbytes)) && errno == EINTR);
  sigset(SIGIO, save_io);
  sigset(SIGALRM, save_alarm);
  return nn;
}
