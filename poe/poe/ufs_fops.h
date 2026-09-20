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
 * $Log:	ufs_fops.h,v $
 * Revision 2.5  94/03/27  17:53:23  mrt
 * 	Added some comments.
 * 	[94/03/27            mrt]
 * 
 * Revision 2.4  91/12/19  20:30:01  mrt
 * 	Updated to new copyright
 * 
 * Revision 2.3  90/10/01  14:06:11  rwd
 * 	Added precalculated ud_nindir.
 * 	[90/09/30            rwd]
 * 
 * Revision 2.2  90/09/08  00:21:50  rwd
 * 	First checkin
 * 	[90/08/31  14:00:32  rwd]
 * 
 */
/*
 *	File:	./ufs_fops.h
 *	Author:	Joseph S. Barrera III, Randall W. Dean
 *
 *	Copyright (c) 1990 Joseph S. Barrera III, Randall W. Dean
 */

#include <sys/types.h>
#include <ufs_param.h>
#include <ufs_fs.h>
#include <ufs_disk.h>
#include <map_info.h>

/*
 *  XXX General solution requires windowing (yuck)
 *  XXX or, perhaps a separate task per device
 */
/* 	
 *	A udev structure is created by ufs_mountfn for each block
 *	device that is mounted. They are linked together starting
 *	at ud_list
 */		
struct udev {
	struct fnfs	ud_fnfs;	/* common fs info */
	struct fnode *	ud_special;	/* special device where fs lives */
	dev_t		ud_dev;		/* unix dev type thingywah */
	struct fs	ud_fs;		/* file system assoc w/ device */
	long 		ud_refcount;	/* number of fnodes using device */
	struct map_info	ud_map_info;
	vm_size_t	ud_size;	/* (XXX how do we find this?) */
	int		ud_nindir[NIADDR+1];
	struct udev *	ud_next;	/* list of udevs */
};

struct uhack {
	vm_offset_t	uh_offset;
	vm_offset_t	uh_data;
	struct uhack *	uh_next;
};

/*
 *	The unode structure is the fnode for a ufs file.
 *	The icommon structure is defined in ufs_disk.h and
 *	must match the structure that is stored on disk
 */
struct unode {
	struct fnode	un_fn;
	struct udev *	un_ud;		/* XXX redundant, with fnfs */
	struct uhash *	un_uh;		/* (dirty) ubuf hash table */
	int		un_pager_refs;	/* reference count for un pager */
	ino_t		un_ino;
	struct uhack *	un_hack;	/* XXX should use ic format */
	struct icommon	i_ic;		/* copy of on-disk inode */
	struct unode *	un_next;	/* list of in-core unodes */
};
