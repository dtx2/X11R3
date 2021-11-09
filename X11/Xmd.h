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
#ifndef XMD_H
#define XMD_H 1
/* $XConsortium: Xmd.h,v 1.34 88/10/22 10:15:47 jim Exp $ */
/*
 *  Xmd.h: MACHINE DEPENDENT DECLARATIONS.
 */


/* Definition of macro used to set constants for size of network structures;
 * machines with preprocessors that can't handle all of the sz_ symbols
 * can define this macro to be sizeof(x) if and only if their compiler doesn't
 * pad out structures (esp. the xTextElt structure which contains only two 
 * one-byte fields).  Network structures should always define sz_symbols.
 *
 * The sz_ prefix is used instead of something more descriptive so that the
 * symbols are no more than 32 characters long (which causes problems for some
 * compilers and preprocessors).*/
#define SIZEOF(x) sz_##x
/* Bitfield suffixes for the protocol structure elements, if you
 * need them.  Note that bitfields are not guarranteed to be signed
 * (or even unsigned) according to ANSI C.*/
#define B32
#define B16

typedef long           INT32;
typedef short          INT16;
typedef char           INT8;

typedef unsigned long CARD32;
typedef unsigned short CARD16;
typedef unsigned char  CARD8;

typedef unsigned long		BITS32;
typedef unsigned short		BITS16;
typedef unsigned char		BYTE;

typedef unsigned char            BOOL;

// definitions for sign-extending bitfields on 64-bit architectures
#define cvtINT8toInt(val) (val)
#define cvtINT16toInt(val) (val)
#define cvtINT32toInt(val) (val)
#define cvtINT8toLong(val) (val)
#define cvtINT16toLong(val) (val)
#define cvtINT32toLong(val) (val)



#ifdef MUSTCOPY
/*
 * This macro must not cast or else pointers will get aligned and be wrong
 */
#define NEXTPTR(p,t)  (((char *) p) + SIZEOF(t))
#else /* else not MUSTCOPY, this is used for 32-bit machines */
/*
 * this version should leave result of type (t *), but that should only be 
 * used when not in MUSTCOPY
 */  
#define NEXTPTR(p,t) (((t *)(p)) + 1)
#endif /* MUSTCOPY - used machines whose C structs don't line up with proto */

#endif /* XMD_H */
