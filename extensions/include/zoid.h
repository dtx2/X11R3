/************************************************************
Copyright 1987 by Sun Microsystems, Inc. Mountain View, CA.

                    All Rights Reserved

Permission  to  use,  copy,  modify,  and  distribute   this
software  and  its documentation for any purpose and without
fee is hereby granted, provided that the above copyright no-
tice  appear  in all copies and that both that copyright no-
tice and this permission notice appear in  supporting  docu-
mentation,  and  that the names of Sun or MIT not be used in
advertising or publicity pertaining to distribution  of  the
software  without specific prior written permission. Sun and
M.I.T. make no representations about the suitability of this
software for any purpose. It is provided "as is" without any
express or implied warranty.

SUN DISCLAIMS ALL WARRANTIES WITH REGARD TO  THIS  SOFTWARE,
INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FIT-
NESS FOR A PARTICULAR PURPOSE. IN NO EVENT SHALL SUN BE  LI-
ABLE  FOR  ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,  DATA  OR
PROFITS,  WHETHER  IN  AN  ACTION OF CONTRACT, NEGLIGENCE OR
OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION  WITH
THE USE OR PERFORMANCE OF THIS SOFTWARE.

********************************************************/

/* $XConsortium: zoid.h,v 1.5 88/10/22 19:55:50 jim Exp $ */
/*
   ZOID uses the following defines to look up the correct procedure
   for its state block at ValidateGC time
*/

#define SOLID_Y     0
#define STIP_Y      1
#define TILE_Y      2
#define SOLID_X     3
#define STIP_X      4
#define TILE_X      5

/*
 * Protocol requests constants and alignment values
 * These would really be in ZOID's X.h and Xproto.h equivalents
 */

#define X_PolyFillZoid         1
#define X_SetZoidAlignment     2
#define XZoid_YAligned         1
#define XZoid_XAligned         2

typedef struct _PolyFillZoid {
    CARD8 reqType;          /* always ZoidReqCode */
    CARD8 zoidReqType;      /* always X_PolyFillZoid */
    CARD16 length B16;
    CARD32 drawable B32;
    CARD32 gc B32;
} xPolyFillZoidReq;     /* followed by either xXTraps or xYTraps */
#define sz_xPolyFillZoidReq 12

typedef struct _xYTraps {
    INT16 x1 B16, x2 B16, x3 B16, x4 B16, y1 B16, y2 B16;
} xYTraps;

typedef struct _xXTraps {
    INT16 y1 B16, y2 B16, y3 B16, y4 B16, x1 B16, x2 B16;
} xXTraps;

typedef union _xXYTraps {
    xXTraps Xt;
    xYTraps Yt;
} xXYTraps;

typedef struct _SetZoidAlignement {
    CARD8 reqType;          /* always ZoidReqCode */
    CARD8 zoidReqType;      /* always X_SetZoidAlignment */
    CARD16 length B16;
    CARD32 gc B32;
    CARD8 alignment;
    CARD8 pad[3];           /* must be aligned on long words */
} xSetZoidAlignmentReq;
#define sz_xSetZoidAlignmentReq 12

#define ZOIDNAME "ZoidExtension"


