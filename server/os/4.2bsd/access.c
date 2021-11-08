/***********************************************************
Copyright 1987 by Digital Equipment Corporation, Maynard, Massachusetts,
and the Massachusetts Institute of Technology, Cambridge, Massachusetts.

                        All Rights Reserved

Permission to use, copy, modify, and distribute this software and its 
documentation for any purpose and without fee is hereby granted, 
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in 
supporting documentation, and that the names of Digital or MIT not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.  

DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.

******************************************************************/

/* $XConsortium: access.c,v 1.26 88/09/06 15:51:03 jim Exp $ */

#include "X.h"
#include "Xproto.h"
#include "misc.h"
#include "site.h"
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>

#ifdef hpux
#include <sys/utsname.h>
#else
#include <net/if.h>
#endif /* hpux */

#include <netdb.h>
#undef NULL
#include <stdio.h>
#include "dixstruct.h"
#include "osdep.h"

#define acmp(a1, a2, len) bcmp((char *)(a1), (char *)(a2), len)
#define acopy(a1, a2, len) bcopy((char *)(a1), (char *)(a2), len)

#define DONT_CHECK -1
extern char	*index();

typedef struct _host {
	short		family;
	short		len;
	unsigned char	addr[4];	/* will need to be bigger eventually */
	struct _host *next;
} HOST;

static HOST *selfhosts = NULL;
static HOST *validhosts = NULL;
static int AccessEnabled = DEFAULT_ACCESS_CONTROL;

typedef struct {
    int af, xf;
} FamilyMap;

static FamilyMap familyMap[] = {
#ifdef     AF_DECnet
    {AF_DECnet, FamilyDECnet},
#endif /* AF_DECnet */
#ifdef     AF_CHAOS
    {AF_CHAOS, FamilyChaos},
#endif /* AF_CHAOS */
#ifdef    AF_INET
    {AF_INET, FamilyInternet}
#endif
};

#define FAMILIES ((sizeof familyMap)/(sizeof familyMap[0]))

#ifdef hpux
/* Define this host for access control.  Find all the hosts the OS knows about 
 * for this fd and add them to the selfhosts list.
 * HPUX version - hpux does not have SIOCGIFCONF ioctl;
 */
DefineSelf (fd)
    int fd;
{
    register int n;
    int	len;
    caddr_t	addr;
    int		family;
    register HOST	*host;

    struct utsname name;
    register struct hostent  *hp;

    union {
	struct  sockaddr   sa;
	struct  sockaddr_in  in;
    } saddr;
	
    struct	sockaddr_in	*inetaddr;

    /* Why not use gethostname()?  Well, at least on my system, I've had to
     * make an ugly kernel patch to get a name longer than 8 characters, and
     * uname() lets me access to the whole string (it smashes release, you
     * see), whereas gethostname() kindly truncates it for me.
     */
    uname(&name);
    hp = gethostbyname (name.nodename);
    if (hp != NULL) {
	saddr.sa.sa_family = hp->h_addrtype;
	inetaddr = (struct sockaddr_in *) (&(saddr.sa));
	acopy ( hp->h_addr, &(inetaddr->sin_addr), hp->h_length);
	family = ConvertAddr ( &(saddr.sa), &len, &addr);
	if ( family > 0) {
	    for (host = selfhosts;
		 host && ( family != host->family ||
			  acmp ( addr, host->addr, len));
		 host = host->next) ;
	    if (!host) {
		/* add this host to the host list.	*/
		host = (HOST *) xalloc (sizeof (HOST));
		host->family = family;
		host->len = len;
		acopy ( addr, host->addr, len);
		host->next = selfhosts;
		selfhosts = host;
	    }
	}
    }
}

#else
/* Define this host for access control.  Find all the hosts the OS knows about 
 * for this fd and add them to the selfhosts list.
 */
DefineSelf (fd)
    int fd;
{
    char		buf[2048];
    struct ifconf	ifc;
    register int	n;
    int 		len;
    pointer 		addr;
    int 		family;
    register HOST 	*host;
    register struct ifreq *ifr;
    
    ifc.ifc_len = sizeof (buf);
    ifc.ifc_buf = buf;
    if (ioctl (fd, (int) SIOCGIFCONF, (pointer) &ifc) < 0)
        Error ("Getting interface configuration");
    for (ifr = ifc.ifc_req, n = ifc.ifc_len / sizeof (struct ifreq); --n >= 0;
     ifr++)
    {
#ifdef DNETCONN
	/*
	 * this is ugly but SIOCGIFCONF returns decnet addresses in
	 * a different form from other decnet calls
	 */
	if (ifr->ifr_addr.sa_family == AF_DECnet) {
		len = sizeof (struct dn_naddr);
		addr = (pointer)ifr->ifr_addr.sa_data;
		family = AF_DECnet;
	} else
#endif /* DNETCONN */
        if ((family = ConvertAddr (&ifr->ifr_addr, &len, &addr)) <= 0)
	    continue;
        for (host = selfhosts; host && (family != host->family ||
	  acmp (addr, host->addr, len)); host = host->next)
	    ;
        if (host)
	    continue;
        host = (HOST *) xalloc (sizeof (HOST));
        host->family = family;
        host->len = len;
        acopy(addr, host->addr, len);
        host->next = selfhosts;
        selfhosts = host;
    }
}
#endif hpux

/* Reset access control list to initial hosts */
ResetHosts (display)
    char *display;
{
    register HOST	*host, *self;
    char 		hostname[120];
    char		fname[32];
    FILE		*fd;
    char		*ptr;
    union {
        struct sockaddr	sa;
    } 			saddr;
    int			family;
    int			len;
    pointer		addr;
    register struct hostent *hp;

    AccessEnabled = DEFAULT_ACCESS_CONTROL;
    while (host = validhosts)
    {
        validhosts = host->next;
        xfree (host);
    }
    for (self = selfhosts; self; self = self->next)
    {
        host = (HOST *) xalloc (sizeof (HOST));
        *host = *self;
        host->next = validhosts;
        validhosts = host;
    }
    strcpy (fname, "/etc/X");
    strcat (fname, display);
    strcat (fname, ".hosts");
    if (fd = fopen (fname, "r")) 
    {
        while (fgets (hostname, sizeof (hostname), fd))
	{
    	if (ptr = index (hostname, '\n'))
    	    *ptr = 0;
        }
        fclose (fd);
    }
}

static Bool
AuthorizedClient(client)
    ClientPtr client;
{
    int    		alen, family;
    struct sockaddr	from;
    pointer		addr;
    register HOST	*host;

    alen = sizeof (from);
    if (!getpeername (((OsCommPtr)client->osPrivate)->fd, &from, &alen))
    {
        if ((family = ConvertAddr (&from, &alen, &addr)) >= 0)
	{
	    if (family == 0)
		return TRUE;
	    for (host = selfhosts; host; host = host->next)
	    {
		if (family == host->family && !acmp (addr, host->addr, alen))
		    return TRUE;
	    }
	}
    }
    return FALSE;
}

/* Add a host to the access control list.  This is the external interface
 * called from the dispatcher */

int
AddHost (client, family, length, pAddr)
    ClientPtr		client;
    int                 family;
    unsigned            length;        /* of bytes in pAddr */
    pointer             pAddr;
{
    int			len;
    register HOST	*host;
    int                 unixFamily;

    if (!AuthorizedClient(client))
	return(BadAccess);
    unixFamily = UnixFamily(family);
    if ((len = CheckFamily (DONT_CHECK, unixFamily)) < 0)
    {
	client->errorValue = family;
        return(-len);
    }

    if (len != length)
    {
	client->errorValue = length;
        return(BadValue);
    }
    for (host = validhosts; host; host = host->next)
    {
        if (unixFamily == host->family && !acmp (pAddr, host->addr, len))
    	    return (Success);
    }
    host = (HOST *) xalloc (sizeof (HOST));
    host->family = unixFamily;
    host->len = len;
    acopy(pAddr, host->addr, len);
    host->next = validhosts;
    validhosts = host;
    return (Success);
}

/* Add a host to the access control list. This is the internal interface 
 * called when starting or resetting the server */
NewHost (family, addr)
    short	family;
    pointer	addr;
{
    int		len;
    register HOST *host;

    if ((len = CheckFamily (DONT_CHECK, family)) < 0)
        return;
    for (host = validhosts; host; host = host->next)
    {
        if (family == host->family && !acmp (addr, host->addr, len))
    	return;
    }
    host = (HOST *) xalloc (sizeof (HOST));
    host->family = family;
    host->len = len;
    acopy(addr, host->addr, len);
    host->next = validhosts;
    validhosts = host;
}

/* Remove a host from the access control list */

int
RemoveHost (client, family, length, pAddr)
    ClientPtr		client;
    int                 family;
    unsigned            length;        /* of bytes in pAddr */
    pointer             pAddr;
{
    int			len,
                        unixFamily;
    register HOST	*host, **prev;

    if (!AuthorizedClient(client))
	return(BadAccess);
    unixFamily = UnixFamily(family);
    if ((len = CheckFamily (DONT_CHECK, unixFamily)) < 0)
    {
	client->errorValue = family;
        return(-len);
    }
    if (len != length)
    {
	client->errorValue = length;
        return(BadValue);
    }
    for (prev = &validhosts;
         (host = *prev) && (unixFamily != host->family ||
		            acmp (pAddr, host->addr, len));
         prev = &host->next)
        ;
    if (host)
    {
        *prev = host->next;
        xfree (host);
    }
    return (Success);
}

/* Get all hosts in the access control list */
int
GetHosts (data, pnHosts, pEnabled)
    pointer		*data;
    int			*pnHosts;
    BOOL		*pEnabled;
{
    int			len;
    register int 	n = 0;
    register pointer	ptr;
    register HOST	*host;
    int			nHosts = 0;
    int			*lengths = (int *) NULL;

    *pEnabled = AccessEnabled ? EnableAccess : DisableAccess;
    for (host = validhosts; host; host = host->next)
    {
        if ((len = CheckFamily (DONT_CHECK, host->family)) < 0)
            return (-1);
	lengths = (int *) xrealloc(lengths, (nHosts + 1) * sizeof(int));
	lengths[nHosts++] = len;
	n += (((len + 3) >> 2) << 2) + sizeof(xHostEntry);
    }
    if (n)
    {
        *data = ptr = (pointer) xalloc (n);
	nHosts = 0;
        for (host = validhosts; host; host = host->next)
	{

	    len = lengths[nHosts++];
	    ((xHostEntry *)ptr)->family = XFamily(host->family);
	    ((xHostEntry *)ptr)->length = len;
	    ptr += sizeof(xHostEntry);
	    acopy (host->addr, ptr, len);
	    ptr += ((len + 3) >> 2) << 2;
        }
    }
    *pnHosts = nHosts;
    xfree(lengths);
    return (n);
}

/* Check for valid address family, and for local host if client modification.
 * Return address length.
 */

CheckFamily (connection, family)
    int			connection;
    int			family;
{
    struct sockaddr	from;
    int	 		alen;
    pointer		addr;
    register HOST	*host;
    int 		len;

    switch (family)
    {
      default:
        return (-BadValue);
    }
    if (connection == DONT_CHECK)
        return (len);
    alen = sizeof (from);
    if (!getpeername (connection, &from, &alen))
    {
        if ((family = ConvertAddr (&from, &alen, &addr)) >= 0)
	{
	    if (family == 0)
		return (len);
	    for (host = selfhosts; host; host = host->next)
	    {
		if (family == host->family &&
		    !acmp (addr, host->addr, alen))
		    return (len);
	    }
	}
    }
    /* Bad Access */
    return (-1);
}

/* Check if a host is not in the access control list. 
 * Returns 1 if host is invalid, 0 if we've found it. */

InvalidHost (saddr, len)
    register struct sockaddr	*saddr;
    int				len;
{
    int 			family;
    pointer			addr;
    register HOST 		*host;
    if ((family = ConvertAddr (saddr, len ? &len : 0, &addr)) < 0)
        return (1);
    if (family == 0)
        return (0);
    if (!AccessEnabled)   /* just let them in */
        return(0);    
    for (host = validhosts; host; host = host->next)
    {
        if (family == host->family && !acmp (addr, host->addr, len))
    	    return (0);
    }
    return (1);
}

ConvertAddr (saddr, len, addr)
    register struct sockaddr	*saddr;
    int				*len;
    pointer			*addr;
{
    if (len == 0)
        return (0);
    switch (saddr->sa_family)
    {
      case AF_UNSPEC:
      case AF_UNIX:
        return (0);
      case AF_INET:
	break;

#ifdef DNETCONN
      case AF_DECnet:
        *len = sizeof (struct dn_naddr);
        *addr = (pointer) &(((struct sockaddr_dn *) saddr)->sdn_add);
        return (AF_DECnet);
#else
	break;
#endif
      default:
        break;
    }
    return (-1);
}

int
ChangeAccessControl(client, fEnabled)
    ClientPtr client;
    int fEnabled;
{
    if (!AuthorizedClient(client))
	return BadAccess;
    AccessEnabled = fEnabled;
    return Success;
}

static int XFamily(af)
    int af;
{
    int i;
    for (i = 0; i < FAMILIES; i++)
        if (familyMap[i].af == af)
            return familyMap[i].xf;
    return -1;
}

static int UnixFamily(xf)
    int xf;
{
    int i;
    for (i = 0; i < FAMILIES; i++)
        if (familyMap[i].xf == xf)
            return familyMap[i].af;
    return -1;
}

