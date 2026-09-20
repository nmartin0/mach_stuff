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
 * $Log:	isofs_node.c,v $
 * Revision 2.3  93/09/15  16:06:20  mrt
 * 	Redid iflush() code to flush for real.
 * 	Fixed race in inode termination.
 * 	[93/08/26  16:56:53  af]
 * 
 * Revision 2.2  93/08/07  16:56:01  mrt
 * 	Lint.
 * 	[93/07/30  00:05:33  af]
 * 
 * 	Minimized includes.
 * 	Parse RockRidge extensions in iget() (except root).
 * 	[93/07/04  22:04:25  af]
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
 *	@(#)isofs_inode.c
 */

#include <sys/param.h>
#include <sys/dir.h>
#include <sys/user.h>
#include <sys/buf.h>
#include <sys/inode.h>

#include <isofs/isofs.h>
#include <isofs/isofs_node.h>

#define	INOHSZ	512
#if	((INOHSZ&(INOHSZ-1)) == 0)
#define	INOHASH(dev,ino)	(((dev)+(ino))&(INOHSZ-1))
#else
#define	INOHASH(dev,ino)	(((unsigned)((dev)+(ino)))%INOHSZ)
#endif

union iso_ihead {
	union  iso_ihead *ih_head[2];
	struct iso_node *ih_chain[2];
} iso_ihead[INOHSZ];

zone_t	isofs_zone;
zone_t	isofs_fid_zone;

extern int dorr;

/*
 * Initialize hash links for inodes.
 */
public void
isofs_init()
{
	register int i;
	register union iso_ihead *ih = iso_ihead;

	for (i = INOHSZ; --i >= 0; ih++) {
		ih->ih_head[0] = ih;
		ih->ih_head[1] = ih;
	}
	isofs_zone = zinit(sizeof(struct iso_node), 1024*1024,
				0, FALSE, "isofs zone");
	isofs_fid_zone = zinit(sizeof(struct ifid), 1024*1024,
				vm_page_size, FALSE, "isofs fid");
}

/*
 * Look up a ISOFS dinode number to find its incore vnode.
 * If it is not in core, read it in from the specified device.
 * If it is in core, wait for the lock bit to clear, then
 * return the inode locked. Detection and handling of mount
 * points must be done by the calling routine.
 */
public int
iso_iget(
	struct iso_node	*xp,
	ino_t		ino,
	struct iso_node	**ipp,
	struct iso_directory_record *isodir,
	int		diroff)
{
	dev_t dev;
	struct vfs *vfsp;
	extern struct vnodeops isofs_vnodeops;
	register struct iso_node *ip;
	register struct vnode *vp;
	struct vnode *nvp;
	union iso_ihead *ih;
	int error, i;
	struct iso_mnt *isomp;

	vfsp = ISO_TOV(xp)->ih_fs;
	dev = xp->o_dev;

trace(isofs_debug,("iso_iget x%x ", ino));

	ih = &iso_ihead[INOHASH(dev, ino)];
loop:
	for (ip = ih->ih_chain[0];
	     ip != (struct iso_node *)ih;
	     ip = (struct iso_node *)ip->i_forw) {
		if (ino != ip->o_number || dev != ip->o_dev)
			continue;
		if ((ip->i_flag&ILOCKED) != 0) {
			ip->i_flag |= IWANT;
			sleep((caddr_t)ip, PINOD);
			goto loop;
		}
		ISO_ILOCK(ip);
		VN_HOLD(ISO_TOV(ip));
		*ipp = ip;
		return(0);
	}
	/*
	 * Allocate a new inode.
	 */
	ZALLOC(isofs_zone, ip, struct iso_node *);
	bzero(ip, sizeof(*ip));
	nvp = ISO_TOV(ip);
	nvp->v_type = ITYPE_ISOFS;
	VN_HOLD(nvp);
	/*
	 * Put it onto its hash chain and lock it so that other requests for
	 * this inode will block if they arrive while we are sleeping waiting
	 * for old data structures to be purged or for the contents of the
	 * disk portion of this inode to be read.
	 */
	ip->o_dev = dev;
	ip->o_number = ino;
	ISO_ILOCK(ip);
	insque(ip, ih);

	ip->iso_extent = isonum_733 (isodir->extent);
	ip->i_size = (off_t) isonum_733 (isodir->size);
	i = iso_date(isodir->date, 0);
	ip->o_atime = ip->o_mtime = ip->o_ctime = i;
	ip->iso_flags = isonum_711 (isodir->flags);

	ip->iso_diroff = diroff;
	ip->iso_dirextent = xp->iso_extent;

	/*
	 * Initialize the associated vnode
	 */
	vp = ISO_TOV(ip);
	VN_INIT(vp,vfsp,0,dev);

	if (ip->iso_flags & ISO_FF_DIRECTORY) {
		vp->v_mode = VDIR;
		ip->o_nlink = 2;
	} else {
		vp->v_mode = VREG;
		ip->o_nlink = 1;
	}
	ip->o_mode = vp->v_mode |
			(VREAD|VEXEC) | ((VREAD|VEXEC)>>3) | ((VREAD|VEXEC)>>6);

	isomp = VFSTOISOFS (vfsp);

	ip->i_mnt = isomp;
	VN_HOLD(isomp->im_devvp);
	ip->o_devvp = VTOI(isomp->im_devvp);

trace(isofs_debug,("(%x %x)", ino, isomp->root_extent ));
	if (ino == isomp->root_extent)
		vp->v_flag |= VROOT;
	else {
		rock_ridge_parse_vnode(isodir, ip);
		vp->v_mode = ip->o_mode & VFMT;
	}

	*ipp = ip;
	return (0);
}

/*
 * Unlock and decrement the reference count of an inode structure.
 */
public int
iso_iput( register struct iso_node *ip)
{
trace(isofs_debug,("iso_iput "));

	if ((ip->i_flag & ILOCKED) == 0)
		panic("iso_iput");
	ISO_IUNLOCK(ip);
	VN_RELE(ISO_TOV(ip));
	return (0);
}

/*
 * Remove any cached inodes that belong to DEV
 * If any are left and we cannot rid of them
 * return their total count.
 */
public int
iso_iflush( dev_t dev)
{
	register struct iso_node *ip, *iq;
	register open = 0;
	union iso_ihead *ih;

	for (ih = iso_ihead; ih < &iso_ihead[INOHSZ]; ih++) {
	  iq = 0;
retry:
	  for(ip = ih->ih_chain[0];
	      ip != (struct iso_node *)ih;
	      ip = (struct iso_node *)ip->i_forw) {

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
isofs_inactive(
	struct vnode	*vp,
	struct ucred	*cred)
{
	register struct iso_node *ip = VTO_ISO(vp);
	int mode, error = 0;

trace(isofs_debug,("isofs_inactive %x", vp));

	/*
	 * Lock against reuse
	 */
	if (ip->i_flag & ILOCKED) {
printf("isofs_inactive: %x was locked\n", ip);
		return 1;
	}
	ip->i_flag |= ILOCKED;

	/*
	 * Remove the inode from its hash chain.
	 */
	remque(ip);
	ip->i_forw = (struct vnode *)ip;
	ip->i_back = (struct vnode *)ip;
	/*
	 * Purge old data structures associated with the inode.
	 */
	dnlc_purge_vp(vp);
	if (ip->o_devvp) {
		VN_RELE(ip->o_devvp);
		ip->o_devvp = 0;
	}

	/*
	 * wakeup anyone that raced us to
	 * this inode in iget, and lost
	 */
	isofs_iunlock(ip);

	/* put it back on freelist or zone */
	ZFREE(isofs_zone, ip);

	return (0);
}

/*
 * Lock an inode. If its already locked, set the WANT bit and sleep.
 */
isofs_ilock(ip)
	register struct iso_node *ip;
{
trace(isofs_debug,("isofs_ilock %x ", ip->o_number));

	while (ip->i_flag & ILOCKED) {
		ip->i_flag |= IWANT;
		if (ip->i_spare0 == u.u_procp->p_pid)
			panic("locking against myself");
		ip->i_spare1 = u.u_procp->p_pid;
		(void) sleep((caddr_t)ip, PINOD);
	}
	ip->i_spare1 = 0;
	ip->i_spare0 = u.u_procp->p_pid;
	ip->i_flag |= ILOCKED;
}

/*
 * Unlock an inode.  If WANT bit is on, wakeup.
 */
isofs_iunlock(ip)
	register struct iso_node *ip;
{
trace(isofs_debug,("isofs_iunlock %x ", ip->o_number));

	if ((ip->i_flag & ILOCKED) == 0)
		printf("iso_iunlock: unlocked inode", ISO_TOV(ip));
	ip->i_spare0 = 0;
	ip->i_flag &= ~ILOCKED;
	if (ip->i_flag&IWANT) {
		ip->i_flag &= ~IWANT;
		wakeup((caddr_t)ip);
	}
}
