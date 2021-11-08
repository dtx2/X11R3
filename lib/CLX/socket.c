/* Copyright	Massachusetts Institute of Technology		1988 */
/* Copyright	Edward Halferty (coincidentally born 1988)	2021 */
#include <stdio.h>
#include <X11/Xos.h>
#include <X11/Xproto.h>
#include <errno.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <netdb.h> 
#include <sys/socket.h>
#include <netinet/tcp.h>

#include <sys/un.h>
#ifndef X_UNIX_PATH
#define X_UNIX_PATH "/tmp/.X11-unix/X"
#endif /* X_UNIX_PATH */
void bcopy();

/* 
 * Attempts to connect to server, given host and display. Returns file 
 * descriptor (network socket) or 0 if connection fails.
 */

int connect_to_server (host, display)
     char *host;
     int display;
{
  struct sockaddr_in inaddr;	/* INET socket address. */
  struct sockaddr *addr;		/* address to connect to */
  struct hostent *host_ptr;
  int addrlen;			/* length of address */
  struct sockaddr_un unaddr;	/* UNIX socket address. */
  extern char *getenv();
  extern struct hostent *gethostbyname();
  int fd;				/* Network socket */
  {
    if ((host[0] == '\0') || 
	(strcmp("unix", host) == 0)) {
	/* Connect locally using Unix domain. */
	unaddr.sun_family = AF_UNIX;
	(void) strcpy(unaddr.sun_path, X_UNIX_PATH);
	sprintf(&unaddr.sun_path[strlen(unaddr.sun_path)], "%d", display);
	addr = (struct sockaddr *) &unaddr;
	addrlen = strlen(unaddr.sun_path) + 2;
	/*
	 * Open the network connection.
	 */
	if ((fd = socket((int) addr->sa_family, SOCK_STREAM, 0)) < 0)
	    return(-1);	    /* errno set by system call. */
    } else
    {
      /* Get the statistics on the specified host. */
      if ((inaddr.sin_addr.s_addr = inet_addr(host)) == -1) 
	{
	  if ((host_ptr = gethostbyname(host)) == NULL) 
	    {
	      /* No such host! */
	      errno = EINVAL;
	      return(-1);
	    }
	  /* Check the address type for an internet host. */
	  if (host_ptr->h_addrtype != AF_INET) 
	    {
	      /* Not an Internet host! */
	      errno = EPROTOTYPE;
	      return(-1);
	    }
	  /* Set up the socket data. */
	  inaddr.sin_family = host_ptr->h_addrtype;
	  bcopy((char *)host_ptr->h_addr, 
		(char *)&inaddr.sin_addr, 
		sizeof(inaddr.sin_addr));
	} 
      else 
	{
	  inaddr.sin_family = AF_INET;
	}
      addr = (struct sockaddr *) &inaddr;
      addrlen = sizeof (struct sockaddr_in);
      inaddr.sin_port = display + X_TCP_PORT;
      inaddr.sin_port = htons(inaddr.sin_port);
      /*
       * Open the network connection.
       */
      if ((fd = socket((int) addr->sa_family, SOCK_STREAM, 0)) < 0){
	return(-1);	    /* errno set by system call. */}
      /* make sure to turn off TCP coalescence */
#ifdef TCP_NODELAY
      {
	int mi = 1;
	setsockopt (fd, IPPROTO_TCP, TCP_NODELAY, &mi, sizeof (int));
      }
#endif
    }
    if (connect(fd, addr, addrlen) == -1) 
      {
	(void) close (fd);
	return(-1); 	    /* errno set by system call. */
      }
  }
  /*
   * Return the id if the connection succeeded.
   */
  return(fd);
}
