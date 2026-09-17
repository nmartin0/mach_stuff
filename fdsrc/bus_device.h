/*
 *  bus_device.h
 *  floppy
 *
 *  Created by dg on Tue Feb 05 2002.
 *  Copyright (c) 2002 MkLinux. All rights reserved.
 *
 */
 
#include <sys/types.h>
#include <machine/spl.h>
#include <mach/mach_types.h>
// #include <mach/mach_ioctl.h>

typedef unsigned long io_return_t;
typedef unsigned long uint32;
typedef unsigned long intr_t;
typedef uint32           dev_flavor_t;
typedef int              *dev_status_t;
// typedef unsigned int	u_int;

#define   D_SUCCESS               0
#define   D_INVALID_OPERATION    -1
#define   D_INVALID_SIZE         -1
#define MAX_PHYS        (256 * 1024)

struct bus_device {
        struct bus_driver  *driver;     /* autoconf info */
        char               *name;       /* my name */
        int                 unit;
        intr_t              intr;
        caddr_t             address;    /* device address */
        int                 am;         /* address modifier */
        caddr_t             phys_address;/* device phys address */
        char                adaptor;
        char                alive;
        char                ctlr;
        char                slave;      
        int                 flags;      
        struct bus_ctlr    *mi;         /* backpointer to controller */
        struct bus_device  *next;       /* optional chaining */
        caddr_t             sysdep;     /* System dependent */
        long                sysdep1;    /* System dependent */
};

/* driver ioctl() commands */
#if 0
#define V_REMOUNT       _IO('v',2)              /* Remount Drive */
#define V_GETPARMS      _IOR('v',4,struct disk_parms)   /* Get drive/partition p
arameters */    
#define V_FORMAT        _IOW('v',5,union io_arg)/* Format track(s) */
#define V_ABS           _IOW('v',9,int)         /* set a sector for an absolute
addr */
#define V_RDABS         _IOW('v',10,struct absio)/* Read a sector at an absolute
 addr */
#define V_WRABS         _IOW('v',11,struct absio)/* Write a sector to absolute a
ddr */
#define V_VERIFY        _IOWR('v',12,union vfy_io)/* Read verify sector(s) */
#define V_SETPARMS      _IOW('v',14,int)        /* Set drivers parameters */
#define V_EJECT         _IO('v',15)             /* Eject disk */
#endif

#if 1
#include "portdef.h"
#include "floppypriv.h"
#include "floppycore.h"
#include "floppysal.h"
#include "floppyhal.h"
#endif


