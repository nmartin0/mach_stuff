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
 * $Log:	dosfs_vnops.c,v $
 * Revision 2.3  93/09/24  15:14:29  mrt
 * 	Fixed dosfs_pagein() for case of last page in file.
 * 	[93/09/24            af]
 * 
 * Revision 2.2  93/09/15  13:30:27  mrt
 * 	Added (disabled) code for large sequential reads.
 * 	Five-fold speedup over BSD 4.3 FFS, but buggy yet.
 * 	Added symlinks.  Enabled all protection checks and handling,
 * 	from Rob.
 * 	Added rdwri(), made old code use it where appropriate.
 * 	Moved syncip() here.
 * 	Added bmap+brelse operations.
 * 	[93/09/14  00:03:04  af]
 * 
 * 	Added pagein/pageout functions.
 * 	[93/08/26  16:34:14  af]
 * 
 * 	First version that can write to the filesystem.
 * 	[93/07/30  00:08:17  af]
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
#include <sys/kernel.h>	/* get_time() */

#include <dosfs/dosfs.h>
#include <dosfs/dosfs_node.h>

/*
 * Open called.
 *
 * Nothing to do.
 */
public int
dosfs_open(
	struct vnode	**vpp,
	int		flag,
	struct ucred	*cred)
{
trace(dosfs_debug,("\nDopen "));
	return (0);
}

/*
 * Close called
 *
 * Update the times on the inode on writeable file systems.
 */
public int
dosfs_close(
	struct vnode	*vp,
	int		fflag,
	struct ucred	*cred)
{
trace(dosfs_debug,("\nDclose "));
	return (0);
}

/*
 * Check mode permission on inode pointer. Mode is READ, WRITE or EXEC.
 * The mode is shifted to select the owner/group/other fields. The
 * super user is granted all permissions.
 */
public int
dosfs_access(
	struct vnode	*vp,
	register int	mode,
	struct ucred	*cred)
{
	register struct dosfs_node *ip;
	int error;

trace(dosfs_debug,("Daccess "));

	ip = VTO_DOS(vp);
	DOS_ILOCK(ip);
	error = dosfs_iaccess(ip, mode);
	DOS_IUNLOCK(ip);
	return (error);
}

public int
dosfs_getattr(
	struct vnode	*vp,
	register struct vattr *vap,
	struct ucred	*cred)
{
	register struct dosfs_node *ip = VTO_DOS(vp);
	int crtime;
	int i;

trace(dosfs_debug,("\nDgetattr%x ", ip->o_number));

	vap->va_mode = ip->o_mode;
	vap->va_uid = ip->o_uid;
	vap->va_gid = ip->o_gid;
	vap->va_fsid = ip->o_dev;
	vap->va_nodeid = ip->o_number;
	vap->va_nlink = ip->o_nlink;
	vap->va_size = ip->i_size;
	vap->va_blocksize = ip->i_mnt->im_bsize;
	vap->va_atime.tv_sec = ip->o_atime;
	vap->va_atime.tv_usec = 0;
	vap->va_mtime.tv_sec = ip->o_mtime;
	vap->va_mtime.tv_usec = 0;
	vap->va_ctime.tv_sec = ip->o_ctime;
	vap->va_ctime.tv_usec = 0;
	vap->va_rdev = vp->v_rdev;
	vap->va_blocks = (vap->va_size + 1023) >> 10;
	return (0);
}

/*
 * Vnode op for reading/writing
 */
int dosfs_readahead = 1;
int dosfs_seqreads = 0;
int dosfs_seqwrites = 0;

public int
dosfs_rdwr(
	struct vnode	*vp,
	register struct uio *uio,
	enum uio_rw	rw,
	int		ioflag,
	struct ucred	*cred)
{
	register struct dosfs_node *ip = VTO_DOS(vp);
	register struct dosfs_mount *imp;
	struct buf *bp;
	daddr_t lbn, bn, rablock;
	int size, chunk_size, diff, error = 0;
	long n, on, type;
	int iupdat_flag = 0;
	uid_t old_VOPuid;	/* xxx I donno whats this for */
	int lastra = -1;

trace(dosfs_debug,("Drdwr %x %c %x ", ip->o_number, (rw==UIO_READ)?'R':'W', uio->uio_resid));

	if (rw == UIO_READ && uio->uio_resid == 0)
		return (0);
	if ((ioflag & IO_APPEND) && (rw == UIO_WRITE)) {
		/*
		 * in append mode start at end of file.
		 */
		uio->uio_offset = ip->i_size;
	}
	if (((int) uio->uio_offset < 0 ||
	     (int) (uio->uio_offset + uio->uio_resid) < 0))
		return (EINVAL);
/*	if (rw == UIO_READ)
		imark(ip, IACC); */
	if (uio->uio_resid == 0)
		return (0);
	type = ip->o_mode&IFMT;
	if (rw == UIO_WRITE && type == IFREG &&
	    uio->uio_offset + uio->uio_resid >
	      u.u_rlimit[RLIMIT_FSIZE].rlim_cur) {
		psignal(u.u_procp, SIGXFSZ);
		return (EFBIG);
	}

	old_VOPuid = u.u_VOPuid;
	u.u_VOPuid = cred->cr_uid;

	/* symlinks are locked already */
	if (type != VLNK)
		DOS_ILOCK(ip);
	imp = ip->i_mnt;
	chunk_size = imp->chunk_size;
	do {
		lbn = dosfs_lblkno(imp, uio->uio_offset);
		on = dosfs_blkoff(imp, uio->uio_offset);
		n = MIN((unsigned)(imp->im_bsize - on), uio->uio_resid);
		if (rw == UIO_READ) {
			diff = ip->i_size - uio->uio_offset;
			if (diff <= 0) {
				error = 0;
				goto out;
			}
			if (diff < n)
				n = diff;
		} else if (uio->uio_offset + n > ip->i_size) {
			error = dosfs_iextend(ip, uio->uio_offset + n, lbn);
			if (error) goto out;
			if (ioflag & IO_SYNC)
				iupdat_flag = 1;
		}
		size = dosfs_blksize(imp, ip, lbn);
		if (rw == UIO_READ) {
if (dosfs_seqreads) {
			daddr_t sbno;
			unsigned int ssiz, tmp;

			/* get a chunk around it */
			bn = dosfs_bmap_seq(ip,lbn,&sbno,&ssiz);
trace(dosfs_debug,("Sqm %x -> %x %x %x ", lbn, bn, sbno, ssiz));
			if (ssiz < size) panic("Sseq screwed\n");
			/* drop past chunks */
			tmp = (((bn - sbno) * DEV_BSIZE) / chunk_size)
				* chunk_size;
			sbno += dosfs_lblkno(imp,tmp) * imp->im_clsiz;
			/* adjust offset into the buffer we'll read */
			on += (bn - sbno) * DEV_BSIZE;
trace(dosfs_debug,("sbo : %x  on : %x ", sbno, on));
			/* no more than one chunk worth */
			if (ssiz > chunk_size)
				ssiz = chunk_size;
			/* do the read(s) */
#if 0
			rablock = ?
			if (lastra == -1 && dosfs_readahead &&
			    ...

#endif
			bp = bread(ip->o_devvp, sbno, ssiz);
			ip->i_lastr = lbn;

trace(dosfs_debug,("rd %x %x -> %x ", sbno, ssiz, bp->b_resid));
			/* adjust size to give to user */
			tmp = MIN((ssiz-on),uio->uio_resid);
			n = MIN(diff,tmp);	/* eof ? */
			size = ssiz;
trace(dosfs_debug,("us : %x %x\n", size, n));
			/* done ! */
} else {
			bn = dosfs_bmap(ip,lbn);
			rablock = lbn + 1;
			if (ip->i_lastr + 1 == lbn && dosfs_readahead &&
			    dosfs_lblktosize(imp, rablock) < ip->i_size)
				bp = breada(ip->o_devvp,
					bn, size,
					dosfs_bmap(ip,rablock),
					dosfs_blksize(imp, ip, rablock));
			else
				bp = bread(ip->o_devvp, bn, size);
			ip->i_lastr = lbn;
}
		} else {
			bn = dosfs_bmap(ip,lbn);
			if (ip->i_vnode.pager != MEMORY_OBJECT_NULL)
			        inode_uncache(DOS_TOV(ip));
			if (n == imp->im_bsize) 
				bp = getblk(ip->o_devvp, bn, size);
			else
				bp = bread(ip->o_devvp, bn, size);
		}
		error = bp->b_error;
		if (error) {
			brelse(bp);
			goto out;
		}

		n = MIN(n, size - bp->b_resid);
		error = uiomove(bp->b_un.b_addr + on, (int)n, rw, uio);
		if (rw == UIO_READ) {
			if (n + on == imp->im_bsize || uio->uio_offset == ip->i_size)
				bp->b_flags |= B_AGE;
			brelse(bp);
		} else {
			if ((ioflag & IO_SYNC) || (ip->o_mode&IFMT) == IFDIR)
				bwrite(bp);
			else if (n + on == imp->im_bsize) {
				bp->b_flags |= B_AGE;
				bawrite(bp);
			} else
				bdwrite(bp);
			imark(ip, IUPD|ICHG);
			if (u.u_ruid != 0)
				ip->o_mode &= ~(ISUID|ISGID);
		}
	} while (error == 0 && uio->uio_resid > 0 && n != 0);
	if (iupdat_flag) {
		dosfs_iupdat(ip, 1);
	}
out:
	if (type != VLNK)
		DOS_IUNLOCK(ip);
	u.u_VOPuid = old_VOPuid;
	return (error);
}

public int
dosfs_ioctl(
	struct vnode	*vp,
	unsigned int	com,
	caddr_t		data,
	int		fflag,
	struct ucred	*cred)
{
trace(dosfs_debug,("Dioctl "));
	return (EINVAL);
}

public int
dosfs_select(
	struct vnode	*vp,
	int		which,
	struct ucred	*cred)
{
trace(dosfs_debug,("Dselect "));
	return (EINVAL);
}

/*
 * Setattr
 */
public int
dosfs_setattr(
	register struct vnode *vp,
	register struct vattr *vap,
	struct ucred *cred)
{
	register struct dosfs_node *ip;
	int chtime = 0;
	int error = 0;

trace(dosfs_debug,("Dsetattr %x ", ip->o_number));
	/*
	 * cannot set these attributes
	 */
	if ((vap->va_nlink != -1) || (vap->va_blocksize != -1) ||
	    (vap->va_rdev != -1) || (vap->va_blocks != -1) ||
	    (vap->va_fsid != -1) || (vap->va_nodeid != -1)) {
		return (EINVAL);
	}

	ip = VTO_DOS(vp);
	DOS_ILOCK(ip);
	/*
	 * Change file access modes.
	 */
	if (vap->va_mode != (u_short)-1) {
/* sleazy */
#undef i_uid
#define i_uid o_uid
		error = OWNER(cred, ip);
		if (error)
			goto out;
		ip->o_mode &= IFMT;
		ip->o_mode |= vap->va_mode & ~IFMT;
		if (cred->cr_uid != 0) {
			if ((ip->o_mode & IFMT) != IFDIR)
				ip->o_mode &= ~ISVTX;
			if (!groupmember(ip->o_gid))
				ip->o_mode &= ~ISGID;
		}
		imark(ip, ICHG);
		if ((vp->v_flag & VTEXT) && ((ip->o_mode & ISVTX) == 0)) {
			if (ip->i_header.pager != MEMORY_OBJECT_NULL)
			        inode_uncache(vp);
		}
	}
	/*
	 * To change file ownership, must be su.
	 * To change group ownership, must be su or owner and in target group.
	 * This is now enforced in chown1() below.
	 */
	/* va_uid and va_gid are short, not unsigned (ie uid_t and gid_t) */
	if ((vap->va_uid != -1) || (vap->va_gid != -1)) {
		error = dosfs_chown1(ip, vap->va_uid, vap->va_gid);
		if (error)
			goto out;
	}

	/*
	 * Truncate file. Must have write permission and not be a directory.
	 */
	if (vap->va_size != -1) {
		if ((ip->o_mode & IFMT) == IFDIR) {
			error = EISDIR;
			goto out;
		}
		error = dosfs_iaccess(ip, IWRITE);
		if (error)
			goto out;
		dosfs_itrunc(ip, vap->va_size);
	}
#if	MACH_NBC
	/*
	 *	Sync out all blocks to prevent delayed writes from
	 *	changing the modified time later.  Need to unlock
	 *	the inode so the pager can page out the pages... this
	 *	is a bit hokey but we'll fix it when we fix
	 *	the rest of the filesystem.
	 */
	DOS_IUNLOCK(ip);
	(void) mfs_fsync(vp);
	DOS_ILOCK(ip);
#endif	MACH_NBC
	/*
	 * Change file access or modified times.
	 */
	if (vap->va_atime.tv_sec != -1) {
		error = OWNER(cred, ip);
		if (error)
			goto out;
		ip->o_atime = vap->va_atime.tv_sec;
		chtime++;
	}
	if (vap->va_mtime.tv_sec != -1) {
		error = OWNER(cred, ip);
		if (error)
			goto out;
		ip->o_mtime = vap->va_mtime.tv_sec;
		chtime++;
	}
	if (chtime) {
		struct timeval time;
		get_time(&time);
		ip->i_flag |= IACC|IUPD|ICHG;
		ip->o_ctime = time.tv_sec;
	}
out:

	dosfs_iupdat(ip, 1);	/* XXX should be asyn for perf */
	DOS_IUNLOCK(ip);
	return (error);
}


/*
 * Perform chown operation on inode ip;
 * inode must be locked prior to call.
 */
dosfs_chown1(ip, uid, gid)
	register struct dosfs_node *ip;
	register uid_t uid;
	gid_t gid;
{
trace(dosfs_debug,("chown1 "));
	if (uid == (uid_t) -1)
		uid = ip->o_uid;
	if (gid == (gid_t) -1)
		gid = ip->o_gid;

	if ((uid == ip->o_uid) && (gid == ip->o_gid))
		return(0);      /* no change at all */

	/* error if not super-user and:
	 *	1) trying to change owner
	 *	2) not current owner
	 *	3) new group is not a member of process group set
	 */
	if ( (u.u_uid != 0) &&
	    ((uid != ip->o_uid) || (u.u_uid != uid) || (!groupmember(gid))) ) {
		return(EPERM);
	}

	ip->o_uid = uid;
	ip->o_gid = gid;
	imark(ip, ICHG);
	if (u.u_uid != 0)
		ip->o_mode &= ~(ISUID|ISGID);
	return (0);
}


public int
dosfs_create(
	struct vnode	*dvp,
	char		*nm,
	struct vattr	*vap,
	enum vcexcl	exclusive,
	int		mode,
	struct vnode	**vpp,
	struct ucred	*cred)
{
	register int error;
	struct dosfs_node *ip;

trace(dosfs_debug,("\nDcreate "));
	/*
	 * can't create directories. use dosfs_mkdir.
	 */
	if ((vap->va_mode&IFMT) == VDIR)
		return (EISDIR);
	ip = (struct dosfs_node *) 0;
	error = dosfs_direnter(VTO_DOS(dvp), nm, DE_CREATE,
		(struct dosfs_node *)0, (struct dosfs_node *)0, vap, &ip, cred);
	/*
	 * if file exists and this is a nonexclusive create,
	 * check for not directory and access permissions
	 * If create/read-only an existing directory, allow it.
	 */
	if (error == EEXIST) {
		if (exclusive == NONEXCL) {
			if (((ip->o_mode & IFMT) == IFDIR) && (mode & IWRITE)) {
				error = EISDIR;
			} else if (mode) {
				error = dosfs_iaccess(ip, mode);
			} else {
				error = 0;
			}
		}
		if (error) {
			dosfs_iput(ip);
		} else if (((ip->o_mode&IFMT) == IFREG) && (vap->va_size == 0)){
			/*
			 * truncate regular files, if required
			 */
			dosfs_itrunc(ip, (u_long) 0);
		}
	} 
	if (error) {
		return (error);
	}
	*vpp = DOS_TOV(ip);
	DOS_IUNLOCK(ip);
	/*
	 * If vnode is a device return special vnode instead
	 */
	if (ISVDEV(((*vpp)->v_mode&VFMT))) {
		struct vnode *newvp, *specvp();

		newvp = specvp(*vpp, (*vpp)->v_rdev);
		VN_RELE(*vpp);
		*vpp = newvp;
	}

	if (vap != (struct vattr *)0) {
		(void) VOP_GETATTR(*vpp, vap, cred);
	}
	return (error);
}

public int
dosfs_remove(
	struct vnode	*vp,
	char		*nm,
	struct ucred	*cred)
{
trace(dosfs_debug,("\nDremove "));
	return dosfs_dirremove(VTO_DOS(vp), nm, (struct dosfs_node *)0, 0);
}

/*
 * Rename a file or directory
 * We are given the vnode and entry string of the source and the
 * vnode and entry string of the place we want to move the source to
 * (the target). The essential operation is:
 *	unlink(target);
 *	link(source, target);
 *	unlink(source);
 * but "atomically".
 * Note that we do not have hard links, so things are easier than UFS.
 */
public int
dosfs_rename(
	struct vnode	*sdvp,		/* old (source) parent vnode */
	char		*snm,		/* old (source) entry name */
	struct vnode	*tdvp,		/* new (target) parent vnode */
	char		*tnm,		/* new (target) entry name */
	struct ucred	*cred)
{
	struct dosfs_node *sip;		/* source inode */
	register struct dosfs_node *sdp;	/* old (source) parent inode */
	register struct dosfs_node *tdp;	/* new (target) parent inode */
	register int error;

trace(dosfs_debug,("\nDrename "));
	sdp = VTO_DOS(sdvp);
	tdp = VTO_DOS(tdvp);
	/*
	 * make sure we can delete the source entry
	 */
	error = dosfs_iaccess(sdp, IWRITE);
	if (error) {
		return (error);
	}
	/*
	 * look up inode of file we're supposed to rename.
	 */
	error = dosfs_dirlook(sdp, snm, (struct vnode **)&sip, cred);
	if (error) {
		return (error);
	}

	DOS_IUNLOCK(sip);			/* unlock inode (it's held) */
	/*
	 * check for renaming '.' or '..' or alias of '.'
	 */
	if ((strcmp(snm, ".") == 0) || (strcmp(snm, "..") == 0) ||
	    (sdp == sip)) {
		error = EINVAL;
		goto out;
	}
	/*
	 * link source to the target
	 */
	error =
	    dosfs_direnter(tdp, tnm, DE_RENAME,
		sdp, sip, (struct vattr *)0, (struct dosfs_node **)0, cred);
	if (error)
		goto out;

	/*
	 * Unlink the source
	 * Remove the source entry. Dirremove checks that the entry
	 * still reflects sip, and returns an error if it doesn't.
	 * If the entry has changed just forget about it. 
	 * Release the source inode.
	 */
	error = dosfs_dirremove(sdp, snm, sip, 0);
	if (error == ENOENT)
		error = 0;
out:
	dosfs_irelease(sip);
	return (error);
}

public int
dosfs_mkdir(
	struct vnode	*dvp,
	char		*nm,
	register struct vattr *vap,
	struct vnode	**vpp,
	struct ucred	*cred)
{
	struct dosfs_node *ip;
	register int error;

trace(dosfs_debug,("\nDmkdir "));
	error =
	    dosfs_direnter(VTO_DOS(dvp), nm, DE_CREATE,
		(struct dosfs_node *)0, (struct dosfs_node *)0, vap, &ip, cred);
	if (error == 0) {
		*vpp = DOS_TOV(ip);
		DOS_IUNLOCK(ip);
	} else if (error == EEXIST) {
		dosfs_iput(ip);
	}
	return (error);
}

public int
dosfs_rmdir(
	struct vnode	*vp,
	char		*nm,
	struct ucred	*cred)
{
trace(dosfs_debug,("\nDrmdir "));
	return dosfs_dirremove(VTO_DOS(vp), nm, (struct dosfs_node *)0, 1);
}

/*
 * Vnode op for readdir
 */
public int
dosfs_readdir(
	struct vnode	*vp,
	register struct uio *uio,
	struct ucred	*cred)
{
	struct direct dirent;
	int dosfs_offset;
	int entryoffsetinblock;
	int error = 0;
	int endsearch;
	struct dosfs_directory_record *ep;
	struct dosfs_mount *imp;
	struct dosfs_node *ip;
	struct buf *bp = NULL;


trace(dosfs_debug,("\nDreaddir %x\n", uio->uio_offset));
	ip = VTO_DOS (vp);
	imp = ip->i_mnt;

	dosfs_offset = uio->uio_offset;

	/*
	 * Before we start. DOS does not have '.' and '..' entries
	 * in the root directory, and Unix does not like it.  So we
	 * just make believe...
	 */
	if ((ip->o_number == DOS_ROOTINO) && (dosfs_offset == 0)) {
		dirent.d_name[0] = '.';
		dirent.d_name[1] = 0;
		dirent.d_namlen = 1;
		dirent.d_ino = DOS_ROOTINO;
		dirent.d_reclen = DIRSIZ (&dirent);
		if (uio->uio_resid >= dirent.d_reclen)
		if (error = uiomove (&dirent, dirent.d_reclen, UIO_READ, uio))
			return error;
	}


	entryoffsetinblock = dosfs_blkoff(imp, dosfs_offset);
	if (entryoffsetinblock != 0) {
		if (error = dosfs_blkatoff(ip, dosfs_offset, (char **)0, &bp))
			return (error);
	}

	endsearch = ip->i_size;

	while (dosfs_offset < endsearch && uio->uio_resid > 0) {
		/*
		 * If offset is on a block boundary,
		 * read the next directory block.
		 * Release previous if it exists.
		 */

		if (dosfs_blkoff(imp, dosfs_offset) == 0) {
			if (bp != NULL)
				brelse(bp);
			if (error = dosfs_blkatoff(ip, dosfs_offset,
						 (char **)0, &bp))
				return (error);
			entryoffsetinblock = 0;
		}
		/*
		 * Get pointer to next entry.
		 */

		ep = (struct dosfs_directory_record *)
			(bp->b_un.b_addr + entryoffsetinblock);

		/*
		 * Special cases
		 */
		if (ep->name[0] == DOS_NAME_EMPTY)
			goto next_one;

		if (ep->name[0] == DOS_NAME_DELETED)
			goto next_one;
		
		/*
		 * Translate the name
		 */
		if (dosfsfntrans(ep, dirent.d_name, &dirent.d_namlen,
				 imp->im_flags & DOSFSMNT_TRANS))
			goto next_one;

		/*
		 * Inum and kludges ..
		 */
		ino_dtou(dirent.d_ino,ep);

		if (ep->attr & DOS_ATTR_LABEL) {
			/* Can only be on root, soo.. */
			dirent.d_name[0] = '.';
			dirent.d_name[1] = '.';
			dirent.d_name[2] = 0;
			dirent.d_namlen = 2;
		}
		dirent.d_name[dirent.d_namlen] = 0;
		dirent.d_reclen = DIRSIZ (&dirent);

		if (uio->uio_resid < dirent.d_reclen)
			break;

		if (error = uiomove (&dirent, dirent.d_reclen, UIO_READ, uio))
			break;

next_one:
		dosfs_offset += sizeof(struct dosfs_directory_record);
		entryoffsetinblock += sizeof(struct dosfs_directory_record);
	}
			
	if (bp)
		brelse (bp);

	uio->uio_offset = dosfs_offset;

	return (error);
}

/*
 * Read/write a specific dos node
 */
private int
dosfs_rdwri(
	enum uio_rw rw,
	struct dosfs_node *ip,
	caddr_t base,
	int len,
	int offset,
	int seg,
	int *aresid)

{
	struct uio auio;
	struct iovec aiov;
	register int error;

trace(dosfs_debug,("Drdwri %x ", len));
	aiov.iov_base = base;
	aiov.iov_len = len;
	auio.uio_iov = &aiov;
	auio.uio_iovcnt = 1;
	auio.uio_offset = offset;
	auio.uio_segflg = seg;
	auio.uio_resid = len;
	error = dosfs_rdwr(DOS_TOV(ip), &auio, rw, 0, u.u_cred);
	if (aresid) {
		*aresid = auio.uio_resid;
	} else if (auio.uio_resid) {
		error = EIO;
	}
	return (error);
}


/*
 * Create a symlink
 */
public int
dosfs_symlink(
	struct vnode	*dvp,
	char		*lnm,
	struct vattr	*vap,
	char		*tnm,
	struct ucred	*cred)
{
	struct dosfs_node *ip;
	int error;

trace(dosfs_debug,("Dsymlink "));
	ip = (struct dosfs_node *) 0;
	vap->va_mode &= ~IFMT;
	vap->va_mode |= VLNK;
	vap->va_rdev = 0;
	error =
	    dosfs_direnter(VTO_DOS(dvp), lnm, DE_CREATE,
		(struct dosfs_node *)0, (struct dosfs_node *)0, vap, &ip, cred);
	if (error == 0) {
		error =
		    dosfs_rdwri(UIO_WRITE, ip,
			tnm, strlen(tnm), 0, UIO_SYSSPACE, (int *)0);
		dosfs_iput(ip);
	} else if (error == EEXIST) {
		dosfs_iput(ip);
	}
	return (error);
}

/*
 * Read a symlink
*/
public int
dosfs_readlink(
	struct vnode	*vp,
	struct uio	*uiop,
	struct ucred	*cred)
{
	struct dosfs_node *ip;
	int error;

trace(dosfs_debug,("Drdlink "));
	if ((vp->v_mode&VFMT) != VLNK)
		return (EINVAL);
	ip = VTO_DOS(vp);

	DOS_ILOCK(ip);
	error = dosfs_rdwr(vp, uiop, UIO_READ, 0, cred);
	DOS_IUNLOCK(ip);

	return (error);
}

/*
 * Make sure all writes for a file are out to disk.
 */
public int
dosfs_syncip(
	struct dosfs_node	*ip,
	struct ucred		*cred)
{
trace(dosfs_debug,("Dsyncip %x ", ip->o_number));

	if (ip->i_size) {
		daddr_t			lbn, mbno;

		DOS_ILOCK(ip);
		mbno = dosfs_lblkno(ip->i_mnt, ip->i_size);
		for (lbn = 0; lbn <= mbno; lbn++)
			blkflush(ip->o_devvp, dosfs_bmap(ip, lbn),
				 dosfs_blksize(ip->i_mnt, ip, lbn));

		DOS_IUNLOCK(ip);
	}
	return (0);
}

/*
 * Just call the block device strategy routine.
 */
public int
dosfs_strategy(
	register struct buf *bp)
{
	register struct dosfs_node *ip;
	struct vnode *vp = ITOV(bp->b_vp);
	int error;

trace(dosfs_debug,("Dstrategy %x ", bp->b_blkno));

	ip = VTO_DOS(vp);
	vp = ip->o_devvp;
	bp->b_dev = vp->v_rdev;
	(*_VOP_(vp)->vn_strategy)(bp);
	return (0);
}

/*
 * Map a block for reading (nfs)
 */
public int
dosfs_nfsbmap(
	struct vnode	*vp,
	daddr_t		lbn,
	struct vnode	**vpp,
	daddr_t		*bnp)
{
	struct dosfs_node *ip;

trace(dosfs_debug,("Dnfbm "));
	ip = VTO_DOS(vp);
	if (vpp)
		*vpp = ip->o_devvp;
	if (bnp)
		*bnp = dosfs_bmap(ip, lbn);
	return (0);
}

/*
 * release a buffer we gave to nfs
 */
public int
dosfs_brelse(
	struct vnode *vp,
	struct buf *bp)
{
trace(dosfs_debug,("Dnfbrl "));
	bp->b_flags |= B_AGE;
	bp->b_resid = 0;
	brelse(bp);
	return (0);
}


/*
 * Read a vm page in
 */
public int
dosfs_pagein(
	struct vnode	*vp,
	vm_offset_t	addr,
	vm_size_t	size,
	vm_offset_t	offset,
	struct ucred	*cred)
{
	int error, resid = 0;

trace(dosfs_debug,("Dpgin %x %x\n", vp, offset));
	error = dosfs_rdwri(UIO_READ, VTO_DOS(vp),
				(caddr_t)addr, (int)size, (int)offset,
				UIO_SYSSPACE, &resid);
	if (error)
		printf("error %d on pagein (dosfs_rdwr)\n", error);
	else if (resid)
		bzero( addr + size - resid, resid);
	return (error);
}

/*
 * Write a vm page out
 */
public int
dosfs_pageout(
	struct vnode	*vp,
	vm_offset_t	addr,
	vm_size_t	size,
	vm_offset_t	offset,
	struct ucred	*cred,
	boolean_t	init)
{
	int error;

trace(dosfs_debug,("Dpgout %x %x\n", vp, offset));
	if (init)
		panic("dosfs_pageout: called from data_initialize");

	error = dosfs_rdwri(UIO_WRITE, VTO_DOS(vp),
				(caddr_t)addr, (int)size, (int)offset,
				UIO_SYSSPACE, (int *)0);
	if (error)
		printf("error %d on pageout (dosfs_rdwr)\n", error);
	return (error);
}

/*
 * How many links into this vnode
 */
public int
dosfs_nlinks(
	struct vnode	*vp,
	int		*l,
	struct ucred	*cred)
{
trace(dosfs_debug,("Dnlinks "));
	*l = VTO_DOS(vp)->o_nlink;
	return (0);
}

/*
 * Update disk status of inode
 */
public int
dosfs_vupdate(
	struct vnode	*vp,
	int		flag)
{
	DOS_ILOCK(VTO_DOS(vp));
	dosfs_iupdat(VTO_DOS(vp),flag);
	DOS_IUNLOCK(VTO_DOS(vp));
	return 0;
}

/*
 * Invalid, unimplemented, ...
 */
public int einval();

/*
 * Global vfs data structures for dosfs
 */
struct vnodeops dosfs_vnodeops = {
	dosfs_open,		/* open */
	dosfs_close,		/* close */
	dosfs_rdwr,		/* rdwr */
	dosfs_ioctl,		/* ioctl */
	dosfs_select,		/* select */
	dosfs_getattr,		/* getattr */
	dosfs_setattr,		/* setattr */
	dosfs_access,		/* access */
	dosfs_lookup,		/* lookup */
	dosfs_create,		/* create */
	dosfs_remove,		/* remove */
	(void *)einval,		/* link */
	dosfs_rename,		/* rename */
	dosfs_mkdir,		/* mkdir */
	dosfs_rmdir,		/* rmdir */
	dosfs_readdir,		/* readdir */
	dosfs_symlink,		/* symlink */
	dosfs_readlink,		/* readlink */
	dosfs_syncip,		/* fsync */
	dosfs_inactive,		/* inactive */
	dosfs_nfsbmap,		/* bmap */
	dosfs_strategy,		/* strategy */
	(void *)einval,		/* bread	????? */
	dosfs_brelse,		/* brelse */
	(void *)einval,		/* lockctl */
	dosfs_fid,		/* fid */
	dosfs_pagein,		/* page_read */
	dosfs_pageout,		/* page_write */
	dosfs_readdir,		/* read1dir */
	dosfs_freefid,		/* freefid */
	dosfs_nlinks,		/* nlinks */
	dosfs_vupdate,		/* iupdate */
};


