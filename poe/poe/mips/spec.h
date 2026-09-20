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
 * Revision 2.2  94/03/25  18:23:09  mrt
 * 	Created for external special device declarations.
 * 	[94/02/20            mrt]
 * 
 */


#ifndef _MIPS_SPEC_H_
#define _MIPS_SPEC_H_

struct special_switch_struct {
	int (*routine)();
	char *name;
};

extern int nblkdev, nchardev;
extern struct special_switch_struct block_spec[];
extern struct special_switch_struct char_spec[];

#define ROOT_DEVICE_DIVISOR 8

#endif /* _MIPS_SPEC_H_ */
