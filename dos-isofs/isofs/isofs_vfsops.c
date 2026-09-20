/* 
 * Mach Operating System
 * Copyright (c) 1993 Carnegie Mellon University
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
 *  Software Distribution Coordinator  or  Software.Distribution@CS.CMU.EDU
 *  School of Computer Science
 *  Carnegie Mellon University
 *  Pittsburgh PA 15213-3890
 * 
 * any improvements or extensions that they make and grant Carnegie Mellon
 * the rights to redistribute these changes.
 */
/*
 * HISTORY
 * $Log:	isofs_vfsops.c,v $
 * Revision 2.3  93/09/15  16:06:29  mrt
 * 	Flush for real on umounts, so that devvp does not
 * 	keep extraneous references that prevent remounting.
 * 	[93/08/26  16:57:57  af]
 * 
 * Revision 2.2  93/08/07  16:56:16  mrt
 * 	Minimized includes.
 * 	[93/07/04  21:58:56  af]
 * 
 * 	Took it from BSDSS, with heavy mods.
 * 	[93/07/03            af]
 * 
 */
/*
 * Copyright (c) 1982, 1986, 1989 Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *	@(#)isofs_vfsops.c
 */
#include <sys/param.h>
#include <sys/dir.h>
#include <sys/user.h>
#include <sys/buf.h>
#include <sys/inode.h>
#include <sys/systm.h>

#include <isofs/isofs.h>
#include <isofs/isofs_node.h>

struct vfsops isofs_vfsops = {
	isofs_mount,
	isofs_unmount,
	isofs_root,
	isofs_statfs,
	isofs_sync,
	isofs_fhtovp
};

int isofs_debug = 0;

/* forward internals */
private int
iso_mountfs(
	struct vnode **devvpp,
	char		*path,
	struct vfs	*vfsp,
	int		flags);


/*
 * Called by vfs_mountroot when isofs is going to be mounted as root.
 */
extern struct vnode *bdevvp();

public int
isofs_mountroot()
{
	register struct vfs *vfsp;
	extern struct vnode *rootvp;
	struct iso_mnt *isomp;
	register struct fs *fs;
	u_int size;
	int error;

trace(isofs_debug,("isofs_mountroot "));

	ZALLOC(vfs_vfs_zone, vfsp, struct vfs *);
	VFS_INIT(vfsp, &isofs_vfsops, (caddr_t)0);

	vfsp->vfs_flag = VFS_RDONLY;
	vfsp->vfs_exroot = 0;
	vfsp->vfs_vnodecovered = NULL;

	if (rootdev != NODEV)
		rootvp = bdevvp(rootdev);
	error = iso_mountfs(&rootvp, "/", vfsp, ISOFSMNT_ROOT);
	if (error) {
		ZFREE(vfs_vfs_zone, vfsp);
		return (error);
	}
	error = vfs_add((struct vnode *)0, vfsp, M_RDONLY);
	if (error) {
		(void)isofs_unmount(vfsp);
		ZFREE(vfs_vfs_zone, vfsp);
		return (error);
	}

	isomp = VFSTOISOFS(vfsp);
	bzero(isomp->im_fsmnt, sizeof(isomp->im_fsmnt));
	isomp->im_fsmnt[0] = '/';
#if bsd44
	bcopy((caddr_t)isomp->im_fsmnt, (caddr_t)vfsp->vfs_stat.f_mntonname,
	    MNAMELEN);
	(void) copystr(ROOTNAME, vfsp->vfs_stat.f_mntfromname, MNAMELEN - 1,
	    &size);
	bzero(vfsp->vfs_stat.f_mntfromname + size, MNAMELEN - size);
	(void) isofs_statfs(vfsp, &vfsp->vfs_stat);
#endif
	vfs_unlock(vfsp);
	/* no inittodr(), disk time is old by defn */
	return (0);
}

/*
 * Flag to allow forcible unmounting.
 */
int iso_doforce = 1;

/*
 * VFS Operations.
 *
 * mount system call
 */
public int
isofs_mount(
	struct vfs *vfsp,
	char *path,
	caddr_t data)
{
	struct vnode *devvp;
	struct ifs_args args;
	u_int size;
	int error;
	dev_t dev;
	struct iso_mnt *isomp;

	trace(isofs_debug,("isofs_mount on %s\n", path));
	/*
	 * Get arguments
	 */
	if (error = copyin(data, &args, sizeof args))
		return error;

	if ((error = getmdev(args.fspec, &dev)) != 0)
		return (error);
	/*
	 * make a special (device) vnode for the filesystem
	 */
	devvp = bdevvp(dev);

	/*
	 * Mount the filesystem.
	 */
	error = iso_mountfs(&devvp, path, vfsp, args.flags & ~ISOFSMNT_ROOT);
	if (error) {
		VN_RELE(devvp);
		return (error);
	}

	isomp = VFSTOISOFS(vfsp);
#if bsd44
	(void) copyinstr(path, isomp->im_fsmnt, sizeof(isomp->im_fsmnt)-1, &size);
	bzero(isomp->im_fsmnt + size, sizeof(isomp->im_fsmnt) - size);
	bcopy((caddr_t)isomp->im_fsmnt, (caddr_t)vfsp->vfs_stat.f_mntonname,
	    MNAMELEN);
	(void) copyinstr(args.fspec, vfsp->vfs_stat.f_mntfromname, MNAMELEN - 1, 
	    &size);
	bzero(vfsp->vfs_stat.f_mntfromname + size, MNAMELEN - size);
	(void) isofs_statfs(vfsp, &vfsp->vfs_stat);
#else
	strncpy(isomp->im_fsmnt, path, sizeof(isomp->im_fsmnt));
#endif
	return (0);
}

/*
 * Common code for mount and mountroot
 */
private int
iso_mountfs(
	struct vnode **devvpp,
	char		*path,
	struct vfs	*vfsp,
	int		flags)
{
	register struct vnode *devvp = *devvpp;
	register struct iso_mnt *isomp = (struct iso_mnt *)0;
	struct buf *bp = NULL;
	dev_t dev = devvp->v_rdev;
	caddr_t base, space;
	int havepart = 0, blks;
	int error = EINVAL, i, size;
	int needclose = 0;
	int ronly = (vfsp->vfs_flag & VFS_RDONLY) != 0;
	extern struct vnode *rootvp;
	int j;
	int iso_bsize;
	int iso_blknum;
	struct iso_volume_descriptor *vdp;
	struct iso_primary_descriptor *pri;
	struct iso_directory_record *rootp;
	int logical_block_size;

	if (!ronly && ((flags & ISOFSMNT_ROOT) == 0))
		return (EROFS);

	/*
	 * Disallow multiple mounts of the same device.
	 * Disallow mounting of a device that is currently in use
	 * (except for root, which might share swap device for miniroot).
	 * Flush out any old buffers remaining from a previous use.
	 */
	tbflush(devvp, 0);
	if (devvp->v_vfsmountedhere && devvp->v_vfsmountedhere != vfsp)
		return (EBUSY);
	if (devvp->v_count > 1 && devvp != rootvp)
		return (EBUSY);
	binval(devvp);
	if (error = VOP_OPEN(devvpp, FREAD, u.u_cred))
		return (error);
	needclose = 1;

	/* This is the "logical sector size".  The standard says this
	 * should be 2048 or the physical sector size on the device,
	 * whichever is greater.  For now, we'll just use a constant.
	 */
	iso_bsize = 2048;

	for (iso_blknum = 16; iso_blknum < 100; iso_blknum++) {

		bp = bread (devvp, iso_blknum * iso_bsize / DEV_BSIZE,
				   iso_bsize);
		error = bp->b_error;
		if (error)
			goto out;

		vdp = (struct iso_volume_descriptor *)bp->b_un.b_addr;
		if (bcmp (vdp->id, ISO_STANDARD_ID, sizeof vdp->id) != 0) {
			error = EINVAL;
			goto out;
		}

		if (isonum_711 (vdp->type) == ISO_VD_END) {
			error = EINVAL;
			goto out;
		}

		if (isonum_711 (vdp->type) == ISO_VD_PRIMARY)
			break;
		brelse(bp);
	}

	if (isonum_711 (vdp->type) != ISO_VD_PRIMARY) {
		error = EINVAL;
		goto out;
	}
	trace(isofs_debug,("at block %d: %s %d\n", iso_blknum, vdp->id, vdp->type));
	
	pri = (struct iso_primary_descriptor *)vdp;

	logical_block_size = isonum_723 (pri->logical_block_size);

	if (logical_block_size < DEV_BSIZE
	    || logical_block_size >= MAXBSIZE
	    || (logical_block_size & (logical_block_size - 1)) != 0) {
		error = EINVAL;
		goto out;
	}

	trace(isofs_debug,("%s %s %d %d\n",
		pri->system_id, pri->volume_id,
		isonum_733(pri->volume_space_size),
		isonum_723(pri->logical_block_size)));

	rootp = (struct iso_directory_record *)pri->root_directory_record;

	isomp = (struct iso_mnt *)malloc(sizeof *isomp);
	bzero(isomp, sizeof(*isomp));

	isomp->dirextent = iso_blknum;
	isomp->diroff = ((char *)rootp) - ((char*)bp->b_un.b_addr);

	isomp->logical_block_size = logical_block_size;
	isomp->volume_space_size = isonum_733 (pri->volume_space_size);
	bcopy (rootp, isomp->root, sizeof isomp->root);
	isomp->root_extent = isonum_733 (rootp->extent);
	isomp->root_size = isonum_733 (rootp->size);

	isomp->im_bsize = logical_block_size;
	isomp->im_bmask = ~(isomp->im_bsize - 1);
	isomp->im_bshift = 0;
	while ((1 << isomp->im_bshift) < isomp->im_bsize)
		isomp->im_bshift++;

	bp->b_flags |= B_INVAL;
	brelse(bp);
	bp = NULL;

	isomp->im_flags = flags;

	vfsp->vfs_bsize = logical_block_size;
	vfsp->vfs_data = (caddr_t)isomp;
	vfsp->vfs_fsid.val[0] = (long)dev;
	vfsp->vfs_fsid.val[1] = MOUNT_ISOFS;
	isomp->im_mountp = vfsp;
	isomp->im_dev = dev;
	isomp->im_devvp = devvp;

	return (0);
out:
	if (bp)
		brelse(bp);
	if (needclose)
		(void)VOP_CLOSE(devvp, FREAD, u.u_cred);
	if (isomp) {
		free((caddr_t)isomp);
		vfsp->vfs_data = (caddr_t)0;
	}
	return (error);
}

/*
 * unmount system call
 */
public int
isofs_unmount(
	struct vfs	*vfsp)
{
	register struct iso_mnt *isomp;
	int i, error = 0;

trace(isofs_debug,("isofs_unmount "));

	isomp = VFSTOISOFS(vfsp);
	i = iso_iflush(isomp->im_dev);
	if (i < 0 && !iso_doforce)
		return (EBUSY);

	if (i == 0) {
		error = VOP_CLOSE(isomp->im_devvp,
			(vfsp->vfs_flag & VFS_RDONLY) ? FREAD : FREAD|FWRITE,
			u.u_cred);
		binval(isomp->im_devvp);
		binvalfree(isomp->im_devvp);
		VN_RELE(isomp->im_devvp);
		free((caddr_t)isomp);
	}
	return (error);
}

/*
 * Return root of a filesystem
 */
public int
isofs_root(
	struct vfs *vfsp,
	struct vnode **vpp)
{
	struct iso_node *nip, tvp;
	int error;
	struct iso_mnt *isomp = VFSTOISOFS (vfsp);

trace(isofs_debug,("isofs_root "));
	tvp.i_vnode.ih_fs = vfsp;
	tvp.i_mnt = isomp;
	tvp.o_dev = isomp->im_dev;
	tvp.iso_dirextent = isomp->dirextent;
	tvp.iso_diroff = isomp->diroff;
	error = iso_iget(&tvp, isomp->root_extent, &nip,
			 (struct iso_directory_record *) isomp->root,
			 isomp->diroff);
	if (error)
		return (error);
	ISO_IUNLOCK(nip);
	*vpp = ISO_TOV(nip);
	return (0);
}

/*
 * Get file system statistics.
 */
public int
isofs_statfs(
	struct vfs	*vfsp,
	register struct statfs *sbp)
{
	register struct iso_mnt *isomp;
	register struct fs *fs;

trace(isofs_debug,("isofs_statfs "));
	isomp = VFSTOISOFS(vfsp);

	sbp->f_type = MOUNT_ISOFS;
	sbp->f_bsize = isomp->logical_block_size;
	sbp->f_blocks = isomp->volume_space_size;
	sbp->f_bfree = 0; /* total free blocks */
	sbp->f_bavail = 0; /* blocks free for non superuser */
	sbp->f_files =  0; /* total files */
	sbp->f_ffree = 0; /* free file nodes */

	/* XXX This should be a real fsid, but this will do for now */
	bcopy((caddr_t)&vfsp->vfs_fsid,
	      (caddr_t)&sbp->f_fsid, sizeof (fsid_t));

	return (0);
}

public int
isofs_sync(
	struct vfs *vfsp)
{
trace(isofs_debug,("isofs_sync "));
	return (0);
}

