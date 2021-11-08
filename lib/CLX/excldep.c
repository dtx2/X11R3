/*
 * Allegro CL dependent C helper routines for CLX
 */

#include <sys/types.h>
#include <sys/errno.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <signal.h>

#define ERROR -1
#define INTERRUPT -2

#define min(x,y) (((x) > (y)) ? (y) : (x))

extern int errno;


int c_howmany_bytes(fd)
    int fd;
{
    int numavail;

    if (ioctl(fd, FIONREAD, (char *)&numavail) < 0) {
	return (ERROR);
    }

    return (numavail);
}

	   
/*
 * Tries to read length characters into array at position start.  Note that
 * this function never blocks.  Return ERROR on eof or error.
 */
int c_read_bytes(fd, array, start, length)
    register int fd, start, length;
    register unsigned char *array;

{
    register int numread, numavail, totread = 0;

    numavail = c_howmany_bytes(fd);

    if (numavail <= 0) {
	return (numavail);
    } else {
	while (numavail > 0) {
	    numread = read(fd, (char *)&array[start], min(numavail, length));
	    if (numread <= 0) {
	return (ERROR);
	    } else {
		totread += numread;
		length -= numread;
		start += numread;
	    }
	    if (length == 0) {
		return (totread);
	    } else {
		numavail = c_howmany_bytes(fd);
	    }
}
    }
    return (totread);
}


/*
 * This is somewhat gross.  When the scheduler is not running we must provide
 * a way for the user to interrupt the read from the X socket from lisp.  So
 * we provide a separate reading function.  Don't return until the
 * full number of bytes have been read.
 */
int c_read_bytes_interruptible(fd, array, start, length)
    int fd, start, length;
    unsigned char *array;

{
    int numwanted, i, readfds, numread;

    readfds = 1 << fd;

    while (length > 0) {
    i = select(32, &readfds, (int *)0, (int *)0, (struct timeval *)0);
    if (i < 0)
	/* error condition */
	if (errno == EINTR)
	    return (INTERRUPT);
	else
	    return (ERROR);

	numread = read(fd, (char *)&array[start], length);
	if (numread <= 0)
	return (ERROR);
	else {
	    length -= numread;
	    start += numread;
	}
    }
    return (0);
}



#define OBSIZE 4096	/* X output buffer size */

static unsigned char output_buffer[OBSIZE];
static int obcount = 0;


/*
 * The inverse of above, which is simpler because there's no timeout. 
 * Don't need to block SIGIO's here, since the write either happens or it
 * fails, it doesn't block.
 */
int c_write_bytes(fd, array, start, end)
    int fd, start, end;
    unsigned char *array;
{
    int numwanted, i;
    void bcopy();

    numwanted = end - start;

    if (numwanted + obcount > OBSIZE)
	/* too much stuff -- we gotta flush. */
	if ((i = c_flush_bytes(fd)) < 0)
	    return (i);

    /* everything's cool, just bcopy */
    bcopy((char *)&array[start], (char *)&output_buffer[obcount],
	  numwanted);
    obcount += numwanted;

    return (numwanted);
}


int c_flush_bytes(fd)
    int fd;
{
    int i = 0, j = 0;

    while (obcount > 0) {
	i = write(fd, (char *)(&output_buffer[j]), obcount);
	if (i > 0) {
	    obcount -= i;
	    j += i;
	}
	else
	    return (ERROR);
    }

    return (i);
}
