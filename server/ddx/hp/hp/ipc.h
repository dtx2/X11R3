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

/** Structure of Shared memory: **/

#define MAX_SERVERS	9
#define MAX_LURKERS	10
#define MAX_HIL		9
#define NO_SERVER	-1

#define NO_CMD		0
#define CLOSE_DEVICE	1
#define OPEN_DEVICE	2
#define FREE_FOR_ALL	3
#define FREE_MY_DEV	4
#define BECOME_VISIBLE	5

#define VERTICAL	1
#define HORIZONTAL	0

#define FB_TYPE_GATORBOX	1
#define FB_TYPE_REN_98720	2
#define FB_TYPE_REN_98721	3
#define FB_TYPE_REN_98721W	4
#define FB_TYPE_TOPCAT		5
#define FB_TYPE_TOPCAT_1M	6
#define FB_TYPE_TOPCAT_2M	7

#define FB_TYPE_X11_SERVER	42


#define SEM_KEY		42
#define SHMEM_KEY	42

/** 
 ** Shared memory commands -
 **
 ** 1 - close device. The server that has is open should close it.
 ** 2 - open device.  The specified server should open the device.
 **                   The parameters exit_x and exit_y describe where the 
 **                   cursor should appear on the new display.
 ** 3 - free for all. Up for grabs.  If a exit location is specified,
 **                   The exit direction should be taken into account 
 **                   when the locator reappears.
 **                   If not, the next server should open the locator.
 **/

struct hil_info_struct {
    char fname[40];	/** fname used for this hil device		**/
    int server_num;	/** server that currently has the device open	**/
    int last_server;	/** the last server to have the device open	**/
    int fd;		/** fd that server has it open with;		**/
    int server_mask;	/** servers that would like to open the device	**/
    int server_count;	/** the number of servers that want the device	**/

    int cmd;		/** shared memory command			**/
    int parm;		/** parameter;					**/
    float exit_x;
    float exit_y;
};

struct shmem_struct {
    int sem_id;		/** semaphore used to lock the shared memory	**/
    int shmem_version;
    int keyboard_mask;
    int locator_mask;
    int wrap_enabled;
    struct hil_info_struct hil_info[MAX_HIL];
    int server_pid[MAX_SERVERS];
    int lurker_pid[MAX_LURKERS];
    int server_type[MAX_SERVERS];
};

