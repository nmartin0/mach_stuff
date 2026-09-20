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
 * $Log:	bsd_misc.c,v $
 * Revision 2.6  92/02/02  13:01:58  rpd
 * 	Removed old IPC vestiges.
 * 
 * Revision 2.5  91/12/19  20:27:28  mrt
 * 	Updated to new copyright
 * 
 * Revision 2.4  91/03/09  14:32:15  rpd
 * 	Fixed Bsd_nothing to signal SIGSYS and return EINVAL.
 * 	[90/12/14            rpd]
 * 
 * Revision 2.3  90/12/04  21:55:17  rpd
 * 	Added Bsd_reboot.
 * 	[90/12/04            rpd]
 * 
 * Revision 2.2  90/09/08  00:07:33  rwd
 * 	Have dprintf print cthread_self also.
 * 	[90/09/04            rwd]
 * 	Define dprintf here.
 * 	[90/07/25            rwd]
 * 
 */
/*
 *	File:	./bsd_misc.c
 *	Author:	Joseph S. Barrera III, Randall W. Dean
 *
 *	Copyright (c) 1990 Joseph S. Barrera III, Randall W. Dean
 */

#include <mach.h>
#include <errno.h>
#include <signal.h>
#include <sys/resource.h>
#include <sys/time.h>
#include <ux_user.h>

extern mach_port_t privileged_host_port;

Bsd_nothing(ut, rval)
	struct ux_task *ut;
	int rval[2];
{
	bsd_sendsig(ut, SIGSYS);
	return EINVAL;
}

Bsd_null(ut, rval)
	struct ux_task *ut;
	int rval[2];
{
	return 0;
}

Bsd_sock(ut, rval)
	struct ux_task *ut;
	int rval[2];
{
	return EPERM;
}

Bsd_quot(ut, rval)
	struct ux_task *ut;
	int rval[2];
{
	return EPERM;
}

Bsd_eperm(ut, rval)
	struct ux_task *ut;
	int rval[2];
{
	return EPERM;
}

bsd_gettimeofday(ut, rval, tp, tzp)
	struct ux_task *ut;
	int rval[2];
	struct timeval *tp;
	struct timezone *tzp;
{
	gettimeofday(tp, tzp);
	return 0;
}

bsd_getrlimit(ut, rval, resource, rlp)
	struct ux_task *ut;
	int rval[2];
	int resource;
	struct rlimit *rlp;
{
	if (resource == RLIMIT_STACK) {
		rlp->rlim_cur = 512 * 1024;
		rlp->rlim_max = 512 * 1024;
		return 0;
	}
	return EINVAL;
}

Bsd_reboot(ut, rval, howto)
	struct ux_task *ut;
	int rval[2];
	int howto;
{
	if (ut->ut_euid != 0)
		return EPERM;

	(void) host_reboot(privileged_host_port, howto);
	return 0;
}

Bsd_getpid(ut, rval)
	struct ux_task *ut;
	int rval[2];
{
	rval[0] = ut->ut_pid;
	if (ut->ut_parent) {
		rval[1] = ut->ut_parent->ut_pid;
	} else {
		rval[1] = 1;	/* XXX */
	}
	return 0;
}

Bsd1_emulator_error(proc_port, message, size)
	mach_port_t	proc_port;
	char *		message;
	int		size;
{
	struct ux_task *ut;

	dprintf("bsd_emulator_error()\n");
	ut = lookup_ux_task(proc_port);
	if (ut) {
		printf("[%d]emulator_error: %s\n", ut->ut_pid, message);
	} else {
		printf("[?]emulator_error: %s\n", message);
	}
	return KERN_SUCCESS;
}

int	silent = 1;

dprintf(fmt, a1, a2, a3, a4, a5, a6, a7, a8)
	char *fmt;
	int a1, a2, a3, a4, a5, a6, a7, a8;
{
	if (! silent) {
		printf("[%x]",cthread_self());
		printf(fmt, a1, a2, a3, a4, a5, a6, a7, a8);
	}
}
