/* 
 * Mach Operating System
 * Copyright (c) 1991 Carnegie Mellon University
 * All Rights Reserved.
 * 
 * Permission to use, copy, modify and distribute this software and its
 * documentation is hereby granted, provided that both the copyright
 * notice and this permission notice appear in all copies of the
 * software, derivative works or modified versions, and any portions
 * thereof, and that both notices appear in supporting documentation.
 * 
 * CARNEGIE MELLON ALLOWS FREE USE OF THIS SOFTWARE IN ITS 
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
 * any improvements or extensions that they make and grant Carnegie the
 * rights to redistribute these changes.
 */
/*
 * HISTORY
 * $Log: kalloc.c,v $
 * Revision 1.1.1.1  1995/05/04  06:56:41  sclawson
 * New files.
 *
 * Revision 2.3  92/01/22  22:53:27  rpd
 * 	Fixed includes to use "" when appropriate.
 * 	[92/01/18            rpd]
 * 
 * Revision 2.2  91/07/06  15:12:19  jsb
 * 	First checkin.
 * 
 */
/*
 *	File:	kalloc.c
 *	Author:	Joseph S. Barrera III
 *	Date:	1991
 *
 *	User-level kalloc/kfree definitions.
 */

#include "kalloc.h"

/*
 * XXX Should eventually not use malloc!
 */

vm_offset_t
kalloc(size)
	vm_size_t size;
{
	return malloc(size);
}

void
kfree(m, size)
	vm_offset_t m;
	int size;
{
	free((char *) m);
}
