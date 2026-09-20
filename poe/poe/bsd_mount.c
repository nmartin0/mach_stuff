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
 * $Log:	bsd_mount.c,v $
 * Revision 2.3  91/12/19  20:27:32  mrt
 * 	Updated to new copyright
 * 
 * Revision 2.2  90/09/08  00:15:22  rwd
 * 	First checkin
 * 	[90/08/31  13:33:01  rwd]
 * 
 */
/*
 *	File:	./bsd_mount.c
 *	Author:	Joseph S. Barrera III, Randall W. Dean
 *
 *	Copyright (c) 1990 Joseph S. Barrera III, Randall W. Dean
 */

#include <mach.h>
#include <fnode.h>
#include <ux_param.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>

extern struct fnode *root_fn;

bsd_mount(fnroot, dirname, readonly)
	struct fnode *fnroot;
	char *dirname;
	boolean_t readonly;	/* XXXX use this! */
{
	int error;
	struct fnode *fnmount;

	error = bsd_lookup(root_fn, dirname, &fnmount, FALSE);
	if (error) {
		/* FOP_UNMOUNT */
		return error;
	}
	error = fn_checktype(fnmount, S_IFDIR, ENOTDIR);
	if (error) {
		/* FOP_UNMOUNT */
		FOP_DECR(fnmount);
		return error;
	}
	if (fnroot != fnroot->fn_fs->fs_root) {
		/* FOP_UNMOUNT */
		printf("bsd_mount: fnroot not root\n");
		return EINVAL;
	}
	if (fnroot->fn_fs->fs_mountpoint) {
		printf("bsd_mount: fnroot already mounted\n");
		return EBUSY;
	}
	if (fnmount->fn_mounted) {
		printf("bsd_mount: fnmount already mounted\n");
		return EBUSY;
	}
	fnroot->fn_fs->fs_mountpoint = fnmount;
	fnmount->fn_mounted = fnroot->fn_fs;
	return 0;
}

Bsd_ufs_mount(specname, dirname, readonly)
	char *specname;
	char *dirname;
	boolean_t readonly;
{
	int error;
	struct fnode *fnroot;

	error = ufs_mount(specname, &fnroot, readonly);
	if (error) {
		return error;
	}
	return bsd_mount(fnroot, dirname, readonly);
}

Bsd_ufs_umount(specname)
	char *specname;
{
	int error;
	struct fnode *fnroot;
	struct fnode *fnmount;

	printf("+++ umount(%s)\n", specname);
	error = ufs_rootbyspec(specname, &fnroot);
	if (error) {
		return error;
	}
	fnmount = fnroot->fn_fs->fs_mountpoint;
	if (fnmount == 0) {
		return EINVAL;
	}
	fnroot->fn_fs->fs_mountpoint = 0;
	fnmount->fn_mounted = 0;
	error = ufs_unmount(fnroot);
	if (error) {
		fnroot->fn_fs->fs_mountpoint = fnmount;
		fnmount->fn_mounted = fnroot->fn_fs;
		return error;
	}
	return 0;
}
