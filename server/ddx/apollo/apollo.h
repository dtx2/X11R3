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

#ifndef _APOLLO_H_
#define _APOLLO_H_

#include "sys/ins/base.ins.c"
#include "sys/ins/gpr.ins.c"
   
#include <errno.h>
#include <stdio.h>
#include <sys/file.h>
#include <sys/time.h>
#include <sys/types.h>

#include    "X.h"
#include    "Xproto.h"
#include    "cursorstr.h"
#include    "gcstruct.h"
#include    "inputstr.h"
#include    "opaque.h"
#include    "scrnintstr.h"
#include    "regionstr.h"
#include    "windowstr.h"

#ifdef COPY_SCREEN
#define DEFAULT_SCREENDUMP_PN "x_screen"
#endif

/*
 * an entry for the Apollo display-specific data array
 */
typedef struct {
        /* GC Creation functions previously in the Screen structure */
    Bool                (*CreateGC)();
    Bool                (*CreateWindow)();
    Bool                (*ChangeWindowAttributes)();
    unsigned int        *(*GetSpans)();
    void                (*GetImage)();
        /* Apollo display data */
    int                 display_unit;   /* OS unit number */
    gpr_$disp_char_t    display_char;   /* display characteristics record */
    gpr_$bitmap_desc_t  display_bitmap; /* GPR bitmap ID */
    pointer             bitmap_ptr;     /* address of bitmap */
    int                 words_per_line; /* number of 16-bit words of address space from one scanline to the next */
    int                 depth;          /* actual depth being used (may be less than available depth) */
#ifdef SHARE
    Bool                sharing;        /* sharing display with other server(s) */
    Bool                ownsRoot;       /* is owner of the root window (must be true if not sharing) */
#endif
#ifdef COPY_SCREEN
    void                (*apCopyScreen)();      /* Routine to dump whole screen */
    char                *dump_file_pn;          /* Pathname of file to dump to */
    int                 dump_file_pnl;          /* Length of pathname */
#endif
#ifdef SWITCHER
    void                (*apReborrower)();      /* Code to reinitialize display */
    short               winSid;                 /* Stream-id for window */
    gpr_$color_vector_t saved_cmap;             /* colormap to restore on "return" */
#endif
    void                (*apTerminate)();       /* Device-dependent exit code */

        /* Drawing state data */
    
    /*  the strategy being used (dealing with current bitmap, attribute blocks, and GCs) is
     *  forcing attribute blocks on different screens to be the same.  To change this
     *  assumption, the serial number of the last validated GC must be bumped in the routine
     *  plfbgpr_$set_bitmap when the bitmap really gets changed.  There is a global declared
     *  in plfbvalidateGC which is a pointer to the last GC validated.  Whenever a NEW gc is
     *  validated then the serial number of the one through this pointer is bumped.  Once the
     *  NEW gc is validated then the pointer is updated to point to this.   -reber
     */

    gpr_$attribute_desc_t attribute_block;  /* gpr's attribute block */
    GCPtr		lastGC, lastShadowGC;		/* pointers to last GCs validated. */
    gpr_$bitmap_desc_t  tile_bitmap;        /* bitmap ID for tile filling */
    gpr_$bitmap_desc_t  opStip_bitmap;      /* bitmap ID for tile filling with opaque stipple */
    gpr_$bitmap_desc_t  HDMCursor;          /* hdm bitmap ID for cursor saving and drawing */
    pointer             cursor_bm_ptr;      /* pointer to hdm cursor bitmap */
    unsigned long       noCrsrChk;          /* set bits mean drawing op does its own cursor check */
#define NO_CRSR_CHK_ITEXT ((unsigned long) 1)
#define NO_CRSR_CHK_PTEXT ((unsigned long) 2)

        /* Depth, visual and colormap data */
    DepthPtr            depths;                 /* Address of screen depths array */
    VisualPtr           visuals;                /* Address of screen visuals array */
    ColormapPtr         installedCmap;          /* Pointer to currently installed colormap */

        /* Cursor rendering data */
    unsigned long       cursBackPix;            /* Pixel value for cursor background color */
    unsigned long       cursForePix;            /* Pixel value for cursor foreground color */

        /* Apollo-display-specific cursor routines */
    Bool                (*apRealizeCurs)();     /* Routine to fill in display-specific cursor data */
    Bool                (*apUnrealizeCurs)();   /* Routine to free display-specific cursor data */
    void                (*apDisplayCurs)();     /* Routine to change cursors */
    void                (*apCursorUp)();        /* Routine to save screen pixels and put cursor up */
    void                (*apCursorDown)();      /* Routine to take cursor down by putting back the screen pixels */
    std_$call void      (*remapColorMem)();     /* Which gpr_$remap_color_memory to call */
/* Don't put any procedure pointers after the above std_$call declaration */
} apDisplayDataRec, *apDisplayDataPtr;

/*
 * private data for Apollo cursors on all display types
 */
typedef struct {
        /* data established at realization time */
    int             realizedWidth;      /* width actually realized */
    int             realizedHeight;     /* height actually realized */
    pointer         pRealizedData;      /* device dependent bits for realization */
        /* cursor state data */
    BoxRec          cursorBox;          /* box on screen which is extent of cursor */
    int             alignment;          /* cursor origin x coordinate modulo 16 */
    pointer         pBitsScreen;        /* 16-bit aligned "address" on screen of cursor origin ... */
                                        /* ... (equals "address" of cursorBox upper left corner) */
    pointer         pStateData;         /* device dependent bits for cursor state */
    } apPrivCursRec, *apPrivCursPtr;

/*
 * private data for Apollo keyboards
 */
typedef struct {
    time_$clock_t   beepTime;                   /* duration of bell */
    Bool            (* LegalModifierPtr) ();    /* LegalModifier function implementation for this keyboard */
    void            (* HandleKeyPtr) ();        /* Key transition event handler for this keyboard */
#ifdef SWITCHER
    void            (*apHWInitKbd)();           /* Code to reinitialize keyboard */
#endif
    } apPrivKbdRec, *apPrivKbdPtr;

/*
 * private data for Apollo pointer devices
 */
typedef struct {
    int         numCurScreen;   /* number of screen pointer is currently on */
    CursorPtr   pCurCursor;     /* current cursor hooked to the pointer */
    int         hotX;           /* x coord of current cursor hot spot */
    int         hotY;           /* y coord of current cursor hot spot */
    int         x;              /* x coord of pointer position */
    int         y;              /* y coord of pointer position */
    BoxRec      constraintBox;  /* screen box that (x,y) is constrained to stay within */
    BoxRec      nonInterestBox; /* screen box within which motion events need not be reported */
                                /* nonInterestBox is currently ignored */
#ifdef SWITCHER
    void       (*apHWInitPointr)();     /* Code to reinitialize pointer */
#endif
    } apPrivPointrRec, *apPrivPointrPtr;


/*
 * The actual array of Apollo display data, indexed by screen number
 */
extern apDisplayDataRec apDisplayData[];

/* All tiles used by gpr are TILE_SIZE wide and high */
#define TILE_SIZE 32

/* Actual cursor always 32x32 */
#define APCURSOR_SIZE 32

/*
 * Variables from the input part of the driver:
 * pointers to the "main" (i.e. only) keyboard and pointer device records,
 * and the last time an event was noted
 */
extern DevicePtr        apKeyboard;
extern DevicePtr        apPointer;
extern int              lastEventTime;

/*
 * Variables from the GPR type manager:
 * the most recent fundamental input event data
 */
extern gpr_$event_t     apEventType;
extern unsigned char    apEventData[1];
extern gpr_$position_t  apEventPosition;

/*
 * Variables from the GPR type manager:
 * pointers to the current input eventcount and the last eventcount value noted
 */
extern long             *apECV;
extern long             *apLastECV;

/*
 * externs for driver routines expected to be common to all Apollo drivers:
 * our replacements for GC creation functions from the Screen structure
 */
extern Bool             apCreateGC();
extern Bool             apCreateWindow();
extern Bool             apChangeWindowAttributes();
extern void             apGetImage();
extern unsigned int     *apGetSpans();

/*
 * externs for driver routines expected to be common to all Apollo drivers:
 * our implementations of DDX cursor functions
 */
extern void             miRecolorCursor();
extern Bool             apRealizeCursor();
extern Bool             apUnrealizeCursor();
extern Bool             apDisplayCursor();
extern Bool             apSetCursorPosition();
extern void             apCursorLimits();
extern void             apPointerNonInterestBox();
extern void             apConstrainCursor();

/*
 * externs for driver routines expected to be common to all Apollo drivers:
 * our implementations of DDX input functions
 */
extern Bool             apSaveScreen();
extern int              apGetMotionEvents();
extern void             apChangePointerControl();
extern void             apChangeKeyboardControl();
extern void             apBell();
extern int              apMouseProc();
extern int              apKeybdProc();

#endif
