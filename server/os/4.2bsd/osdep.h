/***********************************************************
Copyright 1987 by Digital Equipment Corporation, Maynard, Massachusetts,
and the Massachusetts Institute of Technology, Cambridge, Massachusetts.
Copyright 2021 Edward Halferty

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
/* $XConsortium: osdep.h,v 1.15 88/09/06 15:50:47 jim Exp $ */

#ifndef NULL
#define NULL 0
#endif

#define BUFSIZE 4096
#define BUFWATERMARK 8192
#define MAXBUFSIZE (1 << 18)
#define MAXSOCKS 128
#define mskcnt ((MAXSOCKS + 31) / 32)	/* size of bit array */
#define BITMASK(i) (1 << ((i) & 31))
#define MASKIDX(i) ((i) >> 5)
#define MASKWORD(buf, i) buf[MASKIDX(i)]
#define BITSET(buf, i) MASKWORD(buf, i) |= BITMASK(i)
#define BITCLEAR(buf, i) MASKWORD(buf, i) &= ~BITMASK(i)
#define GETBIT(buf, i) (MASKWORD(buf, i) & BITMASK(i))
#define COPYBITS(src, dst) dst[0] = src[0]; dst[1] = src[1]; dst[2] = src[2]; dst[3] = src[3]
#define CLEARBITS(buf) buf[0] = 0; buf[1] = 0; buf[2] = 0; buf[3] = 0
#define MASKANDSETBITS(dst, b1, b2) dst[0] = (b1[0] & b2[0]); dst[1] = (b1[1] & b2[1]); dst[2] = (b1[2] & b2[2]); dst[3] = (b1[3] & b2[3])
#define ORBITS(dst, b1, b2) dst[0] = (b1[0] | b2[0]); dst[1] = (b1[1] | b2[1]); dst[2] = (b1[2] | b2[2]); dst[3] = (b1[3] | b2[3])
#define UNSETBITS(dst, b1) dst[0] &= ~b1[0]; dst[1] &= ~b1[1]; dst[2] &= ~b1[2]; dst[3] &= ~b1[3]
#define ANYSET(src) (src[0] || src[1] || src[2] || src[3])
typedef struct _connectionInput {
    int used;                  /* is this client connected */
    char *buffer;               /* contains current client input */
    char *bufptr;               /* pointer to current start of data */
    int  bufcnt;                /* count of bytes in buffer */
    int lenLastReq;
    int size;
} ConnectionInput;
typedef struct _osComm {
    int fd;
    unsigned char *buf;
    int bufsize;
    int count;
} OsCommRec, *OsCommPtr;
void Notice();
