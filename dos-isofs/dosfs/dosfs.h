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
 * $Log:	dosfs.h,v $
 * Revision 2.2  93/09/15  13:24:20  mrt
 * 	Added our own extensions to record protection&mode fields
 * 	(from Rob).
 * 	Define and move here the argument structure for mount.
 * 	[93/09/14  00:06:59  af]
 * 
 * 	Changed im_dirty into a ternary-logic variable.
 * 	Added KERNEL ifdefs to make life easier to user
 * 	programs like dosfsck and dosmkfs.
 * 	[93/08/26  16:33:27  af]
 * 
 * 	First version that can write to the filesystem.
 * 	[93/07/30  00:09:09  af]
 * 
 * 	Created.
 * 	[93/07/12            af]
 * 
 */
/*
 * File:	dosfs.h
 * Author:	Alessandro Forin at Carnegie Mellon University
 * Date:	7/93
 *
 * The code in this directory was started from the MTOOLS 2.0 public
 * domain toolset.  The Readme file in there had the following
 * attribution of authorship, which is gratefully acknowledged:
 *
 * Emmet P. Gray                           US Army, HQ III Corps & Fort Hood
 * ...!uunet!uiucuxc!fthood!egray          Attn: AFZF-DE-ENV
 * fthood!egray@uxc.cso.uiuc.edu           Directorate of Engineering & Housing
 *                                         Environmental Management Office
 *                                         Fort Hood, TX 76544-5057
 */

/*
 * msdos common header file
 */

#define MDOS_SECTOR_SIZE	512	/* MSDOS sector size in bytes */
#define MDOS_DIR_SIZE		32	/* MSDOS directory size in bytes */

#define	dosfsnum_16(_x_)	((_x_)[0] | ((_x_)[1]<<8))
#define	dosfsnum_32(_x_)	((_x_)[0] | ((_x_)[1]<<8) | ((_x_)[2]<<16) | ((_x_)[3]<<24))

#define	mkdosfsnum_16(_x_,_v_)				\
MACRO_BEGIN						\
	(_x_)[0] = (_v_);				\
	(_x_)[1] = (_v_) >> 8;				\
MACRO_END

#define	mkdosfsnum_32(_x_,_v_)				\
MACRO_BEGIN						\
	(_x_)[0] = (_v_);				\
	(_x_)[1] = (_v_) >>  8;				\
	(_x_)[2] = (_v_) >> 16;				\
	(_x_)[3] = (_v_) >> 24;				\
MACRO_END

/*
 * Information contained in sector zero of a DOS filesystem
 */
#define	MDOS_LABELSECTOR	0

struct bios_partition_info {

	unsigned char	bootid;	/* bootable or not */
#	define BIOS_BOOTABLE	128

	unsigned char	beghead;/* beginning head, sector, cylinder */
	unsigned char	begsect;/* begcyl is a 10-bit number. High 2 bits */
	unsigned char	begcyl;	/*     are in begsect. */

	unsigned char	systid;	/* filesystem type */
#	define	UNIXOS		99

	unsigned char	endhead;/* ending head, sector, cylinder */
	unsigned char	endsect;/* endcyl is a 10-bit number.  High 2 bits */
	unsigned char	endcyl;	/*     are in endsect. */

	unsigned char	offset[4];
	unsigned char	n_sectors[4];
};

struct bios_label {
	char		partitions[4*sizeof(struct bios_partition_info)];
	unsigned char	magic[2];
#	define	BIOS_LABEL_MAGIC	0xaa55
};

struct dosfs_bootsector {
	unsigned char jump[3];		/* Jump to boot code */
	unsigned char banner[8];	/* OEM name & version */
	unsigned char secsiz[2];	/* Bytes per sector hopefully 512 */
	unsigned char clsiz;		/* Cluster size in sectors */
	unsigned char nrsvsect[2];	/* Number of reserved (boot) sectors */
	unsigned char nfat;		/* Number of FAT tables hopefully 2 */
	unsigned char dirents[2];	/* Number of root directory entries */
	unsigned char psect[2];		/* Total sectors on disk */
	unsigned char descr;		/* Media descriptor=first byte of FAT */
	unsigned char fatlen[2];	/* Sectors in FAT */
	unsigned char nsect[2];		/* Sectors/track */
	unsigned char nheads[2];	/* Heads */
	unsigned char nhs[4];		/* number of hidden sectors */
	unsigned char bigsect[4];	/* big total sectors */
	unsigned char driveno;		/* drive number */
	unsigned char reserved;
	unsigned char bsigrec;		/* ext boot signature record (x29) */
	unsigned char volid[4];		/* 32bit volume id */
	unsigned char volume_label[11];	/* */
	unsigned char res1[8];		/* actually, "FATnn   " nn=12/16 */
	unsigned short  bootcode[384/sizeof(short)]; /* boot code */
	struct bios_label label;	/* right at the end */
};

/*
 * Information in a directory entry
 */
struct dosfs_directory_record {
	unsigned char name[8];		/* file name */
#define	DOS_NAME_DELETED	0xe5
#define	DOS_NAME_ESCAPE		0x5
#define	DOS_NAME_EMPTY		0
	unsigned char ext[3];		/* file extension */
	unsigned char attr;		/* attribute byte */
#define	DOS_ATTR_RONLY		0x01
#define	DOS_ATTR_HIDDEN		0x02
#define	DOS_ATTR_SYS		0x04
#define	DOS_ATTR_LABEL		0x08	/* optional entry, only in rootdir */
#define	DOS_ATTR_DIR		0x10
#define	DOS_ATTR_ARCHIVE	0x20	/* e.g. dirty bit */
#define DOS_ATTR_xxx		0xc0
#define DOS_ATTR_DOSFS_TYPE	0x40
	union {
		unsigned char reserved[10];     /* mbz */
		struct {
		    unsigned char mode[2];
#define DOSFS_IFMT		0xf000
#define	  DOSFS_IFCHR		0x2000
#define	  DOSFS_IFDIR		0x4000
#define	  DOSFS_IFBLK		0x6000
#define	  DOSFS_IFREG		0x8000
#define	  DOSFS_IFLNK		0xa000
#define	  DOSFS_IFSOCK		0xc000
#define DOSFS_TYPE_SGID		0x0800
#define DOSFS_TYPE_SUID		0x0400
#define DOSFS_MODE_O_READ	0x0100
#define DOSFS_MODE_O_WRITE	0x0080
#define DOSFS_MODE_O_EXEC	0x0040
#define DOSFS_MODE_G_READ	0x0020
#define DOSFS_MODE_G_WRITE	0x0010
#define DOSFS_MODE_G_EXEC	0x0008
#define DOSFS_MODE_W_READ	0x0004
#define DOSFS_MODE_W_WRITE	0x0002
#define DOSFS_MODE_W_EXEC	0x0001
		    unsigned char uid[2];
		    unsigned char gid[2];
		    unsigned char name2[4];
		} mnt_ext;
	} res_ext;
	unsigned char time[2];		/* time stamp */
	unsigned char date[2];		/* date stamp */
	unsigned char start[2];		/* starting cluster number */
	unsigned char size[4];		/* size of the file */
};


/*
 * Information maintained per-mounted volume
 */
struct dosfs_mount {
	struct vnode *im_devvp;
	int	im_flags;
	dev_t	im_dev;
	uid_t	im_uid;
	gid_t	im_gid;
	int	im_mode;

	int	im_bshift;
	int	im_bmask;
	int	im_bsize;
	int	im_clsiz;
	int	im_size;

	int	im_dirty;
	int	fat_start;
	int	fat_len;
	int	fat_bits;
	int	n_fat;
	struct buf *fat_bp;
	unsigned short	*fat;
#ifdef	KERNEL
	struct mutex	fat_lock;
#endif
	int	fat_entries;

	int	rootdir_entries;
	int	rootdir_start;

	int	clusters_start;

	/* stat info */
	int	im_free;
	int	im_alloc;

	/* alloc hint */
	daddr_t	ffree;

	/* debug */
	char	im_path[20];

	/* optimizations */
	int	chunk_size;
};

#define VFSTODOSFS(_vfs_)	((struct dosfs_mount *)((_vfs_)->vfs_data))

#define dosfs_blkoff(imp, loc) ((loc) & ~(imp)->im_bmask)
#define dosfs_lblkno(imp, loc) ((loc) >> (imp)->im_bshift)
#define dosfs_blksize(imp, ip, lbn) ((imp)->im_bsize)
#define dosfs_lblktosize(imp, blk) ((blk) << (imp)->im_bshift)


/*
 * Mount arguments and options
 */
struct dosfs_args {
	char	*fspec;
	int	flags;
#define DOSFSMNT_TRANS		0x03
#define	DOSFSMNT_NO_TR		0x00
#define	DOSFSMNT_CASE_TR	0x01
#define	DOSFSMNT_MDOT_TR	0x02
#define DOSFSMNT_MUSER		0x04
#define	DOSFSMNT_CHUNK		0x08
#define	DOSFSMNT_PROT		0x10
	int	chunk_size;
	int	gid;
	int	uid;
	int	mode;
};


#ifdef	KERNEL

/*
 * Prototypes
 */
#define private static
#define public

/*
 * Initialization, in dosfs_node.c
 */
void dosfs_init (void);

/*
 * VFS operations, in dosfs_vfsops.c
 */
int dosfs_mountroot (void);
int dosfs_mount (struct vfs *vfsp, char *path, caddr_t data);
int dosfs_unmount (struct vfs *vfsp);
int dosfs_root (struct vfs *vfsp, struct vnode **vpp);
int dosfs_statfs (struct vfs *vfsp, struct statfs *sbp);
int dosfs_sync (struct vfs *vsfp);
/*
 * VFS operations, in dosfs_fhandle.c
 */
int dosfs_fhtovp (struct vfs *vfsp, struct vnode **vpp, struct fid *fhp);
int dosfs_fid ( struct vnode *vp, struct fid **fidpp);
int dosfs_freefid ( struct vnode *vp, struct fid *fidp);

/*
 * VNODE operations, in dosfs_vnops.c
 */
int dosfs_open (struct vnode **vvp, int flag, struct ucred *cred);
int dosfs_close (struct vnode *vp, int fflag, struct ucred *cred);
int dosfs_access (struct vnode *vp, int mode, struct ucred *cred);
int dosfs_getattr (struct vnode *vp, struct vattr *vap, struct ucred *cred);
int dosfs_rdwr (struct vnode *vp, struct uio *uio, enum uio_rw rw, int ioflag,
	struct ucred *cred);
int dosfs_ioctl (struct vnode *vp, unsigned int com, caddr_t data, int fflag,
	struct ucred *cred);
int dosfs_select (struct vnode *vp, int which, struct ucred *cred);
int dosfs_readdir (struct vnode *vp, struct uio *uio, struct ucred *cred);
int dosfs_strategy (struct buf *bp);
int dosfs_nlinks( struct vnode *vp, int *l, struct ucred *cred);


#endif	/* KERNEL */
