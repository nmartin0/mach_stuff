/*
 *  fd.h
 *  floppy
 *
 *  Created by D. Gatwood on Tue Jan 01 2002.
 *  Copyright (c) 2001 The MkLinux Project. All rights reserved.
 *
 */

#define NFD 2

// #undef MACH_DEBUG
// #define MACH_DEBUG 0
// #define SERIAL_DEBUG 0
#ifndef MACH_DEBUG
#define MACH_DEBUG 0
#endif
#ifndef SERIAL_DEBUG
#define SERIAL_DEBUG 0
#else
#if (SERIAL_DEBUG)
#define printf kprintf
#define IOLog kprintf
#undef MACH_DEBUG
#define MACH_DEBUG 1
#endif
#endif
#define POWERMAC_IO(address) (address)

typedef unsigned int uint_t;
typedef volatile unsigned int vuint_t;

/*  
 * A natural_t is the type for the native
 * integer type, e.g. 32 or 64 or.. whatever
 * register size the machine has.  Unsigned, it is
 * used for entities that might be either
 * unsigned integers or pointers, and for
 * type-casting between the two.
 * For instance, the IPC system represents
 * a port in user space as an integer and
 * in kernel space as a pointer.
 */     
#include <mach/mach_types.h>

#define NULL ((void *)0)

#define _BIG_ENDIAN 1

#define EVENT_WAKE 36

int non_bsd_sleep(void *event, unsigned long usec);
void non_bsd_wakeup(void *event);
vm_offset_t org_mklinux_iokit_swim3_get_io_base_addr();

extern int org_mklinux_iokit_swim3_objcount;

