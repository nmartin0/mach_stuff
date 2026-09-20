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
 * $Log:	bsd_exception.c,v $
 * Revision 2.4  91/12/19  20:26:59  mrt
 * 	Updated to new copyright
 * 
 * Revision 2.3  91/03/09  14:32:00  rpd
 * 	Fixed catch_exception_raise to trust the task port,
 * 	not the server port.
 * 	[91/01/12            rpd]
 * 
 * Revision 2.2  90/09/08  00:06:45  rwd
 * 	First checkin
 * 	[90/08/31  13:27:31  rwd]
 * 
 */
/*
 *	File:	./bsd_exception.c
 *	Author:	Joseph S. Barrera III, Randall W. Dean
 *
 *	Copyright (c) 1990 Joseph S. Barrera III, Randall W. Dean
 */

#include <mach.h>
#include <mach/exception.h>
#include <fnode.h>
#include <fentry.h>
#include <ux_user.h>
#include <errno.h>

extern mach_port_t exception_port;

extern mach_port_t 
ux_exception(exception, code, subcode)
	int exception;
	int code;
	int subcode;
{
	switch(exception) {
	    case EXC_BAD_ACCESS:
		if (code == KERN_INVALID_ADDRESS)
		    return SIGSEGV;
		else
		    return SIGBUS;

	    case EXC_BAD_INSTRUCTION:
	        return SIGILL;

	    case EXC_ARITHMETIC:
	        return SIGFPE;

	    case EXC_EMULATION:
		return SIGEMT;

	    case EXC_SOFTWARE:
		return SIGSYS;

	    case EXC_BREAKPOINT:
		return SIGTRAP;
	}

	return SIGIOT;
}

kern_return_t
catch_exception_raise(server, thread, task, exception, code, subcode)
	mach_port_t	server;
	thread_t	thread;
	task_t		task;
	int		exception, code, subcode;
{
	int sig, pc, error;
	struct ux_task *ut;

	if (server != exception_port)
		printf("exception on port %x\n", server);

	if (lookup_ux_task_by_task(task, &ut)) {
		/*
		 *	We don't recognize this task as our own, so tell
		 *	the kernel we aren't handling the exception.
		 */

		(void) mach_port_deallocate(mach_task_self(), task);
		(void) mach_port_deallocate(mach_task_self(), thread);
		return KERN_FAILURE;
	}

	sig = ux_exception(exception, code, subcode);
	error = thread_get_pc(thread, &pc);
	if (error) {
		mach_error("thread_get_pc", error);
		pc = 0xe0e0e0e0;
	}
	printf("exception %d,0x%x,0x%x, pc=0x%x -> signal %d\n",
			exception, code, subcode, pc, sig);
	thread_dump(thread);
	bsd_sendsig(ut, sig);
	(void) mach_port_deallocate(mach_task_self(), task);
	(void) mach_port_deallocate(mach_task_self(), thread);
	return KERN_SUCCESS;
}
