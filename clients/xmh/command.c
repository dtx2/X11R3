#ifndef lint
static char rcs_id[] = "$XConsortium: command.c,v 2.17 88/09/06 17:23:12 jim Exp $";
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

/* command.c -- interface to exec mh commands. */

#include "xmh.h"
#include <sys/stat.h>
#include <sys/signal.h>
#ifndef SYSV
#include <sys/wait.h>
#endif	/* SYSV */
#include <sys/resource.h>

#ifdef macII
#define vfork() fork()
#endif /* macII */

#if defined(SYSV) && !defined(hpux)
#define vfork() fork()
#endif /* SYSV and not hpux */


#ifndef FD_SET
#define NFDBITS         (8*sizeof(fd_set))
#define FD_SETSIZE      NFDBITS
#define FD_SET(n, p)    ((p)->fds_bits[(n)/NFDBITS] |= (1 << ((n) % NFDBITS)))
#define FD_CLR(n, p)    ((p)->fds_bits[(n)/NFDBITS] &= ~(1 << ((n) % NFDBITS)))
#define FD_ISSET(n, p)  ((p)->fds_bits[(n)/NFDBITS] & (1 << ((n) % NFDBITS)))
#define FD_ZERO(p)      bzero((char *)(p), sizeof(*(p)))
#endif FD_SET



/* Return the full path name of the given mh command. */

static char *FullPathOfCommand(str)
  char *str;
{
    static char result[100];
    (void) sprintf(result, "%s/%s", app_resources.defMhPath, str);
    return result;
}


static int childdone;		/* Gets nonzero when the child process
				   finishes. */
ChildDone()
{
    childdone++;
}

/* Execute the given command, and wait until it has finished.  While the
   command is executing, watch the X socket and cause Xlib to read in any
   incoming data.  This will prevent the socket from overflowing during
   long commands. */

DoCommand(argv, inputfile, outputfile)
  char **argv;			/* The command to execute, and its args. */
  char *inputfile;		/* Input file for command. */
  char *outputfile;		/* Output file for command. */
{
    int old_stdin, old_stdout, old_stderr;
    FILEPTR fin, fout, ferr;
    int pid;
    fd_set readfds, fds;
    FD_ZERO(&fds);
    FD_SET(ConnectionNumber(theDisplay), &fds);
    DEBUG1("Executing %s ...", argv[0])

    if (inputfile) {
        old_stdin = dup(fileno(stdin));
	fin = FOpenAndCheck(inputfile, "r");
	(void) dup2(fileno(fin), fileno(stdin));
	myfclose(fin);
    }

    if (outputfile) {
        old_stdout = dup(fileno(stdout));
	fout = FOpenAndCheck(outputfile, "w");
	(void) dup2(fileno(fout), fileno(stdout));
	myfclose(fout);
    }

    if (!app_resources.debug) {		/* Throw away error messages. */
        old_stderr = dup(fileno(stderr));
	ferr = FOpenAndCheck("/dev/null", "w");
	(void) dup2(fileno(ferr), fileno(stderr));
	if (!outputfile) {
	    old_stdout = dup(fileno(stdout));
	    (void) dup2(fileno(ferr), fileno(stdout));
	}
	myfclose(ferr);
    }

    childdone = FALSE;
    (void) signal(SIGCHLD, ChildDone);
    pid = vfork();
    if (inputfile) {
	if (pid != 0) dup2(old_stdin,  fileno(stdin));
	close(old_stdin);
    }
    if (outputfile) {
	if (pid != 0) dup2(old_stdout, fileno(stdout));
	close(old_stdout);
    }
    if (!app_resources.debug) {
	if (pid != 0) dup2(old_stderr, fileno(stderr));
	close(old_stderr);
	if (!outputfile) {
	    if (pid != 0) dup2(old_stdout, fileno(stdout));
	    close(old_stdout);
	}
    }
    if (pid == -1) Punt("Couldn't fork!");
    if (pid) {			/* We're the parent process. */
	while (!childdone) {
	    XEvent event;
	    readfds = fds;
	    (void) select(ConnectionNumber(theDisplay)+1, (int *) &readfds,
			  (int *) NULL, (int *) NULL, (struct timeval *) NULL);
	    if (FD_ISSET(ConnectionNumber(theDisplay), &readfds)) {
		(void) XPending(theDisplay);
		while (XCheckTypedEvent( theDisplay, Expose, &event )) {
		    XtDispatchEvent( &event );
		}
	    }
	}
#ifdef SYSV
	(void) wait((int *) NULL);
#else /* !SYSV */
	(void) wait((union wait *) NULL);
#endif /* !SYSV */

	DEBUG(" done\n")
    } else {			/* We're the child process. */
	(void) execv(FullPathOfCommand(argv[0]), argv);
	(void) execvp(argv[0], argv);
	Punt("Execvp failed!");
    }
}



/* Execute the given command, waiting until it's finished.  Put the output
   in a newly mallocced string, and return a pointer to that string. */

char *DoCommandToString(argv)
char ** argv;
{
    char *result;
    char *file;
    int fid, length;
    file = DoCommandToFile(argv);
    length = GetFileLength(file);
    result = XtMalloc((unsigned) length + 1);
    fid = myopen(file, O_RDONLY, 0666);
    if (length != read(fid, result, length))
	Punt("Couldn't read result from DoCommandToString");
    result[length] = 0;
    DEBUG1("('%s')\n", result)
    (void) myclose(fid);
    DeleteFileAndCheck(file);
    return result;
}
    

#ifdef NOTDEF	/* This implementation doesn't work right on null return. */
char *DoCommandToString(argv)
  char **argv;
{
    static char result[1030];
    int fildes[2], pid, l;
    DEBUG1("Executing %s ...", argv[0])
    (void) pipe(fildes);
    pid = vfork();
    if (pid == -1) Punt("Couldn't fork!");
    if (pid) {
#ifdef SYSV
        while (wait((int *) 0) == -1) ;
#else /* !SYSV */
	while (wait((union wait *) 0) == -1) ;
#endif /* !SYSV */
	l = read(fildes[0], result, 1024);
	if (l <= 0) Punt("Couldn't read result from DoCommandToString");
	(void) myclose(fildes[0]);
	result[l] = 0;
	while (result[--l] == 0) ;
	while (result[l] == '\n') result[l--] = 0;
	DEBUG1(" done: '%s'\n", result)
	return result;
    } else {
	(void) dup2(fildes[1], fileno(stdout));
	(void) execv(FullPathOfCommand(argv[0]), argv);
	(void) execvp(argv[0], argv);
	Punt("Execvp failed!");
	return NULL;
    }
}
#endif NOTDEF



/* Execute the command to a temporary file, and return the name of the file. */

char *DoCommandToFile(argv)
  char **argv;
{
    char *name;
    name = MakeNewTempFileName();
    DoCommand(argv, (char *) NULL, name);
    return name;
}
