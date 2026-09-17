/* 
 * Mach Operating System
 * Copyright (c) 1991
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
 * $Log: zalloc.c,v $
 * Revision 1.1.1.1  1995/05/04  06:56:42  sclawson
 * New files.
 *
 * Revision 2.3  92/01/22  22:55:08  rpd
 * 	Fixed includes to use "" when appropriate.
 * 	[92/01/18            rpd]
 * 
 * Revision 2.2  91/07/06  15:14:09  jsb
 * 	First checkin.
 * 
 */

#include "zalloc.h"

/*
 * XXX Should eventually not use malloc!
 */

/*
 *	zinit initializes a new zone.  The zone data structures themselves
 *	are stored in a zone, which is initially a static structure that
 *	is initialized by zone_init.
 */
zone_t zinit(size, max, alloc, pageable, name)
	vm_size_t	size;		/* the size of an element */
	vm_size_t	max;		/* maximum memory to use */
	vm_size_t	alloc;		/* allocation size */
	boolean_t	pageable;	/* is this zone pageable? */
	char		*name;		/* a name for the zone */
{
	register zone_t		z;

	z = (zone_t) malloc(sizeof(*z));

	if (z == ZONE_NULL)
		panic("zinit");

	z->elem_size = size;
	z->zone_name = name;
	return(z);
}

vm_offset_t zalloc(zone)
	register zone_t	zone;
{
	return (vm_offset_t) malloc(zone->elem_size);
}

void zfree(zone, elem)
	register zone_t	zone;
	vm_offset_t	elem;
{
	free(elem);
}
