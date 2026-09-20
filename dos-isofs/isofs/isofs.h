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
 * $Log:	isofs.h,v $
 * Revision 2.2  93/08/07  16:55:39  mrt
 * 	Added decls for more or less everything the standard defines.
 * 	[93/07/30  00:04:52  af]
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
 *	@(#)isofs.h
 */

/*
 * Basic data types (in isofs_util.c)
 */
#define     isonum_711(_p_) \
	((*((char*)(_p_))) & 0xff)

extern int isonum_712(char *p);
extern int isonum_721(char *p);
extern int isonum_722(char *p);

#define    isonum_723(_p_)	\
	(((char*)(_p_))[0] | ((((char*)(_p_))[1] & 0xff) << 8))

extern int isonum_731(char *p);
extern int isonum_732(char *p);

#define	   isonum_733(_p_) \
	((((char*)(_p_))[0] & 0xff) \
	 | ((((char*)(_p_))[1] & 0xff) << 8) \
	 | ((((char*)(_p_))[2] & 0xff) << 16) \
	 | ((((char*)(_p_))[3] & 0xff) << 24))

extern int iso_date(char *p, int isa_high_sierra);
extern int isofncmp(char *fn, int fnlen, char *isofn, int isolen);
extern void isofntrans(char *infn, int infnlen, char *outfn, short *outfnlen);

/*
 * On-disk data structures
 */
#define ISODCL(from, to) (to - from + 1)

/*
 * Volume descriptors
 */
struct iso_volume_descriptor {
	char type[ISODCL(1,1)]; /* 711 */
	char id[ISODCL(2,6)];
	char version[ISODCL(7,7)];
	char data[ISODCL(8,2048)];
};

/* volume descriptor types */
#define	ISO_VD_BOOT		0
#define ISO_VD_PRIMARY		1
#define	ISO_VD_SUPPLEMENTARY	2
#define	ISO_VD_PARTITION	3
#define ISO_VD_END 255

#define ISO_STANDARD_ID "CD001"

struct iso_boot_descriptor {
	char type			[ISODCL (  1,   1)]; /* 711 */
	char id				[ISODCL (  2,   6)];
	char version			[ISODCL (  7,   7)]; /* 711 */
	char sys_id			[ISODCL (  8,  39)]; /* achars */
	char boot_id			[ISODCL ( 40,  71)]; /* achars */
	char unspec			[ISODCL ( 72,2048)]; /* not specified */
};

struct iso_primary_descriptor {
	char type			[ISODCL (  1,   1)]; /* 711 */
	char id				[ISODCL (  2,   6)];
	char version			[ISODCL (  7,   7)]; /* 711 */
	char unused1			[ISODCL (  8,   8)];
	char system_id			[ISODCL (  9,  40)]; /* achars */
	char volume_id			[ISODCL ( 41,  72)]; /* dchars */
	char unused2			[ISODCL ( 73,  80)];
	char volume_space_size		[ISODCL ( 81,  88)]; /* 733 */
	char unused3			[ISODCL ( 89, 120)];
	char volume_set_size		[ISODCL (121, 124)]; /* 723 */
	char volume_sequence_number	[ISODCL (125, 128)]; /* 723 */
	char logical_block_size		[ISODCL (129, 132)]; /* 723 */
	char path_table_size		[ISODCL (133, 140)]; /* 733 */
	char type_l_path_table		[ISODCL (141, 144)]; /* 731 */
	char opt_type_l_path_table	[ISODCL (145, 148)]; /* 731 */
	char type_m_path_table		[ISODCL (149, 152)]; /* 732 */
	char opt_type_m_path_table	[ISODCL (153, 156)]; /* 732 */
	char root_directory_record	[ISODCL (157, 190)]; /* 9.1 */
	char volume_set_id		[ISODCL (191, 318)]; /* dchars */
	char publisher_id		[ISODCL (319, 446)]; /* achars */
	char preparer_id		[ISODCL (447, 574)]; /* achars */
	char application_id		[ISODCL (575, 702)]; /* achars */
	char copyright_file_id		[ISODCL (703, 739)]; /* 7.5 dchars */
	char abstract_file_id		[ISODCL (740, 776)]; /* 7.5 dchars */
	char bibliographic_file_id	[ISODCL (777, 813)]; /* 7.5 dchars */
	char creation_date		[ISODCL (814, 830)]; /* 8.4.26.1 */
	char modification_date		[ISODCL (831, 847)]; /* 8.4.26.1 */
	char expiration_date		[ISODCL (848, 864)]; /* 8.4.26.1 */
	char effective_date		[ISODCL (865, 881)]; /* 8.4.26.1 */
	char file_structure_version	[ISODCL (882, 882)]; /* 711 */
	char unused4			[ISODCL (883, 883)];
	char application_data		[ISODCL (884, 1395)];
	char unused5			[ISODCL (1396, 2048)];
};

struct iso_supplementary_descriptor {
	char type			[ISODCL (  1,   1)]; /* 711 */
	char id				[ISODCL (  2,   6)];
	char version			[ISODCL (  7,   7)]; /* 711 */
	char flags			[ISODCL (  8,   8)]; /* bits */
	char system_id			[ISODCL (  9,  40)]; /* a1chars */
	char volume_id			[ISODCL ( 41,  72)]; /* d1chars */
	char unused2			[ISODCL ( 73,  80)]; /* mbz */
	char volume_space_size		[ISODCL ( 81,  88)]; /* 733 */
	char escape_sequences		[ISODCL ( 89, 120)]; /* bytes */
	char volume_set_size		[ISODCL (121, 124)]; /* 723 */
	char volume_sequence_number	[ISODCL (125, 128)]; /* 723 */
	char logical_block_size		[ISODCL (129, 132)]; /* 723 */
	char path_table_size		[ISODCL (133, 140)]; /* 733 */
	char type_l_path_table		[ISODCL (141, 144)]; /* 731 */
	char opt_type_l_path_table	[ISODCL (145, 148)]; /* 731 */
	char type_m_path_table		[ISODCL (149, 152)]; /* 732 */
	char opt_type_m_path_table	[ISODCL (153, 156)]; /* 732 */
	char root_directory_record	[ISODCL (157, 190)]; /* 9.1 */
	char volume_set_id		[ISODCL (191, 318)]; /* d1chars */
	char publisher_id		[ISODCL (319, 446)]; /* a1chars */
	char preparer_id		[ISODCL (447, 574)]; /* a1chars */
	char application_id		[ISODCL (575, 702)]; /* a1chars */
	char copyright_file_id		[ISODCL (703, 739)]; /* 7.5 d1chars */
	char abstract_file_id		[ISODCL (740, 776)]; /* 7.5 d1chars */
	char bibliographic_file_id	[ISODCL (777, 813)]; /* 7.5 d1chars */
	char creation_date		[ISODCL (814, 830)]; /* 8.4.26.1 */
	char modification_date		[ISODCL (831, 847)]; /* 8.4.26.1 */
	char expiration_date		[ISODCL (848, 864)]; /* 8.4.26.1 */
	char effective_date		[ISODCL (865, 881)]; /* 8.4.26.1 */
	char file_structure_version	[ISODCL (882, 882)]; /* 711 */
	char unused4			[ISODCL (883, 883)];
	char application_data		[ISODCL (884, 1395)];
	char unused5			[ISODCL (1396, 2048)];
};

struct iso_partition_descriptor {
	char type			[ISODCL (  1,   1)]; /* 711 */
	char id				[ISODCL (  2,   6)];
	char version			[ISODCL (  7,   7)]; /* 711 */
	char unused1			[ISODCL (  8,   8)];
	char system_id			[ISODCL (  9,  40)]; /* achars */
	char partition_id		[ISODCL ( 41,  72)]; /* dchars */
	char partition_location		[ISODCL ( 73,  80)]; /* 733 */
	char partition_size		[ISODCL ( 81,  88)]; /* 733 */
	char system_use			[ISODCL ( 89,2048)]; /* not specified */
};

/*
 * Directories and path tables
 */
struct iso_directory_record {
	char length			[ISODCL (1, 1)]; /* 711 */
	char ext_attr_length		[ISODCL (2, 2)]; /* 711 */
	char extent			[ISODCL (3, 10)]; /* 733 */
	char size			[ISODCL (11, 18)]; /* 733 */
	char date			[ISODCL (19, 25)]; /* 711 short_time */
	char flags			[ISODCL (26, 26)];
	char file_unit_size		[ISODCL (27, 27)]; /* 711 */
	char interleave			[ISODCL (28, 28)]; /* 711 */
	char volume_sequence_number	[ISODCL (29, 32)]; /* 723 */
	char name_len			[ISODCL (33, 33)]; /* 711 */
	char name			[0];
};

/* flags */
#define	ISO_FF_HIDDEN		0x01
#define	ISO_FF_DIRECTORY	0x02
#define	ISO_FF_ASSOCIATED_FILE	0x04
#define	ISO_FF_RECORD		0x08
#define	ISO_FF_PROTECTED	0x10
#define	ISO_FF_reserved		0x60
#define	ISO_FF_MULTI_EXTENT	0x80

struct iso_path_table_record {
	char length			[ISODCL (1, 1)]; /* 711 */
	char ext_attr_length		[ISODCL (2, 2)]; /* 711 */
	char extent			[ISODCL (3, 6)]; /* 73 */
	char parent_dir			[ISODCL (7, 8)]; /* 72 */
	char name			[0];
};

/*
 * Date and time
 */
struct iso_long_timestamp {
	char year			[ISODCL (1, 4)]; /* digits */
	char month			[ISODCL (5, 6)]; /* digits */
	char day			[ISODCL (7, 8)]; /* digits */
	char hour			[ISODCL (9, 10)]; /* digits */
	char minute			[ISODCL (11, 12)]; /* digits */
	char second			[ISODCL (13, 14)]; /* digits */
	char cent			[ISODCL (15, 16)]; /* digits */
	char gmt_offset			[ISODCL (17, 17)]; /* 712 */
};

struct iso_short_timestamp {
	char year			[ISODCL (1, 1)]; /* 711 */
	char month			[ISODCL (2, 2)]; /* 711 */
	char day			[ISODCL (3, 3)]; /* 711 */
	char hour			[ISODCL (4, 4)]; /* 711 */
	char minute			[ISODCL (5, 5)]; /* 711 */
	char second			[ISODCL (6, 6)]; /* 711 */
	char gmt_offset			[ISODCL (7, 7)]; /* 712 */
};

/*
 * Extended attributes
 */
struct iso_extended_attribute {
	char owner			[ISODCL (1, 4)]; /* 723 */
	char group			[ISODCL (5, 8)]; /* 723 */
	char perm			[ISODCL (9, 10)]; /* bits */
	char ctime			[ISODCL (11, 27)]; /* 8.4.26 long_time */
	char mtime			[ISODCL (28, 44)]; /* 8.4.26 long_time */
	char extime			[ISODCL (45, 61)]; /* 8.4.26 long_time */
	char eftime			[ISODCL (62, 78)]; /* 8.4.26 long_time */
	char recform			[ISODCL (79, 79)]; /* 711 */
	char recattr			[ISODCL (80, 80)]; /* 711 */
	char reclen			[ISODCL (81, 84)]; /* 723 */
	char sys_id			[ISODCL (85, 116)]; /* achars */
	char su				[ISODCL (117, 180)]; /* not specified */
	char xattr_version		[ISODCL (181, 181)]; /* 711 */
	char esc_len			[ISODCL (182, 182)]; /* 711 */
	char reserved			[ISODCL (183, 246)]; /* mbz */
	char au_len			[ISODCL (247, 250)]; /* 723 */
	char esc[0];
};

/* permissions */
#define	ISO_PERM_READ		0x01
#define	ISO_PERM_EXEC		0x04
#define	ISO_PERM_MASK		0x05
#define	ISO_PERM_S_SHIFT	0
#define	ISO_PERM_U_SHIFT	4
#define	ISO_PERM_G_SHIFT	8
#define	ISO_PERM_O_SHIFT	12

/* records */
#define	ISO_RF_UNSPEC		0
#define	ISO_RF_FIXED		1	/* 6.10.3 */
#define	ISO_RF_VAR_1		2	/* 6.10.4, RCW is 721 */
#define	ISO_RF_VAR_2		3	/* 6.10.4, RCW is 722 */
					/* 4..127 reserved */
#define	ISO_RF_SYSTEM_USE	0x80	/* 128..255 for system use */

#define	ISO_RA_CRLF		0	/* LF, data, CR */
#define	ISO_RA_VF		1	/* ISO 1539 vert spacing */
#define	ISO_RA_WITHIN		2	/* inside record */

/*
 * High Sierra variant
 */
#define HS_STANDARD_ID "CDROM"

struct  hs_volume_descriptor {
	char foo			[ISODCL (1, 8)]; /* 733 */
	char type			[ISODCL (9, 9)]; /* 711 */
	char id				[ISODCL (10, 14)];
	char version			[ISODCL (15, 15)]; /* 711 */
	char data			[ISODCL (16, 2048)];
};


struct hs_primary_descriptor {
	char foo			[ISODCL (  1,   8)]; /* 733 */
	char type			[ISODCL (  9,   9)]; /* 711 */
	char id				[ISODCL ( 10,  14)];
	char version			[ISODCL ( 15,  15)]; /* 711 */
	char unused1			[ISODCL ( 16,  16)]; /* 711 */
	char system_id			[ISODCL ( 17,  48)]; /* achars */
	char volume_id			[ISODCL ( 49,  80)]; /* dchars */
	char unused2			[ISODCL ( 81,  88)]; /* 733 */
	char volume_space_size		[ISODCL ( 89,  96)]; /* 733 */
	char unused3			[ISODCL ( 97, 128)]; /* 733 */
	char volume_set_size		[ISODCL (129, 132)]; /* 723 */
	char volume_sequence_number	[ISODCL (133, 136)]; /* 723 */
	char logical_block_size		[ISODCL (137, 140)]; /* 723 */
	char path_table_size		[ISODCL (141, 148)]; /* 733 */
	char type_l_path_table		[ISODCL (149, 152)]; /* 731 */
	char unused4			[ISODCL (153, 180)]; /* 733 */
	char root_directory_record	[ISODCL (181, 214)]; /* 9.1 */
};

/* high sierra is identical to iso, except that the date is only 6 bytes, and
   there is an extra reserved byte after the flags */

#define hs_directory_record iso_directory_record

/*
 * Per-filesystem in core information (aka superblock)
 */
struct iso_mnt {
	int	logical_block_size;
	int	volume_space_size;
	struct vnode *im_devvp;
	char	im_fsmnt[50];
	
	int	im_flags;
	struct vfs *im_mountp;
	dev_t	im_dev;

	int	im_bshift;
	int	im_bmask;
	int	im_bsize;

	char	root[ISODCL (157, 190)];
	int	root_extent;
	int	root_size;
	int	dirextent;
	int	diroff;
};

#define VFSTOISOFS(_vfs_)	((struct iso_mnt *)((_vfs_)->vfs_data))

#define iso_blkoff(imp, loc) ((loc) & ~(imp)->im_bmask)
#define iso_lblkno(imp, loc) ((loc) >> (imp)->im_bshift)
#define iso_blksize(imp, ip, lbn) ((imp)->im_bsize)
#define iso_lblktosize(imp, blk) ((blk) << (imp)->im_bshift)


/*
 * Prototypes
 */
#define private static
#define public

/*
 * Initialization, in isofs_node.c
 */
void isofs_init (void);

/*
 * VFS operations, in isofs_vfsops.c
 */
int isofs_mountroot (void);
int isofs_mount (struct vfs *vfsp, char *path, caddr_t data);
int isofs_unmount (struct vfs *vfsp);
int isofs_root (struct vfs *vfsp, struct vnode **vpp);
int isofs_statfs (struct vfs *vfsp, struct statfs *sbp);
int isofs_sync (struct vfs *vsfp);
/*
 * VFS operations, in isofs_fhandle.c
 */
int isofs_fhtovp (struct vfs *vfsp, struct vnode **vpp, struct fid *fhp);
int isofs_fid ( struct vnode *vp, struct fid **fidpp);
int isofs_freefid ( struct vnode *vp, struct fid *fidp);

/*
 * VNODE operations, in isofs_vnops.c
 */
int isofs_open (struct vnode **vvp, int flag, struct ucred *cred);
int isofs_close (struct vnode *vp, int fflag, struct ucred *cred);
int isofs_access (struct vnode *vp, int mode, struct ucred *cred);
int isofs_getattr (struct vnode *vp, struct vattr *vap, struct ucred *cred);
int isofs_rdwr (struct vnode *vp, struct uio *uio, enum uio_rw rw, int ioflag,
	struct ucred *cred);
int isofs_ioctl (struct vnode *vp, unsigned int com, caddr_t data, int fflag,
	struct ucred *cred);
int isofs_select (struct vnode *vp, int which, struct ucred *cred);
int isofs_readdir (struct vnode *vp, struct uio *uio, struct ucred *cred);
int isofs_strategy (struct buf *bp);

int einval (void);

