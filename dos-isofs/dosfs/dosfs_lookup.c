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
 * $Log:	dosfs_lookup.c,v $
 * Revision 2.2  93/09/15  13:29:39  mrt
 * 	Please GCC.
 * 	[93/08/26  16:34:47  af]
 * 
 * 	First version that can write to the filesystem.
 * 	[93/07/30  00:12:51  af]
 * 
 * 	Created.
 * 	[93/07/12            af]
 * 
 */

#include <strings.h>
#include <sys/param.h>
#include <sys/dir.h>
#include <sys/user.h>
#include <sys/buf.h>
#include <sys/inode.h>

#include <vfs/dnlc.h>
#include <dosfs/dosfs.h>
#include <dosfs/dosfs_node.h>

extern struct vnode *specvp(), *dnlc_lookup();

/*
 * See original ufs_lookup for structure.
 */
public int
dosfs_lookup(
	struct vnode	*vdp,
	char		*nm,
	struct vnode	**vpp,
	struct ucred	*cred)
{
	struct vnode *vp;
	register int error;

trace(dosfs_debug,("\nDlookup %s ", nm));
	error = dosfs_dirlook(VTO_DOS(vdp), nm, &vp, cred);
	if (error == 0) {
		*vpp = vp;
		DOS_IUNLOCK(VTO_DOS(vp));
		/*
		 * If vnode is a device return special vnode instead
		 */
		if (ISVDEV(((*vpp)->v_mode&VFMT))) {
			struct vnode *newvp;

			newvp = specvp(*vpp, (*vpp)->v_rdev);
			VN_RELE(*vpp);
			*vpp = newvp;
		}
	}
	return (error);
}

int dos_lu0, dos_lu1, dos_lu2;

public int
dosfs_dirlook(
	register struct dosfs_node *dp,	/* the directory we are searching */
	char		*nm,		/* the name we are looking for */
	struct vnode	**vpp,		/* the vnode we return */
	struct ucred	*cred)		/* credentials to use */
{
	struct vnode *vp;
	register struct dosfs_mount *imp;	/* file system that directory is in */
	struct buf *bp = 0;		/* a buffer of directory entries */
	register struct dosfs_directory_record *ep;
					/* the current directory entry */
	int entryoffsetinblock;		/* offset of ep in bp's buffer */
	int numdirpasses;		/* strategy for directory search */
	int endsearch;			/* offset to end directory search */
	struct dosfs_node *pdp;		/* saved dp during symlink work */
	struct dosfs_node *tdp;		/* returned by iget */
	int error;
	u_int ep_ino;
	off_t offset;
	int dot = 0;
	int nmlen;			/* length of target name */

	imp = dp->i_mnt;

trace(dosfs_debug,("Ddirlook %x\n", dp->o_number));
	/*
	 * Check accessiblity of directory.
	 */
	if ((dp->attr & DOS_ATTR_DIR) == 0)
		return (ENOTDIR);

	/*
	 * Quick sanity check on name
	 */
	nmlen = strlen(nm);
	if (nmlen > 12)
		return (ENOENT);

	/*
	 * We now have a segment name to search for, and a directory to search.
	 *
	 * Before tediously performing a linear scan of the directory,
	 * check the name cache to see if the directory/name pair
	 * we are looking for is known already.
	 */
dos_lu0++;
	vp = dnlc_lookup(DOS_TOV(dp), nm, cred);
	if (vp) {
		VN_HOLD(vp);
dos_lu1++;
		*vpp = vp;
		DOS_ILOCK(VTO_DOS(vp));
		return (0);
	}
dos_lu2++;

	/*
	 * If there is cached information on a previous search of
	 * this directory, pick up where we last left off.
	 * We cache only lookups as these are the most common
	 * and have the greatest payoff. Caching CREATE has little
	 * benefit as it usually must search the entire directory
	 * to determine that the entry does not exist. Caching the
	 * location of the last DELETE or RENAME has not reduced
	 * profiling time and hence has been removed in the interest
	 * of simplicity.
	 */
	DOS_ILOCK(dp);
	if (dp->o_diroff > dp->i_size) {
		dp->o_diroff = 0;
	}
	if (dp->o_diroff == 0) {
		offset = 0;
		numdirpasses = 1;
	} else {
		offset = dp->o_diroff;
		entryoffsetinblock = dosfs_blkoff(imp, offset);
		if (entryoffsetinblock != 0) {
			if (error = dosfs_blkatoff(dp, offset,
			    (char **)0, &bp))
				goto bad;
		}
		numdirpasses = 2;
	}
	endsearch = roundup(dp->i_size, imp->im_bsize);

	/* dot or dotdot */
	if (nm[0] == '.') {
		if (nmlen == 1)
			dot = 1;
		else if ((nmlen == 2) && (nm[1] == '.'))
			dot = 2;
	}

searchloop:
	while (offset < endsearch) {
		/*
		 * If offset is on a block boundary,
		 * read the next directory block.
		 * Release previous if it exists.
		 */
		if (dosfs_blkoff(imp, offset) == 0) {
			if (bp != NULL)
				brelse(bp);
			if (error = dosfs_blkatoff(dp, offset,
						 (char **)0, &bp))
				goto bad;
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
		if (ep->name[0] == DOS_NAME_EMPTY) break;	/* end of dir */

		if (ep->name[0] == DOS_NAME_DELETED)
			goto next_one;
		
		/*
		 * Dot and dotdot
		 */
		ino_dtou(ep_ino,ep);
		if (dot && (ep->name[0] == '.') &&
		    ((dot == 1 && (ep->name[1] == ' ')) ||
		     (dot == 2 && ep->name[1] == '.' && (ep->name[2] == ' '))))
			goto found;

		if (dosfsfncmp(nm, nmlen, ep,
			       (imp->im_flags & DOSFSMNT_TRANS)))
			goto found;

		/*
		 * Nothing here, keep looking
		 */
next_one:
		offset += sizeof(*ep);
		entryoffsetinblock += sizeof(*ep);
	}
/* notfound: */
	/*
	 * If we started in the middle of the directory and failed
	 * to find our target, we must check the beginning as well.
	 */
	if (numdirpasses == 2) {
		numdirpasses--;
		offset = 0;
		endsearch = dp->o_diroff;
		goto searchloop;
	}

	/*
	 * Before we give up. DOS does not have '.' and '..' entries
	 * in the root directory, and Unix does not like it.  So we
	 * just make believe...
	 */

	if (dot && dp->o_number == DOS_ROOTINO) {
		ep_ino = DOS_ROOTINO;
		offset = 0;
		dot = 1;	/* loop .. back to . */
		goto found;
	}

	DOS_IUNLOCK(dp);
	if (bp != NULL)
		brelse(bp);

	return (ENOENT);

found:

	/*
	 * Save directory entry's inode number and offset, and release
	 * directory buffer.
	 *
	 */
	dp->o_diroff = offset;
	brelse(bp);

	/*
	 * Step through the translation in the name.  We do not `iput' the
	 * directory because we may need it again if a symbolic link
	 * is relative to the current directory.  Instead we save it
	 * unlocked as "pdp".  We must get the target inode before unlocking
	 * the directory to insure that the inode will not be removed
	 * before we get it.  We prevent deadlock by always fetching
	 * inodes from the root, moving down the directory tree. Thus
	 * when following backward pointers ".." we must unlock the
	 * parent directory before getting the requested directory.
	 * There is a potential race condition here if both the current
	 * and parent directories are removed before the `iget' for the
	 * inode associated with ".." returns.  We hope that this occurs
	 * infrequently since we cannot avoid this race condition without
	 * implementing a sophisticated deadlock detection algorithm.
	 * Note also that this simple deadlock detection scheme will not
	 * work if the file system has any hard links other than ".."
	 * that point backwards in the directory structure.
	 */
	pdp = dp;
	if (dot == 2) {
		DOS_IUNLOCK(pdp);	/* race to get the inode */
		if (error = dosfs_iget(dp, ep_ino, &tdp, ep, offset)) {
			return (error);
		}
		vp = DOS_TOV(tdp);
	} else if (dp->o_number == ep_ino) {
		vp = DOS_TOV(dp);	/* we want ourself, ie "." */
		VN_HOLD(vp);
	} else {
		if (error = dosfs_iget(dp, ep_ino, &tdp, ep, offset))
			goto bad;
		DOS_IUNLOCK(dp);
		vp = DOS_TOV(tdp);
	}

	/*
	 * Insert name into cache if appropriate.
	 */
	*vpp = vp;
	dnlc_enter(ITOV(dp), nm, vp, NOCRED);
	return (0);
bad:
	DOS_IUNLOCK(dp);
	return error;
}


/*
 * Return buffer with contents of block "offset"
 * from the beginning of directory "ip".  If "res"
 * is non-zero, fill it in with a pointer to the
 * remaining space in the directory.
 */
public int
dosfs_blkatoff(
	struct dosfs_node	*ip,
	off_t		offset,
	char		**res,
	struct buf	**bpp)
{
	register struct dosfs_mount *imp = ip->i_mnt;
	daddr_t lbn;
	int bsize;
	struct buf *bp;
	int error;

	lbn = dosfs_lblkno (imp, offset);
	bsize = dosfs_blksize (imp, ip, lbn);

trace(dosfs_debug,("Dblkatoff x%x x%x ", ip->o_number, offset));
	*bpp = 0;
	bp = bread(ip->o_devvp, dosfs_bmap(ip, lbn), bsize);
	error = bp->b_error;
	if (error) {
		brelse(bp);
		return (error);
	}
	if (res)
		*res = bp->b_un.b_addr + dosfs_blkoff(imp, offset);
	*bpp = bp;

	return (0);
}


