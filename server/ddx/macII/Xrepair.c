/************************************************************ 
Copyright 1988 by Apple Computer, Inc, Cupertino, California
			All Rights Reserved

Permission to use, copy, modify, and distribute this software
for any purpose and without fee is hereby granted, provided
that the above copyright notice appear in all copies.

APPLE MAKES NO WARRANTY OR REPRESENTATION, EITHER EXPRESS,
OR IMPLIED, WITH RESPECT TO THIS SOFTWARE, ITS QUALITY,
PERFORMANCE, MERCHANABILITY, OR FITNESS FOR A PARTICULAR
PURPOSE. AS A RESULT, THIS SOFTWARE IS PROVIDED "AS IS,"
AND YOU THE USER ARE ASSUMING THE ENTIRE RISK AS TO ITS
QUALITY AND PERFORMANCE. IN NO EVENT WILL APPLE BE LIABLE 
FOR DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL
DAMAGES RESULTING FROM ANY DEFECT IN THE SOFTWARE.

THE WARRANTY AND REMEDIES SET FORTH ABOVE ARE EXCLUSIVE
AND IN LIEU OF ALL OTHERS, ORAL OR WRITTEN, EXPRESS OR
IMPLIED.

************************************************************/
#include <sys/stropts.h>
#include <sys/termio.h>
#include <sys/video.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <errno.h>

static struct termio d_tio = {
	(BRKINT|IGNPAR|ISTRIP|ICRNL|IXON)&(~IGNBRK)&(~PARMRK)&(~INPCK)&(~INLCR)&
	(~IGNCR)&(~IUCLC)&(~IXANY)&(~IXOFF),
	(OPOST|ONLCR)&(~OLCUC)&(~OCRNL)&(~ONOCR)&(~ONLRET)&(~OFILL)&(~OFDEL),
	(B9600|CS7|CREAD)&(~CSTOPB)&(~PARENB)&(~PARODD)&(~HUPCL)&(~CLOCAL)&(~LOBLK),
	(ISIG|ICANON|ECHO|ECHOE|ECHOK)&(~XCASE)&(~ECHONL)&(~NOFLSH),
	0,
	{CINTR, CQUIT, CERASE, CKILL, CEOF, CNUL, CNUL, CNUL}
};

/*
 * Check to see that the server restored the "line" streams module
 * on /dev/console. If so, we'll presume all is well. If not, clear 
 * all stacked modules, push "line", and establish workable stty values.
 */
main()
{
	int fd; int line;
	int iarg;
	struct strioctl ctl;
	char buff[FMNAMESZ+1];
	int errors = 0;

	if ((fd = open("/dev/console", O_RDWR)) < 0) {
	    printf("Xrepair: can't open /dev/console\n");
	} else if (ioctl(fd, I_FIND, "line") == 0) {
#ifdef CONS_UNDIRECT
            ctl.ic_len = 0;
            ctl.ic_cmd = CONS_UNDIRECT;
            if (ioctl(fd, I_STR, &ctl) < 0) {
                errors++;
                printf("Failed to ioctl I_STR CONS_UNDIRECT.\r\n");
            }
#endif
	    iarg = 0;
	    if (ioctl(fd, FIONBIO, &iarg) < 0) {
		errors++;
		printf("Could not ioctl FIONBIO. \r\n");
	    }
	    
#ifdef notdef
	    iarg = 0;
	    if (ioctl(fd, FIOASYNC, &iarg) < 0) {
		errors++;
		printf("Could not ioctl FIOASYNC. \r\n");
	    }
#endif
	    
	    if (ioctl(fd, I_FLUSH, FLUSHRW) < 0) {
		errors++;
		printf("Failed to ioctl I_FLUSH FLUSHRW.\r\n");
	    }
	    
	    ctl.ic_len = 0;
	    ctl.ic_cmd = VIDEO_NOMOUSE;
	    if (ioctl(fd, I_STR, &ctl) < 0) {
		errors++;
		printf("Failed to ioctl I_STR VIDEO_NOMOUSE.\r\n");
	    }
	    
	    ctl.ic_len = 0;
	    ctl.ic_cmd = VIDEO_ASCII;
	    if (ioctl(fd, I_STR, &ctl) < 0) {
		errors++;
		printf("Failed to ioctl I_STR VIDEO_ASCII.\r\n");
	    }

	    if(ioctl(fd, I_PUSH, "line") < 0) {
		errors++;
		printf("Failed to ioctl I_PUSH.\r\n");
	    }

	    if (ioctl(fd, TCSETA, &d_tio) < 0) {
		errors++;
		printf("Failed to ioctl TCSETA.\r\n");
	    }
	}
    exit (errors);
}
