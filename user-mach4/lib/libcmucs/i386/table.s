/* 
 * Mach Operating System
 * Copyright (c) 1991,1990,1989 Carnegie Mellon University
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
 * any improvements or extensions that they make and grant Carnegie the
 * rights to redistribute these changes.
 */
/*
 * HISTORY
 * $Log: table.s,v $
# Revision 1.4  1995/08/11  23:57:11  sclawson
# commented out table stuff after the final ret in SYSCALL.
#
# Revision 1.3  1995/06/30  19:43:35  sclawson
# Various cleanups for building on the x86.
#
# Revision 1.2  1995/05/04  07:06:46  sclawson
# define SYS_table.
#
 * Revision 2.3  92/02/16  15:25:05  rpd
 * 	Moved from libcs to libcmucs.
 * 
 * Revision 2.2  92/01/22  23:14:05  rpd
 * 	Moved to libsys from libmach.
 * 	[92/01/19            rpd]
 * 
 * Revision 2.2  91/04/11  11:16:17  mrt
 * 	Copied from /usr/cs/libsys.a
 * 
 * 14-Feb-90  Mary Thompson (mrt) at Carnegie-Mellon University
 *	Created.
 *
 */

#include <i386/SYS.h>
#include <mach/machine/asm.h>
#include <sys/syscall.h>

#ifndef SYS_table
/* Probably ux */
#define SYS_table	(-6)
#endif

SYSCALL(table)
	ret		/* table(id, index, addr, nel, lel) */
