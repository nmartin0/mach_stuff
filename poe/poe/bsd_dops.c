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
 * $Log:	bsd_dops.c,v $
 * Revision 2.4  91/12/19  20:26:57  mrt
 * 	Updated to new copyright
 * 
 * Revision 2.3  90/09/27  13:53:41  rwd
 * 	Add disfunctional Bsd_rename.
 * 	[90/09/21            rwd]
 * 
 * Revision 2.2  90/09/08  00:06:41  rwd
 * 	First checkin
 * 	[90/08/31  13:27:21  rwd]
 * 
 */
/*
 *	File:	./bsd_dops.c
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

Bsd_link(ut, rval, name1, name2)
	struct ux_task *ut;
	int rval[2];
	char *name1;	/* existing file */
	char *name2;	/* link to be created */
{
	int error;
	struct fnode *fn, *fnd;
	char *dir, *base;

	error = fn_path(ut, name1, TRUE, &fn);
	if (error) {
		FOP_DECR(fn);
		return error;
	}
	path_split(name2, &dir, &base);
	error = fn_path(ut, dir, TRUE, &fnd);
	if (error) {
		FOP_DECR(fn);
		return error;
	}
	error = FOP_LINK(fnd, base, fn);
	FOP_DECR(fn);
	FOP_DECR(fnd);
	return error;
}

Bsd_unlink(ut, rval, pathname)
	struct ux_task *ut;
	int rval[2];
	char *pathname;
{
	int error;
	char *dir, *base;
	struct fnode *fnd = 0, *fn = 0;
	struct stat st;

	path_split(pathname, &dir, &base);
	error = fn_path(ut, dir, TRUE, &fnd);
	if (error) {
		goto bad;
	}
	error = fn_checktype(fnd, S_IFDIR, ENOTDIR);
	if (error) {
		goto bad;
	}
	error = FOP_LOOKUP(fnd, base, &fn);
	if (error) {
		goto bad;
	}
	error = FOP_GETSTAT(fn, &st, FATTR_MODE);
	if (error) {
		goto bad;
	}
	if ((st.st_mode & S_IFMT) == S_IFDIR) {
		goto bad;
	}
	error = FOP_UNLINK(fnd, base);
bad:
	if (fnd) {
		FOP_DECR(fnd);
	}
	if (fn) {
		FOP_DECR(fn);
	}
	return error;
}

Bsd_symlink(name1, name2)
	char *name1;
	char *name2;
{
	return EROFS;
}

/*
 * From man page:
 *
 * "Rename causes the link named from to be renamed as to.  If
 * to exists, then it is first removed.  Both from and to must
 * be of the same type (that is, both directories or both non-
 * directories), and must reside on the same file system.
 *
 * "Rename guarantees that an instance of to will always exist,
 * even if the system should crash in the middle of the operation.
 *
 * "If the final component of from is a symbolic link, the symbolic
 * link is renamed, not the file or directory to which it points."
 *
 * Note that this is again a good reason for dlook to return a pointer
 * to the thing it found. I assume we just shove a new ino down
 * its throat.
 */
Bsd_rename(ut, rval, from, to)
	struct ux_task *ut;
	int rval[2];
	char *from;
	char *to;
{
    int error;

    (void) Bsd_unlink(ut, rval, to);
    error = Bsd_link(ut, rval, from, to);
    if (error)
	return error;
    error = Bsd_unlink(ut, rval, from);
    return error;
}

Bsd_mkdir(pathname, mode)
	char *pathname;
	int mode;
{
	return EROFS;
}

Bsd_rmdir(pathname)
	char *pathname;
{
	return EROFS;
}
