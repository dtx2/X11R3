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

#include "apc.h"
#include "misc.h"
#include "cursor.h"
#include "screenint.h"

void apcQueryBestSize(class, pwidth, pheight)
int class;
short *pwidth;
short *pheight;
{
    unsigned width, test;

    switch(class)
    {
      case CursorShape:
	  *pwidth = 16;
	  *pheight = 16;
	  break;
      case TileShape:
      case StippleShape:
 	      if (*pwidth > 32) *pwidth = 32;
              else {
    	          width = *pwidth;
	          /* Return the closes power of two not less than what they gave me */
	          test = 0x00000020;
	          /* Find the highest 1 bit in the width given */
	          while(!(test & width))
	              test >>= 1;
	          /* If their number is greater than that, bump up to the next
	           *  power of two */
	          if((test - 1) & width)
	              test <<= 1;
	          *pwidth = test; 
	      }

 	      if (*pheight > 32) *pheight = 32;
              else {
    	          width = *pheight;
	          /* Return the closes power of two not less than what they gave me */
	          test = 0x00000020;
	          /* Find the highest 1 bit in the width given */
	          while(!(test & width))
	              test >>= 1;
	          /* If their number is greater than that, bump up to the next
	           *  power of two */
	          if((test - 1) & width)
	              test <<= 1;
	          *pheight = test;
	      }
	  break;
    }
}

gpr_$bitmap_desc_t 	LastGPRBitmap = 0;

void apc_$set_bitmap( bitmap)
gpr_$bitmap_desc_t	bitmap;
{
    status_$t	status;                                

    if (LastGPRBitmap != bitmap) {
	gpr_$set_bitmap( bitmap, status);
	LastGPRBitmap = bitmap;
	}
}
        
