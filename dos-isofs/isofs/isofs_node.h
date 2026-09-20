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
 * $Log:	isofs_node.h,v $
 * Revision 2.2  93/08/07  16:56:09  mrt
 * 	Lint.
 * 	[93/07/10  19:23:24  af]
 * 
 * 	RockRidge functions changed, some added.
 * 	[93/07/04  22:03:00  af]
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
 *	@(#)isofs_inode.h
 */

struct iso_node {
	struct	vnode i_vnode;	/* vnode associated with this inode */

	/* NB: short overlay on struct inode */
	dev_t	o_dev;		/* device where inode resides */
	struct	inode *o_devvp;	/* inode for block I/O */
	ino_t	o_number;	/* i number, 1-to-1 with device address */
	int	o_diroff;	/* offset in dir, where we found last entry */
	int	o_id;		/* unique identifier */
	struct	iso_mnt *i_mnt;	/* filesystem associated with this inode */
	void	*xxx1[2];	/* OBSOLETE */
	union {
		daddr_t	if_lastr;	/* last read (read-ahead) */
		struct	{
			struct inode  *if_freef;	/* free list forward */
			struct inode **if_freeb;	/* free list back */
		} i_fr;
	} i_un;
	u_short	o_mode;
	short	o_nlink;
	uid_t	o_uid;
	gid_t	o_gid;
#undef i_size
	off_t	i_size;		/* filesize, in bytes */
	int	xxx2;
	time_t	o_atime;	/* 16: time last accessed */
	int	o_atspare;
	time_t	o_mtime;	/* 24: time last modified */
	int	o_mtspare;
	time_t	o_ctime;	/* 32: last time inode changed */

	int	i_spare0;
	int	i_spare1;

	int	iso_flags;
	int	iso_extent;

	int	iso_dirextent;
	int	iso_diroff;
};

#define	VTO_ISO(_vp_)	((struct iso_node *)(_vp_))
#define ISO_TOV(_ip_)	((struct vnode *)(_ip_))

#if 1 /*debug*/
#define ISO_ILOCK(ip)	isofs_ilock(ip)
#define ISO_IUNLOCK(ip)	isofs_iunlock(ip)
#define trace(_l_,_p_)	if (_l_) printf _p_
extern isofs_debug;
#else
#define ISO_ILOCK(ip)
#define ISO_IUNLOCK(ip)
#define trace(_l_,_p_)
#endif


/*
 * Prototypes
 */
/*
 * VNODE operations, in isofs_lookup.c
 */
int isofs_lookup (struct vnode *vdp, char *nm, struct vnode **vpp, struct ucred *cred);
int iso_blkatoff (struct iso_node *ip, off_t offset, char **res, struct buf **bpp);

/*
 * VNODE operations, in isofs_node.c
 */
int iso_iget( struct iso_node *xp, ino_t ino, struct iso_node **ipp,
		struct iso_directory_record *isodir, int diroff);
int iso_iput( struct iso_node *ip);
int isofs_inactive (struct vnode *vp, struct ucred *cred);
int isofs_ilock( struct iso_node *ip);
int isofs_iunlock( struct iso_node *ip);

/*
 * VNODE operations, in isofs_bmap.c
 */
daddr_t iso_bmap (struct iso_node *ip, daddr_t lblkno);
daddr_t iso_map_extent ( struct iso_node *ip, int extent);

/*
 * RockRidge extensions, in rock_ridge.c
 */
int rock_ridge_find_relocation( struct iso_directory_record * de, 
				struct iso_node * vnode);

int rock_ridge_get_filename(	struct iso_directory_record * de,
				char ** name, int * namlen, struct iso_node * vnode);

int rock_ridge_parse_vnode(	struct iso_directory_record * de,
				struct iso_node * vnode);

int rock_ridge_isa_symlink(	struct iso_directory_record * de,
				struct iso_node * vnode);

char *rock_ridge_get_symlink(	struct iso_node * vnode);

public dev_t rock_ridge_mapdev(	struct iso_mnt *isomp, int high, int low);
