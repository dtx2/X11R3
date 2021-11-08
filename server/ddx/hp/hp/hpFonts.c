/*

Copyright (c) 1986, 1987 by Hewlett-Packard Company
Copyright (c) 1986, 1987 by the Massachusetts Institute of Technology

Permission to use, copy, modify, and distribute this
software and its documentation for any purpose and without
fee is hereby granted, provided that the above copyright
notice appear in all copies and that both that copyright
notice and this permission notice appear in supporting
documentation, and that the name of M.I.T. not be used in
advertising or publicity pertaining to distribution of the
software without specific, written prior permission.

HEWLETT-PACKARD MAKES NO WARRANTY OF ANY KIND WITH REGARD
TO THIS SOFWARE, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR 
PURPOSE.  Hewlett-Packard shall not be liable for errors 
contained herein or direct, indirect, special, incidental or 
consequential damages in connection with the furnishing, 
performance, or use of this material.

This software is not subject to any license of the American
Telephone and Telegraph Company or of the Regents of the
University of California.

*/
/***********************************************************************
 *  file: hpFonts.c
 *
 *  Font optimization/removal and text output routines
 *
 *		Hewlett Packard -- Corvallis Workstation Operation
 *		Project -- port of X11 to HP9000
 *		Harry Phinney -- MTS
 *
 *
 */

#include "Xmd.h"
#include "Xproto.h"
#include "dixfont.h"
#include "dixfontstr.h"
#include "font.h"
#include "fontstruct.h"
#include "scrnintstr.h"
#include "cfb/hpbuf.h"
#include "hpFonts.h"

#include "cfb/cfb.h"
#include "foo.h"	/* so I ain't got imagination */

extern char *NameForAtom();	/* in dix/atom.c */

int
lookUpProperty(pFont, pName)
  register FontRec *pFont;
  register char *pName;
{
  register int n = pFont->pFI->nProps;
  register DIXFontProp *pFP = pFont->pFP;

  for(;n--; pFP++) {
    register char *pPropString;
    if((pPropString = NameForAtom(pFP->name)) && !strcmp(pPropString, pName))
      return (int) pFP->value;
  }
  return 0;
}


/************************************************************************
 *  Routine:	hpRealizeFont
 *		Optimize the font i.e. store it in offscreen memory
 *		This implementation (like our X10) only stores
 *		the characters defined by STARTCHAR thru LASTCHAR in
 *		offscreen.  It breaks them into offscreen chunks
 *		of CHARSPERCHUNK size
 *
 *  Inputs: pScreen points to the ScreenRec we'll try to optimize for
 *	    pFont points to the font we'll store in offscreen
 *  
 *  Returns: TRUE (always)
 *
 *  Side Effects:  sets pFont->devPriv[pScreen->myNum] to point to an
 *		hpFontRec which identifies the location of the font
 *		in offscreen.
 *
 */


Bool
hpRealizeFont(pScreen, pFont)
  ScreenPtr pScreen;
  FontRec *pFont;
{
  int index = pScreen->myNum;
  CharInfoPtr pCI = pFont->pCI;
  FontInfoPtr pFI = pFont->pFI;
  CharInfoPtr pMaxBounds = &pFI->maxbounds;
  CharInfoPtr pMinBounds = &pFI->minbounds;
  unsigned int chDefault = pFI->chDefault;
  unsigned int firstRow = pFI->firstRow;
  unsigned int firstCol = pFI->firstCol;
  unsigned int lastCol = pFI->lastCol;
  int glyphWidth = pMaxBounds->metrics.rightSideBearing -
		   pMinBounds->metrics.leftSideBearing;
  int glyphHeight = pMaxBounds->metrics.ascent + pMaxBounds->metrics.descent;
  hpFontRec *pHpFrec;
  hpCharRange *pRange;
  int i, prop, startChar, lastChar;

  /*
   * if it's right-to-left, or if the
   * glyphs are bigger than 24 pixels wide,
   * then don't optimize it.
   */
  if(pFI->drawDirection || (glyphWidth > 24)) {
    pFont->devPriv[index] = (pointer) NULL;
    return TRUE;
  }

  /*
   * allocate an hpFontRec
   */
  pHpFrec = (hpFontRec *) Xalloc(sizeof (hpFontRec));

  /*
   * allocate the space for stippling a character
   */
  if((pHpFrec->stippleChunk = hpBufAlloc(pScreen, glyphWidth, glyphHeight)) ==
      NULL) {
    Xfree(pHpFrec);
    pFont->devPriv[index] = (pointer) NULL;
    return TRUE;
  }
  
  /*
   * Check to see if the font has properties telling which characters
   * to optimize
   */
  if(prop = lookUpProperty(pFont, "HPSTARTOPTIMIZE"))  startChar = prop;
  else  startChar = STARTCHAR;
  if(prop = lookUpProperty(pFont, "HPENDOPTIMIZE"))  lastChar = prop;
  else  lastChar = LASTCHAR;

  /*
   * allocate the offscreen, and put the chunk info in the hpFontRec
   */
  if(hpAllocFontMem(pScreen, pFont, pHpFrec, startChar, 
		    lastChar, CHARSPERCHUNK) == NULL) {
    hpBufFree(pScreen, pHpFrec->stippleChunk);
    Xfree(pHpFrec);
    pFont->devPriv[index] = (pointer) NULL;
    return TRUE;
  }

  /*
   * attach the hpFontRec to the devPriv field in the Font
   */
  pFont->devPriv[index] = (pointer) pHpFrec;

  pRange = pHpFrec->pRange;

  /*
   * store the font in the offscreen memory
   */
  for(i = 0; i < pHpFrec->NumChunks; i++) {
    hpStoreFont(pScreen, pFont, i, startChar + (i * CHARSPERCHUNK), 
		CHARSPERCHUNK);
    pRange[i].startChar = startChar + (i * CHARSPERCHUNK);
    pRange[i].endChar = pRange[i].startChar + CHARSPERCHUNK - 1;
  }
  pHpFrec->maxWidth = glyphWidth;
  pHpFrec->maxHeight = glyphHeight;
  pHpFrec->firstChar = startChar;
  pHpFrec->lastChar = lastChar;

  /*
   * check for the existence of the default glyph.  If it exists, then
   * hpStoreFont will have put it in place of any nonexistent glyphs.
   * If it doesn't exist, then the text output routines have to skip
   * any non-existent character.
   */
  pHpFrec->fDefaultExists = ((chDefault < firstCol) || 
			     (chDefault > lastCol) ||
                             !pCI[chDefault - firstCol].exists)? FALSE : TRUE;

  return TRUE;
}


/************************************************************************
 *  Routine:	hpUnrealizeFont
 *		If the font had been stored in offscreen memory
 *		then free all the associated memory.
 *
 *  Inputs: pScreen points to the ScreenRec the font was realized for
 *	    pFont points to the font we're unrealizing
 *  
 *  Returns: nothing of importance
 *
 *  Side Effects:  none
 *
 */

Bool
hpUnrealizeFont(pScreen, pFont)
  register ScreenPtr pScreen;
  register FontRec *pFont;
{
  int index = pScreen->myNum;
  FontInfoPtr pFI = pFont->pFI;
  CharInfoPtr pMaxBounds = &pFI->maxbounds;
  CharInfoPtr pMinBounds = &pFI->minbounds;
  register hpFontRec *pHpFrec;
  register int i;

  /*
   * test to see if we ever optimized this font
   */
  if(! pFont->devPriv[index]) {
    return TRUE; 
  }

  pHpFrec = (hpFontRec *) pFont->devPriv[index];

  /*
   * mark the font as unoptimized
   */
  pFont->devPriv[index] = (pointer) NULL;

  /*
   * free the offscreen chunks
   */
  for(i = 0; i < pHpFrec->NumChunks; i++) {
    hpBufFree(pScreen, pHpFrec->ppChunk[i]);
  }

  /*
   * free the stippleChunk
   */
  hpBufFree(pScreen, pHpFrec->stippleChunk);

  /*
   * free the chunk table
   */
  Xfree(pHpFrec->ppChunk);

  /*
   * free the range array
   */
  Xfree(pHpFrec->pRange);

  /*
   * finally, free the hpFontRec itself
   */
  Xfree(pHpFrec);
  return TRUE;
}


/************************************************************************
 *  Routine:	hpAllocFontMem
 *		Get enough offscreen memory on the screen to optomize
 *		the font and store the locations in an hpFontRec
 *
 *  Inputs: pScreen points to the ScreenRec we'll get the memory on 
 *	    pFont points to the font we're allocating for
 *	    phpFontRec we fill out telling where the memory is alloc'd
 *	    first and last specify the first and last glyphs to allocate for
 *	    size specifies the size of each chunk in glyphs
 *  
 *  Returns: 1 for success, 0 for not enough offscreen memory
 *
 *  Side Effects:  none
 *
 */

hpAllocFontMem(pScn, pFnt, pHpFrec, first, last, size)
  ScreenPtr pScn;
  FontRec *pFnt;
  hpFontRec *pHpFrec;
  int first, last, size;
{
  int i;
  int width, height; /* width & height of each chunk */
  int total = last - first + 1; /* total num chars to allocate for */
  int num_chunks = total/size;

  if(total%size) num_chunks++;

  pHpFrec->ppChunk = (hpChunk **) Xalloc(sizeof(hpChunk *) * num_chunks);
  pHpFrec->NumChunks = num_chunks;

  pHpFrec->pRange = (hpCharRange *) Xalloc(sizeof(hpCharRange) * num_chunks);

  /*
   * figure out how much memory we need
   * allocates enough memory to contain "total" number of maximum-sized glyphs
   */
  width = (pFnt->pFI->maxbounds.metrics.rightSideBearing -
	  pFnt->pFI->minbounds.metrics.leftSideBearing) * size;
  
  height = pFnt->pFI->maxbounds.metrics.ascent + 
	   pFnt->pFI->maxbounds.metrics.descent;

  for(i = 0; i < num_chunks; i++) {

    /*
     * get offscreen memory
     */
    if((pHpFrec->ppChunk[i] = hpBufAlloc(pScn, width, height)) == NULL) {
      /*
       * if it fails, we've gotta free up all the data structures
       */
      while(i) {
	hpBufFree(pScn, pHpFrec->ppChunk[--i]);
      }
      pHpFrec->NumChunks = 0;
      Xfree(pHpFrec->ppChunk);
      pHpFrec->ppChunk = (hpChunk **) NULL;
      Xfree(pHpFrec->pRange);
      pHpFrec->pRange = (hpCharRange *) NULL;
      return 0;
    }
  }
  return 1;
}


/************************************************************************
 *  Routine:	hpStoreFont
 *		Write font glyphs into previously allocated offscreen memory
 *		Stores a contiguous range of glyphs e.g. 32-63
 *		Calls hpWholeGlyph (LIE) to put the bits in the framebuffer
 *		with no clipping
 *
 *  Inputs: pScreen points to the ScreenRec whose memory we write to
 *	    pFont points to the font we'll store in offscreen
 *	    index gets us to the chunk to store the glyphs in
 *	    start and num define the range of glyphs to store
 *  
 *  Returns: nothing of importance
 *
 *  Side Effects:  none
 *
 */

hpStoreFont(pScn, pFnt, chunkNum, startGlyph, numGlyphs)
  ScreenPtr pScn;
  register FontRec *pFnt;
  int chunkNum; /* index into array of chunks */
  int startGlyph;
  register numGlyphs;
{
  int glyphNum = startGlyph;
  char *pCurrentGlyph;
  register CharInfoPtr pCi;
  int cellWidth = pFnt->pFI->maxbounds.metrics.rightSideBearing -
		  pFnt->pFI->minbounds.metrics.leftSideBearing;
  int defaultChar = pFnt->pFI->chDefault;
  hpChunk *pChunk = ((hpFontRec *)pFnt->devPriv[pScn->myNum])->
				  ppChunk[chunkNum];
  cfbPrivScreenPtr pPrivScn = (cfbPrivScreenPtr)(pScn->devPrivate);
  register x;
  int y;
  unsigned int firstCol = pFnt->pFI->firstCol;
  unsigned int lastCol = pFnt->pFI->lastCol;
  CharInfoPtr pDefaultCi = (CharInfoPtr) &(pFnt->pCI[defaultChar - firstCol]);
  char *pDefaultGlyph = pFnt->pGlyphs + pDefaultCi->byteOffset;
  int fDefaultExists = ((defaultChar < firstCol) || (defaultChar > lastCol) ||
                         !pDefaultCi->exists)? 0 : 1;


  x = pChunk->x;
  y = pChunk->y;

  while(numGlyphs--) {
    if(glyphNum > lastCol) break;

    pCi = &(pFnt->pCI[glyphNum - firstCol]);
    if((glyphNum >= firstCol) && pCi->exists) {
      pCurrentGlyph = pFnt->pGlyphs + pCi->byteOffset;
      /* XXX this needs to change for screen independence */
      switch(pPrivScn->gcid) {
        case 9:
	      {
 	      CATSEYE *gp_hardware = getGpHardware(pScn);
              SET_CTL_SPACE ;   /* for fireye */
	      if(gp_hardware->id_second < LCC)
                 if (gp_hardware->bits)
		     mrtcWholeGlyph(pScn, pCurrentGlyph, pCi, x, y, GXcopy, ~0,
         	        0, pPrivScn->planesMask);
		 else										     tcWholeGlyph(pScn, pCurrentGlyph, pCi, x, y, GXcopy, ~0,
			0, pPrivScn->planesMask);	
	      else {
		    SET_FRM_SPACE ;
		    ceWholeGlyph(pScn, pCurrentGlyph, pCi, x, y, GXcopy, ~0, 0, 
			pPrivScn->planesMask);
		   }
              }
		break;
        case 10: renWholeGlyph(pScn, pCurrentGlyph, pCi, x, y, GXcopy, ~0, 0, 
		     pPrivScn->planesMask);
		break;
        case 14: davWholeGlyph(pScn, pCurrentGlyph, pCi, x, y, GXcopy, ~0, 0, 
		     pPrivScn->planesMask);
		break;
      }
    }
    else {
      if(fDefaultExists) {
        switch(pPrivScn->gcid) {
          case 9:
	      {
 	      CATSEYE *gp_hardware = getGpHardware(pScn);
              SET_CTL_SPACE  ;
	      if(gp_hardware->id_second < LCC)
                 if (gp_hardware->bits)
		     mrtcWholeGlyph(pScn, pCurrentGlyph, pCi, x, y, GXcopy, ~0,
         	        0, pPrivScn->planesMask);
		 else										     tcWholeGlyph(pScn, pCurrentGlyph, pCi, x, y, GXcopy, ~0,
			0, pPrivScn->planesMask);	
	      else {
		    SET_FRM_SPACE ;
		    ceWholeGlyph(pScn, pDefaultGlyph, pDefaultCi, x, y, GXcopy,
			~0, 0, pPrivScn->planesMask);
		   }
              }
		  break;
          case 10: renWholeGlyph(pScn, pDefaultGlyph, pDefaultCi, x, y, GXcopy, ~0, 0, 
		     pPrivScn->planesMask);
		  break;
          case 14: davWholeGlyph(pScn, pDefaultGlyph, pDefaultCi, x, y, GXcopy, ~0, 0, 
		     pPrivScn->planesMask);
		  break;
        }
      }
    }
    glyphNum++;
    x += cellWidth;
    pCi++;
  }
  SET_FRM_SPACE ;   /* for fireye */
}
