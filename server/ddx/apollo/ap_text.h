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

#ifndef _APOLLO_TEXT_H_
#define _APOLLO_TEXT_H_

/* ap_text.h, jah, 12/2/87      */

/* Declarations private to Apollo text support with the X server */

typedef struct _gprFID *gprFIDPtr;

typedef struct _gprFID {
    gprFIDPtr   nfr, pfr;
    short       use_count;
    FontPtr     theXFont;
    short       nGprFonts;
    short       fontIds[1];
} gprFIDRec;

extern gprFIDPtr mkGprFonts( /*snfFont */ );

extern Bool gprRealizeFont( /* pscr, pFont */ );
extern Bool gprUnrealizeFont( /* pscr, pFont */ );
extern void gprReloadFonts( /* scrNum */);
extern int gprImageText8();
extern int gprImageText16();
extern int gprPolyText8();
extern int gprPolyText16();
extern int nopText();
#endif
