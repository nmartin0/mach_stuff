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
# Revision 1.1.1.1  1995/05/04  06:56:40  sclawson
# New files.
#
 * Revision 2.3  92/02/16  15:25:39  rpd
 * 	Moved from libcs to libcmucs.
 * 
 * Revision 2.2  92/01/22  23:15:57  rpd
 * 	Moved to libsys from libmach.
 * 	[92/01/19            rpd]
 * 
 * Revision 2.2  91/04/11  11:16:43  mrt
 * 	Copied from /usr/cs/libsys.a
 * 
 */

#include "SYS.h"

#define SYS_table	(-6)

SYSCALL(table)
	ret		# table(id, index, addr, nel, lel)
