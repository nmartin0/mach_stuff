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
 * $Log:	dosfs_fhandle.c,v $
 * Revision 2.2  93/09/15  13:29:29  mrt
 * 	Zero lookup hint in fake directory vnode, in dosfs_fhtovp().
 * 	[93/09/13  23:47:59  af]
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

#include <dosfs/dosfs.h>
#include <dosfs/dosfs_node.h>

extern zone_t	dosfs_fid_zone;

int dosfs_fiddebug = 0;

public int
dosfs_fid(
	struct vnode *vp,
	struct fid **fidpp)
{
	register struct ifid *ifid;
	struct dosfs_node	*ip = VTO_DOS(vp);

trace(dosfs_fiddebug,("dosfs_fid %x (%x) ", ip, ip->dosfs_dirextent));
	ZALLOC(dosfs_fid_zone, ifid, struct ifid *);
	bzero((caddr_t)ifid, sizeof(struct ifid));
	ifid->ifid_len = sizeof(struct ifid) - (sizeof(struct fid) - MAXFIDSZ);
	ifid->ifid_ino = ip->o_number;
	ifid->ifid_dirext = ip->dosfs_dirextent;
	ifid->ifid_off = ip->dosfs_diroff;

	*fidpp = (struct fid *)ifid;
	return (0);
}

public int
dosfs_freefid(
	struct vnode *vp,
	struct fid *fidp)
{
trace(dosfs_fiddebug,("dosfs_freefid %x\n", vp));
	ZFREE(dosfs_fid_zone, fidp);
	return (0);
}

/*
 * File handle to vnode
 *
 * Have to be really careful about stale file handles:
 * - check that the inode number is in range
 * - call iget() to get the locked inode
 * - check for an unallocated inode (i_mode == 0)
 * - check that the generation number matches
 */
public int
dosfs_fhtovp(
	register struct vfs *vfsp,
	struct vnode	**vpp,
	struct fid	*fhp)
{
	struct dosfs_node tvp, *nip;
	struct dosfs_directory_record *dirp;
	int error, off, bsiz;
	daddr_t block;
	register struct ifid *ifhp;
	struct dosfs_mount	*dosfsmp;
	struct buf	*bp;

	ifhp = (struct ifid *)fhp;
	dosfsmp = VFSTODOSFS (vfsp);

trace(dosfs_fiddebug,("dosfs_fhtovp %x %x %x %x\n", 
		ifhp->ifid_len, ifhp->ifid_ino,
		ifhp->ifid_dirext, ifhp->ifid_off));

	*vpp = NULL;
	if (ifhp->ifid_ino == DOS_ROOTINO)
		return dosfs_root(vfsp, vpp);


	off = ifhp->ifid_off;
	if (ifhp->ifid_dirext >= dosfsmp->im_size)
		return (EINVAL);

	bsiz = dosfsmp->im_bsize;
	if ((off & (bsiz-1)) + sizeof (struct dosfs_directory_record) >= bsiz)
		return (EINVAL);

	/*
	 * Get to the directory record for the inode
	 */
	tvp.i_vnode.ih_fs = vfsp;
	tvp.o_number = ifhp->ifid_ino; /* not really, but != rootino */
	tvp.i_mnt = dosfsmp;
	tvp.last_lbn = -1;
	tvp.o_dev = dosfsmp->im_dev;
	tvp.extent = ifhp->ifid_dirext;
	tvp.dosfs_diroff = off;

	bp = bread (	dosfsmp->im_devvp,
			dosfs_bmap(&tvp, dosfs_lblkno(dosfsmp,off)),
			bsiz);

	dirp = (struct dosfs_directory_record *)(bp->b_un.b_addr + (off & (bsiz-1)));

	error = dosfs_iget(&tvp, ifhp->ifid_ino, &nip, dirp, off);
	if (error == 0) {
		DOS_IUNLOCK(nip);
		*vpp = DOS_TOV(nip);
	}
out:
	brelse (bp);
	return (error);
}

