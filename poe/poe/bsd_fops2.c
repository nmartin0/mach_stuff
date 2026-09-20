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
 * $Log:	bsd_fops2.c,v $
 * Revision 2.3  91/12/19  20:27:12  mrt
 * 	Updated to new copyright
 * 
 * Revision 2.2  90/09/08  00:07:02  rwd
 * 	First checkin
 * 	[90/08/31  13:29:41  rwd]
 * 
 */
/*
 *	File:	./bsd_fops2.c
 *	Author:	Joseph S. Barrera III, Randall W. Dean
 *
 *	Copyright (c) 1990 Joseph S. Barrera III, Randall W. Dean
 */

#include <mach.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fnode.h>
#include <ux_user.h>

fnode_stat(ut, fn, stp)
	struct ux_task *ut;
	struct fnode *fn;
	struct stat *stp;
{
	int error;

	error = FOP_GETSTAT(fn, stp, FATTR_ALL);
	FOP_DECR(fn);
	if (error) {
		return error;
	}
	return 0;
}

fnode_chmod(ut, fn, mode)
	struct ux_task *ut;
	struct fnode *fn;
	int mode;
{
	struct stat st;
	int error;

	st.st_mode = mode;
	error = FOP_SETSTAT(fn, &st, FATTR_MODE);
	FOP_DECR(fn);
	return error;
}

fnode_chown(ut, fn, uid, gid)
	struct ux_task *ut;
	struct fnode *fn;
	int uid;
	int gid;
{
	struct stat st;
	int error, mask;

	mask = 0;
	if (uid != -1) {
		mask |= FATTR_UID;
		st.st_uid = uid;
	}
	if (gid != -1) {
		mask |= FATTR_GID;
		st.st_gid = gid;
	}
	error = FOP_SETSTAT(fn, &st, mask);
	FOP_DECR(fn);
	return error;
}

fnode_truncate(ut, fn, size)
	struct ux_task *ut;
	struct fnode *fn;
	int size;
{
	struct stat st;
	int error;

	error = FOP_GETSTAT(fn, &st, FATTR_SIZE);
	if (error || st.st_size <= size) {
		FOP_DECR(fn);
		return error;
	}
	st.st_size = size;
	error = FOP_SETSTAT(fn, &st, FATTR_SIZE);
	FOP_DECR(fn);
	return error;
}
