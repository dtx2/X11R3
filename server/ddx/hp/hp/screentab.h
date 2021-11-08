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
/*
 * Header file which declares structures to initialize and close
 * each type of screen supported by the X server.
 *
 */

typedef struct {
    char *productNumber;
    char *productNickname;
    Bool (*InfoScreen)();
    Bool (*InitScreen)();
    Bool (*CloseScreen)();
    } ScreenTableRec;

typedef ScreenTableRec ScreenTable[];

typedef struct {
    int position;
    int left;
    int right;
    } MultiScreenRec;

extern ScreenTable screenTable;

extern MultiScreenRec multiScreenTable[];
