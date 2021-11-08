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
#ifndef XTHREEBUT_H
#define XTHREEBUT_H
/***********************************************************************
 * file: x_threebut.h
 */

# define NO_OPKEY	0x0
# define LOOKAHEAD_SEC	0		/* look ahead delay in seconds */
# define LOOKAHEAD_USEC 100000		/* microsecond	*/
# define REPORTABLE	1		/* characterizes transition type */
# define NOT_REPORTABLE  0

/*
**  Mouse state defines:
**	Lup : Left button is up; Ldn : Left button is down.
**	R : Right button; M: Middle button :
**	Mhalf:	Middle button is on the process of going half.
**
*/

# define Lup_Rup_Mup	0
# define Lup_Rdn_Mup	1
# define Ldn_Rup_Mup	2
# define Lup_Rup_Mdn	3		/* physically Ldn_Rdn	*/
# define Lup_Rdn_Mhalf	4
# define Ldn_Rup_Mhalf	5
# define Ldn_Rdn_Mup	6
# define Ldn_Rdn_Mdn	7		/* Physically impossible, transient*/
# define Ldn_Rup_Mdn	ILLEGAL
# define Lup_Rdn_Mdn	ILLEGAL


/*
**	Stimulii : To change the mouse state.
**
*/

# define  Ldn		0
# define  Lup		1
# define  Rdn		2
# define  Rup		3
# define  Mdn		4
# define  Mup		5
# define  Mhalf		7	/* Middle button halfway up. 	*/

/*
** State Table for the state machine.
**
*/

struct	stat_button
    {	
    short	state;
    short	button;
    short   transition_type;
    };

struct	stat_button stat_table [7] [4] = {
/**	 Ldn		     Lup	   Rdn		        Rup	 **/
/** Lup_Rup_Mup	**/
 {  
	/* Ldn	*/	{Ldn_Rup_Mup, Ldn, REPORTABLE },
	/* Lup	*/	{Lup_Rup_Mup, Lup, REPORTABLE},
	/* Rdn	*/	{Lup_Rdn_Mup, Rdn, REPORTABLE},
	/* Rup	*/	{Lup_Rup_Mup, Rup, REPORTABLE}  },

/** Lup_Rdn_Mup **/
 { 	/* Ldn	*/	{Ldn_Rdn_Mup, Ldn, REPORTABLE},
	/* Lup	*/	{Lup_Rdn_Mup, Rdn, /* Lup, */ REPORTABLE},
	/* Rdn	*/	{Lup_Rdn_Mup, Rdn, REPORTABLE},
	/* Rup	*/	{Lup_Rup_Mup, Rup, REPORTABLE} },

/** Ldn_Rup_Mup **/
 {	/* Ldn	*/	{Ldn_Rup_Mup, Ldn, REPORTABLE},
	/* Lup	*/	{Lup_Rup_Mup, Lup, REPORTABLE},
	/* Rdn	*/	{Ldn_Rdn_Mup, Rdn, REPORTABLE},
	/* Rup	*/	{Ldn_Rup_Mup, Ldn /* Rup */ , REPORTABLE} },
	
/** Lup_Rup_Mdn **/
 {	/* Ldn 	*/	{Ldn_Rup_Mdn, Ldn, REPORTABLE},
	/* Lup	*/	{Lup_Rdn_Mhalf, Mhalf, NOT_REPORTABLE},
	/* Rdn	*/	{Lup_Rdn_Mdn, Rdn, REPORTABLE},   
	/* Rup	*/	{Ldn_Rup_Mhalf, Mhalf, NOT_REPORTABLE} },

/** Lup_Rdn_Mhalf **/
 {	/* Ldn	*/	{Lup_Rup_Mdn, Mdn, NOT_REPORTABLE},
	/* Lup	*/	{Lup_Rdn_Mhalf, Lup, NOT_REPORTABLE},
	/* Rdn	*/	{Lup_Rdn_Mhalf, Rdn, NOT_REPORTABLE}, 
	/* Rup	*/	{Lup_Rup_Mup, Mup, REPORTABLE} },

/** Ldn_Rup_Mhalf **/
 {	/* Ldn	*/	{Ldn_Rup_Mhalf, Ldn, NOT_REPORTABLE},
	/* Lup	*/	{Lup_Rup_Mup, Mup, REPORTABLE},
	/* Rdn	*/	{Lup_Rup_Mdn, Mdn, NOT_REPORTABLE},
	/* Rup	*/	{Ldn_Rup_Mhalf, Rup, NOT_REPORTABLE} },

/** Ldn_Rdn_Mup	  **/
{	/* Ldn	*/	{Ldn_Rdn_Mup, Ldn, REPORTABLE},	/* Can it happen ?*/
	/* Lup	*/	{Lup_Rdn_Mup, Lup, REPORTABLE},	
	/* Rdn	*/	{Ldn_Rdn_Mup, Rdn, REPORTABLE},	/* Can it happen ?*/
	/* Rup	*/	{Ldn_Rup_Mup, Rup, REPORTABLE} }
};
#endif
