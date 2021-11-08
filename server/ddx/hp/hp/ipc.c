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
 *  file: ipc.c
 *
 *  module purpose -- to allow multiple servers to share a common
 *                    set of input devices.
 *
 *	Don Bennett
 *	HP Labs
 */

#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/hilioctl.h>
#include "ipc.h"

#ifdef FB_MULTI_TOPCAT
#include "topcat.h"
#endif FB_MULTI_TOPCAT

#define RootWindow	rootwindow

#ifdef UNDEFINED
extern WINDOW *rootwindow;
extern CURSOR *cursor;			/* Current cursor */
extern DEVICE device;
#endif UNDEFINED

extern int errno;

#ifdef UNDEFINED
extern int XHP_width;
extern int XHP_height;

extern int shiftcount,metacount;	/* defined in ../X/input.c; reset when all
                                         * keyboard devices are closed.
                                         */
#endif UNDEFINED

#ifdef FB_MULTI_TOPCAT
extern FRAME_BUFFER * gp_hardware;
extern int TopcatMeg;
#endif FB_MULTI_TOPCAT

typedef struct sembuf SHMEM_FCN;

SHMEM_FCN shmem_lock_op[1]   = {0,-1,SEM_UNDO};
SHMEM_FCN shmem_unlock_op[1] = {0,1,SEM_UNDO};

#ifdef VERBOSE_REV_INFO
static char HPL2[] = "@(#) HP-Labs Multi-Server Input ver3.1 6/15/87";
#endif

static int sem_id;
static int shmem_id;
static int last_shmem_version = 0;
static int shmem_lock_count = 0;
static int ipc_initialized = 0;

int my_server_num;
static int hil_config_mask;

static int hil_open_mask;
static int hil_want_mask;

static int holder_fd[MAX_HIL];

struct shmem_struct *shmem;

AddToMask()
{ }

RemoveFromMask()
{ }

/********************************/
/** primative access routines; **/
/********************************/

prim_lock_shmem()
{
    int retval;

    if (shmem_lock_count == 0) {
        while((retval = semop(sem_id,shmem_lock_op,1)) == -1 && errno == EINTR) 
            ;
        if (retval == -1)
            printf("semop lock error = %d\n",errno);
    }

    shmem_lock_count++;
}

prim_unlock_shmem()
{
    int retval;

    shmem_lock_count--;

    if (shmem_lock_count == 0) {
        while((retval = semop(sem_id,shmem_unlock_op,1)) == -1 && errno == EINTR)         
            ;
        if (retval == -1)
            printf("semop unlock error = %d\n",errno);
    }
}

prim_shmem_new_version()
{
   prim_lock_shmem();
   (*shmem).shmem_version ++;
   prim_unlock_shmem();
}

prim_shmem_locked()
{
   return(semctl(sem_id,0,GETVAL,0) == 0);
}

shmem_init()
{
    int i;

    prim_lock_shmem();

    (*shmem).sem_id			= sem_id;
    (*shmem).shmem_version 		= 0;

    for (i=0; i<MAX_HIL; i++) {
        strcpy((*shmem).hil_info[i].fname,"");
        (*shmem).hil_info[i].server_num   = -1;
        (*shmem).hil_info[i].last_server  = -1;
        (*shmem).hil_info[i].server_mask  =  0;
        (*shmem).hil_info[i].server_count =  0;

        (*shmem).hil_info[i].cmd = NO_CMD;
    }

    for (i=0; i <MAX_SERVERS; i++) 
        (*shmem).server_pid[i] = 0;

    for (i=0; i <MAX_LURKERS; i++) 
        (*shmem).lurker_pid[i] = 0;

    (*shmem).keyboard_mask = 0;
    (*shmem).locator_mask  = 0;
    (*shmem).wrap_enabled = 1;

    prim_unlock_shmem();
}

sigwindow_handler()
{
    signal(SIGWINDOW,sigwindow_handler);
}

ipc_wrap_enabled()
{
    return((*shmem).wrap_enabled);
}

ipc_set_display_num(display_num)
char *display_num;
{
    if (sscanf(display_num,"%d",&my_server_num) != 1) {
       FatalError("could not convert display number. - '%s'\n",display_num);
    }
}

ipc_fb_type(type)
int type;
{
    (*shmem).server_type[my_server_num] = type;
}

ipc_init()
{
    int i,pid;
    int ipc_block_handler();
    static initialized = 0;

    if( initialized ) return;
    else initialized = 1;

    /******************************************/
    /** create and initialize shared memory; **/
    /******************************************/

    shmem_id = shmget(SHMEM_KEY,sizeof(*shmem),0666 | IPC_CREAT);
    if (shmem_id == -1) {
	if (errno != 22)
	    FatalError("Unknown shmem creation error - errno = %d.\n",errno);
	else {
	    fprintf(stderr,"Could not create shared memory segment - ");
	    fprintf(stderr,"another server probably created one already.\n");
	    exit(1);
	}
    }

    shmem = (struct shmem_struct *) shmat(shmem_id,0xAF0000, 0);
    if ((int)shmem == -1) {
        FatalError("Unknown shmem mapping error - errno = %d.\n",errno);
    }

    /************************************************************/
    /** create and initialize the semaphore we use to control  **/
    /** access to shared memory;                               **/
    /************************************************************/

    sem_id = semget(SEM_KEY,1,0666 | IPC_CREAT | IPC_EXCL);
    if (sem_id != -1) {
        /** initialize the semaphore and shared memory **/
        semctl(sem_id,0,SETVAL,1);
        shmem_init();
    }
    else 
        if (errno != EEXIST) {
            FatalError("Unknown semaphore creation error - errno = %d.\n",errno);
        }
        /** semaphore already exists **/
        else {
	    sem_id = semget(SEM_KEY,1,0666);
	    if (sem_id == -1) {
		FatalError("semaphore get error - errno = %d.\n",errno);
	    }

            /** just got an existing semaphore; it all of the servers who
             ** are using the shared memory segment are gone, reinitialize
             ** the semaphore and shared memory;
             **/
	   
            /** If I get around to it, this is probably a good place to clean
             ** up after servers that may have died and left a mess;
             **/

            for (i=0; i<MAX_SERVERS; i++) 
                if ((pid=(*shmem).server_pid[i]) != 0 &&
		    (kill(pid,0) == 0 || errno != ESRCH))
                    break;     /** break if we find one that is alive **/

	    if (i == MAX_SERVERS) {
                /** initialize the semaphore and shared memory **/
	        semctl(sem_id,0,SETVAL,1);
		shmem_init();
            }
	}

    /***********************/
    /** final setup stuff **/
    /***********************/

    if (my_server_num != -1)
       (*shmem).server_pid[my_server_num] = getpid();

    ipc_fb_type(FB_TYPE_X11_SERVER);

    prim_shmem_new_version();
    ipc_initialized++;
}

ipc_exit()
{
    int i, mymask;

    if ( ! ipc_initialized ) return;

    mymask = 1 << my_server_num;

    (*shmem).server_pid[my_server_num] = -1;
    prim_lock_shmem();
    for (i=0; i<MAX_HIL; i++)
        if ((*shmem).hil_info[i].server_mask & mymask) {
            /*********************************************************/
            /** remove bits in mask that may be set for this server **/
            /*********************************************************/
            (*shmem).hil_info[i].server_mask &= ~mymask;
            (*shmem).hil_info[i].server_count --;

            /*************************************************************/
            /** if we had a device open, mark it as free for all to use **/
            /*************************************************************/
            if ((*shmem).hil_info[i].server_num == my_server_num) {
		close((*shmem).hil_info[i].fd);
                (*shmem).hil_info[i].server_num = -1;
                (*shmem).hil_info[i].last_server = -1;
                (*shmem).hil_info[i].cmd = FREE_FOR_ALL;
            }
        }

    /** remove my pid from the shared memory structure **/
    (*shmem).server_pid[my_server_num] = 0;
    for (i=0; i<MAX_SERVERS; i++) 
        if ((*shmem).server_pid[i] != 0) 
            break;

    /** no more processes left, remove the shared memory segment **/
    if (i == MAX_SERVERS)
        shmctl(shmem_id,IPC_RMID,0);

    prim_shmem_new_version();
    kick_other_servers();
    prim_unlock_shmem();
}

ipc_try_to_open_hil(name,mode)
char *name; int mode;
{
    int mymask, hil_num, fd;
    struct stat statbuf;

    if (stat(name,&statbuf) != -1 && (statbuf.st_mode >> 12) == 002) {
        hil_num = (statbuf.st_rdev & 0xff) >> 4;        

        if ((*shmem).hil_info[hil_num].fname[0] == 0)
            strcpy((*shmem).hil_info[hil_num].fname,name);

        mymask = 1<< my_server_num;
        hil_config_mask |= 1 << hil_num;
        hil_want_mask   |= 1 << hil_num;

        prim_lock_shmem();
        (*shmem).hil_info[hil_num].server_mask |= mymask;
        (*shmem).hil_info[hil_num].server_count ++;
        prim_shmem_new_version();
        prim_unlock_shmem();

        holder_fd[hil_num] = -1;
        fd = open_hil(name,mode,hil_num);
        if (fd != -1) {
           record_hil_type(hil_num,fd);

           /** turn on cursor if a locators is open - this will cause the 
            ** cursor to come on the first time, because record_hil_type
            ** will not have yet set the bit in the locator mask when
            ** open hil is called; This code may need some reorganization.
            **/
           if (((*shmem).locator_mask & hil_open_mask) != 0)
               XHP_locators(1);
        }

        return(fd);
    }
    else
        return(-1);
}

XHP_locators()
{ }

get_hil_from_fd(fd)
int fd;
{
    struct stat statbuf;

    fstat(fd, &statbuf);
    return((statbuf.st_rdev & 0xff) >> 4);
}

open_hil(name,mode,hil_num)
char *name; int mode,hil_num;
{
    int fd;

    prim_lock_shmem();

    /** if no other server has it open, lets grab it! **/

    if ((*shmem).hil_info[hil_num].server_num == -1) {

        /** if we have a place holder open, close it first **/
        if (holder_fd[hil_num] != -1) {
            close(holder_fd[hil_num]);
            holder_fd[hil_num] = -1;
        }

        fd = open(name,mode);
        if (fd != -1) {
            (*shmem).hil_info[hil_num].server_num = my_server_num;
            (*shmem).hil_info[hil_num].fd = fd;

            hil_open_mask |= 1 << hil_num;
            hil_want_mask &= ~(1 << hil_num);

            prim_shmem_new_version();

            if (fixup_device_details(fd)) {
                AddToMask(fd);
		AddEnabledDevice(fd);
	    }
            else
                fd = -1;
        }
    }
    else
        fd = -1;

   /** if the open failed and the placeholder is not already open,
    ** open up a placeholder;
    **/
    if (fd == -1 && holder_fd[hil_num] == -1)
        holder_fd[hil_num] = open("/dev/null",O_RDONLY);

    prim_unlock_shmem();

    /** turn on cursor if a locators is open **/
    if (((*shmem).locator_mask & hil_open_mask) != 0)
        XHP_locators(1);

    return(fd);
}

close_hil(fd)
int fd;
{
    int hil_num;
    struct stat statbuf;   

    RemoveFromMask(fd);
    remove_details_entry(fd);

    hil_num = get_hil_from_fd(fd);
    hil_open_mask &= ~(1 << hil_num);
    hil_want_mask |= 1 << hil_num;

    prim_lock_shmem();

    close(fd);
    RemoveEnabledDevice(fd);

    holder_fd[hil_num] = open("/dev/null",O_RDONLY);

    (*shmem).hil_info[hil_num].last_server = my_server_num;
    (*shmem).hil_info[hil_num].server_num  = -1;
    prim_shmem_new_version();
    prim_unlock_shmem();

#ifdef UNDEFINED
    /** reset shift & meta counts if all keyboards are closed **/
    if ((*shmem).keyboard_mask & hil_open_mask == 0) {
        shiftcount = 0;
        metacount  = 0;   
    }
#endif UNDEFINED

    /** turn off cursor if all locators are closed **/
    if (((*shmem).locator_mask & hil_open_mask) == 0)
        XHP_locators(0);
}

#define KEYBOARD	1
#define LOCATOR		2

record_hil_type(hil_num,fd)
int hil_num, fd;
{
    unsigned int status, type, id;
    unsigned char describe[11];
   

    type = 0;
    status = ioctl(fd, HILID, &describe[0]);
    if (status != -1) {
        id = describe[0];
        if (id >= 0xA0 && id <= 0xFF)
            type = KEYBOARD;
        else if (id >= 0x60 && id <= 0x97) 
            type = LOCATOR;
    }

    /** printf("hil %d id is %02x, type is %d.\n",hil_num,id,type); **/

    prim_lock_shmem();
    if (type == KEYBOARD)
        (*shmem).keyboard_mask |= 1 << hil_num;
    if (type == LOCATOR)
        (*shmem).locator_mask |= 1 << hil_num;
    prim_unlock_shmem();
}

/** The block handler must do 2 things:
 ** (1) Check to see if there is a request in shared memory to close a particular device;
 ** (2) Check to see if we can grab some of the devices we still want;
 **/

ipc_block_handler()
{
    int i, KickThem;

    /** if someone has made an update to shared memory, check to see if there
     ** is a command to do something to one of the hil devices;
     **/

    KickThem = 0;

    if (last_shmem_version != (*shmem).shmem_version) {
        prim_lock_shmem();
        last_shmem_version = (*shmem).shmem_version;

        for (i=0; i<MAX_HIL; i++)
            switch((*shmem).hil_info[i].cmd) {

            /** If I have the device open, I've got to close it now; **/
            case CLOSE_DEVICE:	if ((*shmem).hil_info[i].server_num == my_server_num) {
				    close_hil((*shmem).hil_info[i].fd);
				    (*shmem).hil_info[i].cmd = NO_CMD;
                                }
				break;

            /** If someone wants we to open the device and its not already open **/
            case OPEN_DEVICE:	if ((*shmem).hil_info[i].parm == my_server_num &&
                                    (*shmem).hil_info[i].server_num == -1) {
				    open_hil((*shmem).hil_info[i].fname,O_RDONLY | O_NDELAY,i);
				    (*shmem).hil_info[i].cmd = NO_CMD;
				}
				break;

            /** A server put the device up for grabs;
             ** If an exit location was specified, I'll put the sprite at the
             ** corresponding edge;
             **/
	    case FREE_FOR_ALL:	/** do I want it? **/
                                if (hil_want_mask & (1 << i)) {
                                    int next1, next2, dir, fd, target_x, target_y;
                                    float flt_x,flt_y;

                                    flt_x = (*shmem).hil_info[i].exit_x;
                                    flt_y = (*shmem).hil_info[i].exit_y;
                                    next_in_line((*shmem).hil_info[i].last_server,flt_x,flt_y,&next1,&next2,&dir);

                                    /** take the device if I'm next in line or the other server
                                     ** doesn't want it; This would be a good place to check that the
                                     ** next guy who wants it is still alive;
                                     **/

                                    if ( next1 == my_server_num || 
                                        (((*shmem).hil_info[i].server_mask & (1 << next1)) == 0 &&
                                         next2 == my_server_num) ||
                                        ((*shmem).server_pid[next1] == 0 && (*shmem).server_pid[next2] == 0)
                                        ) {
                                        fd = open_hil((*shmem).hil_info[i].fname,O_RDONLY | O_NDELAY,i);
                                        if (fd != -1) {
#ifdef UNDEFINED
				            if (TopcatMeg) {
					        if (TopcatMeg == 1)
					            gp_hardware->alt_frame = 0;
						else
						    gp_hardware->alt_frame = ~0;
					        RestoreColorMap();
                	                    }
#endif UNDEFINED
                                        }
                                    }
                                }
				break;

	    /** This is where the server tells itself that it should close a device and put
             ** it up for grabs;  close the hil device and convert the command to a 
             ** FREE_FOR_ALL;
             **/
            case FREE_MY_DEV:   if ((*shmem).hil_info[i].server_num == my_server_num) {
				    close_hil((*shmem).hil_info[i].fd);
				    (*shmem).hil_info[i].cmd = FREE_FOR_ALL;
				    KickThem = 1;
                                }
				break;
	    case BECOME_VISIBLE:
#ifdef FB_MULTI_TOPCAT
		                if (TopcatMeg &&
 				    (*shmem).hil_info[i].server_num == my_server_num) {
			            if (TopcatMeg == 1)
					gp_hardware->alt_frame = 0;
				    else
				        gp_hardware->alt_frame = ~0;

				    RestoreColorMap();
				    (*shmem).hil_info[i].cmd = NO_CMD;
				}
#endif FB_MULTI_TOPCAT
				break;

            default:		break;
            }

        prim_unlock_shmem();

        if (KickThem)
            kick_other_servers();
    }
}

static int row_servers[] = {
   0007, 0007, 0007,
   0070, 0070, 0070,
   0700, 0700, 0700 };

static int col_servers[] = {
   0111, 0222, 0444, 
   0111, 0222, 0444,
   0111, 0222, 0444 };

/** other_servers - tell if there are others servers that want
 **                 some of my devices in the same row or column;
 **
 ** for the direction, 0 = horizontal (row), 1 = vertical(col);
 **/

other_servers(dir)
int dir;
{
   int i, other_server_mask;

   /** pick out the mask that will tell which servers are
    ** in the same row/col;
    **/

   other_server_mask = dir ? col_servers[my_server_num] : row_servers[my_server_num];
   other_server_mask &= ~(1 << my_server_num);

   /** return true if there is a hil device I have open that one of the
    ** servers in the other_server_mask also wants open;
    **/

   for (i=0; i<MAX_HIL; i++)
      if ((*shmem).hil_info[i].server_num == my_server_num  &&
          (*shmem).hil_info[i].server_mask & other_server_mask)
         return(VERTICAL);

   return(HORIZONTAL);
}

/** cycle_my_devices - close the devices I have that others want. **/

cycle_my_devices(dir,x_normalized,y_normalized)
int dir; float x_normalized,y_normalized;
{
   int i, other_server_mask;

   other_server_mask = dir ? col_servers[my_server_num] : row_servers[my_server_num];
   other_server_mask &= ~(1 << my_server_num);

   prim_lock_shmem();

   for (i=0; i<MAX_HIL; i++)
      if ((*shmem).hil_info[i].server_num == my_server_num  &&
          (*shmem).hil_info[i].server_mask & other_server_mask) {
         (*shmem).hil_info[i].cmd = FREE_MY_DEV;
         (*shmem).hil_info[i].exit_x = x_normalized;
         (*shmem).hil_info[i].exit_y = y_normalized;
      }         

   prim_shmem_new_version();
   prim_unlock_shmem();
}

kick_other_servers()
{
    int i, ret;

    prim_lock_shmem();

    for(i=0; i<MAX_SERVERS; i++)
        if ((*shmem).server_pid[i] != 0) {
            ret = kill((*shmem).server_pid[i],SIGWINDOW);
            if (ret == -1 && errno == ESRCH)
		(*shmem).server_pid[i] = 0;
        }

    for(i=0; i<MAX_LURKERS; i++)
        if ((*shmem).lurker_pid[i] != 0) {
            ret = kill((*shmem).lurker_pid[i],SIGWINDOW);
            if (ret == -1 && errno == ESRCH)
		(*shmem).lurker_pid[i] = 0;
        }

    prim_unlock_shmem();
}

next_in_line(last_server,x,y,next1,next2,dir)
int last_server; float x,y; int *next1,*next2,*dir;
{
   int next, sign;

   sign = 1;
   if (x == 0.0 || y == 0.0)
      sign = -1;

   *dir = VERTICAL;
   if (x == 0.0 || x == 1.0) {
      *dir = HORIZONTAL;
   }

   if (*dir == HORIZONTAL) {
      int base, offset, offset1, offset2;

      base   = last_server / 3;
      offset = last_server % 3;

      offset1 = last_server + sign;
      offset2 = last_server + sign + sign; 

      if (offset1 < 0) offset1 += 3;
      if (offset2 < 0) offset2 += 3;

      if (offset1 >= 3) offset1 -= 3;
      if (offset2 >= 3) offset2 -= 3;

      *next1 = base * 3 + offset1;
      *next2 = base * 3 + offset2;
   }
   /** cycle vertically **/
   else {
      int step;

      step = sign * 3;
      *next1 = last_server + step;
      *next2 = last_server + step + step;

      if (*next1 < 0) *next1 += 9;
      if (*next2 < 0) *next2 += 9;

      if (*next1 > 8) *next1 -= 9;
      if (*next2 > 8) *next2 -= 9;
   }
}

#ifdef IPCMON

WINDOW *rootwindow;

Warp_mouse()
{ }

Define_block_handler()
{ }

AddToMask()
{ }

RemoveFromMask()
{ }

fixup_device_details()
{ }

remove_details_entry()
{ }

int XHP_width;
int XHP_height;

main(argc,argv)
int argc; char *argv[];
{
   int i,hil_num;
   int cmd,arg;
   float x,y;

   my_server_num = 8;
   ipc_init();

#ifdef UNDEFINED
printf("sending myself sigwindow...\n");
kill(getpid(),SIGWINDOW);
printf("done - and again.\n");
kill(getpid(),SIGWINDOW);
printf("done.\n");
#endif UNDEFINED

   if (argc > 1) {
      prim_lock_shmem();

      sscanf(argv[1],"%d",&hil_num);
      printf("hil %d set to: ",hil_num);

      for (i=2; i<argc; i++)      
        switch(i) {
        case 2: sscanf(argv[i],"%d",&cmd);
                (*shmem).hil_info[hil_num].cmd = cmd;
                printf("cmd: %d ",cmd);
                break;
        case 3: sscanf(argv[i],"%d",&arg);
                (*shmem).hil_info[hil_num].parm = arg;
                printf("arg: %d ",arg);
                break;
        case 4: sscanf(argv[i],"%f",&x);
                (*shmem).hil_info[hil_num].exit_x = x;
                printf("x: %f ",x);
                break;
        case 5: sscanf(argv[i],"%f",&y);
                (*shmem).hil_info[hil_num].exit_y = y;
                printf("y: %f ",y);
                break;
        default: break;
        }
      printf("\n");
      kick_other_servers();
      prim_unlock_shmem();
   }

   printf("shmem version: %d\n",(*shmem).shmem_version);

   for (i=0; i<MAX_SERVERS; i++)
      printf("server pid[%d] = %d\n",i,(*shmem).server_pid[i]);

   for (i=0; i<MAX_HIL; i++) {
      printf("hil[%d]: server: %d last_server: %d server_mask: 0x%02x count: %d fd: %d\n",
         i,(*shmem).hil_info[i].server_num,(*shmem).hil_info[i].last_server,
         (*shmem).hil_info[i].server_mask,(*shmem).hil_info[i].server_count,
         (*shmem).hil_info[i].fd);
      printf("         cmd: %d  parm: %d x: %f y: %f\n",
         (*shmem).hil_info[i].cmd,(*shmem).hil_info[i].parm,
         (*shmem).hil_info[i].exit_x,(*shmem).hil_info[i].exit_y);
   }

   printf("keyboards: %02x\n",(*shmem).keyboard_mask);
   printf("locators:  %02x\n",(*shmem).locator_mask);
}
#endif IPCMON



