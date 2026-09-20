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
 * $Log:	bsd_descrip.c,v $
 * Revision 2.3  91/12/19  20:26:55  mrt
 * 	Updated to new copyright
 * 
 * Revision 2.2  90/09/08  00:06:38  rwd
 * 	First checkin
 * 	[90/08/31  13:27:01  rwd]
 * 
 */
/*
 *	File:	./bsd_descrip.c
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
#include <sys/ioctl.h>
#include <fnode.h>
#include <fentry.h>
#include <fcntl.h>
#include <ux_user.h>
#include <bsd_ioctl.h>

fe_incr(fe)
	struct fentry *fe;
{
	fe->fe_refcount++;
}

fe_decr(fe)
	struct fentry *fe;
{
	if (--fe->fe_refcount == 0) {
		FOP_DECR(fe->fe_fnode);
		free(fe);
	}
}

int
fd_alloc(ut, start, fdp, alloc_fentry)
	struct ux_task *ut;
	int start;
	int *fdp;
	int alloc_fentry;
{
	int fd;

	for (fd = start; fd < DTABLESIZE; fd++) {
		if (ut->ut_fd[fd] == FENTRY_NULL) {
			break;
		}
	}
	if (fd == DTABLESIZE) {
		return EMFILE;
	}
	if (alloc_fentry) {
		ut->ut_fd[fd] = (struct fentry *)
		    malloc(sizeof(struct fentry));
		if (ut->ut_fd[fd] == FENTRY_NULL) {
			return ENFILE;
		}
		bzero(ut->ut_fd[fd], sizeof(struct fentry));
		fe_incr(ut->ut_fd[fd]);
	}
	*fdp = fd;
	return 0;
}

int
fd_lookup(ut, fd, fep)
	struct ux_task *ut;
	int fd;
	struct fentry **fep;
{
	if (fd < 0 || fd >= DTABLESIZE || (ut->ut_fd[fd] == FENTRY_NULL)) {
		return EBADF;
	}
	*fep = ut->ut_fd[fd];
	return 0;
}

Bsd_dup(ut, rval, fd1)
	struct ux_task *ut;
	int rval[2];
	int fd1;
{
	struct fentry *fe;
	int error, fd2;

	error = fd_lookup(ut, fd1, &fe);
	if (error) {
		return error;
	}
	error = fd_alloc(ut, 0, &fd2, FALSE);
	if (error) {
		return error;
	}
	fe_incr(fe);
	ut->ut_fd[fd2] = fe;
	ut->ut_fdflags[fd2] = ut->ut_fdflags[fd1];
	rval[0] = fd2;
	return 0;
}

Bsd_dup2(ut, rval, fd1, fd2)
	struct ux_task *ut;
	int rval[2];
	int fd1, fd2;
{
	struct fentry *fe;
	int error;

	if (fd2 < 0 || fd2 >= DTABLESIZE) {
		return EBADF;
	}
	error = fd_lookup(ut, fd1, &fe);
	if (error) {
		return error;
	}
	if (ut->ut_fd[fd2] != FENTRY_NULL) {
		fe_decr(ut->ut_fd[fd2]);
	}
	fe_incr(fe);
	ut->ut_fd[fd2] = fe;
	ut->ut_fdflags[fd2] = ut->ut_fdflags[fd1];
	rval[0] = fd2;
	return 0;
}

Bsd_close(ut, rval, fd, a2, a3, a4, a5, a6)
	struct ux_task *ut;
	int rval[2];
	int fd;
{
	struct fentry *fe;
	int error;

#if 0
	if (fd >= 666000 && fd <= 666999) {
		return Bsd_backdoor(ut, rval, fd, a2, a3, a4, a5, a6);
	}
#endif
	error = fd_lookup(ut, fd, &fe);
	if (error) {
		return error;
	}
	fe_decr(fe);
	ut->ut_fd[fd] = FENTRY_NULL;
	return 0;
}

Bsd_getdtablesize(ut, rval)
	struct ux_task *ut;
	int rval[2];
{
	rval[0] = DTABLESIZE;
	return 0;
}

#define	FnValidFlags	(FNDELAY | FAPPEND | FASYNC)

Bsd_fcntl(ut, rval, fd, command, arg)
	struct ux_task *ut;
	int rval[2];
	int fd;
	int command;
	int arg;
{
	struct fentry *fe;
	int error;
	int pgrp;

	error = fd_lookup(ut, fd, &fe);
	if (error) {
		return error;
	}
	switch (command) {
	case F_DUPFD:
		if (arg < 0 || arg >= DTABLESIZE) {
			return EINVAL;
		}
		error = fd_lookup(ut, fd, &fe);
		if (error) {
			return error;
		}
		error = fd_alloc(ut, arg, &arg, FALSE);
		if (error) {
			return error;
		}
		fe_incr(fe);
		ut->ut_fd[arg] = fe;
		ut->ut_fdflags[arg] = ut->ut_fdflags[fd];
		ut->ut_fdflags[arg] &= ~1; /* clear close-on-exec */
		rval[0] = arg;
		return 0;

	case F_GETFD:
		rval[0] = ut->ut_fdflags[fd] & 1;
		return 0;

	case F_SETFD:
		ut->ut_fdflags[fd] = (ut->ut_fdflags[fd]&FnValidFlags) | (arg&1);
		return 0;

	case F_GETFL:
		rval[0] = ut->ut_fdflags[fd] & FnValidFlags;
		return 0;

	case F_SETFL:
		ut->ut_fdflags[fd] = (arg&FnValidFlags) | (ut->ut_fdflags[fd]&1);
		return 0;
		
	case F_GETOWN:
		error = fd_lookup(ut, fd, &fe);
		if (error) {
			return error;
		}
		error = FOP_IOCTL(fe->fe_fnode, IOC_OUT, 't', icmd(TIOCGPGRP),
				  &pgrp, sizeof(pgrp));
		if (error) {
			return error;
		}
		rval[0] = pgrp;
		return 0;

	case F_SETOWN:
		error = fd_lookup(ut, fd, &fe);
		if (error) {
			return error;
		}
		return FOP_IOCTL(fe->fe_fnode, IOC_IN, 't', icmd(TIOCSPGRP),
				  &arg, sizeof(arg));

	default:
		return EINVAL;
	}
}
