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
#ifndef X11_HILDEF_H
#define X11_HILDEF_H
/* $XConsortium: x11_hildef.h,v 1.3 88/09/06 15:25:07 jim Exp $ */

/**************************************************************************
*  File: x11_hildef.h
*	Contains special declarations for porting HIL-driver from
*	X/V10 to X/V11.
*	Project: Porting X(V11) to HP9000S300
*	MTS: Sankar L. Chakrabarti.
*	Corvallis WorkStation Operation.
**************************************************************************/


typedef	struct	_locator_pos
		{ short	x;
		  short y;
		}  mouse_pos;

typedef	struct	_box
		{ short	bottom;
		  short	right;
		  short	left;
		  short top;
		} box;

extern PtrPrivRec *other_p[];
extern DevicePtr  hpOther[];   /* Other Devices   */
#endif
