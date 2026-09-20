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
 * $Log:	isofs_vnops.c,v $
 * Revision 2.3  93/09/15  16:06:36  mrt
 * 	Added isofs_pagein().
 * 	[93/08/26  16:55:57  af]
 * 
 * Revision 2.2  93/08/07  16:56:21  mrt
 * 	Moved einval elsewhere, so that it can be shared.
 * 	[93/07/10  19:22:50  af]
 * 
 * 	Handle relocated directories (untested).
 * 	Trust the rockridge code for inode modes.
 * 	Added isofs_readlink(), symlinks work.
 * 	[93/07/04  22:02:01  af]
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
 *	@(#)isofs_vnops.c
 */
/*
 *
 *
 * PATCHES MAGIC                LEVEL   PATCH THAT GOT US HERE
 * --------------------         -----   ----------------------
 * CURRENT PATCH LEVEL:         1       00040
 * --------------------         -----   ----------------------
 *
 * 10 Aug 92	Scott Burris		Fixed "delete from CD-ROM" bug
 */
#include <strings.h>
#include <sys/param.h>
#include <sys/dir.h>
#include <sys/user.h>
#include <sys/buf.h>
#include <sys/inode.h>

#include <isofs/isofs.h>
#include <isofs/isofs_node.h>

/*
 * Open called.
 *
 * Nothing to do.
 */
public int
isofs_open(
	struct vnode	**vpp,
	int		flag,
	struct ucred	*cred)
{
trace(isofs_debug,("isofs_open "));
	return (0);
}

/*
 * Close called
 *
 * Update the times on the inode on writeable file systems.
 */
public int
isofs_close(
	struct vnode	*vp,
	int		fflag,
	struct ucred	*cred)
{
trace(isofs_debug,("isofs_close "));
	return (0);
}

/*
 * Check mode permission on inode pointer. Mode is READ, WRITE or EXEC.
 * The mode is shifted to select the owner/group/other fields. The
 * super user is granted all permissions.
 */
public int
isofs_access(
	struct vnode	*vp,
	register int	mode,
	struct ucred	*cred)
{
trace(isofs_debug,("isofs_access "));
	return (0);
}

public int
isofs_getattr(
	struct vnode	*vp,
	register struct vattr *vap,
	struct ucred	*cred)
{
	register struct iso_node *ip = VTO_ISO(vp);
	int crtime;
	int i;

trace(isofs_debug,("isofs_getattr "));

	vap->va_mode = ip->o_mode;
	vap->va_uid = ip->o_uid;
	vap->va_gid = ip->o_gid;
	vap->va_fsid = ip->o_dev;
	vap->va_nodeid = ip->o_number;
	vap->va_nlink = ip->o_nlink;
	vap->va_size = ip->i_size;
	vap->va_blocksize = ip->i_mnt->logical_block_size;
	vap->va_atime.tv_sec = ip->o_atime;
	vap->va_atime.tv_usec = 0;
	vap->va_mtime.tv_sec = ip->o_mtime;
	vap->va_mtime.tv_usec = 0;
	vap->va_ctime.tv_sec = ip->o_ctime;
	vap->va_ctime.tv_usec = 0;
	vap->va_rdev = 0;
	vap->va_blocks = ((vap->va_size + 1023) >> 10) << 1;
	return (0);
}

/*
 * Vnode op for reading/writing
 */
public int
isofs_rdwr(
	struct vnode	*vp,
	register struct uio *uio,
	enum uio_rw	rw,
	int		ioflag,
	struct ucred	*cred)
{
	register struct iso_node *ip = VTO_ISO(vp);
	register struct iso_mnt *imp;
	struct buf *bp;
	daddr_t lbn, bn, rablock;
	int size, diff, error = 0;
	long n, on, type;

trace(isofs_debug,("isofs_rdwr "));
	if (rw != UIO_READ)
		return (EINVAL);

#if PARANOID
	type = ip->i_mode & IFMT;
	if (type != IFDIR && type != IFREG && type != IFLNK)
		panic("isofs_read type");
#endif

	if (uio->uio_resid == 0)
		return (0);
	if (uio->uio_offset < 0)
		return (EINVAL);
	ip->i_flag |= IACC;
	imp = ip->i_mnt;
	do {
		lbn = iso_lblkno(imp, uio->uio_offset);
		on = iso_blkoff(imp, uio->uio_offset);
		n = MIN((unsigned)(imp->im_bsize - on), uio->uio_resid);
		diff = ip->i_size - uio->uio_offset;
		if (diff <= 0)
			return (0);
		if (diff < n)
			n = diff;
		size = iso_blksize(imp, ip, lbn);
		rablock = lbn + 1;
		if (ip->i_lastr + 1 == lbn &&
		    iso_lblktosize(imp, rablock) < ip->i_size)
			bp = breada(ISO_TOV(ip),
				iso_bmap(ip,lbn), size,
				iso_bmap(ip,rablock),
				iso_blksize(imp, ip, rablock));
		else
			bp = bread(ISO_TOV(ip), iso_bmap(ip, lbn), size);
		error = bp->b_error;
		ip->i_lastr = lbn;
		n = MIN(n, size - bp->b_resid);
		if (error) {
			brelse(bp);
			return (error);
		}

		error = uiomove(bp->b_un.b_addr + on, (int)n, UIO_READ, uio);
		if (n + on == imp->im_bsize || uio->uio_offset == ip->i_size)
			bp->b_flags |= B_AGE;
		brelse(bp);
	} while (error == 0 && uio->uio_resid > 0 && n != 0);
	return (error);
}

public int
isofs_ioctl(
	struct vnode	*vp,
	unsigned int	com,
	caddr_t		data,
	int		fflag,
	struct ucred	*cred)
{
trace(isofs_debug,("isofs_ioctl "));
	return (EINVAL);
}

public int
isofs_select(
	struct vnode	*vp,
	int		which,
	struct ucred	*cred)
{
trace(isofs_debug,("isofs_select "));
	return (EINVAL);
}

/*
 * Vnode op for readdir
 */
public int
isofs_readdir(
	struct vnode	*vp,
	register struct uio *uio,
	struct ucred	*cred)
{
	struct direct dirent;
	int iso_offset;
	int entryoffsetinblock;
	int error = 0;
	int endsearch;
	struct iso_directory_record *ep;
	int reclen;
	struct iso_mnt *imp;
	struct iso_node *ip;
	struct buf *bp = NULL;
	int i;

trace(isofs_debug,("isofs_readdir %x\n", uio->uio_offset));
	ip = VTO_ISO (vp);
	imp = ip->i_mnt;

	iso_offset = uio->uio_offset;

	entryoffsetinblock = iso_blkoff(imp, iso_offset);
	if (entryoffsetinblock != 0) {
		if (error = iso_blkatoff(ip, iso_offset, (char **)0, &bp))
			return (error);
	}

	endsearch = ip->i_size;

	while (iso_offset < endsearch && uio->uio_resid > 0) {
		/*
		 * If offset is on a block boundary,
		 * read the next directory block.
		 * Release previous if it exists.
		 */

		if (iso_blkoff(imp, iso_offset) == 0) {
			if (bp != NULL)
				brelse(bp);
			if (error = iso_blkatoff(ip, iso_offset,
						 (char **)0, &bp))
				return (error);
			entryoffsetinblock = 0;
		}
		/*
		 * Get pointer to next entry.
		 */

		ep = (struct iso_directory_record *)
			(bp->b_un.b_addr + entryoffsetinblock);

		reclen = isonum_711 (ep->length);
		if (reclen == 0) {
			/* skip to next block, if any */
			iso_offset = roundup (iso_offset,
					      imp->logical_block_size);
			continue;
		}

		if (reclen < sizeof (struct iso_directory_record))
			/* illegal entry, stop */
			break;

/* 10 Aug 92*/	if (entryoffsetinblock + reclen -1 >= imp->logical_block_size)
			/* illegal directory, so stop looking */
			break;

		dirent.d_ino    = isonum_733 (ep->extent);
		dirent.d_namlen = isonum_711 (ep->name_len);

		if (reclen < sizeof (struct iso_directory_record)
		    + dirent.d_namlen)
			/* illegal entry, stop */
			break;

		if (ip->i_mnt->im_flags & ISOFSMNT_NORR) {
			isofntrans(ep->name, dirent.d_namlen,
				dirent.d_name, &dirent.d_namlen);
		} else {
			char *nnm = 0; int nl = 0, r;

			r = rock_ridge_get_filename(ep, &nnm, &nl, ip);
			if (r == 1) {
				bcopy(nnm, dirent.d_name, nl);
				dirent.d_namlen = nl;
				free(nnm);
			} else
			if (r == -1)
				goto next_one;	/* relocated directory */
			else
				bcopy(ep->name, dirent.d_name, dirent.d_namlen);
		}


		if (dirent.d_namlen == 1) {
			switch (dirent.d_name[0]) {
			case 0:
				dirent.d_name[0] = '.';
				break;
			case 1:
				dirent.d_name[0] = '.';
				dirent.d_name[1] = '.';
				dirent.d_namlen = 2;
			}
		}
		dirent.d_name[dirent.d_namlen] = 0;
		dirent.d_reclen = DIRSIZ (&dirent);

		if (uio->uio_resid < dirent.d_reclen)
			break;

		if (error = uiomove (&dirent, dirent.d_reclen, UIO_READ, uio))
			break;

next_one:
		iso_offset += reclen;
		entryoffsetinblock += reclen;
	}
			
	if (bp)
		brelse (bp);

#if bsd44
	if ((VTO_ISO(vp)->i_size - iso_offset) <= 0)
		*eofflagp = 1;
	else
		*eofflagp = 0;
#endif

	uio->uio_offset = iso_offset;

	return (error);
}

/*
 * Calculate the logical to physical mapping if not done already,
 * then call the device strategy routine.
 */
public int
isofs_strategy(
	register struct buf *bp)
{
	register struct iso_node *ip;
	struct vnode *vp = ITOV(bp->b_vp);
	int error;

trace(isofs_debug,("isofs_strategy "));
	if ((vp->v_mode&VFMT) == VBLK || (vp->v_mode&VFMT) == VCHR)
		panic("isofs_strategy: spec");

	ip = VTO_ISO(vp);
	if ((long)bp->b_blkno == -1) {
		biodone(bp);
		return (0);
	}
	vp = ITOV(ip->o_devvp);
	bp->b_dev = vp->v_rdev;
	(*_VOP_(vp)->vn_strategy)(bp);
	return (0);
}

/*
 * Read a vm page in
 */
public int
isofs_pagein(
	struct vnode	*vp,
	vm_offset_t	addr,
	vm_size_t	size,
	vm_offset_t	offset,
	struct ucred	*cred)
{
	int error;
	struct uio uio;
	struct iovec iov;

	iov.iov_base = (caddr_t)addr;
	iov.iov_len = size;
	uio.uio_iov = &iov;
	uio.uio_iovcnt = 1;
	uio.uio_offset = offset;
	uio.uio_segflg = UIO_SYSSPACE;
	uio.uio_resid = size;

	error = isofs_rdwr(vp, &uio, UIO_READ, 0, cred);
	if (error)
		printf("error %d on pagein (isofs_rdwr)\n", error);
	return (error);
}

/*
 * How many links into this vnode
 */
int
isofs_nlinks(
	struct vnode	*vp,
	int		*l,
	struct ucred	*cred)
{
trace(isofs_debug,("isofs_nlinks "));
    *l = VTO_ISO(vp)->o_nlink;
    return (0);
}

/*
 * Follow a symlink
 */
int
isofs_readlink(
	struct vnode *vp,
	struct uio *uiop,
	struct ucred *cred)
{
	register struct iso_node *ip = VTO_ISO(vp);
	register int error;
	char	*l_name;

trace(isofs_debug,("isofs_readlink x%x ", ip->o_number));
	if ((vp->v_mode&VFMT) != VLNK)
		return (EINVAL);
	if ((ip->o_mode&VFMT) != VLNK)
		return (EINVAL);

	ISO_ILOCK(ip);
	l_name = rock_ridge_get_symlink(ip);
	ISO_IUNLOCK(ip);

	if (l_name) {
		error = uiomove(l_name, strlen(l_name), UIO_READ, uiop);
		free(l_name);
	} else
		error = EINVAL; /* oops */
	return error;
}

/*
 * Invalid, unimplemented, ...
 */
public int einval();

/*
 * Global vfs data structures for isofs
 */
struct vnodeops isofs_vnodeops = {
	isofs_open,		/* open */
	isofs_close,		/* close */
	isofs_rdwr,		/* rdwr */
	isofs_ioctl,		/* ioctl */
	isofs_select,		/* select */
	isofs_getattr,		/* getattr */
	(void *)einval,		/* setattr */
	isofs_access,		/* access */
	isofs_lookup,		/* lookup */
	(void *)einval,		/* create */
	(void *)einval,		/* remove */
	(void *)einval,		/* link */
	(void *)einval,		/* rename */
	(void *)einval,		/* mkdir */
	(void *)einval,		/* rmdir */
	isofs_readdir,		/* readdir */
	(void *)einval,		/* symlink */
	isofs_readlink,		/* readlink */
	isofs_sync,		/* fsync */
	isofs_inactive,		/* inactive */
	(void *)einval,		/* bmap */
	isofs_strategy,		/* strategy */
	(void *)einval,		/* bread	????? */
	(void *)einval,		/* brelse	????? */
	(void *)einval,		/* lockctl	????? */
	isofs_fid,		/* fid */
	isofs_pagein,		/* page_read */
	(void *)einval,		/* page_write */
	isofs_readdir,		/* read1dir */
	isofs_freefid,		/* freefid */
	isofs_nlinks,		/* nlinks */
};


