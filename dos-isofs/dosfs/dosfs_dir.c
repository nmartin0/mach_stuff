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
 * $Log:	dosfs_dir.c,v $
 * Revision 2.2  93/09/15  13:29:20  mrt
 * 	Removed vestigial hard-link refcounting code.
 * 	Enabled all protection checks, now that we have our extensions.
 * 	Removed silly UFS three-phase commit code for file creation.
 * 	Rename must update the diroff&dirextent fields.
 * 	Use new function dosfs_itdiren() where appropriate.
 * 	[93/09/13  23:54:32  af]
 * 
 * 	Fixed dirbadname().
 * 	[93/08/26  16:35:49  af]
 * 
 * 	Created.
 * 	[93/07/13            af]
 * 
 */

#include <strings.h>
#include <sys/param.h>
#include <sys/dir.h>
#include <sys/user.h>
#include <sys/buf.h>
#include <sys/inode.h>
#include <sys/table.h>	/*  UMODE_P_GID */

#include <vfs/dnlc.h>
#include <dosfs/dosfs.h>
#include <dosfs/dosfs_node.h>

#define ISVDEV(t) (((t) == VCHR) || ((t) == VBLK))

/*
 * If dircheckforname fails to find a name, this structure holds
 * state for direnter as to where there is space for an entry.
 * If dircheckforname succeeds then this structure holds state
 * for dirrename and dirremove as to where the entry is.
 * After dircheckforname succeeds the values are:
 *	status	offset		bp, ep
 *	------	------		------
 *	NONE	end of dir	not valid
 *	FOUND	start of entry	not valid
 *	EXIST	start of entry	valid
 * On success, dirprepareentry makes bp and ep valid.
 */
struct slot {
	enum	{NONE, FOUND, EXIST} status;
	struct dosfs_directory_record *ep;	/* pointer to (free) entry */
	int	offset;				/* offset in dir where found */
	struct buf *bp;				/* dir buf where entry is */
};

/*
 * Write a new directory entry.
 * The directory must not have been removed and must be writeable.
 * There are three operations in building the new entry: creating a file
 * or directory (DE_CREATE), renaming (DE_RENAME) or linking (DE_LINK).
 * Since DOS does not have hard links only the first two apply.
 * There are four possible cases to consider:
 *	Name
 *	found	op			action
 *	----	---			-------------------------------
 *	no	DE_CREATE		create file according to vap and enter
 *	no	DE_RENAME		enter the file sip
 *	yes	DE_CREATE		error EEXIST *ipp = found file
 *	yes	DE_RENAME		remove existing file, enter new file
 */
public int
dosfs_direnter(
	register struct dosfs_node *tdp,/* target directory to make entry in */
	register char		*namep,	/* name of entry */
	enum de_op 		op,	/* entry operation */
	register struct dosfs_node *sdp,/* source inode parent if rename */
	struct dosfs_node	*sip,	/* source inode if link/rename */
	struct vattr		*vap,	/* attributes if new inode needed */
	struct dosfs_node	**ipp,	/* return entered inode (locked) here */
	struct ucred		*cred)	/* credentials to use */
{
	struct dosfs_node *tip;		/* inode of (existing) target file */
	struct slot slot;		/* slot info to pass around */
	register int namlen;		/* length of name */
	register int error;		/* error number */
	struct dosfs_mount *imp;	/* used to get translation type */

trace(dosfs_debug,("Ddirent %x %s %x ", tdp->o_number, namep, op));

	if (ipp)
		*ipp = NULL;

	if (op == DE_LINK)
		return EINVAL;

	namlen = strlen(namep);
	/*
	 * If name is "." or ".." then if this is a create look it up
	 * and return EEXIST. Rename or link TO "." or ".." is forbidden.
	 */
	if (namlen == 1 && namep[0] == '.' ||
	    namlen == 2 && namep[0] == '.' && namep[1] == '.') {
		if (op == DE_RENAME) {
			return (ENOTEMPTY);
		}
		if (ipp) {
			if (error = dosfs_dirlook(tdp, namep,
					(struct vnode **) ipp, cred))
				return (error);
		}
		return (EEXIST);
	}

	imp = tdp->i_mnt;
	if (dosfs_dirbadname(namep, namlen, (imp->im_flags & DOSFSMNT_TRANS)))
		return (EPERM);

	slot.status = NONE;
	slot.bp = NULL;
	/*
	 * For rename lock the source entry and check the link count to
	 * see if it has been removed while it was unlocked.
	 */
	if (op != DE_CREATE) {
		DOS_ILOCK(sip);
		if (sip->o_nlink == 0) {
			DOS_IUNLOCK(sip);
			return (ENOENT);
		}
		DOS_IUNLOCK(sip);
	}
	/*
	 * lock the directory in which we are trying to make the new entry.
	 */
	DOS_ILOCK(tdp);
	/*
	 * Check accessiblity of directory.
	 */
	if ((tdp->o_mode&IFMT) != IFDIR) {
		error = ENOTDIR;
		goto out;
	}
	/*
	 * If target directory has not been removed, then we can consider
	 * allowing file to be created.
	 */
	if (tdp->o_nlink == 0) {
		error = ENOENT;
		goto out;
	}
	/*
	 * Execute access is required to search the directory.
	 */
	if (error = dosfs_iaccess(tdp, IEXEC)) {
		goto out;
	}
	/*
	 * If this is a rename and we are doing a directory and the parent
	 * is different (".." must be changed), then the source directory must
	 * not be in the directory heirarchy above the target, as this would
	 * orphan everything below the source directory. Also the user must
	 * have write permission in the source so as to be able to change "..".
	 */
	if ((op == DE_RENAME) &&
	    ((sip->o_mode&IFMT) == IFDIR) && (sdp != tdp)) {
		if (error = dosfs_iaccess(sip, IWRITE))
			goto out;
		if (error = dosfs_dircheckpath(sip, tdp))
			goto out;
	}
	/*
	 * search for the entry
	 */
	error = dosfs_dircheckforname(tdp, namep, namlen, &slot, &tip);
	if (error) {
		goto out;
	}

	if (tip) {
		switch (op) {

		case DE_CREATE:
			if (ipp) {
				*ipp = tip;
				error = EEXIST;
			} else {
				dosfs_iput(tip);
			}
			break;

		case DE_RENAME:
			if (sip->o_number == tip->o_number) {
				/*
				 * Short circuit rename (foo, foo).
				 */
				error = ESAME;
			} else {
				error =
				    dosfs_dirrename(sdp, sip, tdp, namep, namlen,
					 tip, &slot);
			}
			dosfs_iput(tip);
			break;
		}
	} else {
		/*
		 * The entry does not exist. Check write permission in
		 * directory to see if entry can be created.
		 */
		if (error = dosfs_iaccess(tdp, IWRITE)) {
			goto out;
		}
		/*
		 * When renaming an existing entry to a new entry, and 
	 	 * the parent directory is "sticky", then the user must
		 * own the parent directory or the file to be renamed, or
		 * the user must be root.
		 */
		if (op == DE_RENAME && (sdp->o_mode & ISVTX) && u.u_uid != 0 &&
			u.u_uid != sdp->o_uid && u.u_uid != sip->o_uid) {
			error = EPERM;
			goto out;
		}

		if (op == DE_CREATE) {
			/*
			 * make a new inode and directory as required
			 */
			error = dosfs_dirmakeinode(tdp, &sip, vap, &slot);
			if (error) {
				goto out;
			}
		}
		error = dosfs_diraddentry(tdp, namep, namlen,
					  &slot, sip, sdp, vap);
		if (error) {
			if (op == DE_CREATE) {
				/*
				 * unmake the inode we just made
				 */
				sip->o_nlink = 0;
				sip->i_flag |= ICHG;	/* not imark() */
				dosfs_irelease(sip);
				sip = NULL;
			}
		} else if (ipp) {
			DOS_ILOCK(sip);
			*ipp = sip;
		} else if (op == DE_CREATE) {
			dosfs_irelease(sip);
		}
	}

out:
	if (slot.bp)
		brelse(slot.bp);
#if	MACH_NBC
	if (sip != NULL && sip->o_nlink == 0)
		inode_uncache(DOS_TOV(sip));
#endif	MACH_NBC
	DOS_IUNLOCK(tdp);
	return (error);
}

/*
 * Delete a directory entry
 * If oip is nonzero the entry is checked to make sure it still reflects oip.
 */
public int
dosfs_dirremove(
	register struct dosfs_node *dp,
	char			   *namep,
	struct dosfs_node	   *oip,
	int			   rmdir)
{
	register struct dosfs_directory_record *ep;
	struct direct *pep;
	struct dosfs_node *ip;
	int namlen = strlen(namep);	/* length of name */
	struct slot slot;
	int error = 0;

trace(dosfs_debug,("Ddirrem %x %s %x ", dp->o_number, namep, rmdir));
	/*
	 * return error when removing . and ..
	 */
	if (namlen == 1 && namep[0] == '.')
		return (EINVAL);
	if (namlen == 2 && namep[0] == '.' && namep[1] == '.')
		return (ENOTEMPTY);

	ip = NULL;
	slot.bp = NULL;
	DOS_ILOCK(dp);
	/*
	 * Check accessiblity of directory.
	 */
	if ((dp->o_mode&IFMT) != IFDIR) {
		error = ENOTDIR;
		goto out;
	}

	/*
	 * Execute access is required to search the directory.
	 * Access for write is interpreted as allowing
	 * deletion of files in the directory.
	 */
	if (error = dosfs_iaccess(dp, IEXEC|IWRITE)) {
		goto out;
	}

	slot.status = FOUND;	/* don't need to look for empty slot */
	ip = (struct dosfs_node *)0;
	error = dosfs_dircheckforname(dp, namep, namlen, &slot, &ip);
	if (error) {
		goto out;
	}
	if (ip == (struct dosfs_node *) 0) {
		error = ENOENT;
		goto out;
	}
	if (oip && oip != ip) {
		error = ENOENT;
		goto out;
	}
	/*
	 * If the parent directory is "sticky", then the user must own
	 * either the parent directory or the file to be removed, or the
	 * user must be root. This implements append-only directories.
	 */
	if ((dp->o_mode & ISVTX) && u.u_uid != 0 &&
	    u.u_uid != dp->o_uid && ip->o_uid != u.u_uid) {
		error = EPERM;
		goto out;
	}

	/* There used to be a check here to make sure you are not removing a
	 * a mounted on dir.  This was no longer correct because iget() does
	 * not cross mount points anymore so the the o_dev fields in the inodes
	 * pointed to by ip and dp will never be different.  There does need
	 * to be a check here though, to eliminate the race between mount and 
	 * rmdir (It can also be a race between mount and unlink, if your 
	 * kernel allows you to unlink a directory.)  
	 */

	if (DOS_TOV(ip)->v_vfsmountedhere != (struct vfs *)0) {
		error = EBUSY;
		goto out;
	}


	/*
	 * If the inode being removed is a directory, we must be
	 * sure it only has entries "." and "..".
	 */
	if (rmdir && (ip->o_mode&IFMT) == IFDIR) {
		if (!dosfs_dirempty(ip)) {
			error = ENOTEMPTY;
			goto out;
		}
	}
	/*
	 * Mark entry as deleted.  Should eventually reclaim
	 * room, e.g. if last and only entry in a block. xxx
	 */
	ep = slot.ep;
	ep->name[0] = DOS_NAME_DELETED;
	bwrite(slot.bp);
	slot.bp = NULL;

	/*
	 * Remove the cache'd entry, if any.
	 * Do it after we let the buffer go,
	 * because iupdat might need it.
	 */
	dnlc_remove(DOS_TOV(dp), namep);

	if (rmdir && (ip->o_mode & IFMT) == IFDIR) {
		dnlc_remove(DOS_TOV(ip), ".");
		dnlc_remove(DOS_TOV(ip), "..");
	}
	/*
	 * Remove the inode iff we must.
	 */
	if (oip == (struct dosfs_node *)0)
		ip->o_nlink = 0;
out:
	if (slot.bp)
		brelse(slot.bp);
	if (ip) {
		if (ip->o_nlink == 0) {
#if	MACH_NBC
			/*
			 * Remove it from the cache.
			 */
			inode_uncache(DOS_TOV(ip));
#endif	MACH_NBC
			dosfs_igone(ip);
		}
		dosfs_iput(ip);
	}
#if	MACH_NBC
	if (dp->o_nlink == 0)
		inode_uncache(DOS_TOV(dp));
#endif	MACH_NBC
	DOS_IUNLOCK(dp);
	return (error);

}

/*
 * Check name is ok for a dos file/directory
 */
public int
dosfs_dirbadname(
	register char	*nm,
	register int	l,
	register int	dotrans)
{
	register char c;

trace(dosfs_debug,("Ddirbadn %s ", nm));
	if (dotrans & DOSFSMNT_MDOT_TR) {
		if ((l > 12) || (l > 8 && nm[8] != '.'))
			return -1;
	} else
		if (l > 11)
			return -1;
	
	/* Like Unix, no nulls or high bits (mumble..) */
	while (l--) {
		c = *nm++;
		if ((c == 0) || (c & 0x80)) {
			return -1;
		}
	}
	return (*nm);
}

/*
 * Perform consistency checks on a directory
 */
public int
dosfs_dirmangled(
	struct dosfs_node	*dp,
	struct dosfs_directory_record *ep,
	off_t			block_offset)
{
	/* If we had some info available. But. */
	return 0;
}

/*
 * Complain about a bad directory
 */
dosfs_dirbad(
	struct dosfs_node	*ip,
	char			*how,
	int			offset)
{
	printf("%s: bad dir ino x%x at offset %d: %s\n",
	    ip->i_mnt->im_path, ip->o_number, offset, how);
}

/*
 * Check if the source directory SIP is in the path
 * of the target directory TDP.
 * Target is supplied locked, source is unlocked.
 * The target is always relocked before returning.
 */
public int
dosfs_dircheckpath(
	struct dosfs_node       *sip,
	struct dosfs_node	*tdp)
{
	struct buf			*bp;
	struct dosfs_directory_record	*dirp;
	struct dosfs_node		*ip;
#define RENAME_IN_PROGRESS	0x01
#define RENAME_WAITING		0x02
	static int serialize_flag = 0;
	int error = 0;

trace(dosfs_debug,("Ddirckp %x %x ", sip->o_number, tdp->o_number));
	/*
	 * If two renames of directories were in progress at once, the partially
	 * completed work of one dircheckpath could be invalidated by the other
	 * rename.  To avoid this, all directory renames in the system are
	 * serialized.
	 */
	while (serialize_flag & RENAME_IN_PROGRESS) {
		serialize_flag |= RENAME_WAITING;
		(void) sleep((caddr_t) &serialize_flag, PINOD);
	}
	serialize_flag = RENAME_IN_PROGRESS;
	ip = tdp;
	if (ip->o_number == sip->o_number) {
		error = EINVAL;
		goto out;
	}
	if (ip->o_number == DOS_ROOTINO)
		goto out;

	bp = 0;
	for (;;) {
		ino_t	dotdot_ino;

		if (((ip->o_mode&IFMT) != IFDIR) ||
		    (ip->o_nlink == 0) ||
		    (ip->i_size < sizeof(struct dosfs_directory_record))) {
			dosfs_dirbad(ip, "bad size, unlinked or not dir", 0);
			error = ENOTDIR;
			break;
		}
		if (bp) {
			brelse(bp);
		}
		error = dosfs_blkatoff(ip, (off_t)0, (char **)&dirp, &bp);
		if (error)
			break;

		/*
		 * .. is guaranteed to be there, second entry.
		 */
		dirp++;
		if (dirp->name[0] != '.' || dirp->name[1] != '.' ||
		    dirp->name[2] != ' ' || dirp->ext[0] != ' ') {
			dosfs_dirbad(ip, "mangled .. entry", 0);
			error = ENOTDIR;
			break;
		}

		ino_dtou(dotdot_ino,dirp);

		if (dotdot_ino == DOS_ROOTINO)
			break;

		if (dotdot_ino == sip->o_number) {
			error = EINVAL;
			break;
		}
		if (ip != tdp) {
			dosfs_iput(ip);		/* XXX should be zfree ? XXX */
		} else {
			DOS_IUNLOCK(ip);
		}
		/*
		 * o_dev, extent and ih_fs are still valid after dosfs_iput
		 * This is a race to get ".." just like dirlook.
		 * But I dont like it.
		 */
		dosfs_iget( ip, dotdot_ino, &ip, dirp, sizeof(*dirp));
		if (error)
			break;
	}
	if (bp)
		brelse(bp);
out:
	if (ip) {
		if (ip != tdp) {
			dosfs_iput(ip);
			/*
			 * Relock target and make sure it has not gone away
			 * while it was unlocked.
			 */
			DOS_ILOCK(tdp);
			if ((error == 0) && (tdp->o_nlink == 0)) {
				error = ENOENT;
			}
		}
	}
	/*
	 * unserialize
	 */
	{
		register int t = serialize_flag;

		serialize_flag = 0;
		if (t & RENAME_WAITING)
			wakeup((caddr_t) &serialize_flag);
	}

	return (error);
}

/*
 * Check for the existence of a slot to make a directory entry.
 * On successful return *ipp points at the (locked) inode found.
 * The target directory inode (tdp) is supplied locked.
 * This may not be used on "." or "..", but aliases of "." are ok.
 */
public int
dosfs_dircheckforname(
	struct dosfs_node	*tdp,	/* inode of directory being checked */
	char			*namep,	/* name we're checking for */
	int			namlen,	/* length of name */
	struct slot		*slotp,	/* slot structure */
	struct dosfs_node	**ipp)	/* return inode if we find one */
{
	int dirsize;			/* size of the directory */
	struct buf *bp;			/* pointer to directory block */
	register int entryoffsetinblock;/* offset of ep in bp's buffer */
	register struct dosfs_directory_record *ep;	/* directory entry */
	register int offset;		/* offset in the directory */
	struct dosfs_node *ip;
	int error, dotrans;

trace(dosfs_debug,("Ddirckfn %x %s ", tdp->o_number, namep));
	bp = NULL;
	entryoffsetinblock = 0;
	dotrans = tdp->i_mnt->im_flags & DOSFSMNT_TRANS;
	/*
	 * Must search whole directory
	 */
	dirsize = tdp->i_size;
	offset = 0;
	while (offset < dirsize) {
		/*
		 * If offset is on a block boundary,
		 * read the next directory block.
		 * Release previous if it exists.
		 */
		if (dosfs_blkoff(tdp->i_mnt, offset) == 0) {
			if (bp != NULL)
				brelse(bp);
			if (error = dosfs_blkatoff(tdp, offset,
						 (char **)0, &bp))
				return error;
			entryoffsetinblock = 0;
		}
		ep = (struct dosfs_directory_record *)
			(bp->b_un.b_addr + entryoffsetinblock);

		if (dosfs_dirmangled(tdp, ep, entryoffsetinblock))
			goto next_one;

		/*
		 * If an appropriate sized slot has not yet been found,
		 * check to see if this one is available.
		 */
		{
			register unsigned int	c = ep->name[0];

			if (c == DOS_NAME_EMPTY || c == DOS_NAME_DELETED) {
			    if (slotp->status != FOUND) {

				slotp->status = FOUND;
				slotp->offset = offset;
			    }
			    goto next_one;
			}
		}
		/*
		 * Check for a name match.
		 */
		if (dosfsfncmp(namep, namlen, ep, dotrans)) {
			ino_t	ep_ino;

			tdp->o_diroff = offset;
			ino_dtou(ep_ino,ep);

			if (tdp->o_number == ep_ino) {
				*ipp = tdp;	/* we want ourself, ie "." */
				VN_HOLD(DOS_TOV(tdp));
			} else {
				error = dosfs_iget(tdp, ep_ino, &ip, ep, offset);
				if (error) {
					brelse(bp);
					return (error);
				}
				*ipp = ip;
			}
			slotp->status = EXIST;
			slotp->offset = offset;
			slotp->bp = bp;
			slotp->ep = ep;
			return (0);
		}
next_one:
		offset += sizeof *ep;
		entryoffsetinblock += sizeof *ep;
	}
	if (bp) {
		brelse(bp);
	}
	if (slotp->status == NONE) {
		slotp->offset = offset;
	}
	*ipp = (struct dosfs_node *)0;
	return (0);
}

/*
 * Rename the entry in the directory TDP so that
 * it points to SIP instead of TIP.
 */
public int
dosfs_dirrename(
	struct dosfs_node	*sdp,	/* parent directory of source */
	struct dosfs_node	*sip,	/* source inode */
	struct dosfs_node	*tdp,	/* parent directory of target */
	char			*namep,	/* entry we are trying to change */
	int			namlen,	/* length of entry string */
	struct dosfs_node	*tip,	/* locked target inode */
	struct slot		*slotp)	/* slot for entry */
{
	int error;
	int doingdirectory;
	int parentdifferent;

trace(dosfs_debug,("Ddirren %x %s %x ", tdp->o_number, namep, tip->o_number));
	/*
	 * Must have write permission to rewrite target entry.
	 */
	if (error = dosfs_iaccess(tdp, IWRITE)) {
		return (error);
	}
	/*
	 * If the parent directory is "sticky", then the user must
	 * own the parent directory or the file to be renamed 
	 * and the target, or the user must be root. 
	 * This implements append-only directories.
	 */
	if ((sdp->o_mode & ISVTX) && u.u_uid != 0 &&
	    u.u_uid != sdp->o_uid && (u.u_uid != sip->o_uid ||
	    u.u_uid != tip->o_uid)) {
		return (EPERM);
	}

	doingdirectory = ((sip->o_mode & IFMT) == IFDIR);
	parentdifferent = (sdp != tdp);
	/*
	 * Check that everything is on the same filesystem.
	 */
	if ((tip->i_vnode.v_vfsp != tdp->i_vnode.v_vfsp) ||
	    (tip->i_vnode.v_vfsp != sip->i_vnode.v_vfsp)) {
		return (EXDEV);		/* XXX archaic */
	}
	/*
	 * Ensure source and target are compatible
	 * (both directories or both not directories).
	 * If target is a directory it must be empty
	 * and have no links to it.
	 */
	if ((tip->o_mode & IFMT) == IFDIR) {
		/*
		 * Target is a dir. Source must be a dir and
		 * target must be empty.
		 */
		if (!doingdirectory) {
			error = ENOTDIR;
			goto bad;
		}
		if (!dosfs_dirempty(tip)) {
			error = ENOTEMPTY;
			goto bad;
		}
	} else if (doingdirectory) {
		/*
		 * Source is a dir and target is not.
		 */
		error = ENOTDIR;
		goto bad;
	}
	/*
	 * Rewrite the inode pointer for target name entry
	 * from the target inode (ip) to the source inode (sip).
	 * This prevents the target entry from disappearing
	 * during a crash. Mark the directory inode to reflect the changes.
	 */
	dnlc_remove(DOS_TOV(tdp), namep);
	sip->dosfs_diroff = slotp->offset;
	sip->dosfs_dirextent = tdp->extent;
	(void) dosfs_itdiren(sip,slotp->ep, sip->i_mnt->im_flags & DOSFSMNT_MUSER);
	dnlc_enter(DOS_TOV(tdp), namep, DOS_TOV(sip), NOCRED);
	bwrite(slotp->bp);
	slotp->bp = NULL;

	/*
	 * The target inode is gone.
	 * Fix the ".." entry in sip to point to dp.
	 * This is done after the new entry is on the disk.
	 */
	tip->o_nlink--;
	if (doingdirectory) {
		/*
		 * Decrement target link count once more if it was a directory.
		 */
		if (--tip->o_nlink != 0) {
			panic("direnter: target directory link count");
		}
		/*
		 * Renaming a directory with the parent different requires
		 * ".." to be rewritten. The window is still there for ".."
		 * to be inconsistent, but this is unavoidable.
		 */
		if (parentdifferent) {
			error = dosfs_dirfixdotdot(sip, sdp, tdp);
			if (error) {
				goto bad;
			}
		}
	}
bad:
	if (tip->o_nlink != 0) printf("dirren link %d\n", tip->o_nlink);
#if	MACH_NBC
#else	MACH_NBC
	if ((tip->i_flag & ITEXT) == ITEXT)
#endif	MACH_NBC
		inode_uncache(DOS_TOV(tip));
	dosfs_igone(tip);
	return (error);
}

/*
 * Allocate and initialize a new inode that will go
 * into directory TDP.  Extend TDP if necessary.
 */
public int
dosfs_dirmakeinode(
	struct dosfs_node	*tdp,
	struct dosfs_node	**ipp,
	struct vattr		*vap,
	struct slot		*slotp)
{
	struct dosfs_node *ip;
	u_short type;
	int imode;			/* mode and format as in inode */
	int error = 0;
	struct dosfs_mount	*imp = tdp->i_mnt;

trace(dosfs_debug,("\ndosfs_dirmakeinode %x ", tdp->o_number));
	/*
	 * Sanity checks
	 */
	if (vap == (struct vattr *) 0) {
		panic("dosfs_dirmakeinode: no attributes\n");
	}
	type = (vap->va_mode)&VFMT;
	if (type == VFMT) {
		uprintf("Use dosbadsect for bad blocks\n");
		return EINVAL;
	}

	/*
	 * Allocate a new inode.
	 */
	imode = vap->va_mode;
	error = dosfs_ialloc(tdp, type, imode, &ip, slotp->offset, vap);
	if (error)
		return error;

	imark(ip, IACC|IUPD|ICHG);
	ip->o_mode = imode;

	if (type == VBLK || type == VCHR) {
		ip->i_vnode.v_rdev = vap->va_rdev;
	}

	ip->i_vnode.v_mode = (ip->i_vnode.v_mode&~VFMT) | type;
	if (type == VDIR) {
		ip->o_nlink = 2;
	} else {
		ip->o_nlink = 1;
	}
	ip->o_uid = u.u_uid;
	if ((u.u_modes & UMODE_P_GID) == 0) {
		ip->o_gid = u.u_gid;
	} else {
		ip->o_gid = tdp->o_gid;
	}
	if ((ip->o_mode & ISGID) && !groupmember(ip->o_gid)) {
		ip->o_mode &= ~ISGID;
	}
	/*
	 * Make sure the entry pointing to the inode goes
	 * to disk.
	 * Then unlock inode, since nothing points to it yet.
	 */
	if (type == VDIR) {
		error = dosfs_dirmakedirect(ip, tdp);
	}
	if (error) {
		ip->o_nlink = 0;
		ip->i_flag |= ICHG;		/* not imark() */
		dosfs_iput(ip);
	} else {
		dosfs_iupdat(ip, 1);
		DOS_IUNLOCK(ip);
		*ipp = ip;
	}
	return (error);
}

/*
 * Enter the file sip in the directory tdp with name namep.
 */
public int
dosfs_diraddentry(
	struct dosfs_node	*tdp,
	char			*namep,
	int			namlen,
	struct slot		*slotp,
	struct dosfs_node	*sip,
	struct dosfs_node	*sdp,
	struct vattr		*vap)
{
	int error;
	register struct dosfs_directory_record	*ep;

trace(dosfs_debug,("Ddiradd %x %s %x ", tdp->o_number, namep, sip->o_number));
	/*
	 * Prepare a new entry. Grabs a buffer in slotp->bp.
	 */
	error = dosfs_dirprepareentry(tdp, slotp);
	if (error) {
		return (error);
	}
	/*
	 * Check inode to be linked to see if it is in the
	 * same filesystem.
	 */
	if (tdp->i_vnode.v_vfsp != sip->i_vnode.v_vfsp) {
		error = EXDEV;
		goto bad;
	}
	if ((sip->o_mode & IFMT) == IFDIR) {
		error = dosfs_dirfixdotdot(sip, sdp, tdp);
		if (error) {
			goto bad;
		}
	}
	/*
	 * Fill in entry data
	 */
	ep = slotp->ep;
	dosfsfntodos(ep, namep, namlen, 
		     (tdp->i_mnt->im_flags & DOSFSMNT_TRANS));
	dosfs_dosdate(sip->o_ctime,ep->time,ep->date);
	ep->attr = sip->attr;
	dosfs_itdiren(sip,ep,tdp->i_mnt->im_flags & DOSFSMNT_MUSER);
	
	dnlc_enter(ITOV(tdp), namep, ITOV(sip), NOCRED);
	/*
	 * Write out the directory entry.
	 */
	bwrite(slotp->bp);
	slotp->bp = NULL;
	/*
	 * Mark the directory inode to reflect the changes.
	 */
	tdp->o_diroff = 0;
	return (0);

bad:
	/*
	 * Clear out entry prepared by dirprepareent.
	 */
	slotp->ep->name[0] = DOS_NAME_DELETED;
	bwrite(slotp->bp);
	slotp->bp = NULL;
	return (error);
}

/*
 * Prepare a directory slot to receive an entry.
 */
dosfs_dirprepareentry(
	register struct dosfs_node *dp,	/* directory we are working in */
	register struct slot *slotp)	/* available slot info */
{
	struct dosfs_mount	*imp = dp->i_mnt;
	int error;

trace(dosfs_debug,("Ddirprep %x ", dp->o_number));
	/*
	 * If we didn't find a slot, then indicate that the
	 * new slot belongs at the end of the directory.
	 * If we found a slot, then the new entry can be
	 * put at slotp->offset.
	 */
	if (slotp->status == NONE) {
		if (dosfs_blkoff(imp,slotp->offset)) {
			panic("dosfs_dirprepareentry: new block");
		}
		/*
		 * Allocate a new block.  Cannot do this on root, sigh.
		 */
		if (dp->o_number == DOS_ROOTINO)
			return ENOSPC;
		error = dosfs_iextend(dp, dp->i_size + imp->im_bsize,
				 dosfs_lblkno(imp,dp->i_size));
		if (error)
			return (error);
	}
	/*
	 * Get the block containing the new directory entry.
	 */
	error = dosfs_blkatoff(dp, slotp->offset, (char **)&slotp->ep, &slotp->bp);
	if (error)
		return error;

	switch (slotp->status) {
	case NONE:
		/*
		 * No space in the directory. slotp->offset will be on a
		 * directory block boundary and we will write the new entry
		 * into a fresh block.  Which we have to cleanup.
		 */
		bzero((char *)slotp->ep, imp->im_bsize);
		break;

	case FOUND:
		break;

	default:
		panic("dosfs_dirprepareentry: invalid slot status");
	}
	return (0);
}

/*
 * Is directory IP empty ?
 */
public int
dosfs_dirempty(
	struct dosfs_node	*ip)
{
	struct direct dirent;
	int dosfs_offset;
	int entryoffsetinblock;
	int error = 0, dotrans;
	int endsearch;
	struct dosfs_directory_record *ep;
	struct dosfs_mount *imp;
	struct buf *bp = NULL;

trace(dosfs_debug,("Ddirempty %x ", ip->o_number));

	if (ip->o_number == DOS_ROOTINO)
		return 0;	/* sanity */

	imp = ip->i_mnt;
	dotrans = imp->im_flags & DOSFSMNT_TRANS;
	dosfs_offset = entryoffsetinblock = 0;
	endsearch = ip->i_size;

	while (dosfs_offset < endsearch) {
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
				return (0);
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
		if (dosfsfntrans(ep, dirent.d_name, &dirent.d_namlen, dotrans))
			goto out; /* ?? */

		/*
		 * "empty" means only . and ..
		 */
		if (dirent.d_namlen > 2)
			goto out;
		if (dirent.d_name[0] != '.')
			goto out;
		if ( ! (dirent.d_namlen == 1 || dirent.d_name[1] == '.') )
			goto out;
next_one:
		dosfs_offset += sizeof(struct dosfs_directory_record);
		entryoffsetinblock += sizeof(struct dosfs_directory_record);
	}
	error = 1;
out:
	if (bp)
		brelse (bp);
	return error;
}

/*
 * Fix the ".." entry of the child directory from the old parent to the
 * new parent directory.
 * Assumes dp is a directory and that all the inodes are on the same
 * file system.
 */
public int
dosfs_dirfixdotdot(
	register struct dosfs_node *dp,		/* child directory */
	register struct dosfs_node *opdp,	/* old parent directory */
	register struct dosfs_node *npdp)	/* new parent directory */
{
	struct buf *bp;
	struct dosfs_directory_record *dirp;
	register int error = 0;
	ino_t	dotdot_ino;

trace(dosfs_debug,("Ddirfix %x %x %x ", 
	dp->o_number, opdp->o_number, npdp->o_number));
	/*
	 * check whether this is an ex-directory
	 * [uhu?]
	 */
	DOS_ILOCK(dp);
	if ((dp->o_nlink == 0) || (dp->i_size < sizeof(struct dosfs_directory_record))) {
		DOS_IUNLOCK(dp);
		return (0);
	}
	error = dosfs_blkatoff(dp, (off_t)0, (char **) &dirp, &bp);
	if (error)
		goto bad;

	dirp++;	/* swear its second entry */
	ino_dtou(dotdot_ino,dirp);

	if (dosfsfncmp("..",2,dirp,dp->i_mnt->im_flags&DOSFSMNT_TRANS) == 0){
		dosfs_dirbad(dp, "mangled .. entry", 0);
		error = EINVAL;
		goto bad;
	}
	if (dotdot_ino == npdp->o_number) {   /* just a no-op */
		goto bad;
	}

	/*
	 * Rewrite the child ".." entry and force it out.
	 */
	dnlc_remove(ITOV(dp), "..");
	ino_utod(npdp->o_number,dirp);
	dnlc_enter(ITOV(dp), "..", ITOV(npdp), NOCRED);
	bwrite(bp);
	DOS_IUNLOCK(dp);
	return (0);
bad:
	if (bp)
		brelse(bp);
	DOS_IUNLOCK(dp);
	return (error);
}

/*
 * Make an empty directory in the inode ip.
 */
struct dosfs_directory_record dosfs_mastertemplate[2] =
{ { ".       ", "   ", DOS_ATTR_DIR, },
  { "..      ", "   ", DOS_ATTR_DIR, } };

dosfs_dirmakedirect(
	register struct dosfs_node *ip,		/* new directory */
	register struct dosfs_node *dp)		/* parent directory */
{
	register struct dosfs_directory_record *dirp;
	struct buf *bp;
	struct dosfs_mount	*imp;
	daddr_t bn;
	int error, size;

trace(dosfs_debug,("\ndosfs_dirmakedirect %x %x ", dp->o_number, ip->o_number));
	/*
	 * Allocate space for the directory we're creating.
	 */
	imp = ip->i_mnt;
	size = dosfs_blksize(imp,ip,0);
	error = dosfs_iextend(ip, size, 0);
	if (error)
		return error;
	imark(ip, IUPD|ICHG);

	/*
	 * Get and clear the directory's first block
	 */
	bn = dosfs_bmap(ip, (daddr_t)0);
	bp = getblk(ip->o_devvp, bn, size);
	clrbuf(bp);
	dirp = (struct dosfs_directory_record *)bp->b_un.b_addr;

	/*
	 * Now initialize the directory we're creating with the
	 * "." and ".." entries.  Then write it out to disk.
	 */
	dirp[0] = dosfs_mastertemplate[0];
	ino_utod(ip->o_number,&dirp[0]);
	dirp[1] = dosfs_mastertemplate[1];
	ino_utod(dp->o_number,&dirp[1]);
	bwrite(bp);
	return (0);
}
