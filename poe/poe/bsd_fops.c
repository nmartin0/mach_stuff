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
 * $Log:	bsd_fops.c,v $
 * Revision 2.3  91/12/19  20:27:07  mrt
 * 	Updated to new copyright
 * 
 * Revision 2.2  90/09/08  00:06:58  rwd
 * 	Lets do FOP_OPEN, shall we.
 * 	[90/07/20            rwd]
 * 
 */
/*
 *	File:	./bsd_fops.c
 *	Author:	Joseph S. Barrera III, Randall W. Dean
 *
 *	Copyright (c) 1990 Joseph S. Barrera III, Randall W. Dean
 */

#include <mach.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/uio.h>
#include <sys/file.h>
#include <fnode.h>
#include <fentry.h>
#include <ux_user.h>

fn_path(ut, pathname, follow, fnp)
	struct ux_task *ut;
	char *pathname;
	int follow;
	struct fnode *fnp;
{
	return bsd_lookup(ut->ut_cdir, pathname, fnp, follow);
}

fn_fd(ut, fd, fnp)
	struct ux_task *ut;
	int fd;
	struct fnode **fnp;
{
	int error;
	struct fentry *fe;

	error = fd_lookup(ut, fd, &fe);
	if (error) {
		return error;
	}
	*fnp = fe->fe_fnode;
	FOP_INCR(*fnp); /* to match fn_path */
	return 0;
}

fn_checktype(fn, type, type_error)
	struct fnode *fn;
	int type;
	int type_error;
{
	int error;
	struct stat st;

	error = FOP_GETSTAT(fn, &st, FATTR_MODE);
	if (error) {
		return error;
	}
	if ((st.st_mode & S_IFMT) != type) {
		return type_error;
	}
	return 0;
}

Bsd_open(ut, rval, pathname, flags, mode)
	struct ux_task *ut;
	int rval[2];
	char *pathname;
	int flags;
	int mode;
{
	struct fnode *fn;
	int fd, error;
	struct stat st;

	/* XXX doesn't handle FEXCL for symbolic links */
	if ((flags & 0x3) == 0x3) {
		return EINVAL;
	}
	flags += 1;	/* maps O_{RD,WR}* to {FREAD,FWRITE} ! */
	error = bsd_lookup(ut->ut_cdir, pathname, &fn, TRUE);
	if (error) {
		char *dirname, *basename;
		struct fnode *fnd;

		/*
		 *  This path is taken when creating a new file.
		 */
		if (error != ENOENT || (flags & FCREAT) == 0) {
			return error;
		}
		path_split(pathname, &dirname, &basename);
		error = fn_path(ut, dirname, TRUE, &fnd);
		if (error) {
			return error;
		}
		error = FOP_CREATE(fnd, &fn);
		if (error) {
			FOP_DECR(fnd);
			return error;
		}
		error = FOP_LINK(fnd, basename, fn);
		FOP_DECR(fnd);
		if (error) {
			FOP_DECR(fn); /* which should un-alloc it */
			return error;
		}
	} else {
		/*
		 *  This path is taken when opening an existing file.
		 */
		if ((flags & (FCREAT|FEXCL)) == (FCREAT|FEXCL)) {
			FOP_DECR(fn);
			return EEXIST;
		}
		if (flags & FTRUNC) {
			st.st_size = 0;
			error = FOP_SETSTAT(fn, &st, FATTR_SIZE);
			if (error) {
				FOP_DECR(fn);
				return error;
			}
		}
	}
		
	error = fd_alloc(ut, 0, &fd, TRUE);
	if (error) {
		FOP_DECR(fn);
		return error;
	}

	error = FOP_OPEN(fn);
	if (error) {
		struct fentry *fe;
		FOP_DECR(fn);
		if (!fd_lookup(ut, fd, &fe))
		    fe_decr(fe);
		return error;
	}

	ut->ut_fd[fd]->fe_rdwr = (flags & (FREAD|FWRITE));
	ut->ut_fd[fd]->fe_append = ((flags & FAPPEND) != 0);
	ut->ut_fd[fd]->fe_fnode = fn;
	rval[0] = fd;
	return 0;
}

Bsd_creat(ut, rval, pathname, mode)
	struct ux_task *ut;
	int rval[2];
	char *pathname;
	int mode;
{
	return Bsd_open(ut, rval, pathname, O_CREAT|O_WRONLY|O_TRUNC, mode);
}

Bsd_pipe(ut, rval, pp)
	struct ux_task *ut;
	int rval[2];
	int *pp;
{
	struct fnode *fnr, *fnw;
	int fd, error, p[2];

	error = pipe_open(&fnr, &fnw);
	if (error) {
		return error;
	}
	error = fd_alloc(ut, 0, &p[0], TRUE);
	if (! error) {
		error = fd_alloc(ut, 0, &p[1], TRUE);
	}
	if (error) {
		FOP_DECR(fnr);
		FOP_DECR(fnw);
		return error;
	}
	ut->ut_fd[p[0]]->fe_fnode = fnr;
	ut->ut_fd[p[0]]->fe_rdwr = FREAD;
	ut->ut_fd[p[1]]->fe_fnode = fnw;
	ut->ut_fd[p[1]]->fe_rdwr = FWRITE;
	rval[0] = p[0];
	rval[1] = p[1];
	return 0;
}

Bsd_access(ut, rval, pathname, mode)
	struct ux_task *ut;
	int rval[2];
	char *pathname;
	int mode;
{
	struct fnode *fn;
	int error;

	error = bsd_lookup(ut->ut_cdir, pathname, &fn, TRUE);
	if (error) {
		return error;
	}
	/* XXXXXXXXXXXX ! */
	FOP_DECR(fn);
	return 0;
}

bsd_stat(ut, rval, pathname, stp)
	struct ux_task *ut;
	int rval[2];
	char *pathname;
	struct stat *stp;
{
	int error;
	struct fnode *fn;

	error = fn_path(ut, pathname, TRUE, &fn);
	if (error) {
		return error;
	}
	return fnode_stat(ut, fn, stp);
}

bsd_lstat(ut, rval, pathname, stp)
	struct ux_task *ut;
	int rval[2];
	char *pathname;
	struct stat *stp;
{
	int error;
	struct fnode *fn;

	error = fn_path(ut, pathname, FALSE, &fn);
	if (error) {
		return error;
	}
	return fnode_stat(ut, fn, stp);
}

bsd_fstat(ut, rval, fd, stp)
	struct ux_task *ut;
	int rval[2];
	int fd;
	struct stat *stp;
{
	int error;
	struct fnode *fn;

	error = fn_fd(ut, fd, &fn);
	if (error) {
		return error;
	}
	return fnode_stat(ut, fn, stp);
}

Bsd_chmod(ut, rval, pathname, mode)
	struct ux_task *ut;
	int rval[2];
	char *pathname;
	int mode;
{
	int error;
	struct fnode *fn;

	error = fn_path(ut, pathname, TRUE, &fn);
	if (error) {
		return error;
	}
	return fnode_chmod(ut, fn, mode);
}

Bsd_fchmod(ut, rval, fd, mode)
	struct ux_task *ut;
	int rval[2];
	int fd;
	int mode;
{
	int error;
	struct fnode *fn;

	error = fn_fd(ut, fd, &fn);
	if (error) {
		return error;
	}
	return fnode_chmod(ut, fn, mode);
}

Bsd_truncate(ut, rval, pathname, size)
	struct ux_task *ut;
	int rval[2];
	char *pathname;
	int size;
{
	int error;
	struct fnode *fn;

	error = fn_path(ut, pathname, TRUE, &fn);
	if (error) {
		return error;
	}
	return fnode_truncate(ut, fn, size);
}

Bsd_ftruncate(ut, rval, fd, size)
	struct ux_task *ut;
	int rval[2];
	int fd;
	int size;
{
	int error;
	struct fnode *fn;

	error = fn_fd(ut, fd, &fn);
	if (error) {
		return error;
	}
	return fnode_truncate(ut, fn, size);
}

Bsd_chown(ut, rval, pathname, uid, gid)
	struct ux_task *ut;
	int rval[2];
	char *pathname;
	int uid, gid;
{
	int error;
	struct fnode *fn;

	error = fn_path(ut, pathname, TRUE, &fn);
	if (error) {
		return error;
	}
	return fnode_chown(ut, fn, uid, gid);
}

Bsd_fchown(ut, rval, fd, uid, gid)
	struct ux_task *ut;
	int rval[2];
	int fd;
	int uid, gid;
{
	int error;
	struct fnode *fn;

	error = fn_fd(ut, fd, &fn);
	if (error) {
		return error;
	}
	return fnode_chown(ut, fn, uid, gid);
}

Bsd_chxdir(ut, rval, pathname, xfnp)
	struct ux_task *ut;
	int rval[2];
	char *pathname;
	struct fnode **xfnp;
{
	int error;
	struct fnode *fn;

	error = fn_path(ut, pathname, TRUE, &fn);
	if (error) {
		return error;
	}
	error = fn_checktype(fn, S_IFDIR, ENOTDIR);
	if (error) {
		return error;
	}
	FOP_DECR(*xfnp);
	*xfnp = fn;
	return 0;
}

bsd_utimes(ut, rval, pathname, tvp)
	struct ux_task *ut;
	int rval[2];
	char *pathname;
	struct timeval *tvp;
{
	int error;
	struct fnode *fn;
	struct stat st;

	error = fn_path(ut, pathname, TRUE, &fn);
	if (error) {
		FOP_DECR(fn);
		return error;
	}
	st.st_atime = tvp[0].tv_sec;
	st.st_mtime = tvp[1].tv_sec;
	error = FOP_SETSTAT(fn, &st, FATTR_ATIME|FATTR_MTIME);
	FOP_DECR(fn);
	return error;
}

Bsd_chdir(ut, rval, pathname)
	struct ux_task *ut;
	int rval[2];
	char *pathname;
{
	return Bsd_chxdir(ut, rval, pathname, &ut->ut_cdir);
}

Bsd_chroot(ut, rval, pathname)
	struct ux_task *ut;
	int rval[2];
	char *pathname;
{
	return Bsd_chxdir(ut, rval, pathname, &ut->ut_rdir);
}

Bsd_umask(ut, rval, umask)
	struct ux_task *ut;
	int rval[2];
	int umask;
{
	rval[0] = ut->ut_umask;
	ut->ut_umask = umask;
	return 0;
}
