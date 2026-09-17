/* 
 * Mach Operating System
 * Copyright (c) 1993 Carnegie Mellon University
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
 * $Log:	ms_timer_cancel.c,v $
 * Revision 2.2  93/11/17  18:59:59  dbg
 * 	Created.
 * 	[93/07/15            dbg]
 * 
 */

#include <mach.h>
#include <mach/message.h>

kern_return_t
timer_cancel(
	mach_timer_t	timer,
	int		flags)
{
	kern_return_t kr;

	kr = syscall_timer_cancel(timer, flags);
	if (kr == MACH_SEND_INTERRUPTED)
		kr = mig_timer_cancel(timer, flags);

	return kr;
}
