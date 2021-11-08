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

/* mkGprFont.c, jah, 12/2/87 */

/* Given pointers to tables for a given snf font, produce and load the
   one or more gpr fonts containing the same glyphs, for use within the
   server  */

#include    "X.h"
#include    "Xmd.h"
#include    "Xproto.h"
#include    "fontstruct.h"
#include    "dixfontstr.h"
#include    "scrnintstr.h"
#include    "screenint.h"

#include    "sys/ins/base.ins.c"
#include    "sys/ins/gpr.ins.c"

#include    "apcfont.h"
#include    "apctext.h"

#define fontPName "/tmp/gprXfont"
#define fontPNLen ((short)(sizeof(fontPName) - 1))

#define ABORT return( cleanup(gfp, fx, fid) )


static gprFIDPtr cleanup(gfp, fx, fid)
    gprFIDPtr           gfp;
    int                 fx;
    int                 fid;
{
    status_$t           st;

    font_$close( fid, true, st );
    unlink( fontPName );
    while (--fx >= 0)
        gpr_$unload_font_file( gfp->fontIds[fx], st );
    Xfree( gfp );
    return( NULL );
}


gprFIDPtr mkGprFonts(snfFont)
    FontPtr snfFont;
{

    short               nf;
    gprFIDPtr           gfp;
    short               fx;
    short               cx;
    short               fid;
    uid_$t              fuid;
    CharInfoPtr         pci;
    font_$char_info_t   chinfo;
    status_$t           st;

    nf = (snfFont->pFI->lastRow - snfFont->pFI->firstRow + 1) * (snfFont->pFI->lastCol / 128 + 1);
    gfp = (gprFIDPtr) Xalloc( sizeof(gprFIDRec) + (nf-1) * sizeof(short) );
    if (!gfp) return(gfp);

    gfp->nGprFonts = nf;
    gfp->theXFont = snfFont;

    fx = -1;
    cx = snfFont->pFI->firstCol;
    pci = snfFont->pCI;

    unlink( fontPName );

    font_$init();

    for (;;) {
        if ( cx == snfFont->pFI->firstCol || cx == 128 ) {

            /* close old font, gpr-font-load it, and delete-when-unlocked it */
            if ( fx >= 0 ) {
                font_$close( fid, true, st );
                if (st.all != status_$ok) ABORT;
                gpr_$load_font_file( *fontPName, fontPNLen, gfp->fontIds[fx], st );
                if (st.all != status_$ok) ABORT;
                unlink( fontPName );
            }

            if ( ++fx >= nf ) return( gfp );

            if ( snfFont->pFI->firstCol >= 128 )
                gfp->fontIds[fx++] = 0;   /* lower half is empty -- don't make a font */

            /* make a new font */
            font_$create( *fontPName, fontPNLen, fid, st );
            if (st.all != status_$ok) ABORT;
            font_$set_value( fid, font_$h_spacing, (long) 0, st );
            font_$set_value( fid, font_$space_size, (long) 0, st );
        }

        /* put in one character */

        if ( pci->exists ) {
            chinfo.left = -pci->metrics.leftSideBearing;
            chinfo.right = pci->metrics.rightSideBearing;
            chinfo.up = pci->metrics.ascent;
            chinfo.down = pci->metrics.descent;
            chinfo.width = pci->metrics.characterWidth;
            /* special case for zero-size glyphs */
            if (chinfo.left+chinfo.right==0 || chinfo.up+chinfo.down==0) {
                if (chinfo.width != 0) { /* just omit if width is also zero */
                    /* put in a one-by-one glyph with value zero */
                    /* it happens that "variable.snf" exercises this code */
                    chinfo.left = 0;
                    chinfo.right = 1;
                    chinfo.up = 1;
                    chinfo.down = 0;
                    font_$set_char_info( fid, (short) cx & 0x7F, chinfo, st );
                    if (st.all != status_$ok) ABORT;
                    font_$set_char_image( fid, (short) cx & 0x7F, 0, (short) 1, (short) 1, st );
                    if (st.all != status_$ok) ABORT;
                }
            }
            else { /* normal glyph */
                font_$set_char_info( fid, (short) cx & 0x7F, chinfo, st );
                if (st.all != status_$ok) ABORT;
                font_$set_char_image( fid, (short) cx & 0x7F, *(snfFont->pGlyphs+pci->byteOffset),
                                      (short) (chinfo.left+chinfo.right), (short) (chinfo.up+chinfo.down), st );
                if (st.all != status_$ok) ABORT;
            }
        }

        pci++;
        if (++cx > snfFont->pFI->lastCol) cx = snfFont->pFI->firstCol;

    }
}


/* this list exists mainly in case there are multiple screens.  Also,
   it is used to reload the fonts in connection with the switcher */

static gprFIDPtr fontList = NULL;

/* gprRealizeFont - build corresponding gpr fonts on the fly, and store font
   ids in devPriv */
Bool
gprRealizeFont( pscr, pFont )
    ScreenPtr	pscr;
    FontPtr	pFont;
{
    gprFIDPtr gfp = fontList;

    /* look for existing font */
    while (gfp) {
        if (gfp->theXFont == pFont) {
            gfp->use_count++;
            pFont->devPriv[pscr->myNum] = (pointer) gfp;
            return( TRUE );
        }
        gfp = gfp->nfr;
    }

    /* make new gpr font */
    gfp = mkGprFonts( pFont );
    if (gfp) {
        if (fontList) fontList->pfr = gfp;
        gfp->nfr = fontList;
        gfp->pfr = NULL;
        fontList = gfp;
        gfp->use_count = 1;
    }
    /* and store as devPriv */
    pFont->devPriv[pscr->myNum] = (pointer) gfp;
    return( TRUE );
}


/* gprUnrealizeFont -- deallocate the font id record */
Bool
gprUnrealizeFont( pscr, pFont)
    ScreenPtr	pscr;
    FontPtr	pFont;
{
    gprFIDPtr gfp = (gprFIDPtr) pFont->devPriv[pscr->myNum];

    /* if all uses gone, then unlink and remove */
    if (gfp && !--(gfp->use_count)) {
        if (gfp->nfr) gfp->nfr->pfr = gfp->pfr;
        if (gfp->pfr) gfp->pfr->nfr = gfp->nfr;
                 else fontList = gfp->nfr;
        Xfree( pFont->devPriv[pscr->myNum] );
    }
    return (TRUE);
}

#ifdef SWITCHER

/* gprReloadFonts() -- used after return from switching to DM */

void gprReloadFonts(scrNum)
    int scrNum;
{
    gprFIDPtr oldgfp = fontList;
    gprFIDPtr gfp;

    fontList = NULL;
    while (oldgfp) {
        gfp = mkGprFonts( oldgfp->theXFont );
        if (fontList) fontList->pfr = gfp;
        gfp->nfr = fontList;
        gfp->pfr = NULL;
        fontList = gfp;
        gfp->use_count = oldgfp->use_count;
        oldgfp->theXFont->devPriv[scrNum] = (pointer) gfp;
        Xfree( oldgfp );
        oldgfp = oldgfp->nfr;
    }
}

#endif
