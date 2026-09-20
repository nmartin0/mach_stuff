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
 * $Log:	ufs_fops.c,v $
 * Revision 2.4  91/12/19  20:29:59  mrt
 * 	Updated to new copyright
 * 
 * Revision 2.3  90/09/27  13:55:57  rwd
 * 	Call map_file_init.  Use map_file_lock/unlock.
 * 	[90/09/11            rwd]
 * 
 * Revision 2.2  90/09/08  00:21:45  rwd
 * 	Convert fileio to mapped window.
 * 	[90/08/28            rwd]
 * 
 */
/*
 *	File:	./ufs_fops.c
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

extern int null_open();
extern int ufs_close();
extern int ufs_read();
extern int ufs_write();
extern int ufs_getstat();
extern int ufs_setstat();
extern int ufs_select();
extern int ufs_ioctl();
extern int ufs_getpager();
extern int ufs_create();
extern int ufs_lookup();
extern int ufs_link();
extern int ufs_unlink();

struct fops ufs_fops = {
	null_open,
	ufs_close,
	ufs_read,
	ufs_write,
	ufs_getstat,
	ufs_setstat,
	ufs_select,
	ufs_ioctl,
	ufs_getpager,
	ufs_lookup,
	ufs_create,
	ufs_link,
	ufs_unlink,
};

struct unode *
unode_alloc()
{
	struct unode *un;

	un = (struct unode *) malloc(sizeof(*un));

	un->un_fn.fn_fs			= 0; /* filled in elsewhere */
	un->un_fn.fn_mounted		= 0;
	map_file_init(&un->un_fn.fn_map_info);
	un->un_fn.fn_refcount		= 1;
	un->un_hack			= 0;

	return un;
}

ufs_lookup(un, elt, unp)
	struct unode *un;
	char *elt;
	struct unode **unp;
{
	int error, fmt;
	ino_t inumber;

	/* XXX check for dir type should be here, not dlook? */
	error = dlook(elt, un, &inumber);
	if (error) {
		return error;
	}
	error = openi(un->un_ud, inumber, unp);
	if (error) {
		return error;
	}
	fmt = ((*unp)->i_mode & S_IFMT);
	if (fmt != S_IFREG && fmt != S_IFDIR && fmt != S_IFLNK) {
		return dev_special(unp, fmt, (*unp)->i_rdev);
	}
	return 0;
}

ufs_getstat(un, stp, mask)
	struct unode *un;
	struct stat *stp;
	unsigned long mask;
{
	struct stat st;

	if (mask & FATTR_DEV)	stp->st_dev = un->un_ud->ud_dev;
	if (mask & FATTR_INO)	stp->st_ino = un->un_ino;
	if (mask & FATTR_MODE)	stp->st_mode = un->i_mode;
	if (mask & FATTR_NLINK)	stp->st_nlink = un->i_nlink;
	if (mask & FATTR_UID)	stp->st_uid = un->i_uid;
	if (mask & FATTR_GID)	stp->st_gid = un->i_gid;
	if (mask & FATTR_RDEV)	stp->st_rdev = un->i_rdev;
	if (mask & FATTR_SIZE)	stp->st_size = un->i_size;
	if (mask & FATTR_ATIME)	stp->st_atime = un->i_atime;
	if (mask & FATTR_MTIME)	stp->st_mtime = un->i_mtime;
	if (mask & FATTR_CTIME)	stp->st_ctime = un->i_ctime;
	if (mask & FATTR_BLKSIZE) stp->st_blksize = un->un_ud->ud_fs.fs_bsize;
	if (mask & FATTR_BLOCKS) stp->st_blocks = un->i_blocks;
	return 0;
}

ufs_setstat(un, stp, mask)
	struct unode *un;
	struct stat *stp;
	unsigned long mask;
{
	if (mask == FATTR_SIZE) {
		/* XXX protection checks! */
		/* XXX (block size, too) */
		un->i_size = stp->st_size;
		return 0;
	}
	if (mask & FATTR_MODE) {
		return EROFS;
	} else {
		return EPERM;
	}
}

ufs_ZZZ_read(un, ut, ubuf, offset, resid)
	struct unode *un;
	struct ux_task *ut;
	char *ubuf;
	vm_offset_t offset;
	int *resid;
{
	int error;
	char *buf;
	int len;

	error = bsd_map(un);
	if (error) {
		return error;
	}
	len = (long)un->i_size - (long)offset;
	if (len > *resid) len = *resid;
	if (len > 0) {
		if (ut) {
			int addr;
			map_file_lock(&un->un_fn.fn_map_info);
			map_file_remap(&un->un_fn.fn_map_info, offset, len);
			addr = un->un_fn.fn_map_info.mi_address -
				un->un_fn.fn_map_info.mi_offset +
				offset;
			error = copyout(ut, addr, ubuf, len);
			map_file_unlock(&un->un_fn.fn_map_info);
			if (error) {
				printf("error %d in copyout, off=%d\n",
				       error, offset);
			}
		} else {
			map_file_read(&un->un_fn.fn_map_info, ubuf,offset,len);
		}
		*resid -= len;
	}
	return 0;
}

ufs_read(un, offset, buffer, size, resid)
	struct unode *un;
	vm_offset_t offset;
	vm_offset_t buffer;
	long size;
	vm_size_t *resid;
{
	*resid = size;
	return ufs_ZZZ_read(un, 0, buffer, offset, resid);
}

/*
 * We should be clever about not zero-filling pages that we are just going
 * to overwrite.... XXX not sure how to do this in emulation space...
 */
ufs_write(un, offset, buffer, size, resid)
	struct unode *un;
	vm_offset_t offset;
	vm_offset_t buffer;
	vm_size_t size;
	vm_size_t *resid;
{
	int error;

	error = bsd_map(un);
	if (error) {
		return error;
	}
	map_file_write(&un->un_fn.fn_map_info, buffer, offset, size);
	un->i_size += size;
	*resid = 0;
	return 0;
}

ufs_close(un)
	struct unode *un;
{
}

ufs_ioctl(un, rw, type, command, param, psize)
	struct unode *un;
	int rw;
	int type;
	int command;
	char *param;
	int psize;
{
	if (type != 'f') {
		if (command == icmd(TIOCGPGRP)) {
			return 0; /* gm0w getdirentries garbage */
		}
		return ENOTTY;
	}
	return EINVAL;
}

ufs_select()
{
}
