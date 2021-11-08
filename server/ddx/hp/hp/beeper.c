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
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;
; File:         beeper.c
; SCCS:         %A% %G% %U%
; Description:  Access Gator/Bobcat beeper
; Author:       Andreas Paepcke, HPLabs/ATL
; Created:      2-Aug-85
; Modified:     Thu Oct 15 12:53:00 1987 (Don Bennett) bennett@hpldpb
; Language:     C
; Package:      PSL
; Status:       Experimental (Do Not Distribute)
;
; (c) Copyright 1985, Hewlett-Packard Company, all rights reserved.
;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
*/

/* Public functions:

       beep
       noise
       beep_or_noise_left

   We offer three voices and a noise source. Each sound is controllable
   in pitch, volume and duration. Pitch goes from 0 to 1023, volume goes
   from 0 to 15, duration is between 0 and 255 10msec intervalls. A
   duration of 0 turns the voice on continuously. A volume of 0 turns
   it off.
   The manufacturing specs give details
   on the programming interface. Here is a summary:

   The beeper is accessed through ioctl calls. The request argument is
   either "Send data to beeper" or "Read voice values from beeper". The
   argument is a pointer to a 4 byte buffer. These four bytes
   are defined here.

   R0-R3: Register address field. In the order R2, R1, R0:
     
     0 0 0: Voice 1 frequency
     0 0 1: Voice 1 attenuation
     0 1 0: Voice 2 frequency
     0 1 1: Voice 2 attenuation
     1 0 0: Voice 3 frequency
     1 0 1: Voice 3 attenuation
     1 1 0: Noise control
     1 1 1: Noise attentuation

  F0-F9: 10 bits pitch
  A0-A3: Attenuation
  D0-D7: Duration in 10msec's

  The placement of data in the buffer is a bit srewy:

  Byte 0 (Frequency 1):  1 R2 R1 R0 F3 F2 F1 F0     LSB
  Byte 1(Frequency 2):  0  0 F9 F8 F7 F6 F5 F4
  Byte 2 (Attenuator) :  1 R2 R1 R0 A3 A2 A1 A0
  Byte 3 (Duration)   : D7 D6 D5 D4 D3 D2 D1 D0

  The volume is inversely proportional to the attenuation. In order
  to provide rising numbers for rising loudness to the user, we
  expect a volume and modify to get the attenuation. The same goes
  for the pitch. In order to calculate frequency of the pitch,
  use: 

           83333/(1023-pitch)

  It is possible at any time to request the time any voice has
  left to run. This is done by:
 
  F4: Read voice1 timer
  F5: Read voice2 timer
  F6: Read voice3 timer
  F7: Read voice4 timer (noise)

  Noise is generated using a shift register. The following controls
  are possible for noise:
 
  - Attenuation
  - Duration
  - Periodic or white noise
  - 3 shift rates or output of voice 4 as shift rate

  Bytes 0 and 1 of the data buffer must both have identical contents
  to control the noise. Attenuation and duration are as in the other
  voices. Bytes 0 and 1 should look like this:

  1 R2 R1 R0 0 FB NF1 NF0   LSB

  R2, R1 and R0 must be 1, 1 and 0. If FB is 0, periodic noise
  is generated. If FB is 1, white noise is produced.

  NF1 and NF2 control the shift rate of the noise generator:

  NF1     NF2     Shift Rate
  --------------------------
  0       0       M/64
  0       1       M/128
  1       0       M/256
  1       1       Uses tone generator 3 output


  M is related to the clock rate.

  The voice start routines return 0 if all is well, -1 if we had
  trouble accessing the device file for the beeper and -2 if given
  parameters were out of range:
*/

#include <fcntl.h>
#include <sys/hilioctl.h>

/*********************************************************************
*DEFINES:                                                            *
*********************************************************************/

#define ALL_OK           0
#define ACCESS_PROBLEM  -1
#define BAD_RANGE       -2
#define BEEPER_DEVICE   "/dev/rhil"

#define VOICE1_FREQ_REG 0x80          /* Top nibbles for byte0 for all voices: */
#define VOICE2_FREQ_REG 0xA0
#define VOICE3_FREQ_REG 0xC0
#define NOISE_FREQ_REG  0xE0

#define VOICE1_VOL_REG  0x90          /* Top nibbles for byte2 for all voices: */
#define VOICE2_VOL_REG  0xB0
#define VOICE3_VOL_REG  0xD0
#define NOISE_VOL_REG   0xF0

#define MIN_VOICE       1             /* Legal ranges for parms from user: */
#define MAX_VOICE       3
#define MIN_PITCH       0
#define MAX_PITCH       1023
#define MIN_DURATION    0
#define MAX_DURATION    255
#define MIN_VOLUME      0
#define MAX_VOLUME      15
#define MIN_TYPE        0
#define MAX_TYPE        1
#define MIN_RATE        0
#define MAX_RATE        3

static int beeper_fd = -1;

/*********************************************************************
* PUBLIC FUNCTIONS:                                                  *
*********************************************************************/

/*****************************************************************************
*                                                                            *
* Beep using specified voice:                                                *
*                                                                            *
*   TAKES:                                                                   *
*                                                                            *
*   VOICE    : from 1 to 3                                                   *
*   PITCH    : from 0 to 1023 (incl)                                         *
*   VOLUME   : from 0 to 15   (incl). Zero turns voice off.                  *
*   DURATION : from 0 to 255  (incl). Zero turns voice on continuously.      *
*                                                                            *
*   RETURNS:                                                                 *
*                                                                            *
*   0        : All ok                                                        *
*   -1       : Cannot access beeper device file                              *
*   -2       : Parameter out of range                                        *
*                                                                            *
******************************************************************************/

#ifdef hp9000s800
extern char *display ;
static char beeper_name[] = "/dev/hilkbd\0 " ;

beep(voice,pitch,volume,duration)
int voice,pitch,volume,duration;
 {
  char vol ;
  /* Check whether beeper device has already been opened: */
  if (beeper_fd < 0)
   {
    strcat(beeper_name,display) ;
    if ((beeper_fd = open(beeper_name,O_RDWR)) < 0)
      return(ACCESS_PROBLEM);
   }
   /*
    * map input range of 0 - 15 to driver range range of 0 - 255
    */
   if ( !volume ) return ;
   if ( voice == 2 )  /*  2 == CLICK_VOICE  which means key click */
    {
      /* doesn't appear there's any way to set the volume or duration 
         on 800 so I'll just pinch the volume for key clicks.  t.houser
       */
      vol = (char) volume * (KBD_MAXVOLUME/45);
    }
   else                         /* beeper */
    {
      vol = (char) volume * (KBD_MAXVOLUME/15);
    }
   ioctl(beeper_fd, KBD_BEEP, &vol);
 }

#else   /* must be a 300 */

beep(voice,pitch,volume,duration)

int voice,pitch,volume,duration;

{

unsigned char buffer[4];

  /* Check ranges of parameters: */
  if (
       (voice < MIN_VOICE)       ||
       (voice > MAX_VOICE)       ||
       (pitch < MIN_PITCH)       ||
       (pitch > MAX_PITCH)       ||
       (volume < MIN_VOLUME)     ||
       (volume > MAX_VOLUME)     ||
       (duration < MIN_DURATION) ||
       (duration > MAX_DURATION)
     )
    return(BAD_RANGE);
  /* Check whether beeper device has already been opened: */
  if (beeper_fd < 0)
    if ((beeper_fd = open(BEEPER_DEVICE,O_RDWR)) < 0)
      return(ACCESS_PROBLEM);
  /* Init the voice dependent data bytes. Note the inversion of user's
     volume and pitch specs to attenuation:
  */
  volume = MAX_VOLUME - volume;
  pitch  = MAX_PITCH  - pitch;
  switch (voice)
  {
    case 1: buffer[0] = VOICE1_FREQ_REG | (pitch & 0x0000000f);
            buffer[2] = VOICE1_VOL_REG  | (volume & 0x0000000f);
            break;
    case 2: buffer[0] = VOICE2_FREQ_REG | (pitch & 0x0000000f);
            buffer[2] = VOICE2_VOL_REG  | (volume & 0x0000000f);
            break;
    case 3: buffer[0] = VOICE3_FREQ_REG | (pitch & 0x0000000f);
            buffer[2] = VOICE3_VOL_REG  | (volume & 0x0000000f);
            break;
  };
  /* The high 6 bits of the pitch go into byte 1: */
  buffer[1] = 0x0000003f & (pitch >> 4);
  /* Duration: */
  buffer[3] = duration;
  if (ioctl(beeper_fd,EFTSBP,buffer) < 0)
    return(ACCESS_PROBLEM);
  return(ALL_OK);
} /* end function beep */
/*----------------------------------------------------*/

/******************************************************************************
* Produce noise.                                                              *
*                                                                             *
*   TAKES:                                                                    *
*                                                                             *
*   TYPE     : from 0 to 1.   0 is periodic noise. 1 is white noise.          *
*   RATE     : from 0 to 3.   0 is M/64. 1 is M/128. 2 is M/256. 3 means rate *
*                             determined by output of voice 3.                *
*   VOLUME   : from 0 to 15   (incl). Zero turns voice off.                   *
*   DURATION : from 0 to 255  (incl). Zero turns voice on continuously.       *
*                                                                             *
*   RETURNS:                                                                  *
*                                                                             *
*   0        : All ok                                                         *
*   -1       : Cannot access beeper device file                               *
*   -2       : Parameter out of range                                         *
******************************************************************************/

noise(type,rate,volume,duration)

int type,rate,volume,duration;

{

unsigned char buffer[4];

  /* Check ranges of parameters: */
  if (
       (type < MIN_TYPE)         || 
       (type > MAX_TYPE)         ||
       (rate < MIN_RATE)         ||
       (rate > MAX_RATE)         ||
       (volume < MIN_VOLUME)     ||
       (volume > MAX_VOLUME)     ||
       (duration < MIN_DURATION) ||
       (duration > MAX_DURATION)
     )
    return(BAD_RANGE);
  /* Check whether beeper device has already been opened: */
  if (beeper_fd < 0)
    if ((beeper_fd = open(BEEPER_DEVICE,O_RDWR)) < 0)
      return(ACCESS_PROBLEM);
  /* Invert the volume provided by the user getting the attenuation: */
  volume = MAX_VOLUME - volume;
  type   = type << 2;
  buffer[0] = NOISE_FREQ_REG | (type | rate);
  /* Byte one must be identical to byte 0: */
  buffer[1] = buffer[0];
  buffer[2] = NOISE_VOL_REG  | (volume & 0x0000000f);
  buffer[3] = duration;
  if (ioctl(beeper_fd,EFTSBP,buffer) < 0)
    return(ACCESS_PROBLEM);
  return(ALL_OK);
} /* end function noise */
/*----------------------------------------------------*/

/****************************************************************************
*                                                                           *
* Read how many 10msec intervalls are left till a voice has                 *
*   beeped to the end. This routine is good for noise also.                 *
*                                                                           *
*   TAKES:                                                                  *
*                                                                           *
*   voice    : from 1 to 4                                                  *
*                                                                           *
*   RETURNS:                                                                *
*                                                                           *
*   0        : Beeper has finished                                          *
*   >0       : Number of 10msec intervalls                                  *
*   -1       : Cannot access beeper device file                             *
*   -2       : Parameter out of range                                       *
*****************************************************************************/

beep_or_noise_left(voice)

int voice;

{

unsigned char buffer[4];

  /* Check for legal parameter range. The '+1' is to include noise generator: */
  if (
       (voice < MIN_VOICE)           ||
       (voice > (MAX_VOICE + 1))
     )
    return(BAD_RANGE);
  /* Check whether beeper device has already been opened: */
  if (beeper_fd < 0)
    if ((beeper_fd = open(BEEPER_DEVICE,O_RDWR)) < 0)
      return(ACCESS_PROBLEM);
  /* Get the timer values into the buffer bytes: */
  if (ioctl(beeper_fd,EFTRT,buffer) < 0)
    return(ACCESS_PROBLEM);
  return((int) buffer[voice-1]);
} /* end function beep_or_noise_left */
#endif

/*---------------------------------------------------*/
