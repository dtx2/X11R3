/***********************************************************
Copyright 1987 by Digital Equipment Corporation, Maynard, Massachusetts,
and the Massachusetts Institute of Technology, Cambridge, Massachusetts.

                        All Rights Reserved

Permission to use, copy, modify, and distribute this software and its 
documentation for any purpose and without fee is hereby granted, 
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in 
supporting documentation, and that the names of Digital or MIT not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.  

DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.

******************************************************************/

/* $XConsortium: os.h,v 1.30 88/10/25 13:40:36 keith Exp $ */

#ifndef OS_H
#define OS_H
#include "misc.h"
#define NullFID ((FID) 0)
#define SCREEN_SAVER_ON   0
#define SCREEN_SAVER_OFF  1
#define SCREEN_SAVER_FORCER 2
#define MAX_REQUEST_SIZE 16384
typedef pointer	FID;
#ifndef ALLOCATE_LOCAL
char *malloc();
#define ALLOCATE_LOCAL(size) malloc((unsigned)((size) > 0 ? (size) : 1))
#define DEALLOCATE_LOCAL(ptr) free((char *)(ptr))
#endif /* ALLOCATE_LOCAL */

#define xalloc(size) Xalloc((unsigned long)(size))
#define xrealloc(ptr, size) Xrealloc((pointer)(ptr), (unsigned long)(size))
#define xfree(ptr) Xfree((pointer)(ptr))

/*
 * The declaration for ReadRequestFromClient should be a xReq *, but
 * then other files must also include Xproto.h and (boo hoo) that
 * would introduce many, many more symbols which would break the less
 * intelligent strain of compilers available today.
 */
char		*ReadRequestFromClient();
char		*strcat();
char		*strncat();
char		*strcpy();
char		*strncpy();
FontPathPtr	GetFontPath();
FontPathPtr	ExpandFontNamePattern();
FID		    FiOpenForRead();
void		CreateUnixSocket();
void		FreeFontRecord();
void		SetFontPath();
void		ErrorF();
void		Error();
void		FatalError();
void		Xfree();
void		FlushIfCriticalOutputPending();
unsigned long	*Xalloc();
unsigned long	*Xrealloc();
long		GetTimeInMillis();

#endif /* OS_H */
