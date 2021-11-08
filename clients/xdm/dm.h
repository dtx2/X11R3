/*
 * xdm - display manager daemon
 *
 * $XConsortium: dm.h,v 1.9 88/11/17 17:04:53 keith Exp $
 *
 * Copyright 1988 Massachusetts Institute of Technology
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted, provided
 * that the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of M.I.T. not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  M.I.T. makes no representations about the
 * suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 *
 * Author:  Keith Packard, MIT X Consortium
 */

/*
 * dm.h
 *
 * public interfaces for greet/verify functionality
 */

# include	<X11/Xos.h>

# include	<sys/param.h>	/* for NGROUPS */

#ifdef SYSV
# define waitCode(w)	((w) & 0xff)
# define waitSig(w)	(((w) >> 8) & 0xff)
typedef int		waitType;
#else
# include	<sys/wait.h>
# define waitCode(w)	((w).w_T.w_Retcode)
# define waitSig(w)	((w).w_T.w_Termsig)
typedef union wait	waitType;
#endif

#ifdef UDP_SOCKET
#include	<sys/types.h>
#include	<netinet/in.h>
#endif

# define waitVal(w)	(waitSig(w) ? (waitSig(w) * 256 + 1) : waitCode (w))

typedef enum displayStatus { running, notRunning } DisplayStatus;

#ifndef FD_ZERO
typedef	struct	my_fd_set { int fds_bits[1]; } my_fd_set;
# define FD_ZERO(fdp)	bzero ((fdp), sizeof (*(fdp)))
# define FD_SET(f,fdp)	((fdp)->fds_bits[(f) / (sizeof (int) * 8)] |=  (1 << ((f) % (sizeof (int) * 8))))
# define FD_CLR(f,fdp)	((fdp)->fds_bits[(f) / (sizeof (int) * 8)] &= ~(1 << ((f) % (sizeof (int) * 8))))
# define FD_ISSET(f,fdp)	((fdp)->fds_bits[(f) / (sizeof (int) * 8)] & (1 << ((f) % (sizeof (int) * 8))))
# define FD_TYPE	my_fd_set
#else
# define FD_TYPE	fd_set
#endif

/*
 * local     - server runs on local host
 * foreign   - server runs on remote host
 * permanent - session restarted when it exits
 * transient - session not restarted when it exits
 * secure    - cannot be disabled
 * insecure  - can be disabled
 */

typedef struct displayType {
	unsigned int	location:1;
	unsigned int	lifetime:1;
	unsigned int	mutable:1;
} DisplayType;

# define Local		1
# define Foreign	0

# define Permanent	1
# define Transient	0

# define Secure		1
# define Insecure	0

extern DisplayType parseDisplayType ();

typedef enum fileState { NewEntry, OldEntry, MissingEntry } FileState;

struct display {
	struct display	*next;
	char		*name;		/* DISPLAY name */
	char		**argv;		/* program name and arguments */
	DisplayStatus	status;		/* current status */
	int		pid;		/* process id of child */
	FileState	state;		/* state during HUP processing */
	char		*resources;	/* resource file */
	char		*xrdb;		/* xrdb program */
	char		*cpp;		/* cpp program */
	char		*startup;	/* Xstartup program */
	char		*reset;		/* Xreset program */
	char		*session;	/* Xsession program */
	char		*userPath;	/* path set for session */
	char		*systemPath;	/* path set for startup/reset */
	char		*systemShell;	/* interpreter for startup/reset */
	char		*failsafeClient;/* a client to start when the session fails */
	int		openDelay;	/* open delay time */
	int		openRepeat;	/* open attempts to make */
	int		openTimeout;	/* abort open attempt timeout */
	int		terminateServer;/* restart for each session */
	int		grabTimeout;	/* time to wait for grab */
	DisplayType	displayType;	/* method to handle with */
#ifdef UDP_SOCKET
	struct sockaddr_in	addr;	/* address used in connection */
#endif
};

struct greet_info {
	char		*name;		/* user name */
	char		*password;	/* user password */
	char		*string;	/* random string */
};

struct verify_info {
	int		uid;		/* user id */
#ifdef NGROUPS
	int		groups[NGROUPS];/* group list */
	int		ngroups;	/* number of elements in groups */
#else
	int		gid;		/* group id */
#endif
	char		**argv;		/* arguments to session */
	char		**userEnviron;	/* environment for session */
	char		**systemEnviron;/* environment for startup/reset */
};

/* session exit status definitions. */

# define OBEYTERM_DISPLAY	0	/* obey terminateServer resource */
# define RESTART_DISPLAY	1	/* force session restart */
# define ABORT_DISPLAY		2	/* force server restart */
# define DISABLE_DISPLAY	3	/* unmanage this display */

/* display manager exit status definitions */

# define OBEYSESS_DISPLAY	0	/* obey multipleSessions resource */
# define REMANAGE_DISPLAY	1	/* force remanage */
# define UNMANAGE_DISPLAY	2	/* force deletion */

extern char	*servers;
extern int	request_port;
extern int	debugLevel;
extern char	*errorLogFile;
extern int	daemonMode;
extern char	*pidFile;

extern struct display	*FindDisplayByName (),
			*FindDisplayByPid (),
			*NewDisplay ();

extern char	*malloc (), *realloc (), *strcpy ();

#ifdef UDP_SOCKET

# define START		"START"
# define TERMINATE	"TERMINATE"
# define RESTART	"RESTART"

# define POLL_PROVIDERS "POLL"
# define ADVERTISE	"ADVERTISE"

#endif
