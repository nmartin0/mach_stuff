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
 * $Log:	dosfs_node.h,v $
 * Revision 2.2  93/09/15  13:29:57  mrt
 * 	Renamed unused field.
 * 	[93/09/13  23:46:30  af]
 * 
 * 	First version that can write to the filesystem.
 * 	[93/07/30  00:11:34  af]
 * 
 * 	Created.
 * 	[93/07/12            af]
 * 
 */

struct dosfs_node {
	struct	vnode i_vnode;	/* vnode associated with this inode */

	/* NB: short overlay on struct inode */
	dev_t	o_dev;		/* device where inode resides */
	struct	vnode *o_devvp;	/* vnode for block I/O */
	ino_t	o_number;	/* i number, 1-to-1 with cluster no, except.. */
	int	o_diroff;	/* offset in dir, where we found last entry */
	int	o_xxx;
	struct	dosfs_mount *i_mnt;	/* filesystem associated with this inode */
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

	int	extent;
	int	attr;

	int	dosfs_dirextent;
	int	dosfs_diroff;

	daddr_t	last_lbn;	/* bmap cache */
	daddr_t	last_cln;
};

/*
 * Map from/to dos clusters and unix inodes
 */
#define	DOS_ROOTINO	1
#define	ino_dtou(_i_,_e_)				\
MACRO_BEGIN						\
	if ((_i_ = dosfsnum_16((_e_)->start)) == 0)	\
		_i_ = DOS_ROOTINO;			\
MACRO_END
#define	ino_utod(_i_,_e_)				\
MACRO_BEGIN						\
	register ino_t	ino = (_i_);			\
	if (ino == DOS_ROOTINO)  ino = 0;		\
	mkdosfsnum_16((_e_)->start,ino);		\
MACRO_END

/*
 * Map from/to dosnodes and vnodes
 */
#define	VTO_DOS(_vp_)	((struct dosfs_node *)(_vp_))
#define DOS_TOV(_ip_)	((struct vnode *)(_ip_))

/*
 * Locking and tracing
 */
#if 1 /*debug*/
#define DOS_ILOCK(ip)	dosfs_ilock(ip)
#define DOS_IUNLOCK(ip)	dosfs_iunlock(ip)
#define trace(_l_,_p_)	if (_l_) printf _p_
extern dosfs_debug;
#else
#define DOS_ILOCK(ip)
#define DOS_IUNLOCK(ip)
#define trace(_l_,_p_)
#endif

#define ISVDEV(t) (((t) == VCHR) || ((t) == VBLK))

/*
 * Prototypes
 */
/*
 * VNODE operations, in dosfs_lookup.c
 */
int dosfs_lookup (struct vnode *vdp, char *nm, struct vnode **vpp, struct ucred *cred);
int dosfs_dirlook(struct dosfs_node *dp, char *nm, struct vnode **vpp,
		struct ucred *cred);
int dosfs_blkatoff (struct dosfs_node *ip, off_t offset, char **res, struct buf **bpp);

/*
 * VNODE operations, in dosfs_node.c
 */
int dosfs_iget( struct dosfs_node *xp, ino_t ino, struct dosfs_node **ipp,
		struct dosfs_directory_record *dosfsdir, int diroff);
int dosfs_iput( struct dosfs_node *ip);
int dosfs_iflush( dev_t dev );
int dosfs_inactive (struct vnode *vp, struct ucred *cred);
int dosfs_syncip( struct dosfs_node *ip, struct ucred *cred);
int dosfs_ilock( struct dosfs_node *ip);
int dosfs_iunlock( struct dosfs_node *ip);

/*
 * VNODE operations, in dosfs_bmap.c
 */
daddr_t dosfs_bmap (struct dosfs_node *ip, daddr_t lblkno);
int dosfs_iextend(struct dosfs_node *ip, off_t lenght, int alloced);

