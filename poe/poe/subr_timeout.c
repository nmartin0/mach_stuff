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
 * $Log:	subr_timeout.c,v $
 * Revision 2.4  92/02/02  13:02:42  rpd
 * 	Fixed cthread_fork argument types.
 * 	[92/01/31            rpd]
 * 
 * Revision 2.3  91/12/19  20:29:15  mrt
 * 	Updated to new copyright
 * 
 * Revision 2.2  90/09/08  00:20:26  rwd
 * 	Wire cthread.
 * 	[90/08/16            rwd]
 * 
 */
/*
 *	File:	./subr_timeout.c
 *	Author:	Joseph S. Barrera III, Randall W. Dean
 *
 *	Copyright (c) 1990 Joseph S. Barrera III, Randall W. Dean
 */

#include <mach.h>
#include <ux_user.h>

/*
 *  Brain-dead implementation for now...
 */

struct timeout {
	int		sec;
	int		usec;
	int		(*func)();
	int		a1, a2, a3, a4, a5, a6, a7, a8;
	struct ux_task	*ut;
	boolean_t	cancelled;
};

static
do_timeout(to)
	struct timeout *to;
{
	cthread_set_kernel_limit(cthread_kernel_limit() + 1);
	cthread_wire();
	tv_sleep(to->sec, to->usec);
	if (! to->cancelled) {
		(*to->func)(to->a1, to->a2, to->a3, to->a4,
			    to->a5, to->a6, to->a7, to->a8);
	}
	free(to);
	cthread_set_kernel_limit(cthread_kernel_limit() - 1);
	cthread_exit(0);
}

/*
 *  the child thread that we create will not have the master lock
 */
struct timeout *
post_timeout(sec, usec, func, a1, a2, a3, a4, a5, a6, a7, a8)
	int sec, usec;
	int (*func)();
	int a1, a2, a3, a4, a5, a6, a7, a8;
{
	struct timeout *to;

	to = (struct timeout *) malloc(sizeof(*to));
	to->sec = sec;
	to->usec = usec;
	to->func = func;
	to->a1 = a1;
	to->a2 = a2;
	to->a3 = a3;
	to->a4 = a4;
	to->a5 = a5;
	to->a6 = a6;
	to->a7 = a7;
	to->a8 = a8;
	to->ut = (struct ux_task *) ut_self();
	to->cancelled = FALSE;
	cthread_detach(cthread_fork((any_t (*)()) do_timeout, (any_t) to));
	return to;
}

cancel_timeout(to)
	struct timeout *to;
{
	to->cancelled = TRUE;
}
