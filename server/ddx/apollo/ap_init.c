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
 * Functions implementing Apollo-display-independent parts of the driver
 * having to do with driver initialization.
 */

#include "apollo.h"
#include "servermd.h"

extern char *index();

/*
 * defns for argument processing
 */

#define MIN_DISPLAY_UNIT 1
#define MAX_DISPLAY_UNIT 2
#define MAX_UNITS (MAX_DISPLAY_UNIT-MIN_DISPLAY_UNIT+1)
#define MAX_DEPTH_SUPPORTED 8

static char nullArgString[] = "";

/*
 * Statics holding info passed from InitOutput to apUnitScreenInit.
 * This hackery exists because InitOutput does not call apUnitScreenInit
 * directly, rather it passes the address of apUnitScreenInit to AddScreen,
 * which is the actual caller.  Thus we have no control over arguments passed
 * to apUnitScreenInit; they must be the standard ones.  So we store the stuff
 * we really want to pass here.
 */

typedef struct _Unit {
    short               unit;           /* OS unit number */
    short               actualDepth;    /* actual depth to use for this unit */
    char               *argString;      /* pointer to arg string for this unit, or null string if none given */
    gpr_$disp_char_t    dispChar;       /* display characteristics for this unit */
} UnitRec;

static int      nUnits = 0;
static UnitRec  unitList[MAX_UNITS];
static int      currentUnitIndex;

/*
 * Put the actual display data here.
 */
apDisplayDataRec    apDisplayData[MAXSCREENS];

/*
 * List of externs for the various display type initialization routines.
 */
extern Bool     apMonoScreenInit();
extern Bool     apClrScreenInit();

/*
 * apUnitScreenInit -- DDX interface (screen)
 *      Try to initialize the display described in the above unitList, at index
 *      given by currentUnitIndex, by figuring out which Apollo display-dependent
 *      initializer to call.
 *
 *      We ignore the command line.
#ifdef SHARE
 *      However, we do the following arg string parsing here:
 *              s+  server shares display with other window systems (notably DM)
 *              s-  server borrows display (stands alone)
 *              r+  server "owns" the root rectangle (forced if borrowing display)
 *              r-  server assumes someone else "owns" root rectangle
 *      Default is "s-r+"; if "s+" is specified, default for root rectangle is "r-".
#endif
 */
static Bool
apUnitScreenInit(screenIndex, pScreen, argc, argv)
    int         screenIndex;
    ScreenPtr   pScreen;
    int         argc;   /* these two may NOT be changed */
    char      **argv;
{
    apDisplayDataPtr    pDisp;
    UnitRec            *pUnit;
    char               *arg_p;

    pDisp = &apDisplayData[screenIndex];
    pUnit = &unitList[currentUnitIndex];

    pDisp->display_unit = pUnit->unit;
    pDisp->display_char = pUnit->dispChar;
    pDisp->depth = pUnit->actualDepth;

#ifdef SHARE
    if ( (arg_p = index(pUnit->argString, 's')) &&
         (*(++arg_p) == '+') )
    {
        pDisp->sharing = TRUE;
        if ( (arg_p = index(pUnit->argString, 'r')) &&
             (*(++arg_p) == '+') )
            pDisp->ownsRoot = TRUE;
        else
            pDisp->ownsRoot = FALSE;
    }
    else
    {
        pDisp->sharing = FALSE;
        pDisp->ownsRoot = TRUE;
    }
#endif

/*
 *  Here we must determine the display type and depth, and call one of several
 *  initialization routines, depending on what they are.
 */
    if ((pDisp->display_char.controller_type) == gpr_$ctl_none)
            /* whoops, made a BIG mistake! No display here! */
        return (FALSE);
                                  
    if (pDisp->depth == 1) return (apMonoScreenInit(screenIndex, pScreen, argc, argv));
    else return (apClrScreenInit(screenIndex, pScreen, argc, argv));
}

/*
 * InitOutput -- DDX interface (server)
 *      Establish the pixmap bit and byte orders, and scanline data,
 *      and the supported pixmap formats.
 *
 *      Then try to initialize screens.  If no "-D" options were given
 *      on the command line, initialize all display units that exist,
 *      using the default screen behaviors.
 *      If there are any -D options, initialize only the units explicitly
 *      named.
 *
 *      We parse the arg string for depth specifications here, because they
 *      affect the set of formats we must declare for the screen.  Other arg
 *      string parsing should be done inside apUnitScreenInit.
 *
 *      We save our work in static data, so that on the second and subsequent
 *      times through, we skip the argument parsing and hardware inquiry.
 *      However, we do reconstruct the format list each time through.  Why?
 *      Because we have to copy it to the screenInfo every time anyway, so
 *      we wouldn't save much by squirreling it away.
 */
InitOutput(screenInfo, argc, argv)
    ScreenInfo *screenInfo;
    int argc;
    char *argv[];
{
    int         i, j, depth, num_formats;
    char       *arg_p;
    short       unit;
    short       disp_len_ret;
    status_$t   status;

    if (!nUnits)        /* First time, must parse args and look for hardware */
    {

/*
 * Try to find -D<unit> options, and write down the unit number and a pointer to the arg string
 * (or a null string if there is no arg string) for each one found.
 *
 * Argument syntax recognized here is:
 *      <arg> ::= -D<unit> [<arg-string>]
 *      <arg-string> ::= [arg-option]*
 *      <arg-option> ::= { d<depth> | <other-stuff> }
 * where <unit> is an integer representing the display hardware unit number (1 or greater),
 * and <depth> is an integer representing the number of bitplanes to use (up to the number
 * actually provided by the hardware).  Only depths of "reasonable" round numbers are supported.
 * <other-stuff> is whatever is parsed inside apUnitScreenInit, if anything.
 * For example,
 *      -D1 d4
 * means initialize display unit 1 to depth 4.
 *
 * Since we recognize an arg string following the -D<unit> by the lack of a leading "-",
 * the bogus server options "c", "r" and "v" should not follow a -D<unit> option that has no
 * arg string.  I expect the ridiculous server option syntax (implemented in ProcessCommandLine
 * in server/os/4.2bsd/utils.c) to be changed, so I refuse to worry about this.
 */
        for (i = 1; i < argc; i++)
        {
            if ((argv[i][0] ==  '-') && (argv[i][1] == 'D'))
            {
                unit = unitList[nUnits].unit = atoi(argv[i]+2);
                if ((unit >= MIN_DISPLAY_UNIT) &&
                    (unit <= MAX_DISPLAY_UNIT))
                {
                    if (++i >= argc)
                        unitList[nUnits].argString = nullArgString;
                    else if (argv[i][0] == '-')
                    {
                        unitList[nUnits].argString = nullArgString;
                        i--;
                    }
                    else
                        unitList[nUnits].argString = argv[i];
                    gpr_$inq_disp_characteristics (gpr_$borrow, unit, sizeof(gpr_$disp_char_t),
                                                   unitList[nUnits].dispChar, disp_len_ret, status);
                    if (unitList[nUnits].dispChar.controller_type != gpr_$ctl_none)
                        nUnits++;
                }
            }
        }

/*
 * If we didn't find any -D<unit> options, find and enter all hardware units into
 * the unit list.
 */
        if (!nUnits)
        {
            for (unit = MIN_DISPLAY_UNIT; unit <= MAX_DISPLAY_UNIT; unit++)
            {
                gpr_$inq_disp_characteristics (gpr_$borrow, unit, sizeof(gpr_$disp_char_t),
                                               unitList[nUnits].dispChar, disp_len_ret, status);
                if (unitList[nUnits].dispChar.controller_type != gpr_$ctl_none)
                {
                    unitList[nUnits].unit = unit;
                    unitList[nUnits].argString = nullArgString;
                    nUnits++;
                }
            }
        }

/*
 * OK, now figure out the desired depth for each unit.  That's the actual depth the hardware
 * supports, unless there was a d<depth> in the arg string somewhere, in which case it's the
 * given depth if that's smaller.  If the result isn't "reasonable" (e.g. 5), make it be.
 */
        for (i = 0; i < nUnits; i++)
        {
            depth = min(MAX_DEPTH_SUPPORTED, unitList[i].dispChar.n_planes);
            if (arg_p = index(unitList[i].argString, 'd')) {
		++arg_p;
                depth = min(depth, atoi(arg_p));
		}
            depth &= ~(0x3);
            if (!depth) depth = 1;
            unitList[i].actualDepth = depth;
        }
    }
/*
 * At this point, we have a valid set of unit data in our trusty static storage.
 */

/*
 * Set up constant screenInfo stuff, including the first format (bitmap).
 */
    screenInfo->imageByteOrder = IMAGE_BYTE_ORDER;
    screenInfo->bitmapScanlineUnit = BITMAP_SCANLINE_UNIT;
    screenInfo->bitmapScanlinePad = BITMAP_SCANLINE_PAD;
    screenInfo->bitmapBitOrder = BITMAP_BIT_ORDER;

    screenInfo->formats[0].depth = 1;
    screenInfo->formats[0].bitsPerPixel = 1;
    screenInfo->formats[0].scanlinePad = BITMAP_SCANLINE_PAD;
    screenInfo->formats[1].depth = 4;
    screenInfo->formats[1].bitsPerPixel = 8;
    screenInfo->formats[1].scanlinePad = BITMAP_SCANLINE_PAD;
    screenInfo->formats[2].depth = 8;
    screenInfo->formats[2].bitsPerPixel = 8;
    screenInfo->formats[2].scanlinePad = BITMAP_SCANLINE_PAD;
    num_formats = 3;

/*
 * Based on the unit list, fill in any additional pixmap formats supported.
 */

    for (i = 0; i < nUnits; i++)
    {
        depth = unitList[i].actualDepth;
        for (j = 0; j < num_formats; j++)
            if (screenInfo->formats[j].depth == depth) break;
        if ((j == num_formats) && (j < MAXFORMATS))
        {
            screenInfo->formats[num_formats].depth = depth;
            depth = (depth + 7) & ~(0x7);
            screenInfo->formats[num_formats].bitsPerPixel = depth;
            screenInfo->formats[num_formats].scanlinePad = BITMAP_SCANLINE_PAD;
            num_formats++;
        }
    }
    screenInfo->numPixmapFormats = num_formats;

/*
 * Finally, loop over the unit list and call AddScreen for each one.
 */

    for (i = 0; i < nUnits; i++)
    {
        currentUnitIndex = i;
        AddScreen (apUnitScreenInit, argc, argv);
    }
}

/*
 * InitInput -- DDX interface (server)
 *      Initialize and register the keyboard and mouse.
 *
 *      We ignore the command line for now.
 */
void
InitInput(argc, argv)
    int argc;
    char *argv[];
{
    DevicePtr   ptr_dev, kbd_dev;
    
    ptr_dev = AddInputDevice (apMouseProc, TRUE);
    kbd_dev = AddInputDevice (apKeybdProc, TRUE);

/*
 * We don't implement saved motion events.  Sorry.
 */
#define MOTION_BUFFER_SIZE 0

    RegisterPointerDevice (ptr_dev, MOTION_BUFFER_SIZE);
    RegisterKeyboardDevice (kbd_dev);
}

/*
 * DDX - specific abort routine.  Called by AbortServer().
 */
void
AbortDDX()
{
}

/* Called by GiveUp(). */
void
ddxGiveUp()
{
}

int
ddxProcessArgument (argc, argv, i)
    int	argc;
    char *argv[];
    int	i;
{
    return 0;
}

void
ddxUseMsg()
{
}
