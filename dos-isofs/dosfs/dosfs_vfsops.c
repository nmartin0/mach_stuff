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
 * $Log:	dosfs_vfsops.c,v $
 * Revision 2.2  93/09/15  13:30:16  mrt
 * 	Debugged mounting root, works fine on Alpha.
 * 	Say we want root to be unmounted on shutdown.
 * 	Added various mount options.
 * 	[93/09/13  23:49:37  af]
 * 
 * 	Indicate that our filesystem should be unmounted on shutdown.
 * 	Redid mount/unmount code to make sure the block device we
 * 	insist upon has proper reference counting. Which also means
 * 	properly flushing buffers on umount.
 * 	Keep fat in anonimous buffer, to make fsck of the block
 * 	device work.
 * 	Redid Fat-sync code.  Now we write only the primary on synch,
 * 	and all the alternates only on umount. 
 * 	[93/08/26  16:30:04  af]
 * 
 * 	Now we write.
 * 	[93/07/30  00:07:34  af]
 * 
 * 	Created.
 * 	[93/07/12            af]
 * 
 */

#include <sys/param.h>
#include <sys/dir.h>
#include <sys/user.h>
#include <sys/buf.h>
#include <sys/inode.h>
#include <sys/systm.h>

#include <dosfs/dosfs.h>
#include <dosfs/dosfs_node.h>

struct vfsops dosfs_vfsops = {
	dosfs_mount,
	dosfs_unmount,
	dosfs_root,
	dosfs_statfs,
	dosfs_sync,
	dosfs_fhtovp
};

int dosfs_debug = 0;

/* forward internals */
private int
dosfs_mountfs(
	struct vnode **devvpp,
	char		*path,
	struct vfs	*vfsp,
	struct dosfs_args *args);


/*
 * Called by vfs_mountroot to check if the
 * root filesystem can be mounted by us.
 */
extern struct vnode *bdevvp();

public int
dosfs_mountroot()
{
	register struct vfs *vfsp;
	extern struct vnode *rootvp;
	struct dosfs_mount *dosfsmp;
	register struct fs *fs;
	int error;
	struct dosfs_args args;

trace(dosfs_debug,("\ndosfs_mountroot "));

	ZALLOC(vfs_vfs_zone, vfsp, struct vfs *);
	VFS_INIT(vfsp, &dosfs_vfsops, (caddr_t)0);

	vfsp->vfs_flag = 0;
	vfsp->vfs_exroot = 0;
	vfsp->vfs_vnodecovered = NULL;

	if (rootdev != NODEV)
		rootvp = bdevvp(rootdev);
	args.flags = 0;
	error = dosfs_mountfs(&rootvp, "/", vfsp, &args);
	if (error) {
		ZFREE(vfs_vfs_zone, vfsp);
		return (error);
	}
	error = vfs_add((struct vnode *)0, vfsp, 0);
	if (error) {
		(void)dosfs_unmount(vfsp);
		ZFREE(vfs_vfs_zone, vfsp);
		return (error);
	}
	/* vfs_add clobbers this */
	vfsp->vfs_devvp = rootvp;

	dosfsmp = VFSTODOSFS(vfsp);

	vfs_unlock(vfsp);
	/* no inittodr(), there is no disk time */
	return (0);
}

/*
 * Flag to allow forcible unmounting.
 */
int dosfs_doforce = 1;

/*
 * VFS Operations.
 *
 * mount system call
 */
public int
dosfs_mount(
	struct vfs *vfsp,
	char *path,
	caddr_t data)
{
	struct vnode *devvp;
	struct dosfs_args args;
	int error;
	dev_t dev;

	trace(dosfs_debug,("\ndosfs_mount on %s\n", path));
	/*
	 * Get arguments
	 */
	if (error = copyin(data, &args, sizeof args)) {
		/* compat */
		bzero(&args,sizeof(args));
		if (error = copyin(data, &args, sizeof(struct ifs_args)))
			return error;
	}

	if ((error = getmdev(args.fspec, &dev)) != 0)
		return (error);
	/*
	 * make a special (device) vnode for the filesystem
	 */
	devvp = bdevvp(dev);
trace(dosfs_debug,("devvp: %x %d\n", devvp, devvp->v_count));

	/*
	 * Mount the filesystem.
	 */
	error = dosfs_mountfs(&devvp, path, vfsp, &args);
	if (error) {
		VN_RELE(devvp);
	}
	return (error);
}

/*
 * Common code for mount and mountroot
 */
private int
dosfs_mountfs(
	struct vnode **devvpp,
	char		*path,
	struct vfs	*vfsp,
	struct dosfs_args *args)
{
	register struct vnode *devvp = *devvpp;
	register struct dosfs_mount *dosfsmp = (struct dosfs_mount *)0;
	struct buf *bp = NULL;
	dev_t dev = devvp->v_rdev;
	int error = EINVAL, i;
	int needclose = 0;
	int ronly = (vfsp->vfs_flag & VFS_RDONLY) != 0;
	extern struct vnode *rootvp;
	struct dosfs_bootsector *vdp;
	struct dosfs_directory_record *rootp;
	int logical_block_size;

	/*
	 * Disallow multiple mounts of the same device.
	 * Disallow mounting of a device that is currently in use
	 * (except for root, which might share swap device for miniroot).
	 * Flush out any old buffers remaining from a previous use.
	 */
	if (devvp->v_vfsmountedhere && devvp->v_vfsmountedhere != vfsp)
		return (EBUSY);
	if (devvp->v_count > 1 && devvp != rootvp)
		return (EBUSY);
	binval(devvp);
	if (error = VOP_OPEN(devvpp, (ronly) ? FREAD : FREAD|FWRITE, u.u_cred))
		return (error);
	needclose = 1;

	bp = bread (devvp, MDOS_LABELSECTOR, MDOS_SECTOR_SIZE);
	error = bp->b_error;
	if (error)
		goto out;

	vdp = (struct dosfs_bootsector *)bp->b_un.b_addr;

	if (dosfsnum_16(vdp->label.magic) != BIOS_LABEL_MAGIC) {
		error = EINVAL;
		goto out;
	}

	dosfsmp = (struct dosfs_mount *)malloc(sizeof *dosfsmp);
	bzero(dosfsmp, sizeof(*dosfsmp));

	dosfsmp->im_clsiz = vdp->clsiz;
	logical_block_size = dosfsnum_16(vdp->secsiz) * vdp->clsiz;

	dosfsmp->im_bsize = logical_block_size;
	dosfsmp->im_bmask = ~(dosfsmp->im_bsize - 1);
	dosfsmp->im_bshift = 0;
	while ((1 << dosfsmp->im_bshift) < dosfsmp->im_bsize)
		dosfsmp->im_bshift++;

	dosfsmp->fat_start = dosfsnum_16(vdp->nrsvsect);
	dosfsmp->fat_len = dosfsnum_16(vdp->fatlen);
	dosfsmp->n_fat = vdp->nfat;
	i = dosfsnum_16(vdp->psect);
	if (i == 0)
		i = dosfsnum_32(vdp->bigsect);
	dosfsmp->im_size = i / vdp->clsiz;
	dosfsmp->fat_bits =  (i > 4087) ? 16 : 12;
	mutex_init(&dosfsmp->fat_lock);

	dosfsmp->rootdir_entries = dosfsnum_16(vdp->dirents);
	dosfsmp->rootdir_start = dosfsmp->fat_start +
				 (dosfsmp->n_fat * dosfsmp->fat_len);

	i = dosfsmp->rootdir_entries * MDOS_DIR_SIZE;
	i = (i + MDOS_SECTOR_SIZE - 1) / (unsigned) MDOS_SECTOR_SIZE;
	dosfsmp->clusters_start = dosfsmp->rootdir_start + i;

	if (ronly) {
		bp->b_flags |= B_INVAL;
		brelse(bp);
	} else {
		bcopy(path, vdp->volume_label, sizeof vdp->volume_label);
		bforce(bp);
	}
	bp = NULL;

	/* Read all of the FAT in */
	i = dosfsmp->fat_len * MDOS_SECTOR_SIZE;
	bp = bread (devvp, (dosfsmp->fat_start * MDOS_SECTOR_SIZE) / DEV_BSIZE, i);

	dosfsmp->fat_bp = geteblk(i);
	dosfsmp->fat = (u_short *)dosfsmp->fat_bp->b_un.b_addr;
	bcopy(bp->b_un.b_addr, dosfsmp->fat, i);

	brelse(bp);
	bp = NULL;

	dosfs_stat_fat(dosfsmp);

	/* Look at mount options */
	if ((args->flags & DOSFSMNT_TRANS) != DOSFSMNT_TRANS)
		args->flags |= DOSFSMNT_MUSER;
	dosfsmp->im_flags = args->flags;

	if (args->flags & DOSFSMNT_CHUNK)
		dosfsmp->chunk_size = args->chunk_size;
	else
		dosfsmp->chunk_size = 16*1024;

	if (args->flags & DOSFSMNT_PROT) {
		dosfsmp->im_uid = args->uid;
		dosfsmp->im_gid = args->gid;
		dosfsmp->im_mode = args->mode;
	} else {
		dosfsmp->im_uid = u.u_uid;
		dosfsmp->im_gid = u.u_gid;
		dosfsmp->im_mode = (VREAD|VEXEC) | ((VREAD|VEXEC)>>3) | ((VREAD|VEXEC)>>6);
	}

	vfsp->vfs_bsize = logical_block_size;
	vfsp->vfs_data = (caddr_t)dosfsmp;
	vfsp->vfs_fsid.val[0] = dev;
	vfsp->vfs_fsid.val[1] = MOUNT_DOSFS;
	vfsp->vfs_devvp = devvp;
	vfsp->vfs_flag |= VFS_DOUNMOUNT;
	dosfsmp->im_dev = dev;
	dosfsmp->im_devvp = devvp;
	bcopy(path,dosfsmp->im_path,(sizeof dosfsmp->im_path)-1);

	return (0);
out:
	if (bp)
		brelse(bp);
	if (needclose)
		(void)VOP_CLOSE(devvp, FREAD, u.u_cred);
	if (dosfsmp) {
		free((caddr_t)dosfsmp);
		vfsp->vfs_data = (caddr_t)0;
	}
	return (error);
}

/*
 * unmount system call
 */
public int
dosfs_unmount(
	struct vfs	*vfsp)
{
	register struct dosfs_mount *dosfsmp;
	struct vnode *devvp;
	int i, error = 0;

trace(dosfs_debug,("\ndosfs_unmount "));

	dosfsmp = VFSTODOSFS(vfsp);

	dosfs_sync_fat(dosfsmp, 1);

	i = dosfs_iflush(dosfsmp->im_dev);
	if (i < 0 && !dosfs_doforce)
		return (EBUSY);

	if (i == 0) {
		struct vnode *devvp = dosfsmp->im_devvp;

		brelse(dosfsmp->fat_bp);
		error = VOP_CLOSE(devvp,
			(vfsp->vfs_flag & VFS_RDONLY) ? FREAD : FREAD|FWRITE,
			u.u_cred);
		tbflush(devvp, 0);
		binval(devvp);
		binvalfree(devvp);
		VN_RELE(devvp);
		free((caddr_t)dosfsmp);
	}
	return (error);
}

/*
 * Return root of a filesystem
 * Minor hassle: it would be natural to
 * use '0' as the inum for the root, since
 * that is the way DOS has it.  But Unix
 * for some reason does not like it.
 */
public int
dosfs_root(
	struct vfs *vfsp,
	struct vnode **vpp)
{
	struct dosfs_node *nip, tvp;
	struct dosfs_directory_record proot;
	int error;
	struct dosfs_mount *dosfsmp = VFSTODOSFS (vfsp);

trace(dosfs_debug,("\ndosfs_root "));

	tvp.i_vnode.ih_fs = vfsp;
	tvp.i_mnt = dosfsmp;
	tvp.o_dev = dosfsmp->im_dev;
	tvp.extent = 0;

	bzero(&proot, sizeof(proot));
	proot.attr = DOS_ATTR_DIR;

	error = dosfs_iget(&tvp, DOS_ROOTINO, &nip, &proot, 0);
	if (error)
		return (error);

	nip->i_size = dosfsmp->rootdir_entries * sizeof(proot);

	DOS_IUNLOCK(nip);
	*vpp = DOS_TOV(nip);

	return (0);
}

/*
 * Get file system statistics.
 */
public int
dosfs_statfs(
	struct vfs	*vfsp,
	register struct statfs *sbp)
{
	register struct dosfs_mount *dosfsmp;
	register struct fs *fs;

trace(dosfs_debug,("\ndosfs_statfs "));
	dosfsmp = VFSTODOSFS(vfsp);

	sbp->f_type = MOUNT_DOSFS;
	sbp->f_bsize = dosfsmp->im_bsize;
	sbp->f_blocks = dosfsmp->im_size;
	sbp->f_bfree = dosfsmp->im_free; /* total free blocks */
	sbp->f_bavail = sbp->f_bfree; /* blocks free for non superuser */
	sbp->f_files = dosfsmp->im_free + dosfsmp->im_alloc; /* total files */
	sbp->f_ffree = dosfsmp->im_free; /* free file nodes */

	/* XXX This should be a real fsid, but this will do for now */
	bcopy((caddr_t)&vfsp->vfs_fsid,
	      (caddr_t)&sbp->f_fsid, sizeof (fsid_t));

	return (0);
}

/*
 * Save superblock info on disk
 */
public int
dosfs_sync(
	struct vfs *vfsp)
{
	return dosfs_sync_fat(VFSTODOSFS(vfsp), 0);
}

public int
dosfs_sync_fat(
	register struct dosfs_mount *dosfsmp,
	int	save_alternates)
{
	register struct buf *bp = NULL;
	int i, len;
	daddr_t bno;

trace(dosfs_debug,("dosfs_sync %d ", dosfsmp->im_dirty));

	/*
	 * Save primary FAT
	 */
	len = dosfsmp->fat_len * MDOS_SECTOR_SIZE;
	bno = (dosfsmp->fat_start * MDOS_SECTOR_SIZE) / DEV_BSIZE;
	if (dosfsmp->im_dirty > 1) {

		bp = getblk( dosfsmp->im_devvp, bno, len);
		bcopy(dosfsmp->fat, bp->b_un.b_addr, len);

		dosfsmp->im_dirty = 1;
		bwrite(bp);
	}

	/*
	 * Now for the other FAT copies (on umount)
	 */
	if (save_alternates && (dosfsmp->im_dirty > 0)) {
		dosfsmp->im_dirty = 0;

again:
		bp = getblk( dosfsmp->im_devvp, bno, len);
		if (bp->b_flags & B_DELWRI) {
			bforce(bp);
			goto again;
		}
		bcopy(dosfsmp->fat, bp->b_un.b_addr, len);

		for (i = 1; i < dosfsmp->n_fat; i++) {
			daddr_t	lbsave = bp->b_blkno;

			bp->b_flags &= ~(B_ASYNC|B_READ|B_DONE|B_ERROR);
			bp->b_flags |= (B_DUPLICATE);
			bp->b_resid = 0;
			bp->b_blkno = lbsave +
				i * ((dosfsmp->fat_len * MDOS_SECTOR_SIZE) / DEV_BSIZE);
			VOP_STRATEGY(bp);
			biowait(bp);

			bp->b_blkno = lbsave;
		}
		bp->b_flags |= B_USELESS | B_NOCACHE;
		brelse(bp);
	}
	return (0);
}

