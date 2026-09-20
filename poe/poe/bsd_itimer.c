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
 * $Log:	bsd_itimer.c,v $
 * Revision 2.4  91/12/19  20:27:24  mrt
 * 	Updated to new copyright
 * 
 * Revision 2.3  90/12/04  21:55:13  rpd
 * 	Fixed bug in bsd_setitimer.
 * 	[90/12/04            rpd]
 * 
 * Revision 2.2  90/09/08  00:07:25  rwd
 * 	First checkin
 * 	[90/08/31  13:31:52  rwd]
 * 
 */
/*
 *	File:	./bsd_itimer.c
 *	Author:	Joseph S. Barrera III, Randall W. Dean
 *
 *	Copyright (c) 1990 Joseph S. Barrera III, Randall W. Dean
 */

#include <mach.h>
#include <errno.h>
#include <ux_user.h>
#include <sys/time.h>

extern struct timeout *post_timeout();

timer_ring(ut, sig)
	struct ux_task *ut;
	int sig;
{
	bsd_master(ut);
	bsd_sendsig(ut, sig);
	bsd_release();
}

bsd_setitimer(ut, rval, which, value, ovalue)
	struct ux_task *ut;
	int which;
	struct itimerval *value;
	struct itimerval *ovalue;
{
	int error;
	struct timeval *tvp;

	if (which != ITIMER_REAL) {
		dprintf("[%d]xxx non-real itimer\n");
		return EINVAL;
	}
	if (ovalue) {
		bcopy(&ut->ut_ritimer, ovalue, sizeof(*ovalue));
	}
	tvp = &value->it_interval;
	if (tvp->tv_sec || tvp->tv_usec) {
		dprintf("[%d]xxx real itimer with non-zero interval\n");
		return EINVAL;
	}
	if (ut->ut_ritimeout) {
		cancel_timeout(ut->ut_ritimeout);
		ut->ut_ritimeout = 0;
	}
	tvp = &value->it_value;
	if (tvp->tv_sec || tvp->tv_usec) {
		ut->ut_ritimeout = post_timeout(tvp->tv_sec, tvp->tv_usec,
			timer_ring, ut, SIGALRM);
	}
/*
 * XXXXXXX
 * this timeout should be stored as time-of-day!
 */
	ut->ut_ritimer = *value;
	return 0;
}

Bsd_getitimer()
{
	return EINVAL;
}
