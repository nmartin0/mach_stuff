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
 * $Log:	subr_fops.c,v $
 * Revision 2.5  94/03/25  18:23:14  mrt
 * 	Added comments.
 * 	[94/03/09            mrt]
 * 
 * Revision 2.4  91/12/19  20:29:11  mrt
 * 	Updated to new copyright
 * 
 * Revision 2.3  90/09/27  13:55:38  rwd
 * 	Call map_file_init.
 * 	[90/09/11            rwd]
 * 
 * Revision 2.2  90/09/08  00:20:19  rwd
 * 	First checkin
 * 	[90/08/31  13:55:49  rwd]
 * 
 */
/*
 *	File:	./subr_fops.c
 *	Author:	Joseph S. Barrera III, Randall W. Dean
 *
 *	Copyright (c) 1990 Joseph S. Barrera III, Randall W. Dean
 */
/*
 *	
 */
#include <mach.h>
#include <fnode.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <subr_fops.h>

#define	SPEC_BLKSIZE	8192	/* ??? */

struct fops enxio_fops = {
	enxio_op,
	spec_close,
	enxio_op,
	enxio_op,
	spec_getstat,
	spec_setstat,
	enxio_op,
	nil_ioctl,
	nil_getpager,
	nil_lookup,
	nil_create,
	nil_link,
	nil_unlink,
};
	
struct fnfs enxio_fs = {
	0,		/* fs_mountpoint */
	0,		/* fs_root */
	&enxio_fops,	/* fs_fops */
	FALSE,		/* fs_mayseek */
	FALSE,		/* fs_maymap */
};

/* spec_make_special allocates and initialized and snode
   and returns it as fnp. fmt and dev aren't used. fs is
   the fnfs structure to be placed in the snode/fnode
 */
spec_make_special(fnp, fmt, dev, fs)
	struct fnode **fnp;
	int fmt;
	dev_t dev;
	struct fnfs *fs;
{
	struct snode *sn;

	sn			= (struct snode *) malloc(sizeof(*sn));
	sn->sn_fn.fn_fs		= fs;
	sn->sn_fn.fn_mounted	= 0;
	map_file_init(&sn->sn_fn.fn_map_info);
	sn->sn_fn.fn_refcount	= 1;
	sn->sn_entry		= *fnp;
	*fnp			= &sn->sn_fn;
	return 0;
}

enxio_special(fnp, fmt, dev)
	struct fnode *fnp;
	int fmt;
	dev_t dev;
{
	return spec_make_special(fnp, fmt, dev, &enxio_fs);
}

dev_getstat(fn, stp, mask, bsize)
	struct fnode *fn;
	struct stat *stp;
	unsigned long mask;
	int bsize;
{
	int error;

	error = FOP_GETSTAT(fn, stp, mask);
	if (error) {
		return error;
	}
	if (mask & FATTR_BLKSIZE) {
		stp->st_blksize = bsize;
	}
	return 0;
}

dev_setstat(fn, stp, mask)
	struct fnode *fn;
	struct stat *stp;
	unsigned long mask;
{
	if (mask & FATTR_BLKSIZE) {
		return EINVAL;
	}
	return FOP_SETSTAT(fn, stp, mask);
}

spec_close(sn)
	struct snode *sn;
{
	FOP_DECR(sn->sn_entry);
	free(sn);
	return 0;
}

spec_getstat(sn, stp, mask)
	struct snode *sn;
	struct stat *stp;
	unsigned long mask;
{
	return dev_getstat(sn->sn_entry, stp, mask, SPEC_BLKSIZE);
}

spec_setstat(sn, stp, mask)
	struct snode *sn;
	struct stat *stp;
	unsigned long mask;
{
	return dev_setstat(sn->sn_entry, stp, mask);
}

nil_lookup(fn, elt, fnp)
	struct fnode *fn;
	char *elt;
	struct fnode **fnp;
{
	return ENOTDIR;
}

nil_create(fnd, stp, fnp)
	struct fnode *fnd;
	struct stat *stp;
	struct fnode *fnp;
{
	return ENOTDIR;
}

nil_link(fnd, name, fn)
	struct fnode *fnd;
	char *name;
	struct fnode *fn;
{
	return ENOTDIR;
}

nil_unlink(fnd, name)
	struct fnode *fnd;
	char *name;
{
	return ENOTDIR;
}

nil_ioctl(fn, rw, type, command, param, psize)
	struct fnode *fn;
	int rw;
	int type;
	int command;
	char *param;
	int psize;
{
	return ENOTTY;
}

nil_getpager(fn)
	struct fnode *fn;
{
	return EINVAL;
}

null_open(fn)
	struct fnode *fn;
{
	return 0;
}

null_read(fn, offset, buffer, size, resid)
	struct fnode *fn;
	vm_offset_t offset;
	vm_offset_t buffer;
	vm_size_t size;
	vm_size_t *resid;
{
	*resid = size;
	return 0;
}

null_write(fn, offset, buffer, size, resid)
	struct fnode *fn;
	vm_offset_t offset;
	char *buffer;
	vm_size_t size;
	vm_size_t *resid;
{
	*resid = 0;
	return 0;
}

null_select(fn)
	struct fnode *fn;
{
	return 0;
}

enxio_op()
{
	return ENXIO;
}

st_copy(fstp, tstp, mask)
	struct stat *fstp;
	struct stat *tstp;
	unsigned long mask;
{
	if (mask & FATTR_DEV)		tstp->st_dev	= fstp->st_dev;
	if (mask & FATTR_INO)		tstp->st_ino	= fstp->st_ino;
	if (mask & FATTR_MODE)		tstp->st_mode	= fstp->st_mode;
	if (mask & FATTR_NLINK)		tstp->st_nlink	= fstp->st_nlink;
	if (mask & FATTR_UID)		tstp->st_uid	= fstp->st_uid;
	if (mask & FATTR_GID)		tstp->st_gid	= fstp->st_gid;
	if (mask & FATTR_RDEV)		tstp->st_rdev	= fstp->st_rdev;
	if (mask & FATTR_SIZE)		tstp->st_size	= fstp->st_size;
	if (mask & FATTR_ATIME)		tstp->st_atime	= fstp->st_atime;
	if (mask & FATTR_MTIME)		tstp->st_mtime	= fstp->st_mtime;
	if (mask & FATTR_CTIME)		tstp->st_ctime	= fstp->st_ctime;
	if (mask & FATTR_BLKSIZE) 	tstp->st_blksize= fstp->st_blksize;
	if (mask & FATTR_BLOCKS) 	tstp->st_blocks = fstp->st_blocks;
}
