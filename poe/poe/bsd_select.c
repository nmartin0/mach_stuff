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
 * $Log:	bsd_select.c,v $
 * Revision 2.3  91/12/19  20:27:43  mrt
 * 	Updated to new copyright
 * 
 * Revision 2.2  90/09/08  00:15:39  rwd
 * 	Change select to allow for NULL fd_sets.
 * 	[90/07/14            rwd]
 * 
 */
/*
 *	File:	./bsd_select.c
 *	Author:	Joseph S. Barrera III, Randall W. Dean
 *
 *	Copyright (c) 1990 Joseph S. Barrera III, Randall W. Dean
 */

#include <mach.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/uio.h>
#include <sys/file.h>
#include <fnode.h>
#include <fentry.h>
#include <ux_param.h>
#include <ux_user.h>

#if 1
bsd_select(ut, rval, nfds, readfds, writefds, exceptfds, timeout)
	struct ux_task *ut;
	int rval[2];
	int nfds;
	unsigned long *readfds;
	unsigned long *writefds;
	unsigned long *exceptfds;
	struct timeval *timeout;
{
#if 0
if (readfds) printf("<readfds=0x%x\n", *readfds);
if (writefds) printf("<writefds=0x%x\n", *writefds);
if (exceptfds) printf("<exceptfds=0x%x\n", *exceptfds);
#endif
	if (timeout) {
		server_sleep(timeout->tv_sec, timeout->tv_usec);
		rval[0] = 0;
		return 0;
	} else {
		rval[0] = 1;
		return 0;
	}
}
#else
bsd_select(ut, rval, nfds, readfds, writefds, exceptfds, timeout)
	struct ux_task *ut;
	int rval[2];
	int nfds;
	unsigned long *readfds;
	unsigned long *writefds;
	unsigned long *exceptfds;
	struct timeval *timeout;
{
	int fd;
	char flags[DTABLESIZE];
	struct condition cd;

	if (nfds < 0 || nfds > DTABLESIZE) {
		return EBADF; /* ??? */
	}
	/*
	 * Decode, and do preliminary fd validity check.
	 * This could certainly be made more efficient,
	 * for example, by checking each long against 0
	 * before looping over each fd.
	 */
	bzero(flags, sizeof(flags));
	for (fd = 0; fd < nfds; fd++) {
		if (ut->ut_fd[fd] == FENTRY_NULL) {
			return EBADF;
		}
		if (readfds)
		if (FD_ISSET(readfds, fd)) {
			flags[fd] |= FREAD;
		}
		if (writefds)
		if (FD_ISSET(writefds, fd)) {
			flags[fd] |= FWRITE;
		}
		if (exceptfds)
		if (FD_ISSET(exceptfds, fd)) {
			flags[fd] |= FEXCEPT;
		}
	}
	/*
	 * Do initial nonblocking poll.
	 * fd checking already done above.
	 */
	done = FALSE;
	for (fd = 0; fd < nfds; fd++) {
		if (flags[fd]) {
			outflags[fd] = flags[fd];
			/* no error return for now. do we need one? */
			FOP_SELECT(ut->ut_fd[fd]->fe_fnode, &outflags[fd]);
			if (outflags[fd]) {
				done = TRUE;
			}
		}
	}
	if (done) {
		goto out;
	}
	if (timeout && timeout->tv_sec == 0 && timeout->tv_usec <= 1000) {
		goto out;
	}
	/*
	 * Do blocking poll.
	 */
	condition_init(&cd);
	
	if (timeout) {
		server_sleep(timeout->tv_sec, timeout->tv_usec);
		rval[0] = 0;
		return 0;
	} else {
		rval[0] = 1;
		return 0;
	}
}
#endif
