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
 * $Log:	fnode.h,v $
 * Revision 2.4  94/03/27  17:53:20  mrt
 * 	Added some comments.
 * 	[94/03/27            mrt]
 * 
 * Revision 2.3  91/12/19  20:28:06  mrt
 * 	Updated to new copyright
 * 
 * Revision 2.2  90/09/08  00:18:58  rwd
 * 	Convert to a mapped window.
 * 	[90/08/28            rwd]
 * 
 */
/*
 *	File:	./fnode.h
 *	Author:	Joseph S. Barrera III, Randall W. Dean
 *
 *	Copyright (c) 1990 Joseph S. Barrera III, Randall W. Dean
 */


#include <map_info.h>

/* 
 *	an fops structure is kept in the fnode structure
 *	It defines the procedures that are used to
 *	perform the generic operation requested for
 *	the type of object the fnode represents
 */
struct fops {
	int		(*fop_open)();
	int		(*fop_close)();
	int		(*fop_read)();
	int		(*fop_write)();
	int		(*fop_getstat)();
	int		(*fop_setstat)();
	int		(*fop_select)();
	int		(*fop_ioctl)();
	int		(*fop_getpager)();
	int		(*fop_lookup)();
	int		(*fop_create)();
	int		(*fop_link)();
	int		(*fop_unlink)();
};

/*
 *	information about the filesystem where
 *	the object is located
 */
struct fnfs {
	struct fnode *	fs_mountpoint;	/* fnode where this filesystem is mounted */
					/* 0 if this fnfs is for a device */
	struct fnode *	fs_root;	/* root fnode of this filesystem for files */
					/* ptr to udev if this fnfs is for a device */
	struct fops *	fs_fops;	/* XXX make non-ptr to save deref */
	boolean_t	fs_mayseek;
	boolean_t	fs_maymap;
};
/*
 *	an fnode structure represents any kind of object that
 *	we perform operations such as read, write, getpager ... on.
 *	Can be a file, a special device e.g. tty, disk partition
 *	This structure is only the common part of an object node
 *	stucture. The snode and unode structures provide additional
 *	information.
 */
struct fnode {
	struct fnfs *	fn_fs;
	struct fnfs *	fn_mounted;	/* union with something? */
	struct map_info	fn_map_info;	
	int		fn_refcount;
};

#define	fn_fops		fn_fs->fs_fops
#define	fn_mayseek	fn_fs->fs_mayseek
#define	fn_maymap	fn_fs->fs_maymap

#define	FEXCEPT		0x0004	/* for FOP_SELECT */

#define	FATTR_DEV	0x0001
#define FATTR_INO	0x0002
#define FATTR_MODE	0x0004
#define FATTR_NLINK	0x0008
#define FATTR_UID	0x0010
#define FATTR_GID	0x0020
#define FATTR_RDEV	0x0040
#define FATTR_SIZE	0x0080
#define FATTR_ATIME	0x0100
#define FATTR_MTIME	0x0200
#define FATTR_CTIME	0x0400
#define FATTR_BLKSIZE	0x0800
#define FATTR_BLOCKS	0x1000

#define	FATTR_ALL	0x1fff

#define	FOP_OPEN(fn)						\
	(*fn->fn_fops->fop_open)(fn)

#define	FOP_READ(fn, offset, buffer, size, residp)		\
	(*fn->fn_fops->fop_read)(fn, offset, buffer, size, residp)

#define	FOP_WRITE(fn, offset, buffer, size, residp)		\
	(*fn->fn_fops->fop_write)(fn, offset, buffer, size, residp)

#define	FOP_LOOKUP(fn, pathnamep, fnp)				\
	(*fn->fn_fops->fop_lookup)(fn, pathnamep, fnp)

#define	FOP_GETSTAT(fn, stp, mask)				\
	(*fn->fn_fops->fop_getstat)(fn, stp, mask)

#define	FOP_SETSTAT(fn, stp, mask)				\
	(*fn->fn_fops->fop_setstat)(fn, stp, mask)

#define	FOP_CREATE(fnd, fnp)					\
	(*fnd->fn_fops->fop_create)(fnd, fnp)

#define	FOP_LINK(fnd, name, fn)					\
	(*fnd->fn_fops->fop_link)(fnd, name, fn)

#define	FOP_UNLINK(fnd, name)					\
	(*fnd->fn_fops->fop_unlink)(fnd, name)

#define	FOP_IOCTL(fn, rw, type, command, param, psize)		\
	(*fn->fn_fops->fop_ioctl)(fn, rw, type, command, param, psize)

#define	FOP_SELECT(fn, mode, pass)				\
	(*fn->fn_fops->fop_select)(fn, mode, pass)

#define	FOP_GETPAGER(fn)					\
	(*fn->fn_fops->fop_getpager)(fn)

#define	FOP_INCR(fn)						\
	((void)((fn)->fn_refcount++))

#define	FOP_DECR(fn)						\
	((void)((--(fn)->fn_refcount == 0)			\
		? (fn)->fn_fops->fop_close(fn)			\
		: 0)						\
	 )
