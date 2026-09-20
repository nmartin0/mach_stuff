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
 * Revision 2.4  91/12/19  20:28:24  mrt
 * 	Updated to new copyright
 * 
 * Revision 2.3  90/11/16  11:42:33  rwd
 * 	Add USRTEXT.
 * 	[90/11/13            rwd]
 * 
 * Revision 2.2  90/09/27  13:54:57  rwd
 * 	First Checkin
 * 	[90/09/10  19:06:13  rwd]
 * 
 */
/*
 *	File:	./i386/vmparam.h
 *	Author:	Joseph S. Barrera III, Randall W. Dean
 *
 *	Copyright (c) 1990 Joseph S. Barrera III, Randall W. Dean
 */


/*
 * Machine dependent constants for I386
 */
/*
 * Base address for U*X system call emulator.
 */

#define	EMULATOR_BASE	0xa0000000
#define	EMULATOR_END	0xa0040000

#define STACK_END	(0xc0000000 - sizeof(int[5]))
#define STACK_SIZE	((128*1024) - sizeof(int[5]))

#define USRTEXT		0x10000
#define USRSTACK	STACK_END

