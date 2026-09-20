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
 * $Log:	vmparam.h,v $
 * Revision 2.5  91/12/19  20:29:00  mrt
 * 	Updated to new copyright
 * 
 * Revision 2.4  90/11/16  11:42:42  rwd
 * 	Add USRTEXT.
 * 	[90/11/13            rwd]
 * 
 * Revision 2.3  90/09/27  13:55:30  rwd
 * 	Fix STACK_END.
 * 	[90/09/26            rwd]
 * 
 * Revision 2.2  90/09/08  00:19:57  rwd
 * 	Created.
 * 	[90/07/14            rwd]
 * 
 */
/*
 *	File:	./mips/vmparam.h
 *	Author:	Joseph S. Barrera III, Randall W. Dean
 *
 *	Copyright (c) 1990 Joseph S. Barrera III, Randall W. Dean
 */

/*
 * Machine dependent constants for MIPS
 */

/*
 * Base address for U*X system call emulator.
 */
#define	EMULATOR_BASE	0x0fc00000
#define	EMULATOR_END	0x10000000

#define	STACK_END	0x7ffff000
#define STACK_SIZE	(128*1024)

#define USRTEXT		0x400000
#define USRSTACK	STACK_END
