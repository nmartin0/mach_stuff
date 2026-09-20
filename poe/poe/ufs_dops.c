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
 * $Log:	ufs_dops.c,v $
 * Revision 2.3  91/12/19  20:29:54  mrt
 * 	Updated to new copyright
 * 
 * Revision 2.2  90/09/08  00:21:36  rwd
 * 	First checkin
 * 	[90/08/31  13:59:58  rwd]
 * 
 */
/*
 *	File:	./ufs_dops.c
 *	Author:	Joseph S. Barrera III, Randall W. Dean
 *
 *	Copyright (c) 1990 Joseph S. Barrera III, Randall W. Dean
 */

#include <mach.h>
#include <fnode.h>
#include <errno.h>
#include <ux_user.h>
#include <ufs_fops.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <bsd_ioctl.h>

static int ufs_bogus_ino = 1000000;

/*
 *  I guess mkdir should indeed be separate.
 */
ufs_create(und, unp)
	struct unode *und;
	struct unode **unp;
{
	struct ux_task *ut;
	struct unode *un;
	int error;

	ut = (struct ux_task *) ut_self();
	error = un_fakealloc(und->un_ud, &un);
	if (error) {
		return error;
	}
	un->un_ino = ufs_bogus_ino++;
	un->i_mode = S_IFREG | (0666 &~ ut->ut_umask);
	un->i_nlink = 0;
	un->i_uid = und->i_uid; /* I know it's more complicated that this */
	un->i_gid = und->i_gid; /* " " XXX */
	un->i_rdev = 0; /* ??? */
	un->i_size = 0;
	un->i_atime = time(0);
	un->i_mtime = un->i_atime;
	un->i_ctime = un->i_atime;
	un->i_blocks = 0;

	*unp = un;
	return 0;
}

ufs_link(und, name, un)
	struct unode *und;
	char *name;
	struct unode *un;
{
	return denter(name, und, un->un_ino);
}

ufs_unlink(und, name)
	struct unode *und;
	char *name;
{
	/* check for type? or is that done above, in bsd_unlink? */
	return dremove(name, und);
}
