/*   KBD.INS.C, /sys/ins, JBT, 05/08/85 

   Changes:
      10/01/85 ltk  added F0, F9 and keypad escape
      05/08/85 ems  added mouse button definitions. */

/******************************************************************
Copyright 1987 by Apollo Computer Inc., Chelmsford, Massachusetts.
     
                        All Rights Reserved

Permission to use, duplicate, change, and distribute this software and
its documentation for any purpose and without fee is granted, provided
that the above copyright notice appear in such copy and that this
copyright notice appear in all supporting documentation, and that the
names of Apollo Computer Inc. or MIT not be used in advertising or publicity
pertaining to distribution of the software without written prior permission.
******************************************************************/

/*   Definitions for non-ascii keyboard keys, for use in borrow
    mode and direct mode in gpr_$event_wait.   */


#define KBD_$L1  0x81
#define KBD_$L2  0x82
#define KBD_$L3  0x83
#define KBD_$L4  0x84
#define KBD_$L5  0x85
#define KBD_$L6  0x86
#define KBD_$L7  0x87
#define KBD_$L8  0x88
#define KBD_$L9  0x89
#define KBD_$LA  0x8A
#define KBD_$LB  0x8B
#define KBD_$LC  0x8C
#define KBD_$LD  0x8D
#define KBD_$LE  0x8E
#define KBD_$LF  0x8F
    
#define KBD_$L1U 0xA1
#define KBD_$L2U 0xA2
#define KBD_$L3U 0xA3
#define KBD_$L4U 0xA4
#define KBD_$L5U 0xA5
#define KBD_$L6U 0xA6
#define KBD_$L7U 0xA7
#define KBD_$L8U 0xA8
#define KBD_$L9U 0xA9
#define KBD_$LAU 0xAA
#define KBD_$LBU 0xAB
#define KBD_$LCU 0xAC
#define KBD_$LDU 0xAD
#define KBD_$LEU 0xAE
#define KBD_$LFU 0xAF

#define KBD_$L1S 0xC9
#define KBD_$L2S 0xCA
#define KBD_$L3S 0xCB
#define KBD_$L4S 0xCC
#define KBD_$L5S 0xCD
#define KBD_$L6S 0xCE
#define KBD_$L7S 0xCF
#define KBD_$L8S 0xD8
#define KBD_$L9S 0xD9
#define KBD_$LAS 0xDA
#define KBD_$LBS 0xDB
#define KBD_$LCS 0xDC
#define KBD_$LDS 0xDD
#define KBD_$LES 0xDE
#define KBD_$LFS 0xDF

#define KBD_$L1A 0xE8
#define KBD_$L2A 0xE9
#define KBD_$L3A 0xEA
#define KBD_$L1AS 0xEC
#define KBD_$L2AS 0xED
#define KBD_$L3AS 0xEE
#define KBD_$L1AU 0xF8
#define KBD_$L2AU 0xF9
#define KBD_$L3AU 0xFA

#define KBD_$R1  0x90
#define KBD_$R2  0x91
#define KBD_$R3  0x92
#define KBD_$R4  0x93
#define KBD_$R5  0x94
#define KBD_$R6  0xEB
    
#define KBD_$R1U 0xB0
#define KBD_$R2U 0xB1
#define KBD_$R3U 0xB2
#define KBD_$R4U 0xB3
#define KBD_$R5U 0xB4
#define KBD_$R6U 0xFB

#define KBD_$R1S 0xC8
#define KBD_$R2S 0xB5
#define KBD_$R3S 0xB6
#define KBD_$R4S 0xB7
#define KBD_$R5S 0xB8
#define KBD_$R6S 0xEF

#define KBD_$BS  0x95
#define KBD_$CR  0x96
#define KBD_$TAB 0x97
#define KBD_$STAB 0x98
#define KBD_$CTAB 0x99

#define KBD_$F1  0xC0
#define KBD_$F2  0xC1
#define KBD_$F3  0xC2
#define KBD_$F4  0xC3
#define KBD_$F5  0xC4
#define KBD_$F6  0xC5
#define KBD_$F7  0xC6
#define KBD_$F8  0xC7
#define KBD_$F9  0xBB
#define KBD_$F0  0xBA
    
#define KBD_$F1S 0xD0
#define KBD_$F2S 0xD1
#define KBD_$F3S 0xD2
#define KBD_$F4S 0xD3
#define KBD_$F5S 0xD4
#define KBD_$F6S 0xD5
#define KBD_$F7S 0xD6
#define KBD_$F8S 0xD7
#define KBD_$F9S 0xBF
#define KBD_$F0S 0xBE

#define KBD_$F1U 0xE0
#define KBD_$F2U 0xE1
#define KBD_$F3U 0xE2
#define KBD_$F4U 0xE3
#define KBD_$F5U 0xE4
#define KBD_$F6U 0xE5
#define KBD_$F7U 0xE6
#define KBD_$F8U 0xE7
#define KBD_$F9U 0xBD
#define KBD_$F0U 0xBC
                    
#define KBD_$F1C 0xF0
#define KBD_$F2C 0xF1
#define KBD_$F3C 0xF2
#define KBD_$F4C 0xF3
#define KBD_$F5C 0xF4
#define KBD_$F6C 0xF5
#define KBD_$F7C 0xF6
#define KBD_$F8C 0xF7
#define KBD_$F9C 0xFD
#define KBD_$F0C 0xFC

    /* mouse buttons */

#define KBD_$M1D 'a' /* left-down   */
#define KBD_$M1U 'A' /* left-up     */ 
#define KBD_$M2D 'b' /* middle-down */
#define KBD_$M2U 'B' /* middle-up   */
#define KBD_$M3D 'c' /* right-down  */
#define KBD_$M3U 'C' /* right-up    */

    /* some tablets have 4 button pucks */

#define KBD_$M4D 'd'
#define KBD_$M4U 'D'

    /* the following names refer to the key cap names on the APOLLO 1 and
      APOLLO 2 keyboards. These names can be used only if the key caps are
      in the standard APOLLO locations.  IF a 1 follows a defined name then 
      the key cap name refers to an APOLLO 1 keyboard if a 2 follows then 
      the name refers to an APOLLO 2.  A '1' or '2'  has only been added to 
      key cap names that are common to both keyboards, but the key caps are in 
      different locations on the keyboards. */


    /* key caps common to both APOLLO 1 and APOLLO 2 keyboards */
    
#define KBD_$LINE_DEL     KBD_$L2
#define KBD_$CHAR_DEL     KBD_$L3                            
#define KBD_$CMD          KBD_$L5
#define KBD_$UP_ARROW     KBD_$L8
#define KBD_$LEFT_ARROW   KBD_$LA
#define KBD_$NEXT_WIN     KBD_$LB
#define KBD_$RIGHT_ARROW  KBD_$LC
#define KBD_$DOWN_ARROW   KBD_$LE
#define KBD_$L_BAR_ARROW  KBD_$L4
#define KBD_$R_BAR_ARROW  KBD_$L6
#define KBD_$L_BOX_ARROW  KBD_$L7
#define KBD_$R_BOX_ARROW  KBD_$L9
#define KBD_$READ         KBD_$R3
#define KBD_$EDIT         KBD_$R4
     
    
    /* APOLLO 1 keys */

#define KBD_$DOWN_BOX_ARROW1  KBD_$LD
#define KBD_$UP_BOX_ARROW1    KBD_$LF
#define KBD_$INS_MODE1        KBD_$L1
#define KBD_$MARK1            KBD_$R1
#define KBD_$SHELL1           KBD_$R2
#define KBD_$HOLD1            KBD_$R5


    /* APOLLO 2 keys */
                     
#define KBD_$DOWN_BOX_ARROW2  KBD_$LF
#define KBD_$UP_BOX_ARROW2    KBD_$LD
#define KBD_$INS_MODE2        KBD_$L1S
#define KBD_$COPY             KBD_$L1A
#define KBD_$CUT              KBD_$L1AS
#define KBD_$PASTE            KBD_$L2A
#define KBD_$UNDO             KBD_$L2AS
#define KBD_$MARK2            KBD_$L1
#define KBD_$SHELL2           KBD_$L5S
#define KBD_$GROW             KBD_$L3A
#define KBD_$MOVE             KBD_$L3AS
#define KBD_$POP              KBD_$R1
#define KBD_$AGAIN            KBD_$R2
#define KBD_$EXIT             KBD_$R5
#define KBD_$HOLD2            KBD_$R6
#define KBD_$SAVE             KBD_$R4S
#define KBD_$ABORT            KBD_$R5S
#define KBD_$HELP             KBD_$R6S

    /* APOLLO 3 key */

#define KBD_$NUMERIC_KEYPAD   0x9E
