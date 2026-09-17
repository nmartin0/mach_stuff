/* 
 * Mach Operating System
 * Copyright (c) 1993,1991,1990,1989 Carnegie Mellon University
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
 * $Log:	syscall_subr.h,v $
 * Revision 2.6  93/11/17  17:26:38  dbg
 * 	ANSI-fied.  Removed ancient history.
 * 	[93/10/13            dbg]
 * 
 * Revision 2.5  91/05/18  14:33:56  rpd
 * 	Added thread_depress_timeout.
 * 	[91/03/31            rpd]
 * 
 * Revision 2.4  91/05/14  16:47:37  mrt
 * 	Correcting copyright
 * 
 * Revision 2.3  91/02/05  17:29:40  mrt
 * 	Changed to new Mach copyright
 * 	[91/02/01  16:18:24  mrt]
 * 
 * Revision 2.2  90/06/02  14:56:22  rpd
 * 	Created.
 * 	[90/03/26  23:52:40  rpd]
 * 
 */

#ifndef	_KERN_SYSCALL_SUBR_H_
#define _KERN_SYSCALL_SUBR_H_

#include <mach/boolean.h>
#include <mach/message.h>
#include <kern/kern_types.h>

extern boolean_t
swtch(void);		/* obsolete */

extern boolean_t
swtch_pri(int pri);	/* obsolete */

extern kern_return_t
thread_switch(
	mach_port_t	thread_name,
	int		option,
	mach_msg_timeout_t option_time);

extern kern_return_t
thread_depress_abort(
	thread_t	thread);

#endif	/* _KERN_SYSCALL_SUBR_H_ */
