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
/*
 * Screentab.c - table of frame buffers supported by the X11 server
 */

#include "misc.h"
#include "screentab.h"

/* Declare the ScreenInfo(), ScreenInit() and ScreenClose() functions here.
 */
extern Bool mobScreenInfo(), mobScreenInit(), mobScreenClose();
extern Bool gbxScreenInfo(), gbxScreenInit(), gbxScreenClose();
extern Bool topcatScreenInfo(), topcatScreenInit(), topcatScreenClose();
extern Bool mrtopcatScreenInfo(), mrtopcatScreenInit(), mrtopcatScreenClose();
extern Bool catseyeScreenInfo(), catseyeScreenInit(), catseyeScreenClose();
extern Bool renScreenInfo(), renScreenInit(), renScreenClose();
extern Bool orenScreenInfo(), orenScreenInit(), orenScreenClose();
extern Bool davinciScreenInfo(), davinciScreenInit(), davinciScreenClose();
extern Bool oDavinciScreenInit();
/*
 * Table of known frame buffers
 */

ScreenTable screenTable = {
    {"98633", "moberly", mobScreenInfo, mobScreenInit, mobScreenClose},
    {"98700", "gatorbox", gbxScreenInfo, gbxScreenInit, gbxScreenClose},
    {"98547", "topcat", topcatScreenInfo, topcatScreenInit, topcatScreenClose},
    {"98543", "mrtopcat", mrtopcatScreenInfo, mrtopcatScreenInit, 
		mrtopcatScreenClose},
    {"98550", "catseye", catseyeScreenInfo, catseyeScreenInit, catseyeScreenClose},
    {"98720", "renaissance", renScreenInfo, renScreenInit, renScreenClose},
    {"98720", "orenaissance", orenScreenInfo, orenScreenInit, orenScreenClose},
    {"98730", "davinci",
		davinciScreenInfo, davinciScreenInit, davinciScreenClose},
    {"98730", "odavinci",
		davinciScreenInfo, oDavinciScreenInit, davinciScreenClose},
    {(char *) NULL, (char *) NULL, (Bool (*)()) NULL, (Bool (*)()) NULL,
        (Bool (*)()) NULL}
    };
