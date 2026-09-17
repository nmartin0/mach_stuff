/*
 * MacMach Operating System
 * Copyright (C) 1992 Carnegie Mellon University
 * All Rights Reserved.
 *
 * MacMach was developed by CMU with support from Apple Computer, Inc.
 * Use of this software is constrained by the MacMach End-User license.
 */

#ifndef _INODE_H_
#define _INODE_H_

struct inode {
	struct 	icommon	i_ic;
};

#define i_fs		i_un.ui_fs
#define	i_mode		i_ic.ic_mode
#define	i_nlink		i_ic.ic_nlink
#define	i_uid		i_ic.ic_uid
#define	i_gid		i_ic.ic_gid
#define i_size		i_ic.ic_size.val[1]
#define	i_db		i_ic.ic_db
#define	i_ib		i_ic.ic_ib
#define	i_atime		i_ic.ic_atime
#define	i_mtime		i_ic.ic_mtime
#define	i_ctime		i_ic.ic_ctime
#define i_blocks	i_ic.ic_blocks

struct	iob {
	struct	inode i_ino;		/* inode */
	dev_t	i_dev;				/* device being accessed */
	daddr_t	i_boff;				/* block offset on device */
	off_t	i_offset;			/* seek offset in file */
	daddr_t	i_bn;				/* 1st block # of next read */
	char	*i_ma;				/* memory address of i/o buffer */
	long	i_cc;				/* character count of transfer */
	char	i_buf[MAXBSIZE];	/* i/o buffer */
	union {
		struct fs ui_fs;		/* file system super block info */
		char dummy[SBSIZE];
	} i_un;
	char	*i_b[NIADDR+1];		/* buffers for indirect blocks */
	daddr_t i_blknos[NIADDR+1];	/* block numbers of indirect blocks */
};

#endif /* inode.h */