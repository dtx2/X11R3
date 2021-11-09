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
 * Functions implementing Apollo monochrome display-specific parts of the driver
 * having to do with driver initialization.
 */
                    
#include "apollo.h"
#include "mi.h"
#include "mfb.h"

#ifdef SWITCHER
#include "sys/ins/ec2.ins.c"
#include "sys/ins/mutex.ins.c"
#include "sys/ins/sfcb.ins.c"

#include "switcher.h"
#endif

/*
 * externs for monochrome colormap support
 */
extern void     apMonoResolveColor();
extern void     apMonoCreateColormap();
extern void     apMonoDestroyColormap();

/*
 * externs for monochrome cursor support
 */
extern void     apMonoCursorUp();
extern void     apMonoCursorDown();
extern Bool     apMonoRealizeCurs();
extern Bool     apMonoUnrealizeCurs();
extern void     apMonoDisplayCurs();

/*
 * externs for display-independent cursor support
 */
extern void     apInitCursor();

/*
 * apMonoQueryBestSize -- DDX interface (server)
 *      Return optimal sizes for cursors, tiles and stipples
 *      for our monochrome driver.
 */
static void
apMonoQueryBestSize(class, pwidth, pheight)
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
          width = *pwidth;
          if (width > 0) {
          /* Return the closest power of two not less than what they gave me */
          test = 0x80000000;
          /* Find the highest 1 bit in the width given */
          while(!(test & width))
             test >>= 1;
          /* If their number is greater than that, bump up to the next
           *  power of two */
          if((test - 1) & width)
             test <<= 1;
          *pwidth = test;
          /* We don't care what height they use */
          }
          break;
    }
}

/*
 * doGPRInit -- Driver internal code
 *      Given a pointer to the display data record, do all the screen
 *      hardware initialization work.
 */
static void doGPRInit(pDisp)
    apDisplayDataPtr    pDisp;
{
    status_$t           status;
    gpr_$offset_t       disp;
    short               wpl;
    short               n_entries;
    gpr_$pixel_value_t  start_index;
    gpr_$color_vector_t colors;

    disp.x_size = pDisp->display_char.x_visible_size;
    disp.y_size = pDisp->display_char.y_visible_size;

    gpr_$init(gpr_$borrow, (short)(pDisp->display_unit),
              disp, (gpr_$rgb_plane_t)0, pDisp->display_bitmap, status);

    colors[0] = gpr_$black;
    colors[1] = gpr_$white;
    start_index = 0;
    n_entries = 2;
    gpr_$set_color_map (start_index, n_entries, colors, status);
    gpr_$inq_bitmap_pointer (pDisp->display_bitmap, pDisp->bitmap_ptr, wpl, status);
    pDisp->words_per_line = wpl;

    gpr_$enable_direct_access (status);
}

/* 
 * apMonoTerminate -- gpr_$terminate, and clean up any other
 *                   mess needed.
 */

static void apMonoTerminate(pDisp)
    apDisplayDataPtr    pDisp;
{
    status_$t           status;

    gpr_$terminate( false, status );
}

#ifdef SWITCHER
/*
 * apMonoReborrow -- Driver internal code
 *      Return the given screen number, then wait to take it back again.
 */

extern xoid_$t xoid_$nil;

static int mytype[2] = {0x37E231FB,0xA000AF6A};

static void apMonoReborrow (numScr)
    int numScr;
{
    apDisplayDataPtr    pDisp;
    time_$clock_t       wtime;
    switcher_sfcb       *mysfcb;
    short               ssize;
    int                 waitval;
    status_$t           status;

    pDisp = &apDisplayData[numScr];

    gpr_$terminate (TRUE, status);
/*
 * Wait for the restart via a shared eventcount, found in an sfcb
 */
    ssize = sizeof(*mysfcb);
    sfcb_$get (mytype, xoid_$nil, ssize, mysfcb, status);
    if (mysfcb->use_count == 1)
    {
        ec2_$init (mysfcb->theEc);
        mysfcb->use_count = 2;
        mysfcb->xGo = 0;
    }
    mutex_$unlock (mysfcb->slock);
    waitval = mysfcb->theEc.value + 1;
    while (!mysfcb->xGo)
        ec2_$wait (&mysfcb->theEc, waitval, (short) 1, status);
    mysfcb->xGo = 0;
/*
 * Restart now
 */
    doGPRInit(pDisp);
/*
 * HACK ALERT!  Update the mfb display memory address (might not be the same as
 *      the original value)
 */
    ((PixmapPtr)(screenInfo.screen[numScr].devPrivate))->devPrivate = pDisp->bitmap_ptr;
}
#endif

/*
 * apMonoScreenInit -- DDX (screen)
 *      Perform monochrome screen-specific initialization.
 *      This basically involves device initialization, and filling in
 *      the entry in the apDisplayData array and the Screen record.
 */
Bool
apMonoScreenInit(int index, ScreenPtr pScreen) {
    PixmapPtr pPixmap;
    Bool retval;
    ColormapPtr pColormap;
    apDisplayDataPtr pDisp;
    int dpix, dpiy;
    status_$t status;
    short i;
    pDisp = &apDisplayData[index];
    if (!(pDisp->bitmap_ptr)) { doGPRInit(pDisp); }
    // It sure is moronic to have to convert metric to English units so that mfbScreenInit can undo it!
    dpix = (pDisp->display_char.x_pixels_per_cm * 254) / 100;
    dpiy = (pDisp->display_char.y_pixels_per_cm * 254) / 100;
    retval = mfbScreenInit(index, pScreen, pDisp->bitmap_ptr,
                           pDisp->display_char.x_visible_size,
                           pDisp->display_char.y_visible_size,
                           dpix, dpiy);
    // Apollo screens may have large amounts of extra bitmap to the right of the visible area, therefore the
    // PixmapBytePad macro in mfbScreenInit gave the wrong value to the devKind field of the Pixmap it made for the
    // screen. So we fix it here.
    pPixmap = (PixmapPtr)(pScreen->devPrivate);
    pPixmap->devKind = (pDisp->words_per_line) << 1;
    pScreen->QueryBestSize = apMonoQueryBestSize;
    pScreen->SaveScreen = apSaveScreen;
    pScreen->RealizeCursor = apRealizeCursor;
    pScreen->UnrealizeCursor = apUnrealizeCursor;
    pScreen->DisplayCursor = apDisplayCursor;
    pScreen->SetCursorPosition = apSetCursorPosition;
    pScreen->CursorLimits = apCursorLimits;
    pScreen->PointerNonInterestBox = apPointerNonInterestBox;
    pScreen->ConstrainCursor = apConstrainCursor;
    pScreen->RecolorCursor = miRecolorCursor;
    // save the original Mfb assigned routines, and put ours in
    pDisp->CreateGC = pScreen->CreateGC;
    pDisp->CreateWindow = pScreen->CreateWindow;
    pDisp->ChangeWindowAttributes = pScreen->ChangeWindowAttributes;
    pDisp->GetImage = pScreen->GetImage;
    pDisp->GetSpans = pScreen->GetSpans;
    pScreen->CreateGC = apCreateGC;
    pScreen->CreateWindow = apCreateWindow;
    pScreen->ChangeWindowAttributes = apChangeWindowAttributes;
    pScreen->GetImage = apGetImage;
    pScreen->GetSpans = apGetSpans;
    // initialize monochrome cursor processing vectors
    pDisp->apRealizeCurs = apMonoRealizeCurs;
    pDisp->apUnrealizeCurs = apMonoUnrealizeCurs;
    pDisp->apDisplayCurs = apMonoDisplayCurs;
    pDisp->apCursorUp = apMonoCursorUp;
    pDisp->apCursorDown = apMonoCursorDown;
    pDisp->apTerminate = apMonoTerminate;
    // initialize monochrome colormap
    pScreen->ResolveColor = apMonoResolveColor;
    pScreen->CreateColormap = apMonoCreateColormap;
    pScreen->DestroyColormap = apMonoDestroyColormap;
    CreateColormap(pScreen->defColormap, pScreen, LookupID(pScreen->rootVisual, RT_VISUALID, RC_CORE), &pColormap, AllocNone, 0);
    mfbInstallColormap(pColormap);
    // initialize Apollo SW cursor
    pDisp->noCrsrChk = 0;
    apInitCursor();
    return(retval);
}
