/*
 * @OSF_COPYRIGHT@
 */
/*
 * HISTORY
 * $Log: mach_traps.h,v $
 * Revision 1.2.8.1  1994/09/23  02:39:16  ezf
 * 	change marker to not FREE
 * 	[1994/09/22  21:41:12  ezf]
 *
 * Revision 1.2.2.5  1993/08/05  19:09:21  jeffc
 * 	CR9508 - Delete dead code. Remove MACH_IPC_COMPAT
 * 	[1993/08/03  17:09:09  jeffc]
 * 
 * Revision 1.2.2.4  1993/08/03  18:29:31  gm
 * 	CR9596: Change KERNEL to MACH_KERNEL.
 * 	[1993/08/02  18:05:49  gm]
 * 
 * Revision 1.2.2.3  1993/07/12  18:07:28  gm
 * 	CR9339: Changed to use full ANSI-C prototypes for standard functions.
 * 	[1993/07/12  13:36:52  gm]
 * 
 * Revision 1.2.2.2  1993/06/09  02:41:35  gm
 * 	Added to OSF/1 R1.3 from NMK15.0.
 * 	[1993/06/02  21:17:00  jeffc]
 * 
 * Revision 1.2  1993/04/19  16:35:20  devrcs
 * 	correct endif tags for ansi
 * 	[1993/02/25  17:58:53  david]
 * 
 * Revision 1.1  1992/09/30  02:31:17  robert
 * 	Initial revision
 * 
 * $EndLog$
 */
/* CMU_HIST */
/*
 * Revision 2.5.2.1  92/03/03  16:22:09  jeffreyh
 * 	Changes from TRUNK
 * 	[92/02/26  12:09:51  jeffreyh]
 * 
 * Revision 2.6  92/01/15  13:44:25  rpd
 * 	Changed MACH_IPC_COMPAT conditionals to default to not present.
 * 
 * Revision 2.5  91/05/14  16:54:55  mrt
 * 	Correcting copyright
 * 
 * Revision 2.4  91/02/05  17:33:35  mrt
 * 	Changed to new Mach copyright
 * 	[91/02/01  17:18:21  mrt]
 * 
 * Revision 2.3  90/08/06  17:06:07  rpd
 * 	Removed mach_host_priv_self.
 * 	Removed definition of _MACH_INIT_.
 * 	[90/08/04            rpd]
 * 
 * Revision 2.2  90/06/02  14:58:30  rpd
 * 	Converted to new IPC.
 * 	[90/03/26  22:33:07  rpd]
 * 
 * Revision 2.1  89/08/03  15:59:29  rwd
 * Created.
 * 
 * Revision 2.4  89/02/25  18:37:15  gm0w
 * 	Changes for cleanup.
 * 
 * Revision 2.3  89/02/19  12:57:44  rpd
 * 	Moved from kern/ to mach/.
 * 
 * Revision 2.2  89/01/15  16:24:46  rpd
 * 	Updated includes for the new mach/ directory.
 * 	[89/01/15  15:03:03  rpd]
 * 
 * 18-Jan-88  David Golub (dbg) at Carnegie-Mellon University
 *	Add thread_reply.  Leave in task_data as an alternate name -
 *	they are functionally indistinguishable.
 *
 * 15-Oct-86  Avadis Tevanian (avie) at Carnegie-Mellon University
 *	Include ../kern/mach_types.h instead of <kern/mach_types.h> when
 *	building for the kernel.
 *
 *  1-Sep-86  Michael Young (mwyoung) at Carnegie-Mellon University
 *	Created, mostly to help build the lint library.
 *	Should eventually include this in "syscall_sw.c".
 *
 */
/* CMU_ENDHIST */
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
 */
/*
 *	Definitions of general Mach system traps.
 *
 *	IPC traps are defined in <mach/message.h>.
 *	Kernel RPC functions are defined in <mach/mach_interface.h>.
 */

#ifndef	_MACH_MACH_TRAPS_H_
#define _MACH_MACH_TRAPS_H_

#include <mach/port.h>

mach_port_t	mach_reply_port(void);

mach_port_t	mach_thread_self(void);

mach_port_t	(mach_task_self)(void);

mach_port_t	mach_host_self(void);

#endif	/* _MACH_MACH_TRAPS_H_ */
