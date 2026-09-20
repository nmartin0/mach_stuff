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
 * $Log:	bsd_copy.c,v $
 * Revision 2.3  91/12/19  20:26:53  mrt
 * 	Updated to new copyright
 * 
 * Revision 2.2  90/09/08  00:06:34  rwd
 * 	First checkin
 * 	[90/08/31  13:26:51  rwd]
 * 
 */
/*
 *	File:	./bsd_copy.c
 *	Author:	Joseph S. Barrera III, Randall W. Dean
 *
 *	Copyright (c) 1990 Joseph S. Barrera III, Randall W. Dean
 */

#include <mach.h>
#include <errno.h>
#include <signal.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <ux_user.h>

/*
 * Stubs for routines that do copyins.
 */

Bsd_utimes(ut, rval, pathname, tvp)
	struct ux_task *ut;
	int rval[2];
	char *pathname;
	struct timeval *tvp;
{
	int error;

	error = copyin(ut, tvp, 2 * sizeof(*tvp), &tvp);
	if (error) {
		return error;
	}
	error = bsd_utimes(ut, rval, pathname, tvp);
	uncopyin(tvp, 2 * sizeof(*tvp));
	return error;
}

Bsd_sethostname(ut, rval, name, namelen)
	struct ux_task *ut;
	int rval[2];
	char *name;
	int namelen;
{
	int error;

	if (namelen < 0) {
		return EINVAL;
	}
#if 0
	if (namelen > sizeof(hostname) - 1) {
		namelen = sizeof(hostname) - 1;
	}
#endif
	error = copyin(ut, name, namelen, &name);
	if (error) {
		return error;
	}
	error = bsd_sethostname(ut, rval, name, namelen);
	if (error) {
		return error;
	}
	uncopyin(name, namelen);
	return 0;
}

Bsd_write(ut, rval, fd, buf, len)
	struct ux_task *ut;
	int rval[2];
	int fd;
	char *buf;
	int len;
{
	int error;

	error = copyin(ut, buf, len, &buf);
	if (error) {
		return error;
	}
	error = bsd_write(ut, rval, fd, buf, len);
	uncopyin(buf, len);
	return error;
}

Bsd_setitimer(ut, rval, which, value, ovalue)
	struct ux_task *ut;
	int which;
	struct itimerval *value;
	struct itimerval *ovalue;
{
	int error;
	struct itimerval _ovalue;

	error = copyin(ut, value, sizeof(*value), &value);
	if (error) {
		return error;
	}
	error = bsd_setitimer(ut, rval, which, value, &_ovalue);
	uncopyin(value, sizeof(*value));
	if (error) {
		return error;
	}
	if (ovalue) {
		error = copyout(ut, &_ovalue, ovalue, sizeof(*ovalue));
	}
	return error;
}

Bsd_select(ut, rval, nfds, readfds, writefds, exceptfds, timeout)
	struct ux_task *ut;
	int rval[2];
	int nfds;
	unsigned long *readfds;
	unsigned long *writefds;
	unsigned long *exceptfds;
	struct timeval *timeout;
{
	int error;

	error = copyin(ut, timeout, sizeof(*timeout), &timeout);
	if (error) {
		return error;
	}
	error = bsd_select(ut, rval, nfds, readfds, writefds, exceptfds,
			   timeout);
	uncopyin(timeout, sizeof(*timeout));
	return error;
}

Bsd_setgroups(ut, rval, ngids, gids)
	struct ux_task *ut;
	int rval[2];
	int ngids;
	int *gids;
{
	int error;

	if (ngids < 0 || ngids > NGIDS) {
		return EINVAL;
	}
	error = copyin(ut, gids, ngids * sizeof(int), &gids);
	if (error) {
		return error;
	}
	error = bsd_setgroups(ut, rval, ngids, gids);
	uncopyin(gids, ngids * sizeof(int));
	return error;
}

Bsd_sigstack(ut, rval, ss, oss)
	struct ux_task *ut;
	int rval[2];
	struct sigstack *ss, *oss;
{
	int error;
	struct sigstack _oss;

	if (ss) {
		error = copyin(ut, ss, sizeof(*ss), &ss);
		if (error) {
			return error;
		}
	}
	error = bsd_sigstack(ut, rval, ss, &_oss);
	if (ss) {
		uncopyin(ss, sizeof(*ss));
	}
	if (! error && oss) {
		error = copyout(ut, &_oss, oss, sizeof(*oss));
	}
	return error;
}

Bsd_sigvec(ut, rval, sig, vec, ovec)
	struct ux_task *ut;
	int rval[2];
	int sig;
	struct sigvec *vec, *ovec;
{
	int error;
	struct sigvec _ovec;

	if (vec) {
		error = copyin(ut, vec, sizeof(*vec), &vec);
		if (error) {
			return error;
		}
	}
	error = bsd_sigvec(ut, rval, sig, vec, &_ovec);
	if (vec) {
		uncopyin(vec, sizeof(*vec));
	}
	if (! error && ovec) {
		error = copyout(ut, &_ovec, ovec, sizeof(*ovec));
	}
	return error;
}

Bsd_stat(ut, rval, pathname, stp)
	struct ux_task *ut;
	int rval[2];
	char *pathname;
	struct stat *stp;
{
	int error;
	struct stat st;

	error = bsd_stat(ut, rval, pathname, &st);
	if (error) {
		return error;
	}
	return copyout(ut, &st, stp, sizeof(st));
}

Bsd_lstat(ut, rval, pathname, stp)
	struct ux_task *ut;
	int rval[2];
	char *pathname;
	struct stat *stp;
{
	int error;
	struct stat st;

	error = bsd_lstat(ut, rval, pathname, &st);
	if (error) {
		return error;
	}
	return copyout(ut, &st, stp, sizeof(st));
}

Bsd_fstat(ut, rval, fd, stp)
	struct ux_task *ut;
	int rval[2];
	int fd;
	struct stat *stp;
{
	int error;
	struct stat st;

	error = bsd_fstat(ut, rval, fd, &st);
	if (error) {
		return error;
	}
	return copyout(ut, &st, stp, sizeof(st));
}

Bsd_readlink(ut, rval, pathname, buf, len)
	struct ux_task *ut;
	int rval[2];
	char *pathname;
	char *buf;
	int len;
{
	int error;
	char pathbuf[MAXPATHLEN];

	error = bsd_readlink(ut, rval, pathname, pathbuf, len);
	if (error) {
		return error;
	}
	return copyout(ut, pathbuf, buf, rval[0]);
}

Bsd_gettimeofday(ut, rval, tp, tzp)
	struct ux_task *ut;
	int rval[2];
	struct timeval *tp;
	struct timezone *tzp;
{
	int error;
	struct timeval t;
	struct timezone tz;

	error = bsd_gettimeofday(ut, rval, &t, &tz);
	if (! error && tp) {
		error = copyout(ut, &t, tp, sizeof(t));
	}
	if (! error && tzp) {
		error = copyout(ut, &tz, tzp, sizeof(tz));
	}
	return error;
}

Bsd_getrlimit(ut, rval, resource, rlp)
	struct ux_task *ut;
	int rval[2];
	int resource;
	struct rlimit *rlp;
{
	int error;
	struct rlimit rl;

	error = bsd_getrlimit(ut, rval, resource, rlp);
	if (error) {
		return error;
	}
	return copyout(ut, &rl, rlp, sizeof(rl));
}

Bsd_getgroups(ut, rval, ngids, gids)
	struct ux_task *ut;
	int rval[2];
	int ngids;
	int *gids;
{
	int error;
	int _gids[NGIDS];

	error = bsd_getgroups(ut, rval, ngids, _gids);
	if (error) {
		return error;
	}
	copyout(ut, _gids, gids, rval[0] * sizeof(int));
	return 0;
}

