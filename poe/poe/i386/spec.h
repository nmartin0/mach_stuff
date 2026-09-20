/* 
 * Mach Operating System
 * Copyright (c) 1991,1990 Carnegie Mellon University
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
 *  Software Distribution Coordinator   or   Software.Distribution@CS.CMU.EDU
 *  School of Computer Science
 *  Carnegie Mellon University
 *  Pittsburgh PA 15213-3890
 * 
 * any improvements or extensions that they made and grant Carnegie Mellon
 * the rights to redistribute these changes.
 */
/*
 * HISTORY
 * $Log:	spec.h,v $
 * Revision 2.2  94/03/25  18:22:51  mrt
 * 	Created for external special device declarations.
 * 	[94/02/20            mrt]
 * 
 */


#ifndef _I386_SPEC_H_
#define _I386_SPEC_H_

struct special_switch_struct {
	int (*routine)();
	char *name;
};

extern int nblkdev, nchardev;
extern struct special_switch_struct block_spec[];
extern struct special_switch_struct char_spec[];

/* 
 *  _NET_BSD_ case is compatible with NetBSD 0.9 
 *  the other case is compatible with CMU's Mach i386 
 */

#ifndef _NET_BSD_
#define ROOT_DEVICE_DIVISOR 16
#else
#define ROOT_DEVICE_DIVISOR 8
#endif /* _NET_BSD_ */

#endif /* _I386_SPEC_H_ */
