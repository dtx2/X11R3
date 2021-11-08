/*
 * $XConsortium: Xos.h,v 1.10 88/09/06 14:30:21 jim Exp $
 * 
 * Copyright 1987 by the Massachusetts Institute of Technology
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted, provided 
 * that the above copyright notice appear in all copies and that both that 
 * copyright notice and this permission notice appear in supporting 
 * documentation, and that the name of M.I.T. not be used in advertising
 * or publicity pertaining to distribution of the software without specific, 
 * written prior permission. M.I.T. makes no representations about the 
 * suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 *
 * The X Window System is a Trademark of MIT.
 *
 */

/* This is a collection of things to try and minimize system dependencies
 * in a "signficant" number of source files.
 */

#ifndef _XOS_H_
#define _XOS_H_

/*
 * Get major data types (esp. caddr_t)
 */

#ifdef CRAY
#ifndef __TYPES__
#define __TYPES__
#include <sys/types.h>			/* forgot to protect it... */
#endif /* __TYPES__ */
#else
#include <sys/types.h>
#endif /* CRAY */


/*
 * Just about everyone needs the strings routines.  For uniformity, we use
 * the BSD-style index() and rindex() in application code, so any systems that
 * don't provide them need to have #defines here.  Unfortunately, we can't
 * use #if defined() here since makedepend will get confused.
 *
 * The list of systems that currently needs System V stings includes:
 *
 *	hpux
 * 	macII
 *	CRAY
 */

#ifdef SYSV
#define SYSV_STRINGS
#endif /* SYSV */

#ifdef macII				/* since it doesn't define SYSV */
#define SYSV_STRINGS
#endif /* macII */

#ifdef SYSV_STRINGS
#include <string.h>
#define index strchr
#define rindex strrchr
#undef SYSV_STRINGS
#else
#include <strings.h>
#endif /* SYSV_STRINGS */


/*
 * Get open(2) constants
 */

#ifdef SYSV
#include <fcntl.h>
#endif /* SYSV */
#include <sys/file.h>


/*
 * Get struct timeval
 */

#ifdef macII
#include <time.h>		/* need this as well as sys/time.h */
#endif /* macII */

#ifdef SYSV
#include <time.h>
# ifdef mips
# include <bsd/sys/time.h>
# endif /* mips */
#else /* else not SYSV */
#include <sys/time.h>
#endif /* SYSV */

/*
 * More BSDisms
 */

#ifdef SYSV
#define SIGCHLD SIGCLD
#endif /* SYSV */


/*
 * Put system-specific definitions here
 */

#ifdef hpux
#define sigvec sigvector
#endif /* hpux */

#ifdef mips
# ifdef SYSTYPE_SYSV
# include <bsd/sys/ioctl.h>
# include <bsd/sys/file.h>
# endif /* SYSTYPE_SYSV */
#endif /* mips */


#endif /* _XOS_H_ */
