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
 * Functions implementing Apollo color display-specific parts of the driver
 * having to do with driver initialization.
 */
                    
#include "apollo.h"
#include "mi.h"
#include "sys/ins/pfm.ins.c"

#ifdef SWITCHER
#include "sys/ins/pad.ins.c"
#include "sys/ins/ec2.ins.c"
#include "sys/ins/mutex.ins.c"
#include "sys/ins/sfcb.ins.c"
#include "sys/ins/ios.ins.c"

#include "switcher.h"
#endif

/*
 * externs for apc drawing routines
 */
extern Bool     apcScreenInit();
extern void     apcQueryBestSize();

/*
 * externs for pseudocolor colormap support
 */
extern void     apClrCreateColormap();
extern void     apClrDestroyColormap();
extern void     apScanAndUpdateColormap();
extern void     apClrInstallColormap();
extern void     apClrUninstallColormap();
extern void     apClrListInstalledColormaps();
extern void     apClrStoreColors();
extern void     apClrResolveColor();

/*
 * externs for color cursor support
 */
extern void     apClrCursorUp();
extern void     apClrCursorDown();
extern Bool     apClrRealizeCursor();
extern Bool     apClrUnrealizeCursor();
extern void     apClrDisplayCursor();

/*
 * externs for display-independent cursor support
 */
extern void     apInitCursor();

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
    gpr_$attribute_desc_t tile_attrib;
    time_$clock_t       acq_timeout; 
#ifdef SWITCHER
    pad_$window_desc_t  pwin;
#endif

#define winName "/tmp/xRootWindow"
#define winNameL (sizeof(winName)-1)

    disp.x_size = pDisp->display_char.x_visible_size;
    disp.y_size = pDisp->display_char.y_visible_size;

#ifdef SWITCHER
    pwin.top = pwin.left = 0;
    pwin.width = pDisp->display_char.x_visible_size;
    pwin.height = pDisp->display_char.y_visible_size;
    pad_$create_window( *winName, winNameL, pad_$transcript,
                        (short)(pDisp->display_unit), pwin, 
                        pDisp->winSid, status );
    if (status.all == status_$ok) {
        pad_$set_border( pDisp->winSid, (short) 1, false, status );
        pad_$set_auto_close( pDisp->winSid, (short) 1, true, status );
        gpr_$init (gpr_$direct, pDisp->winSid, disp,
                   (gpr_$rgb_plane_t)(pDisp->depth-1),
                   pDisp->display_bitmap, status);
        if (status.all == status_$ok) {
            /* acquire display and keep forever */
            acq_timeout.high = 0x0FFFFFFF; /* 8.5 years */
            gpr_$set_acq_time_out( acq_timeout, status );
            gpr_$set_obscured_opt( gpr_$block_if_obs, status );
            gpr_$acquire_display( status );
            gpr_$inq_color_map( (gpr_$pixel_value_t) 0,
                                (short) (1<<pDisp->depth),
                                *pDisp->saved_cmap, status );
        }
    }
#else
    gpr_$init (gpr_$borrow, (short)(pDisp->display_unit),
               disp, (gpr_$rgb_plane_t)(pDisp->depth-1), pDisp->display_bitmap, status);
#endif
    if (status.all != status_$ok) pfm_$signal( status );

    pDisp->attribute_block = gpr_$attribute_block (pDisp->display_bitmap, status);

    gpr_$inq_bitmap_pointer (pDisp->display_bitmap, pDisp->bitmap_ptr, wpl, status);
    pDisp->words_per_line = wpl;

    /* allocate bitmaps for tiles */
    gpr_$allocate_attribute_block( tile_attrib, status );
    disp.x_size = TILE_SIZE;
    disp.y_size = TILE_SIZE;
    gpr_$allocate_bitmap (disp, (gpr_$rgb_plane_t)(pDisp->depth-1),
                          tile_attrib, pDisp->tile_bitmap, status);
    gpr_$allocate_bitmap (disp, (gpr_$rgb_plane_t) 1,
                          tile_attrib, pDisp->opStip_bitmap, status);
    gpr_$set_bitmap( pDisp->tile_bitmap, status );
    gpr_$set_clipping_active( true, status );
    gpr_$set_bitmap( pDisp->display_bitmap, status );

    /* allocate and init cursor bitmaps and stuff */
    disp.x_size = APCURSOR_SIZE * 2;
    disp.y_size = APCURSOR_SIZE;
    gpr_$allocate_hdm_bitmap( disp, (gpr_$rgb_plane_t)(pDisp->depth-1),
                              pDisp->attribute_block, pDisp->HDMCursor, status);
  
    gpr_$set_clipping_active (true, status);
}

/* 
 * apClrTerminate -- gpr_$terminate, and clean up any other
 *                   mess, like switcher root window pad.
 */

static void apClrTerminate(pDisp)
    apDisplayDataPtr    pDisp;
{
    status_$t           status;

#ifdef SWITCHER
    gpr_$set_color_map( (gpr_$pixel_value_t) 0,
                        (short) (1<<pDisp->depth),
                        *pDisp->saved_cmap, status );
#endif

    gpr_$terminate( false, status );

#ifdef SWITCHER
    ios_$delete( pDisp->winSid, status );
#endif
}

#ifdef SWITCHER
/*
 * apClrReborrow -- Driver internal code
 *      Return the given screen number, then wait to take it back again.
 */

extern xoid_$t xoid_$nil;

static int mytype[2] = {0x37E231FB,0xA000AF6A};

static void apClrReborrow (numScr)
    int numScr;
{
    apDisplayDataPtr    pDisp;
    time_$clock_t       wtime;
    switcher_sfcb       *mysfcb;
    short               ssize;
    int                 waitval;
    status_$t           status;

    pDisp = &apDisplayData[numScr];

/* 
 * Restore DM's color map
 */
    gpr_$set_color_map( (gpr_$pixel_value_t) 0,
                        (short) (1<<pDisp->depth),
                        *pDisp->saved_cmap, status );
/* release the display and make the X root window invisible */
    gpr_$release_display( status );
    pad_$make_invisible( pDisp->winSid, (short) 1, status );

/* wait for a signal from the tox program, and then make
   the window visible again */
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

    pad_$select_window( pDisp->winSid, (short) 1, status );

/*
 * Restart now
 */
    gpr_$acquire_display( status );

/* save again -- it may have changed */
    gpr_$inq_color_map( (gpr_$pixel_value_t) 0,
                        (short) (1<<pDisp->depth),
                        *pDisp->saved_cmap, status );
/*
 * Reestablish currently installed color map
 */
    apScanAndUpdateColormap (pDisp, pDisp->installedCmap);

/* get cursor image into hdm bitmap */
    (*pDisp->apDisplayCurs) ( numScr,
           ((apPrivPointrPtr) apPointer->devicePrivate)->pCurCursor );
}
#endif

/*
 * apClrScreenInit -- DDX (screen)
 *      Perform color screen-specific initialization.
 *      This basically involves device initialization, and filling in
 *      the entry in the apDisplayData array and the Screen record.
 */
Bool
apClrScreenInit(int index, ScreenPtr pScreen) {
    register PixmapPtr  pPixmap;
    Bool                retval;
    ColormapPtr         pColormap;
    VisualPtr           pVis;
    long               *pVids;
    apDisplayDataPtr    pDisp;
    int                 dpix, dpiy;
    status_$t           status;
    gpr_$rop_prim_set_t rop_set;
    short               i;
    pDisp = &apDisplayData[index];
    if (!(pDisp->bitmap_ptr))
        doGPRInit(pDisp);
    pDisp->lastGC = NULL;
                               
    /* HACK - setting prim set to ALL three members, fill, blt and line */
    rop_set = (gpr_$rop_prim_set_t)7;
    gpr_$raster_op_prim_set( rop_set, status);
                              
    /* It sure is moronic to have to convert metric to English units so that ScreenInit can undo it! */
                                                                                         
    dpix = (pDisp->display_char.x_pixels_per_cm * 254) / 100;
    dpiy = (pDisp->display_char.y_pixels_per_cm * 254) / 100;
    retval = apcScreenInit(index, pScreen, pDisp->display_bitmap,
                           pDisp->display_char.x_visible_size,
                           pDisp->display_char.y_visible_size,
                           dpix, dpiy);

    pScreen->QueryBestSize = apcQueryBestSize;
    pScreen->SaveScreen = apSaveScreen;

    pScreen->RealizeCursor = apRealizeCursor;
    pScreen->UnrealizeCursor = apUnrealizeCursor;
    pScreen->DisplayCursor = apDisplayCursor;
    pScreen->SetCursorPosition = apSetCursorPosition;
    pScreen->CursorLimits = apCursorLimits;
    pScreen->PointerNonInterestBox = apPointerNonInterestBox;
    pScreen->ConstrainCursor = apConstrainCursor;
    pScreen->RecolorCursor = miRecolorCursor;
    pScreen->backingStoreSupport = Always;
    pScreen->saveUnderSupport = NotUseful;

/* save the original Screeninit assigned routines, and put ours in */
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

/* initialize color cursor rendering vectors */
    pDisp->apRealizeCurs = apClrRealizeCursor;
    pDisp->apUnrealizeCurs = apClrUnrealizeCursor;
    pDisp->apDisplayCurs = apClrDisplayCursor;
    pDisp->apCursorUp = apClrCursorUp;
    pDisp->apCursorDown = apClrCursorDown;

    pDisp->apTerminate = apClrTerminate;
#ifdef SWITCHER
    pDisp->apReborrower = apClrReborrow;
#endif
#ifdef COPY_SCREEN
    pDisp->apCopyScreen = apClrCopyScreen;
    pDisp->dump_file_pn = (char *) Xalloc (sizeof(DEFAULT_SCREENDUMP_PN));
    strcpy(pDisp->dump_file_pn, DEFAULT_SCREENDUMP_PN);
    pDisp->dump_file_pnl = sizeof(DEFAULT_SCREENDUMP_PN) - 1;   /* don't count the trailing null */
#endif

/* initialize pseudocolor colormap */
    pScreen->CreateColormap = apClrCreateColormap;
    pScreen->DestroyColormap = apClrDestroyColormap;
    pScreen->InstallColormap = apClrInstallColormap;
    pScreen->UninstallColormap = apClrUninstallColormap;
    pScreen->ListInstalledColormaps = apClrListInstalledColormaps;
    pScreen->StoreColors = apClrStoreColors;
    pScreen->ResolveColor = apClrResolveColor;

/* create depth and visual data for screen */

    pDisp->visuals = pVis = (VisualPtr) Xalloc (sizeof(VisualRec));     /* only 1 visual: pseudocolor */
    pVis->vid = FakeClientID (0);
    pVis->screen = index;
    pVis->class = PseudoColor;
    pVis->redMask = pVis->greenMask = pVis->blueMask = 0;
    pVis->offsetRed = pVis->offsetGreen = pVis->offsetBlue = 0;
    pVis->bitsPerRGBValue = pDisp->display_char.lut_width_per_primary;
    pVis->ColormapEntries = 1 << (pDisp->depth);
    if ((pDisp->depth) = (pDisp->display_char.n_planes))
        pVis->ColormapEntries -= 2;                                     /* keep 2 slots for cursor */
    pVis->nplanes = pDisp->depth;
    AddResource(pVis->vid, RT_VISUALID, pVis, NoopDDA, RC_CORE);        /* will deallocate in CloseScreen */

    pDisp->depths = (DepthPtr) Xalloc (2 * sizeof(DepthRec));   /* only 2 depths: 1 and full-depth */
    pDisp->depths[0].depth = 1;
    pDisp->depths[0].numVids = 0;
    pDisp->depths[0].vids = NULL;
    pDisp->depths[1].depth = pDisp->depth;
    pDisp->depths[1].numVids = 1;
    pDisp->depths[1].vids = pVids = (long *) Xalloc (sizeof(long));
    pVids[0] = pVis->vid;

    pScreen->numDepths = 2;
    pScreen->allowedDepths = pDisp->depths;
    pScreen->rootDepth = pDisp->depth;
    pScreen->numVisuals = 1;
    pScreen->visuals = pVis;
    pScreen->rootVisual = pVis->vid;

/* set up default colormap */

    pScreen->blackPixel = 0;
    pScreen->whitePixel = 1;
    pScreen->minInstalledCmaps = 1;
    pScreen->maxInstalledCmaps = 1;
    pScreen->defColormap = FakeClientID (0);
    CreateColormap(pScreen->defColormap, pScreen, pVis, &pColormap, AllocNone, 0);
    pDisp->installedCmap = NULL;
    pScreen->InstallColormap (pColormap);

/* initialize Apollo SW cursor */
    pDisp->cursForePix = (1 << (pDisp->display_char.n_planes)) - 1;
    pDisp->cursBackPix = pDisp->cursForePix - 1;
    apInitCursor();

    gpr_$enable_direct_access (status);

    return(retval);
}
