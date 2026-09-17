/* 
 * Mach Operating System
 * Copyright (c) 1991,1990,1989,1988,1987 Carnegie Mellon University
 * All Rights Reserved.
 * 
 * Permission to use, copy, modify and distribute this software and its
 * documentation is hereby granted, provided that both the copyright
 * notice and this permission notice appear in all copies of the
 * software, derivative works or modified versions, and any portions
 * thereof, and that both notices appear in supporting documentation.
 * 
 * CARNEGIE MELLON ALLOWS FREE USE OF THIS SOFTWARE IN ITS "AS IS"
 * CONDITION.  CARNEGIE MELLON DISCLAIMS ANY LIABILITY OF ANY KIND FOR
 * ANY DAMAGES WHATSOEVER RESULTING FROM THE USE OF THIS SOFTWARE.
 * 
 * Carnegie Mellon requests users of this software to return to
 * 
 *  Software Distribution Coordinator  or  Software.Distribution@CS.CMU.EDU
 *  School of Computer Science
 *  Carnegie Mellon University
 *  Pittsburgh PA 15213-3890
 * 
 * any improvements or extensions that they make and grant Carnegie Mellon
 * the rights to redistribute these changes.
 */
/*
 * HISTORY
 * $Log: zalloc.h,v $
 * Revision 1.1.1.1  1995/05/04  06:56:42  sclawson
 * New files.
 *
 * Revision 2.2  91/07/06  15:15:06  jsb
 * 	First checkin.
 * 
 */

#ifndef	_KERN_ZALLOC_H_
#define _KERN_ZALLOC_H_

#include <mach.h>

typedef struct zone {
	vm_size_t	elem_size;	/* size of an element */
	char		*zone_name;	/* a name for the zone */
} *zone_t;

#define		ZONE_NULL	((zone_t) 0)

extern vm_offset_t	zalloc();
extern zone_t		zinit();
extern void		zfree();

#endif	_KERN_ZALLOC_H_
