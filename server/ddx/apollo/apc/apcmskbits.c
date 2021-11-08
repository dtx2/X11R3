/******************************************************************
Copyright 1987 by Apollo Computer Inc., Chelmsford, Massachusetts.
     
                        All Rights Reserved

Permission to use, duplicate, change, and distribute this software and
its documentation for any purpose and without fee is granted, provided
that the above copyright notice appear in such copy and that this
copyright notice appear in all supporting documentation, and that the
names of Apollo Computer Inc. or MIT not be used in advertising or publicity
pertaining to distribution of the software without written prior permission.
******************************************************************/

/*
   these tables are used by several macros in the apc code.

   the vax numbers everything left to right, so bit indices on the
screen match bit indices in longwords.  the pc-rt and Sun number
bits on the screen the way they would be written on paper,
(i.e. msb to the left), and so a bit index n on the screen is
bit index 32-n in a longword

   see also apcmskbits.h
*/
#include	<X.h>
#include	<Xmd.h>
#include	<servermd.h>

#if	(BITMAP_BIT_ORDER == MSBFirst)
/* NOTE:
the first element in starttab could be 0xffffffff.  making it 0
lets us deal with a full first word in the middle loop, rather
than having to do the multiple reads and masks that we'd
have to do if we thought it was partial.
*/
int apcstarttab[] =
    {
	0x00000000,
	0x00FFFFFF,
	0x0000FFFF,
	0x000000FF
    };

int apcendtab[] =
    {
	0x00000000,
	0xFF000000,
	0xFFFF0000,
	0xFFFFFF00
    };

/* a hack, for now, since the entries for 0 need to be all
   1 bits, not all zeros.
   this means the code DOES NOT WORK for segments of length
   0 (which is only a problem in the horizontal line code.)
*/
int apcstartpartial[] =
    {
	0xFFFFFFFF,
	0x00FFFFFF,
	0x0000FFFF,
	0x000000FF
    };

int apcendpartial[] =
    {
	0xFFFFFFFF,
	0xFF000000,
	0xFFFF0000,
	0xFFFFFF00
    };
#else		/* (BITMAP_BIT_ORDER == LSBFirst) */
/* NOTE:
the first element in starttab could be 0xffffffff.  making it 0
lets us deal with a full first word in the middle loop, rather
than having to do the multiple reads and masks that we'd
have to do if we thought it was partial.
*/
int apcstarttab[] = 
	{
	0x00000000,
	0xFFFFFF00,
	0xFFFF0000,
	0xFF000000
	};

int apcendtab[] = 
	{
	0x00000000,
	0x000000FF,
	0x0000FFFF,
	0x00FFFFFF
	};

/* a hack, for now, since the entries for 0 need to be all
   1 bits, not all zeros.
   this means the code DOES NOT WORK for segments of length
   0 (which is only a problem in the horizontal line code.)
*/
int apcstartpartial[] = 
	{
	0xFFFFFFFF,
	0xFFFFFF00,
	0xFFFF0000,
	0xFF000000
	};

int apcendpartial[] = 
	{
	0xFFFFFFFF,
	0x000000FF,
	0x0000FFFF,
	0x00FFFFFF
	};
#endif	(BITMAP_BIT_ORDER == MSBFirst)


/* used for masking bits in bresenham lines
   mask[n] is used to mask out all but bit n in a longword (n is a
screen position).
   rmask[n] is used to mask out the single bit at position n (n
is a screen posiotion.)
*/

#if	(BITMAP_BIT_ORDER == MSBFirst)
int apcmask[] =
    {
	0xFF000000, 0x00FF0000, 0x0000FF00, 0x000000FF
    }; 
int apcrmask[] = 
    {
	0x00FFFFFF, 0xFF00FFFF, 0xFFFF00FF, 0xFFFFFF00
    };
#else	/* (BITMAP_BIT_ORDER == LSBFirst) */
int apcmask[] =
    {
	0x000000FF, 0x0000FF00, 0x00FF0000, 0xFF000000
    }; 
int apcrmask[] = 
    {
	0xFFFFFF00, 0xFFFF00FF, 0xFF00FFFF, 0x00FFFFFF
    };
#endif	(BITMAP_BIT_ORDER == MSBFirst)

#ifdef	vax
#undef	VAXBYTEORDER
#endif
