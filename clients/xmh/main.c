#ifndef lint
static char rcs_id[] = "$XConsortium: main.c,v 2.11 88/09/06 17:23:21 jim Exp $";
#endif lint
/*
 *			  COPYRIGHT 1987
 *		   DIGITAL EQUIPMENT CORPORATION
 *		       MAYNARD, MASSACHUSETTS
 *			ALL RIGHTS RESERVED.
 *
 * THE INFORMATION IN THIS SOFTWARE IS SUBJECT TO CHANGE WITHOUT NOTICE AND
 * SHOULD NOT BE CONSTRUED AS A COMMITMENT BY DIGITAL EQUIPMENT CORPORATION.
 * DIGITAL MAKES NO REPRESENTATIONS ABOUT THE SUITABILITY OF THIS SOFTWARE FOR
 * ANY PURPOSE.  IT IS SUPPLIED "AS IS" WITHOUT EXPRESS OR IMPLIED WARRANTY.
 *
 * IF THE SOFTWARE IS MODIFIED IN A MANNER CREATING DERIVATIVE COPYRIGHT RIGHTS,
 * APPROPRIATE LEGENDS MAY BE PLACED ON THE DERIVATIVE WORK IN ADDITION TO THAT
 * SET FORTH ABOVE.
 *
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted, provided
 * that the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting documentation,
 * and that the name of Digital Equipment Corporation not be used in advertising
 * or publicity pertaining to distribution of the software without specific,
 * written prior permission.
 */

/* main.c */

#define MAIN 1			/* Makes global.h actually declare vars */
#include "xmh.h"
#include <signal.h>


static XtIntervalId timerid;

/* This gets called every five minutes. */

static void NeedToCheckScans()
{
    int i;
    DEBUG("[magic toc check ...")
    for (i = 0; i < numScrns; i++) {
	if (scrnList[i]->toc)
	    TocRecheckValidity(scrnList[i]->toc);
	if (scrnList[i]->msg)
	    TocRecheckValidity(MsgGetToc(scrnList[i]->msg));
    }
    DEBUG(" done]\n")
}



/*ARGSUSED*/
static void CheckMail(client_data, id)
    caddr_t client_data;	/* unused */
    XtIntervalId *id;		/* unused */
{
    static int count = 0;
    int i;
    timerid = XtAddTimeOut((int)60000, CheckMail, NULL);
    if (app_resources.defNewMailCheck) {
	DEBUG("(Checking for new mail...")
	TocCheckForNewMail();
	DEBUG(" done)\n")
    }
    if (count++ % 5 == 0) {
	NeedToCheckScans();
	if (app_resources.defMakeCheckpoints) {
	    DEBUG("(Checkpointing...")
	    for (i=0 ; i<numScrns ; i++)
		if (scrnList[i]->msg) 
		    MsgCheckPoint(scrnList[i]->msg);
	    DEBUG(" done)\n")
	}
    }
}

/* Main loop. */

main(argc, argv)
unsigned int argc;
char **argv;
{
    InitializeWorld(argc, argv);
    if (app_resources.defNewMailCheck)
	TocCheckForNewMail();
    timerid = XtAddTimeOut((int)60000, CheckMail, NULL);
    XtMainLoop();
}
