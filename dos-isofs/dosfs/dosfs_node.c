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
 * $Log:	dosfs_node.c,v $
 * Revision 2.2  93/09/15  13:29:49  mrt
 * 	Added extensions to record on disk uid/gid/mode 
 * 	(and dev for special devices), from Rob.
 * 	Moved syncip elsewhere. Added dosfs_itdiren().
 * 	[93/09/13  23:58:24  af]
 * 
 * 	Added dosfs_zfree(), fixed termination race.
 * 	Redid iflush() code, now meaningful.
 * 	Only flush buffers for non-zero length inodes.
 * 	Iupdat should keep IUPD if it doesnt.
 * 	[93/08/26  16:26:09  af]
 * 
 * 	First version that can write to the filesystem.
 * 	[93/07/30  00:12:28  af]
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
#include <sys/kernel.h> /* get_time */

#include <dosfs/dosfs.h>
#include <dosfs/dosfs_node.h>

#define	INOHSZ	128
#if	((INOHSZ&(INOHSZ-1)) == 0)
#define	INOHASH(dev,ino)	(((dev)+(ino))&(INOHSZ-1))
#else
#define	INOHASH(dev,ino)	(((unsigned)((dev)+(ino)))%INOHSZ)
#endif

union dosfs_ihead {
	union  dosfs_ihead *ih_head[2];
	struct dosfs_node *ih_chain[2];
} dosfs_ihead[INOHSZ];

zone_t	dosfs_zone;
zone_t	dosfs_fid_zone;

/*
 * Initialize hash links for inodes.
 */
public void
dosfs_init()
{
	register int i;
	register union dosfs_ihead *ih = dosfs_ihead;

	for (i = INOHSZ; --i >= 0; ih++) {
		ih->ih_head[0] = ih;
		ih->ih_head[1] = ih;
	}
	dosfs_zone = zinit(sizeof(struct dosfs_node), 1024*1024,
				0, FALSE, "dosfs zone");
	dosfs_fid_zone = zinit(sizeof(struct ifid), 1024*1024,
				vm_page_size, FALSE, "dosfs fid");
}

/*
 * Look up a DOSFS dinode number to find its incore vnode.
 * If it is not in core, read it in from the specified device.
 * If it is in core, wait for the lock bit to clear, then
 * return the inode locked. Detection and handling of mount
 * points must be done by the calling routine.
 */
public int
dosfs_iget(
	struct dosfs_node	*xp,
	ino_t			ino,
	struct dosfs_node	**ipp,
	struct dosfs_directory_record *dosfsdir,
	int			diroff)
{
	dev_t dev;
	struct vfs *vfsp;
	extern struct vnodeops dosfs_vnodeops;
	register struct dosfs_node *ip;
	register struct vnode *vp;
	struct vnode *nvp;
	union dosfs_ihead *ih;
	int error, i;
	struct dosfs_mount *dosfsmp;

	vfsp = DOS_TOV(xp)->ih_fs;
	dev = xp->o_dev;

trace(dosfs_debug,("Diget x%x ", ino));

	ih = &dosfs_ihead[INOHASH(dev, ino)];
loop:
	for (ip = ih->ih_chain[0];
	     ip != (struct dosfs_node *)ih;
	     ip = (struct dosfs_node *)ip->i_forw) {
		if (ino != ip->o_number || dev != ip->o_dev)
			continue;
		if (ip->i_flag&ILOCKED) {
			ip->i_flag |= IWANT;
			sleep((caddr_t)ip, PINOD);
			goto loop;
		}
		DOS_ILOCK(ip);
		VN_HOLD(DOS_TOV(ip));
		*ipp = ip;
		return(0);
	}
	/*
	 * Allocate a new inode.
	 */
	ZALLOC(dosfs_zone, ip, struct dosfs_node *);
	bzero(ip, sizeof(*ip));
	nvp = DOS_TOV(ip);
	nvp->v_type = ITYPE_DOSFS;
	VN_HOLD(nvp);
	/*
	 * Put it onto its hash chain and lock it so that other requests for
	 * this inode will block if they arrive while we are sleeping waiting
	 * for old data structures to be purged or for the contents of the
	 * disk portion of this inode to be read.
	 */
	ip->o_dev = dev;
	ip->o_number = ino;
	DOS_ILOCK(ip);
	insque(ip, ih);

	ip->dosfs_diroff = diroff;
	ip->dosfs_dirextent = xp->extent;

	ip->extent = dosfsnum_16 (dosfsdir->start);
	ip->i_size = (off_t) dosfsnum_32 (dosfsdir->size);
	i = dosfs_date(dosfsdir->date, dosfsdir->time);
	ip->o_atime = ip->o_mtime = ip->o_ctime = i;
	ip->attr = dosfsdir->attr;

	ip->last_lbn = -1;
	ip->i_lastr = -1;

	/*
	 * Initialize the associated vnode
	 */
	vp = DOS_TOV(ip);
	VN_INIT(vp,vfsp,0,dev);

	dosfsmp = VFSTODOSFS (vfsp);

	if (ip->attr & DOS_ATTR_DIR) {
		ip->o_nlink = 2;
		ip->i_size = dosfs_file_len(dosfsmp, ip->extent);
		vp->v_mode = VDIR;
	} else {
		ip->o_nlink = 1;
		vp->v_mode = VREG;
	}

	if (ip->attr & DOS_ATTR_DOSFS_TYPE) {
		ip->o_uid = dosfsnum_16(dosfsdir->res_ext.mnt_ext.uid);
		ip->o_gid = dosfsnum_16(dosfsdir->res_ext.mnt_ext.gid);
		ip->o_mode = dosfsnum_16(dosfsdir->res_ext.mnt_ext.mode);
		vp->v_mode = ip->o_mode & VFMT;
		/*
		 * Correction to VN_INIT above
		 */
		if (ISVDEV(vp->v_mode))
			vp->v_rdev = dosfsnum_32(dosfsdir->res_ext.mnt_ext.name2);
	} else {
		ip->o_uid = dosfsmp->im_uid;
		ip->o_gid = dosfsmp->im_gid;
		ip->o_mode = vp->v_mode | dosfsmp->im_mode;

		if ((ip->attr & DOS_ATTR_RONLY) == 0)
			ip->o_mode |= VWRITE /* | (VWRITE>>3) | (VWRITE>>6) */;
	}


	ip->i_mnt = dosfsmp;
	VN_HOLD(dosfsmp->im_devvp);
	ip->o_devvp = dosfsmp->im_devvp;

	if (ino == DOS_ROOTINO)
		vp->v_flag |= VROOT;

	*ipp = ip;
	return (0);
}

/*
 * Allocate a brand new inode, directory or file,
 * which will go into directory XP.
 */
public int
dosfs_ialloc(
	struct dosfs_node	*xp,
	int			type,
	int			mode,
	struct dosfs_node	**ipp,
	off_t			diroff,
	struct vattr		*vap)
{
	dev_t dev;
	struct vfs *vfsp;
	extern struct vnodeops dosfs_vnodeops;
	register struct dosfs_node *ip;
	register struct vnode *vp;
	struct vnode *nvp;
	union dosfs_ihead *ih;
	int error, i;
	struct dosfs_mount *dosfsmp;

	vfsp = DOS_TOV(xp)->ih_fs;
	dev = xp->o_dev;

trace(dosfs_debug,("Dialloc %x %x %x ", xp->o_number, mode, diroff));

	ZALLOC(dosfs_zone, ip, struct dosfs_node *);
	bzero(ip, sizeof(*ip));
	nvp = DOS_TOV(ip);
	nvp->v_type = ITYPE_DOSFS;
	VN_HOLD(nvp);
	/*
	 * Need a cluster to get a unique inode number.
	 */
	ip->extent = 0;
	ip->i_size = 0;
	ip->o_dev = dev;
	ip->i_mnt = xp->i_mnt;
	ip->last_lbn = -1;
	ip->i_lastr = -1;
	ip->dosfs_diroff = diroff;
	ip->dosfs_dirextent = xp->extent;

	{
	    struct timeval t;
	    get_time(&t);
	    ip->o_atime = ip->o_mtime = ip->o_ctime = t.tv_sec;
	}

	/*
	 * Initialize the associated vnode
	 */
	vp = DOS_TOV(ip);
	VN_INIT(vp,vfsp,type,dev);
	/*
	 * Do we need to change this VN_INIT here?  Yep.
	 */
	if (ISVDEV(type))
		vp->v_rdev = vap->va_rdev;


	dosfsmp = VFSTODOSFS (vfsp);

	if (type == VDIR) {
		ip->attr = DOS_ATTR_DIR;
	} else {
		ip->attr = DOS_ATTR_ARCHIVE;
	}

	ip->i_mnt = dosfsmp;
	VN_HOLD(dosfsmp->im_devvp);
	ip->o_devvp = dosfsmp->im_devvp;

	/*
	 * We want to set this attr bit if we've been mounted multi user.
	 * Even IFREG files need the uid, etc.
	 */
	   
	if (xp->i_mnt->im_flags & DOSFSMNT_MUSER)
		ip->attr |= DOS_ATTR_DOSFS_TYPE;

	/*
	 * Now go get our inum
	 */
	error = dosfs_iextend(ip, 1, -1);
	ip->i_size = 0;
	ip->o_number = ip->extent;
trace(dosfs_debug,("[dosfs_ialloc-> %x] ", ip->o_number));

	DOS_ILOCK(ip);
	ih = &dosfs_ihead[INOHASH(dev, ip->o_number)];
	insque(ip, ih);

	*ipp = ip;
	return (0);
}


/*
 * Unlock and decrement the reference count of an inode structure.
 */
public int
dosfs_iput( register struct dosfs_node *ip)
{
trace(dosfs_debug,("Diput %x ", ip->o_number));

	if ((ip->i_flag & ILOCKED) == 0)
		panic("dosfs_iput");
	DOS_IUNLOCK(ip);
	VN_RELE(DOS_TOV(ip));
	return (0);
}

/*
 * Put an inode to rest
 * Inode must be locked.
 */
private int
dosfs_zfree(
	struct dosfs_node *ip)
{
trace(dosfs_debug,("Dzfree %x ", ip->o_number));

	/*
	 * Remove the inode from its hash chain.
	 */
	remque(ip);
	ip->i_forw = (struct vnode *)ip;
	ip->i_back = (struct vnode *)ip;

	/*
	 * Purge old data structures associated with the inode.
	 */
	dnlc_purge_vp(DOS_TOV(ip));
	if (ip->o_devvp) {
		VN_RELE(ip->o_devvp);
		ip->o_devvp = 0;
	}
	if (ip->o_nlink <= 0) {
		if (ip->i_size == 0) dosfs_igone(ip);
	}

	/*
	 * wakeup anyone that raced us to
	 * this inode in iget, and lost
	 */
	dosfs_iunlock(ip);

	/* put it back on freelist or zone */
	ZFREE(dosfs_zone, ip);

	return (0);
}


/*
 * Remove any cached inodes that belong to DEV
 * If any are left and we cannot rid of them
 * return their total count.
 */
public int
dosfs_iflush( dev_t dev)
{
	register struct dosfs_node *ip, *iq;
	register open = 0;
	union dosfs_ihead *ih;

	for (ih = dosfs_ihead; ih < &dosfs_ihead[INOHSZ]; ih++) {
	  iq = 0;
retry:
	  for(ip = ih->ih_chain[0];
	      ip != (struct dosfs_node *)ih;
	      ip = (struct dosfs_node *)ip->i_forw) {

		if (ip->o_dev == dev) {
			/*
			 * Inode in active use. We tried to flush
			 * it and failed. Give up.
			 */
			if (ip == iq)
				return (-1);
			/*
			 * See if the inode pager can rid of it
			 */
			if (inode_uncache_try(ip))
				/* program is active */
				return (-1);
			/*
			 * Inode pager either knows nothing about it,
			 * or was able to uncache it.  To avoid locking
			 * other inodes just make a note and rescan
			 * the chain; the inode will not be on any
			 * chain if we flushed it above.
			 */
			iq = ip;
			goto retry;
		}
		else if ((ip->o_mode&IFMT)==IFBLK &&
		    /* xxxx is this right xxxx */
		    ip->i_vnode.v_rdev == dev)
			open++;
	  }
	}
	return (open);
}

/*
 * Last reference to an inode, write the inode out and if necessary,
 * truncate and deallocate the file.
 */
public int
dosfs_inactive(
	struct vnode	*vp,
	struct ucred	*cred)
{
	register struct dosfs_node *ip = VTO_DOS(vp);
	int mode, error = 0;

trace(dosfs_debug,("Dinactive %x %x %x\n", ip->o_number, ip->o_nlink, ip->i_size));

	DOS_ILOCK(ip);
	dosfs_iupdat(ip, 0);

	return dosfs_zfree(ip);
}

/*
 * Access check
 */
public int
dosfs_iaccess(
	struct dosfs_node *ip,
	int		  mode)
{
	int m;
	gid_t *gp;

	if (ip->i_mnt->im_flags & DOSFSMNT_MUSER) {
		/*
		 * Super user always gets access....
		 */
		if (u.u_uid == 0)
			return (0);

		m = mode;
		if (u.u_uid != ip->o_uid) {
			m >>= 3;
			if (u.u_gid == ip->o_gid)
				goto found;
			gp = u.u_groups;
			for (; gp < &u.u_groups[NGROUPS] && *gp != NOGROUP;
			     gp++)
				if (ip->o_gid == *gp)
					goto found;
			m >>= 3;
		}
found:
		if ((ip->o_mode & m) == m)
			return (0);
		u.u_error = EACCES;
		return (EACCES);

	} else
	if ((mode & IWRITE) && (ip->attr & DOS_ATTR_RONLY))
		return EROFS;
	return 0;
}

/*
 * If node size has been modified, force it to disk.
 * No need to do this for directories.
 * If waitfor is given, then must insure
 * i/o order so wait for write to complete.
 */
public int
dosfs_iupdat(
	struct dosfs_node	*ip,
	int			waitfor)
{
	struct buf *bp;
	register struct dosfs_mount *imp;
	struct dosfs_node tvp;
	register int chk, extchk;
	register struct dosfs_directory_record *dirp;

trace(dosfs_debug,("Diupdat %x %x ", ip->o_number, ip->i_size));

	/* if igone() got to it */
	if (ip->extent == 0)
		return;

	extchk = ip->i_mnt->im_flags & DOSFSMNT_MUSER;
	chk = IUPD;
	if (extchk)
		chk |= IACC | ICHG;
	else
	if (ip->attr & DOS_ATTR_DIR)
		return;

	chk = ip->i_flag & chk;
	if (chk) {
		daddr_t bn;

		ip->i_flag &= ~chk;

		if (ip->i_vnode.ih_fs->vfs_flag & VFS_RDONLY ||
		    ip->o_number == DOS_ROOTINO)
			return;

		imp = ip->i_mnt;
		tvp.extent = ip->dosfs_dirextent;
		tvp.o_number = (tvp.extent == 0) ? DOS_ROOTINO : tvp.extent;
		tvp.i_mnt = imp;
		tvp.last_lbn = -1;
		bn = dosfs_bmap( &tvp, dosfs_lblkno(imp,ip->dosfs_diroff));
		/*
		 * This happens when a new file is added to a directory
		 * and the directory did not have nuf room. No need to
		 * bother here zeroing and allocating that block.
		 */
		if (bn == -1) {
			ip->i_flag |= chk;
			return;
		}
		bp = bread( ip->o_devvp, bn, imp->im_bsize);
		if (bp->b_flags & B_ERROR) {
			brelse(bp);
			return;
		}

		dirp = (struct dosfs_directory_record *)
			(bp->b_un.b_addr + dosfs_blkoff(imp, ip->dosfs_diroff));

		if (dosfs_itdiren(ip, dirp, extchk)) {
			if (waitfor == 2)
				baforce(bp);
			else if (waitfor == 1)
				bwrite(bp);
			else
				bdwrite(bp);
		} else
			brelse(bp);

	}

}

public int
dosfs_itdiren(
	struct dosfs_node	*ip,
	struct dosfs_directory_record *dirp,
	int			extensions)
{
	register int tmp, upd = 0;;

	tmp = dosfsnum_32(dirp->size);
	if (ip->attr & DOS_ATTR_DIR) {
		/* sanity for DOS */
		if (tmp != 0) {
			mkdosfsnum_32(dirp->size,0);
			upd++;
		}
	} else {
		if (tmp != ip->i_size) {
			upd++;
			tmp = ip->i_size;
			mkdosfsnum_32(dirp->size,tmp);
		}
	}

	ino_dtou(tmp,dirp);
	if (tmp != ip->o_number) {
		/* should only happen on file creation */
		upd++;
		ino_utod(ip->o_number,dirp);
	}

	if (!extensions)
		return upd;

	tmp = dosfsnum_16(dirp->res_ext.mnt_ext.mode);
	if (tmp != ip->o_mode) {
		upd++;
		tmp = ip->o_mode;
		mkdosfsnum_16(dirp->res_ext.mnt_ext.mode, tmp);
	}

	tmp = dosfsnum_16(dirp->res_ext.mnt_ext.gid);
	if (tmp != ip->o_gid) {
		upd++;
		tmp = ip->o_gid;
		mkdosfsnum_16(dirp->res_ext.mnt_ext.gid, tmp);
	}

	tmp = dosfsnum_16(dirp->res_ext.mnt_ext.uid);
	if (tmp != ip->o_uid) {
		upd++;
		tmp = ip->o_uid;
		mkdosfsnum_16(dirp->res_ext.mnt_ext.uid, tmp);
	}

	/*
	 * For now, put the device numbers in the extended name area
	 */
	if (ISVDEV(ip->o_mode & VFMT)) {
		tmp = dosfsnum_32(dirp->res_ext.mnt_ext.name2);
		if (tmp != ip->i_vnode.v_rdev) {
			upd++;
			tmp = ip->i_vnode.v_rdev;
			mkdosfsnum_32(dirp->res_ext.mnt_ext.name2, tmp);
		}
	}
	return upd;	
}

public void
dosfs_irelease(
	struct dosfs_node	*ip)
{
	if (ip->i_flag & ILOCKED)
		panic("dosfs_irelease");
	VN_RELE(DOS_TOV(ip));
}

/*
 * Lock an inode. If its already locked, set the WANT bit and sleep.
 */
dosfs_ilock(ip)
	register struct dosfs_node *ip;
{
trace(dosfs_debug,("L %x ", ip->o_number));

	while (ip->i_flag & ILOCKED) {
		ip->i_flag |= IWANT;
		(void) sleep((caddr_t)ip, PINOD);
	}
	ip->i_flag |= ILOCKED;
}

/*
 * Unlock an inode.  If WANT bit is on, wakeup.
 */
dosfs_iunlock(ip)
	register struct dosfs_node *ip;
{
trace(dosfs_debug,("U %x ", ip->o_number));

	if ((ip->i_flag & ILOCKED) == 0)
		printf("dosfs_iunlock: unlocked inode", DOS_TOV(ip));
	ip->i_flag &= ~ILOCKED;
	if (ip->i_flag&IWANT) {
		ip->i_flag &= ~IWANT;
		wakeup((caddr_t)ip);
	}
}
