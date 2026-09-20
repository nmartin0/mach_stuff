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
 * $Log:	isofs_lookup.c,v $
 * Revision 2.2  93/08/07  16:55:54  mrt
 * 	Lint.
 * 	[93/07/30  00:05:15  af]
 * 
 * 	Minor optims for .. and .. cases.
 * 	[93/07/10  19:24:19  af]
 * 
 * 	Changed meaning of inode numbers (see BIGNOTE in the code).
 * 	Handle relocated directories (untested).
 * 	[93/07/04  22:05:37  af]
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
 *	@(#)ufs_lookup.c	7.33 (Berkeley) 5/19/91
 *
 * PATCHES MAGIC                LEVEL   PATCH THAT GOT US HERE
 * --------------------         -----   ----------------------
 * CURRENT PATCH LEVEL:         1       00040
 * --------------------         -----   ----------------------
 *
 * 10 Aug 92    Scott Burris            Fixed "delete from CD-ROM" bug
 */

#include <sys/param.h>
#include <sys/dir.h>
#include <sys/user.h>
#include <sys/buf.h>
#include <sys/inode.h>

#include <vfs/dnlc.h>
#include <isofs/isofs.h>
#include <isofs/isofs_node.h>

extern struct vnode *specvp(), *dnlc_lookup();

#define ISVDEV(t) (((t) == VCHR) || ((t) == VBLK))

/* forward internals */
private int
isofs_dirlook(
	register struct iso_node *dp,	/* the directory we are searching */
	char		*nm,		/* the name we are looking for */
	struct vnode	**vpp,		/* the vnode we return */
	struct ucred	*cred);		/* credentials to use */


/*
 * Convert a component of a pathname into a pointer to a locked inode.
 * This is a very central and rather complicated routine.
 * If the file system is not maintained in a strict tree hierarchy,
 * this can result in a deadlock situation (see comments in code below).
 *
 * The flag argument is LOOKUP, CREATE, RENAME, or DELETE depending on
 * whether the name is to be looked up, created, renamed, or deleted.
 * When CREATE, RENAME, or DELETE is specified, information usable in
 * creating, renaming, or deleting a directory entry may be calculated.
 * If flag has LOCKPARENT or'ed into it and the target of the pathname
 * exists, lookup returns both the target and its parent directory locked.
 * When creating or renaming and LOCKPARENT is specified, the target may
 * not be ".".  When deleting and LOCKPARENT is specified, the target may
 * be "."., but the caller must check to ensure it does an vrele and iput
 * instead of two iputs.
 *
 * Overall outline of ufs_lookup:
 *
 *	check accessibility of directory
 *	look for name in cache, if found, then if at end of path
 *	  and deleting or creating, drop it, else return name
 *	search for name in directory, to found or notfound
 * notfound:
 *	if creating, return locked directory, leaving info on available slots
 *	else return error
 * found:
 *	if at end of path and deleting, return information to allow delete
 *	if at end of path and rewriting (RENAME and LOCKPARENT), lock target
 *	  inode and return info to allow rewrite
 *	if not at end, add name to cache; if at end and neither creating
 *	  nor deleting, add name to cache
 *
 * NOTE: (LOOKUP | LOCKPARENT) currently returns the parent inode unlocked.
 */
public int
isofs_lookup(
	struct vnode	*vdp,
	char		*nm,
	struct vnode	**vpp,
	struct ucred	*cred)
{
	struct vnode *vp;
	register int error;

trace(isofs_debug,("isofs_lookup %s \n", nm));
	error = isofs_dirlook(VTO_ISO(vdp), nm, &vp, cred);
	if (error == 0) {
		*vpp = vp;
		ISO_IUNLOCK(VTO_ISO(vp));
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

int dorr = 1;

private int
isofs_dirlook(
	register struct iso_node *dp,	/* the directory we are searching */
	char		*nm,		/* the name we are looking for */
	struct vnode	**vpp,		/* the vnode we return */
	struct ucred	*cred)		/* credentials to use */
{
	struct vnode *vp;
	register struct iso_mnt *imp;	/* file system that directory is in */
	struct buf *bp = 0;		/* a buffer of directory entries */
	register struct iso_directory_record *ep;
					/* the current directory entry */
	int entryoffsetinblock;		/* offset of ep in bp's buffer */
	enum {NONE, COMPACT, FOUND} slotstatus;
	int slotoffset = -1;		/* offset of area with free space */
	int slotsize;			/* size of area at slotoffset */
	int slotfreespace;		/* amount of space free in slot */
	int slotneeded;			/* size of the entry we're seeking */
	int numdirpasses;		/* strategy for directory search */
	int endsearch;			/* offset to end directory search */
	struct iso_node *pdp;		/* saved dp during symlink work */
	struct iso_node *tdp;		/* returned by iget */
	int error, doRR;
	u_int ep_ino;
	off_t offset;
	int dot = 0;

	int reclen;
	int nmlen;			/* length of target name */
	int namelen;			/* length of current candidate */

	imp = dp->i_mnt;

trace(isofs_debug,("isofs_dirlook "));
	/*
	 * Check accessiblity of directory.
	 */
	if ((dp->iso_flags & ISO_FF_DIRECTORY) == 0)
		return (ENOTDIR);

	/*
	 * We now have a segment name to search for, and a directory to search.
	 *
	 * Before tediously performing a linear scan of the directory,
	 * check the name cache to see if the directory/name pair
	 * we are looking for is known already.
	 */
	vp = dnlc_lookup(ISO_TOV(dp), nm, cred);
	if (vp) {
		VN_HOLD(vp);
		*vpp = vp;
		ISO_ILOCK(VTO_ISO(vp));
		return (0);
	}

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
	ISO_ILOCK(dp);
	if (dp->o_diroff > dp->i_size) {
		dp->o_diroff = 0;
	}
	if (dp->o_diroff == 0) {
		offset = 0;
		numdirpasses = 1;
	} else {
		offset = dp->o_diroff;
		entryoffsetinblock = iso_blkoff(imp, offset);
		if (entryoffsetinblock != 0) {
			if (error = iso_blkatoff(dp, offset,
			    (char **)0, &bp))
				goto bad;
		}
		numdirpasses = 2;
	}
	endsearch = roundup(dp->i_size, imp->logical_block_size);

	doRR = (dp->i_mnt->im_flags & ISOFSMNT_NORR) == 0;

	/* dot or dotdot */
	nmlen = strlen(nm);
	if (nm[0] == '.') {
		if (nmlen == 1)
			dot = 1;
		else if (nmlen == 2 && nm[1] == '.')
			dot = 2;
	}

searchloop:
	while (offset < endsearch) {
		/*
		 * If offset is on a block boundary,
		 * read the next directory block.
		 * Release previous if it exists.
		 */
		if (iso_blkoff(imp, offset) == 0) {
			if (bp != NULL)
				brelse(bp);
			if (error = iso_blkatoff(dp, offset,
						 (char **)0, &bp))
				goto bad;
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
			offset =
				roundup (offset,
					 imp->logical_block_size);
			continue;
		}

		if (reclen < sizeof (struct iso_directory_record))
			/* illegal entry, stop */
			break;

/* 10 Aug 92*/	if (entryoffsetinblock + reclen -1 >= imp->logical_block_size)
			/* entries are not allowed to cross boundaries */
			break;

		/*
		 * Check for a name match.
		 */
		namelen = isonum_711 (ep->name_len);

		if (reclen < sizeof (struct iso_directory_record) + namelen)
			/* illegal entry, stop */
			break;

		/*
		 * Dot and dotdot
		 */
		if ((namelen == 1) && dot && (ep->name[0] == (dot - 1)))
			goto found;

		/*
		 * If RockRidge, look for true name or compare exactly
		 */
		if (doRR) {
			char *nnm = 0; int nl = 0, r;
			
			r = rock_ridge_get_filename(ep, &nnm, &nl, dp);
			if (r == 1) {
				int gotit;
				gotit = (nmlen == nl);
				if (gotit) gotit = (bcmp(nnm,nm,nl) == 0);
				free(nnm);
				if (gotit) goto found;
			} else
			if (r == -1)
				goto next_one;
			else
			if (bcmp(nm,ep->name,nmlen) == 0)
				goto found;
		} else
		/*
		 * No RockRidge, compare with case-translation
		 */
		if ((namelen >= nmlen)
			 && isofncmp(nm, nmlen, ep->name, namelen))
				goto found;

		/*
		 * Nothing here, keep looking
		 */
next_one:
		offset += reclen;
		entryoffsetinblock += reclen;
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

	ISO_IUNLOCK(dp);
	if (bp != NULL)
		brelse(bp);

	return (ENOENT);

found:

	/*
	 * Save directory entry's inode number and offset, and release
	 * directory buffer.
	 *
	 * BIGNOTE: This is the one and only place where we know what
	 * inode numbers are and where they come from in this filesystem.
	 * They do not come from data on disk, we just make them up.
	 *
	 * One option is to just take their 'extent' e.g. logical block
	 * number.  But it does not work with RockRidge symlinks, they
	 * have no unique extent associated with them.
	 * Another option is then to let the RR code decide, by parsing
	 * the RR records, what unique number to come up with.
	 *
	 * We take a middle-ground approach, which should still work
	 * with other future extensions as well.  The high bits of the
	 * inum are extent, and for regular files and directories
	 * the low bits are zero.  For symlinks, the high bits are the
	 * extent of the directory where the symlink lives, the low bits
	 * are a function of the directory entry (e.g. offset in extent).
	 * A specific extension only has to tell if this is a symlink or not.
	 *
	 */
#define	isofs_makeinum(_m_,_b_,_o_)	(iso_lblktosize(_m_,_b_) + (_o_))
	{
		int isa_symlink = rock_ridge_isa_symlink(ep, dp);
		ep_ino = (isa_symlink) ?
			    isofs_makeinum(imp, dp->iso_extent, offset)
			  : isofs_makeinum(imp,	isonum_733 (ep->extent), 0);
#undef	isofs_makeinum
	}
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
		ISO_IUNLOCK(pdp);	/* race to get the inode */
		if (error = iso_iget(dp, ep_ino, &tdp, ep, offset)) {
			return (error);
		}
		vp = ISO_TOV(tdp);
	} else if (dp->o_number == ep_ino) {
		vp = ISO_TOV(dp);	/* we want ourself, ie "." */
		VN_HOLD(vp);
	} else {
		if (error = iso_iget(dp, ep_ino, &tdp, ep, offset))
			goto bad;
		ISO_IUNLOCK(dp);
		vp = ISO_TOV(tdp);
	}

	/*
	 * Insert name into cache if appropriate.
	 */
	*vpp = vp;
	dnlc_enter(ITOV(dp), nm, vp, NOCRED);
	return (0);
bad:
	ISO_IUNLOCK(dp);
	return error;
}


/*
 * Return buffer with contents of block "offset"
 * from the beginning of directory "ip".  If "res"
 * is non-zero, fill it in with a pointer to the
 * remaining space in the directory.
 */
public int
iso_blkatoff(
	struct iso_node	*ip,
	off_t		offset,
	char		**res,
	struct buf	**bpp)
{
	register struct iso_mnt *imp = ip->i_mnt;
	daddr_t lbn;
	int bsize;
	struct buf *bp;
	int error;

	lbn = iso_lblkno (imp, offset);
	bsize = iso_blksize (imp, ip, lbn);

trace(isofs_debug,("iso_blkatoff x%x x%x", offset, lbn));
	*bpp = 0;
	bp = bread(ISO_TOV(ip), iso_bmap(ip, lbn), bsize);
	error = bp->b_error;
	if (error) {
		brelse(bp);
		return (error);
	}
	if (res)
		*res = bp->b_un.b_addr + iso_blkoff(imp, offset);
	*bpp = bp;

	return (0);
}


