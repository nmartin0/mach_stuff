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
 * $Log:	isofs_fhandle.c,v $
 * Revision 2.2  93/08/07  16:55:50  mrt
 * 	It almost worked first time, all it needed was to special case 
 * 	the root directory.  More useful trace debugging printouts.
 * 	[93/07/10  19:21:55  af]
 * 
 * 	Minimized includes.  This module still untested.
 * 	[93/07/04  21:57:54  af]
 * 
 * 	Separate out the non-existant file handle code and made
 * 	it real.  Added the missing extra functions.
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
 *	@(#)isofs_vfsops.c	[original file where fhtovp() was]
 */

#include <sys/param.h>
#include <sys/dir.h>
#include <sys/user.h>
#include <sys/buf.h>
#include <sys/inode.h>

#include <isofs/isofs.h>
#include <isofs/isofs_node.h>

extern zone_t	isofs_fid_zone;

int isofs_fiddebug = 0;

public int
isofs_fid(
	struct vnode *vp,
	struct fid **fidpp)
{
	register struct ifid *ifid;
	struct iso_node	*ip = VTO_ISO(vp);

trace(isofs_fiddebug,("isofs_fid %x %x (%x %x %x %x) ", ip, vp->v_flag,
		ip->iso_dirextent, ip->o_number,
		ip->iso_diroff, ip->i_mnt->root_extent));
	ZALLOC(isofs_fid_zone, ifid, struct ifid *);
	bzero((caddr_t)ifid, sizeof(struct ifid));
	ifid->ifid_len = sizeof(struct ifid) - (sizeof(struct fid) - MAXFIDSZ);
	ifid->ifid_ino = ip->o_number;
	ifid->ifid_dirext = (vp->v_flag & VROOT) ? ip->i_mnt->dirextent : ip->iso_dirextent;
	ifid->ifid_off = ip->iso_diroff;

	*fidpp = (struct fid *)ifid;
	return (0);
}

public int
isofs_freefid(
	struct vnode *vp,
	struct fid *fidp)
{
trace(isofs_fiddebug,("isofs_freefid %x\n", vp));
	ZFREE(isofs_fid_zone, fidp);
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
isofs_fhtovp(
	register struct vfs *vfsp,
	struct vnode	**vpp,
	struct fid	*fhp)
{
	struct iso_node tvp, *nip;
	struct iso_directory_record *dirp;
	int error, off, bsiz;
	daddr_t block;
	register struct ifid *ifhp;
	struct iso_mnt	*isomp;
	struct buf	*bp;

	ifhp = (struct ifid *)fhp;
	isomp = VFSTOISOFS (vfsp);

trace(isofs_fiddebug,("isofs_fhtovp %x %x %x %x\n", 
		ifhp->ifid_len, ifhp->ifid_ino,
		ifhp->ifid_dirext, ifhp->ifid_off));
trace(isofs_fiddebug,("imp-> %x %x %x\n", 
		isomp->dirextent, isomp->diroff, isomp->root_extent));

	off = ifhp->ifid_off;
	if (ifhp->ifid_dirext + iso_lblkno(isomp,off) >= isomp->volume_space_size)
		return (EINVAL);

	bsiz = isomp->im_bsize;
	if ((off & (bsiz-1)) + sizeof (struct iso_directory_record) >= bsiz)
		return (EINVAL);

	tvp.i_vnode.ih_fs = vfsp;
	tvp.i_mnt = isomp;
	tvp.o_dev = isomp->im_dev;
	tvp.iso_dirextent = ifhp->ifid_dirext;
	tvp.iso_diroff = off;

	bp = bread (isomp->im_devvp,
		iso_map_extent(&tvp, ifhp->ifid_dirext + iso_lblkno(isomp,off)),
		bsiz);

	off &= bsiz-1;

	dirp = (struct iso_directory_record *)(bp->b_un.b_addr + off);

	if (off + isonum_711 (dirp) >= bsiz) {
		brelse (bp);
		return (EINVAL);
	}

	if (error = iso_iget(&tvp, ifhp->ifid_ino, &nip, dirp, ifhp->ifid_off)) {
		*vpp = NULL;
		brelse (bp);
		return (error);
	}

	ISO_IUNLOCK(nip);
	*vpp = ISO_TOV(nip);
	brelse (bp);
	return (0);
}

