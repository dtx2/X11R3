#ifndef X_HIL_H
#define X_HIL_H
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

/* MAXHILEVENTS is the maximum number of X events that can
   be put on the events queue as the result of reading a 
   single HIL data packet.  The HIL definition is that 
   a packet may contain one motion event and up to 8 bytes
   of key data.  Motion events are not queued, so only the
   key data need be considered.  If the key device is being
   treated as an extension device, there will be two X
   events produced for each HIL event.  The maximum number
   of events from a single HIL data packet is therefore 16 */

#define MAXHILEVENTS		16
#define MOTION_BUFFER_SIZE	100
#define MAX_EVENTS		128
#define WR_EVENTS		MAX_EVENTS-1

struct	x11EventQueue
	{
	xEvent *events;
	int	size;
	int	head;
	int	tail;
	};		

#endif
