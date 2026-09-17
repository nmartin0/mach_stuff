/*
 * Copyright 1996 1995 by Open Software Foundation, Inc.   
 *              All Rights Reserved 
 *  
 * Permission to use, copy, modify, and distribute this software and 
 * its documentation for any purpose and without fee is hereby granted, 
 * provided that the above copyright notice appears in all copies and 
 * that both the copyright notice and this permission notice appear in 
 * supporting documentation. 
 *  
 * OSF DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE 
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS 
 * FOR A PARTICULAR PURPOSE. 
 *  
 * IN NO EVENT SHALL OSF BE LIABLE FOR ANY SPECIAL, INDIRECT, OR 
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM 
 * LOSS OF USE, DATA OR PROFITS, WHETHER IN ACTION OF CONTRACT, 
 * NEGLIGENCE, OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION 
 * WITH THE USE OR PERFORMANCE OF THIS SOFTWARE. 
 */
/*
 * pmk1.1
 */
/*
 * HISTORY
 * $Log: ffs_compat.h,v $
 * Revision 1.1.3.1  1995/08/21  19:24:27  devrcs
 * 	Copied from CMU LINUX boot.
 * 	[95/03/21            bernadat]
 *
 * Revision 1.1.2.1  1995/06/30  13:27:35  bernadat
 * 	Copied from CMU LINUX boot.
 * 	[95/03/21            bernadat]
 *
 * Revision 1.1.1.2  1995/06/30  11:59:57  bernadat
 * 	Copied from CMU LINUX boot.
 * 	[95/03/21            bernadat]
 *
 *      Creation
 *      [94/03/10            card]
 */
/*
 * BSD FFS like declarations used to ease porting bootstrap to Linux ext2 fs
 */

#define	SBSIZE		EXT2_MIN_BLOCK_SIZE	/* Size of superblock */
#define	SBLOCK		((daddr_t) 2)		/* Location of superblock */

#define	NDADDR		EXT2_NDIR_BLOCKS
#define	NIADDR		(EXT2_N_BLOCKS - EXT2_NDIR_BLOCKS)

#define	MAXNAMLEN	255

#define	ROOTINO		EXT2_ROOT_INO

#define	NINDIR(fs)	EXT2_ADDR_PER_BLOCK(fs)

#define	IC_FASTLINK

#define	IFMT		00170000
#define	IFSOCK		0140000
#define	IFLNK		0120000
#define	IFREG		0100000
#define	IFBLK		0060000
#define	IFDIR		0040000
#define	IFCHR		0020000
#define	IFIFO		0010000
#define	ISUID		0004000
#define	ISGID		0002000
#define	ISVTX		0001000

int 	ino2blk(
		struct ext2_super_block *fs,
		struct ext2_group_desc *gd,
		int ino);

int 	fsbtodb(
		struct ext2_super_block *fs,
		int b);

int 	itoo(
	     struct ext2_super_block *fs,
	     int ino);

int 	blkoff(
	       struct ext2_super_block *fs,
	       vm_offset_t offset);

int	lblkno(
		struct ext2_super_block *fs,
		vm_offset_t offset);

typedef	struct	ext2fs_file *ext2fs_file_t;

int	blksize(
		struct ext2_super_block *fs,
		ext2fs_file_t fp,
		daddr_t file_block);




