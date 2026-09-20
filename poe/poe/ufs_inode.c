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
 * $Log:	ufs_inode.c,v $
 * Revision 2.5  92/02/02  13:03:04  rpd
 * 	Removed old IPC vestiges.
 * 	[92/01/31            rpd]
 * 
 * Revision 2.4  91/12/19  20:30:06  mrt
 * 	Updated to new copyright
 * 
 * Revision 2.3  90/10/01  14:06:25  rwd
 * 	Convert to new inode code from XUX26/from BSD4.3-Reno.
 * 	[90/09/30            rwd]
 * 
 * Revision 2.2  90/09/08  00:21:59  rwd
 * 	Fix indirect code.
 * 	[90/09/04            rwd]
 * 	First checkin
 * 	[90/08/31  14:02:39  rwd]
 * 
 */
/*
 *	File:	./ufs_inode.c
 *	Author:	Joseph S. Barrera III, Randall W. Dean
 *
 *	Copyright (c) 1990 Joseph S. Barrera III, Randall W. Dean
 */

#include <mach.h>
#include <fnode.h>
#include <errno.h>
#include <ux_user.h>
#include <ufs_fops.h>
#include <device/device_types.h>
#include <sys/types.h>
#include <sys/stat.h>

extern mach_port_t device_server_port;
extern struct unode *unode_alloc();

struct unode *un_head = 0;

struct unode *
un_by_memory_object(memory_object)
	mach_port_t memory_object;
{
	return (struct unode *)memory_object;
}

/*
 * Open an unode.
 */
int
openi(ud, ino, unp)
	struct udev *ud;
	ino_t ino;
	struct unode **unp;
{
	vm_offset_t buf;
	int error;
	register struct dinode *dp;
	int index, size;
	struct unode *un;

	/*
	 * First, see if there's an appropriate active unode already.
	 */
	for (un = un_head; un; un = un->un_next) {
		if (ud == un->un_ud && ino == un->un_ino) {
			FOP_INCR(&un->un_fn);
			*unp = un;
			return 0;
		}
	}

	/*
	 * Read block containing inode from disk,
	 * and pull the inode out of it.
	 */
	index = itoo(&ud->ud_fs, ino);
	size = (index+1)*sizeof(struct dinode);
	buf = (vm_offset_t)malloc(size);
	error = ud_read(ud, itod(&ud->ud_fs, ino), buf, size);
	if (error) {
		printf("bn.1 %d: read error\n", itod(&ud->ud_fs, ino));
		free(buf);
		return error;
	}
	dp = (struct dinode *)buf;
	un = unode_alloc();
	un->i_ic = dp[index].di_ic;
	free(buf);
	un->un_fn.fn_fs = &ud->ud_fnfs; /* XXX refcount */
	un->un_ud = ud; /* xxx refcount + redundant */
	un->un_ino = ino;
	un->un_next = un_head;
	un_head = un;
	FOP_INCR(&un->un_fn);
	*unp = un;
	return 0;
}

un_fakealloc(ud, unp)
	struct udev *ud;
	struct unode **unp;
{
	int error;
	struct unode *un;
	static int fakei = 1000000;

	un = unode_alloc();
	un->i_size = 0;
	un->un_fn.fn_fs = &ud->ud_fnfs;
	un->un_ud = ud; /* XXX but hopefully won't be used; and redundant */
	un->un_ino = fakei++;
	un->un_next = un_head;
	un->un_fn.fn_maymap = 1;
	error = bsd_map(un);
	if (error) return error;
	un_head = un;
	FOP_INCR(&un->un_fn);
	*unp = un;
	return 0;
}

sbmap(un, file_block, disk_block_p)
register struct unode	*un;
register daddr_t	file_block;
daddr_t			*disk_block_p;	/* out */
{
	int		level;
	int		idx;
	daddr_t		ind_block_num;
	kern_return_t	rc;

	/*
	 * Index structure of an inode:
	 *
	 * i_db[0..NDADDR-1]	hold block numbers for blocks
	 *			0..NDADDR-1
	 *
	 * i_ib[0]		index block 0 is the single indirect
	 *			block
	 *			holds block numbers for blocks
	 *			NDADDR .. NDADDR + NINDIR(fs)-1
	 *
	 * i_ib[1]		index block 1 is the double indirect
	 *			block
	 *			holds block numbers for INDEX blocks
	 *			for blocks
	 *			NDADDR + NINDIR(fs) ..
	 *			NDADDR + NINDIR(fs) + NINDIR(fs)**2 - 1
	 *
	 * i_ib[2]		index block 2 is the triple indirect
	 *			block
	 *			holds block numbers for double-indirect
	 *			blocks for blocks
	 *			NDADDR + NINDIR(fs) + NINDIR(fs)**2 ..
	 *			NDADDR + NINDIR(fs) + NINDIR(fs)**2
	 *				+ NINDIR(fs)**3 - 1
	 */

	if (file_block < NDADDR) {
	    /* Direct block. */
	    *disk_block_p = un->i_db[file_block];
	    return (0);
	}

	file_block -= NDADDR;

	/*
	 * nindir[0] = NINDIR
	 * nindir[1] = NINDIR**2
	 * nindir[2] = NINDIR**3
	 *	etc
	 */

	for (level = 0; level < NIADDR; level++) {
	    if (file_block < un->un_ud->ud_nindir[level])
		break;
	    file_block -= un->un_ud->ud_nindir[level];
	}
	if (level == NIADDR) {
	    /* Block number too high */
	    return (EIO);
	}

	ind_block_num = un->i_ib[level];

	for (; level >= 0; level--) {
	    daddr_t	*bap;
	    kern_return_t error;

	    if (ind_block_num == 0) {
		*disk_block_p = 0;	/* missing */
		return (0);
	    }

	    if (level > 0) {
		idx = file_block / un->un_ud->ud_nindir[level-1];
		file_block %= un->un_ud->ud_nindir[level-1];
	    }
	    else
		idx = file_block;

	    bap = (daddr_t *)malloc((idx+1)*(sizeof(ind_block_num)));
	    error = ud_read(un->un_ud, ind_block_num, bap,
			    (idx+1)*(sizeof(ind_block_num)));
	    if (error) {
		printf("bn.2 %d: read error\n", ind_block_num);
		free(bap);
		return error;
	    }

	    ind_block_num = bap[idx];
	    free(bap);
	}

	*disk_block_p = ind_block_num;

	return (0);
}

int
read_fs(fn, fs)
	struct fnode *fn;
	struct fs *fs;
{
	int	error;
	vm_offset_t	buf;
	vm_size_t	buf_size;

	error = bdev_device_read(fn, 0, (recnum_t) SBLOCK, SBSIZE,
				 (char **) &buf, &buf_size);
	if (error) {
		return error;
	}

	if (SBSIZE != buf_size) {
		printf("read_fs: SBSIZE=%d, buf_size=%d\n", SBSIZE, buf_size);
		return EINVAL;
	}

	bcopy(buf, fs, sizeof(struct fs));
	(void) vm_deallocate(mach_task_self(), buf, buf_size);

	if (fs->fs_magic != FS_MAGIC ||
	    fs->fs_bsize > MAXBSIZE ||
	    fs->fs_bsize < sizeof(struct fs)) {
		return EINVAL;
	}
	/* don't read cylinder groups - we aren't modifying anything */
	return 0;
}
