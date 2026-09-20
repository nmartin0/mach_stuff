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
 * $Log:	ufs_mount.c,v $
 * Revision 2.6  94/04/14  13:30:50  mrt
 * 		Changed ufs_mountroot to take root device and well as root
 * 		name as input. No longer needs to call OLD_xxX to makeup
 * 		root device.
 * 	[94/03/27            mrt]
 * 
 * Revision 2.5  92/02/02  13:03:18  rpd
 * 	Removed old IPC vestiges.
 * 
 * Revision 2.4  91/12/19  20:30:08  mrt
 * 	Updated to new copyright
 * 
 * Revision 2.3  90/10/01  14:06:30  rwd
 * 	Calculate indirect blocks at mount time.
 * 	[90/09/30            rwd]
 * 
 * Revision 2.2  90/09/08  00:22:03  rwd
 * 	First checkin
 * 	[90/08/31  14:02:50  rwd]
 * 
 */
/*
 *	File:	./ufs_mount.c
 *	Author:	Joseph S. Barrera III, Randall W. Dean
 *
 *	Copyright (c) 1990 Joseph S. Barrera III, Randall W. Dean
 */

#include <mach.h>
#include <fnode.h>
#include <errno.h>
#include <ux_user.h>
#include <ufs_fops.h>
#include <subr_fops.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <bsd_ioctl.h>
#include <device/device_types.h>


struct unode *	root_fn = 0;	/* root fnode */
dev_t root_dev;			/* root device major/minor */
char root_name[8];		/* root partition name */

extern mach_port_t device_server_port;


ud_read(ud, nb, bufp, size)
	struct udev	*ud;
	daddr_t		nb;
	char		*bufp;
	int		size;
{
	int error;
	vm_offset_t offset;

	offset = 512 * fsbtodb(&ud->ud_fs, nb); /* XXX */
	if (offset >= ud->ud_size) {
		return EIO;
	}
	return map_file_read(&ud->ud_map_info, bufp, offset, size);
}

/* We don't expect a large number of these */
struct udev *ud_list = 0;

struct udev *
ud_by_special(fns)
	struct fnode *fns;
{
	struct udev *ud;

	for (ud = ud_list; ud; ud = ud->ud_next) {
		if (ud->ud_special == fns) {
			return ud;
		}
	}
	return 0;
}

/*
 * XXX only in support for fake device pager
 */
struct udev *
ud_by_memory_object(memory_object)
	mach_port_t memory_object;
{
	struct udev *ud;

#if 0
	for (ud = ud_list; ud; ud = ud->ud_next) {
		if (ud->ud_pager == memory_object) {
			return ud;
		}
	}
	return 0;
#endif 0
	return (struct udev *)memory_object;
}

/*
 * XXX
 * Needs reference counting.
 * Multiple opens should return same device; last close should unmap, etc.
 * How do we determine size???
 */
static
ufs_mountfn(fn, dev, unp)
	struct fnode *fn;
	dev_t dev;
	struct unode **unp;
{
	struct udev *ud;
	int error;
	extern struct fops ufs_fops;
	struct unode *un;

	ud = ud_by_special(fn);
	if (ud) {
		*unp = (struct unode *)ud->ud_fnfs.fs_root;
		return 0;
	}
	ud = (struct udev *) malloc(sizeof(*ud)); /* XXX if already open? */
	ud->ud_dev = dev;
	ud->ud_special = fn;
	error = read_fs(fn, &ud->ud_fs); /* XXX should use map */
	if (error) {
		printf("error reading filesystem\n");
		return error;
	}

	/*
	 * Calculate indirect block levels.
	 */
	{
	    register int	mult;
	    register int	level;

	    mult = 1;
	    for (level = 0; level < NIADDR; level++) {
		mult *= NINDIR(&ud->ud_fs);
		ud->ud_nindir[level] = mult;
	    }
	}
	ud->ud_fnfs.fs_mountpoint = 0;
	ud->ud_fnfs.fs_fops = &ufs_fops;
	ud->ud_fnfs.fs_mayseek = TRUE;
	ud->ud_fnfs.fs_maymap = TRUE;

	ud->ud_refcount = 666; /* XXX ??? */
	ud->ud_size = ud->ud_fs.fs_size * 1024; /* XXX why 1024? */
	map_file_init(&ud->ud_map_info);
	error = ufs_devmap(ud);
	if (error) {
		return error;
	}

	/* XXX
	 * The only reason we insert this into list before vm_map
	 * is because fake device server needs to look it up.
	 */
	ud->ud_next = ud_list;
	ud_list = ud;

	error = openi(ud, ROOTINO, &un);
	if (error) {
		printf("can't open %o root inode!\n", dev);
		return error;
	}
	un->un_ud->ud_fnfs.fs_root = (struct fnode *) un;
	*unp = un;
	return 0;
}

int
ufs_mount(specname, unp, readonly)
	char *specname;
	struct unode **unp;
	boolean_t readonly; /* XXX */
{
	struct fnode *fn;
	int error;
	struct stat st;

	error = bsd_lookup(root_fn, specname, &fn, TRUE);
	if (error) {
		printf("can't open %s!\n", specname);
		return error;
	}
	error = FOP_GETSTAT(fn, &st, FATTR_RDEV | FATTR_MODE);
	if (error) {
		FOP_DECR(fn);
		return error;
	}
	if ((st.st_mode & S_IFMT) != S_IFBLK) {
		FOP_DECR(fn);
		return ENOTBLK;
	}
	error = ufs_mountfn(fn, st.st_rdev, unp);
	if (error) {
		printf("can't open %s device!\n", specname);
		return error;
	}
	return 0;
}

ufs_rootbyspec(specname, fnrootp)
	char *specname;
	struct fnode *fnrootp;
{
	return EINVAL;
}

ufs_unmount(fnroot)
	struct fnode *fnroot;
{
	return EBUSY;
}

int
ufs_mountroot(name,rootdev)
	char * name;
	dev_t rootdev;
{
	int error;
	struct snode *sn;
/* 
 * we use a phony dev for the root to keep fsck from recognizing
 * it as the root device and attempting to treat as as a block
 * rather than raw device, since block i/o to special devices is not
 * supported by poe.
 */
	dev_t dev = makedev(055,066);

	sn = (struct snode *) malloc(sizeof(*sn));

	/* opens the device, gets and stashes device port */
	bdev_make_special(sn, name, D_READ | D_WRITE);

	/* mallocs and inits a udev */
	error = ufs_mountfn((struct fnode *)sn, dev, &root_fn);
	
	if (error) {
		return error;
	}
	root_fn->un_fn.fn_fs->fs_mountpoint = &root_fn->un_fn;
	return 0;
}

/*
 * XXX move this to fs-independent part
 *  This code looks for the CMU style local root at
 *  /RFS/.LOCALROOT and if it exists set the root
 *  node to point to it rather than the "real" root.
 *  This is equivalent to the choot in the Unix server.
 *  if /RFS/.LOCALROOT does not exist, this does nothing.
 */
rfs_walk()
{
	int error;
	struct unode *rfs_root;

	error = ufs_lookup(root_fn, "RFS", &rfs_root);
	if (error) {
		return 0;
	}
	error = ufs_lookup(rfs_root, ".LOCALROOT", &rfs_root);
	if (error) {
		FOP_DECR(&rfs_root->un_fn);
		return 0;
	}
	root_fn = rfs_root; /* no need to decr old root_fn */
	return 0;
}
