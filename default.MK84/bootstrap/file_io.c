/* 
 * Mach Operating System
 * Copyright (c) 1993,1992,1991,1990 Carnegie Mellon University
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
 * $Log:	file_io.c,v $
 * Revision 2.9  93/11/17  15:58:41  dbg
 * 	remove_file_direct returns 'void'.
 * 	[93/09/23            dbg]
 * 
 * 	Heavy surgery to cope with multiple filesystem types.
 * 	Split out UFS code in a separate file.
 * 	Removed unused code, so the addition of Dos is a wash.
 * 	Merged open_file_direct and add_file_direct into a
 * 	single make_file_direct.
 * 	[93/09/14  11:57:38  af]
 * 
 * Revision 2.8  93/05/10  19:40:24  rvb
 * 	Changed "" includes to <>
 * 	[93/04/30            mrt]
 * 
 * Revision 2.7  93/05/10  17:44:48  rvb
 * 	Include files specified with quotes dont work properly
 * 	when the C file in in the master directory but the
 * 	include file is in the shadow directory. Change to
 * 	using angle brackets.
 * 	Ian Dall <DALL@hfrd.dsto.gov.au>	4/28/93
 * 	[93/05/10  13:15:31  rvb]
 * 
 * Revision 2.6  93/01/14  17:09:10  danner
 * 	64bit clean.  Just type-bug fixes, actually.
 * 	[92/11/30            af]
 * 
 * Revision 2.5  92/03/01  00:39:32  rpd
 * 	Fixed device_get_status argument types.
 * 	[92/02/29            rpd]
 * 
 * Revision 2.4  92/02/23  22:25:43  elf
 * 	Removed debugging printf.
 * 	[92/02/23  13:16:45  af]
 * 
 * 	Added variation of file_direct to page on raw devices.
 * 	[92/02/22  18:53:04  af]
 * 
 * 	Added remove_file_direct().  Commented out some unused code.
 * 	[92/02/19  17:31:01  af]
 * 
 * Revision 2.3  92/01/24  18:14:31  rpd
 * 	Added casts to device_read dataCnt argument to mollify
 * 	buggy versions of gcc.
 * 	[92/01/24            rpd]
 * 
 * Revision 2.2  92/01/03  19:57:13  dbg
 * 	Make file_direct self-contained: add the few fields needed from
 * 	the superblock.
 * 	[91/10/17            dbg]
 * 
 * 	Deallocate unused superblocks when switching file systems.  Add
 * 	file_close routine to free space taken by file metadata.
 * 	[91/09/25            dbg]
 * 
 * 	Move outside of kernel.
 * 	Unmodify open_file to put arrays back on stack.
 * 	[91/09/04            dbg]
 * 
 * Revision 2.8  91/08/28  11:09:42  jsb
 * 	Added struct file_direct and associated functions.
 * 	[91/08/19            rpd]
 * 
 * Revision 2.7  91/07/31  17:24:07  dbg
 * 	Call vm_wire instead of vm_pageable.
 * 	[91/07/30  16:38:01  dbg]
 * 
 * Revision 2.6  91/05/18  14:28:45  rpd
 * 	Changed block_map to avoid blocking operations
 * 	while holding an exclusive lock.
 * 	[91/04/06            rpd]
 * 	Added locking in block_map.
 * 	[91/04/03            rpd]
 * 
 * Revision 2.5  91/05/14  15:22:53  mrt
 * 	Correcting copyright
 * 
 * Revision 2.4  91/02/05  17:01:23  mrt
 * 	Changed to new copyright
 * 	[91/01/28  14:54:52  mrt]
 * 
 * Revision 2.3  90/10/25  14:41:42  rwd
 * 	Modified open_file to allocate arrays from heap, not stack.
 * 	[90/10/23            rpd]
 * 
 * Revision 2.2  90/08/27  21:45:27  dbg
 * 	Reduce lint.
 * 	[90/08/13            dbg]
 * 
 * 	Created from include files, bsd4.3-Reno (public domain) source,
 * 	and old code written at CMU.
 * 	[90/07/17            dbg]
 * 
 */

/*
 * Stand-alone file reading package.
 */

#include <strings.h>
#include <device/device_types.h>
#include <device/device.h>

#include <mach/mach_traps.h>
#include <mach/mach_interface.h>

#include <file_io.h>

extern struct filesystem_ops ufs_ops, dosfs_ops;

struct filesystem_ops *all_filesys[] = 
	{ &ufs_ops, &dosfs_ops, (struct filesystem_ops *)0 };

/*
 * Open a file.
 */
int
open_file(
	mach_port_t	master_device_port,
	char *		path,
	struct file	*fp)
{
	register char	*cp, *component;
	register int	c;	/* char */
	register int	rc;
	struct filesystem_ops	**fsops;

	char	namebuf[MAXPATHLEN+1];

rescan:
	fp->f_flags = 0;

	if (path == 0 || *path == '\0') {
	    return FS_NO_ENTRY;
	}

	/*
	 * Copy name into buffer to allow modifying it.
	 */
	strcpy(namebuf, path);
	path = namebuf;

	/*
	 * Look for '/dev/xxx' at start of path, for
	 * root device.
	 */
	if (!strprefix(path, "/dev/")) {
	    printf("no device name\n");
	    return FS_NO_ENTRY;
	}

	cp = path + 5;	/* device */
	component = cp;
	while ((c = *cp) != '\0' && c != '/') {
	    cp++;
	}
	*cp = '\0';

	rc = device_open(master_device_port,
			D_READ|D_WRITE,
			component,
			&fp->f_dev);
	*cp = c;

	if (rc)
	    return rc;

	if (c == 0) {
	    goto out_ok;
	}

	for (fsops = all_filesys; *fsops; fsops++)
		if (((*fsops)->isa_fs)(fp))
			break;

	if (*fsops == (struct filesystem_ops *)0)
		return FS_INVALID_FS;

	fp->f_ops = *fsops;
	fp->f_flags = F_FSYS;

	rc = (*fp->f_ops->open)(fp, cp);
	if (rc == FS_ISA_SYMLINK) {
		close_file(fp);
		path = cp;
		goto rescan;
	}

	/*
	 * At error exit, close file to free storage.
	 */
	if (rc) {
	    printf("%s: not found\n", path);
	    close_file(fp);
	    return rc;
	}

    out_ok:
	mutex_init(&fp->f_lock);
	return 0;
}

/*
 * Close file - free all storage used.
 */
void
close_file(
	register struct file	*fp)
{
	if (fp->f_ops)
		(*fp->f_ops->close)(fp);
	fp->f_ops = 0;
}

/*
 * Copy a portion of a file into kernel memory.
 * Cross block boundaries when necessary.
 */
int
read_file(
	register struct file	*fp,
	vm_offset_t		offset,
	vm_offset_t		start,
	vm_size_t		size,
	vm_size_t		*resid)	/* out */
{
	if (fp->f_ops)
		return (*fp->f_ops->read)(fp, offset, start, size, resid);
	else
		return FS_INVALID_PARAMETER;
}


/* simple utility: only works for 2^n */
log2(register unsigned int n)
{
	register int i = 0;

	while ((n & 1) == 0) {
		i++;
		n >>= 1;
	}
	return i;
}

/*
 * Make an empty file_direct for a device.
 */
int
make_file_direct(
	struct file	*fp,
	register struct file_direct *fdp)
{
	int rc;

	if (!file_is_structured(fp)) {

		int	result[DEV_GET_SIZE_COUNT];
		natural_t count;

		fdp->fd_dev     = fp->f_dev;
		fdp->fd_blocks  = (daddr_t *) 0;
		fdp->fd_bsize   = vm_page_size;
		fdp->fd_bshift  = log2(vm_page_size);
		fdp->fd_fsbtodb = 0;	/* later */
		fdp->fd_size    = 0;	/* later */

		count = DEV_GET_SIZE_COUNT;
		rc = device_get_status(	fdp->fd_dev, DEV_GET_SIZE,
					result, &count);
		if (rc)
			return rc;
		fdp->fd_size = result[DEV_GET_SIZE_DEVICE_SIZE] >> fdp->fd_bshift;
		fdp->fd_fsbtodb = log2(fdp->fd_bsize/result[DEV_GET_SIZE_RECORD_SIZE]);
		return 0;
	}

	/*
	 * File-structured, be specific
	 */
	if (fp->f_ops)
		rc = (*fp->f_ops->make_direct)(fp, fdp);
	else
		rc = FS_INVALID_PARAMETER;
	return rc;
}

/*
 * Tear down a file_direct
 */
void remove_file_direct(
	struct file_direct	*fdp)
{
	if (fdp->fd_blocks)
	(void) vm_deallocate(mach_task_self(),
			     (vm_offset_t) fdp->fd_blocks,
			     (vm_size_t) (fdp->fd_size * sizeof(daddr_t)));
	fdp->fd_blocks = 0; /* sanity */
	/* xxx should lose a ref to fdp->fd_dev here (and elsewhere) xxx */
}

/*
 * Read and write routines for default pager.
 * Assume that all offsets and sizes are multiples
 * of DEV_BSIZE.
 */

#define	fdir_blkoff(fdp, offset)	/* offset % fd_bsize */ \
	((offset) & ((fdp)->fd_bsize - 1))
#define	fdir_lblkno(fdp, offset)	/* offset / fd_bsize */ \
	((offset) >> (fdp)->fd_bshift)

#define	fdir_fsbtodb(fdp, block)	/* offset * fd_bsize / DEV_BSIZE */ \
	((block) << (fdp)->fd_fsbtodb)

/*
 * Read all or part of a data block, and
 * return a pointer to the appropriate part.
 * Caller must deallocate the block when done.
 */
int
page_read_file_direct(
	register struct file_direct *fdp,
	vm_offset_t	offset,
	vm_size_t	size,
	vm_offset_t	*addr,			/* out */
	mach_msg_type_number_t *size_read)	/* out */
{
	vm_offset_t		off;
	register daddr_t	file_block;
	daddr_t			disk_block;

	if (offset % DEV_BSIZE != 0 ||
	    size % DEV_BSIZE != 0)
	    panic("page_read_file_direct");

	if (offset >= (fdp->fd_size << fdp->fd_bshift))
	    return FS_NOT_IN_FILE;

	off = fdir_blkoff(fdp, offset);
	file_block = fdir_lblkno(fdp, offset);

	if (file_is_device(fdp)) {
	    disk_block = file_block;
	} else {
	    disk_block = fdp->fd_blocks[file_block];
	    if (disk_block == 0)
		return FS_NOT_IN_FILE;
	}

	if (size > fdp->fd_bsize)
	    size = fdp->fd_bsize;

	return device_read(fdp->fd_dev,
			0,
			(recnum_t) (fdir_fsbtodb(fdp, disk_block) + btodb(off)),
			(int) size,
			(char **) addr,
			size_read);
}

/*
 * Write all or part of a data block, and
 * return the amount written.
 */
int
page_write_file_direct(
	register struct file_direct *fdp,
	vm_offset_t	offset,
	vm_offset_t	addr,
	vm_size_t	size,
	vm_offset_t	*size_written)	/* out */
{
	vm_offset_t		off;
	register daddr_t	file_block;
	daddr_t			disk_block;
	int			rc, num_written;
	vm_offset_t		block_size;

	if (offset % DEV_BSIZE != 0 ||
	    size % DEV_BSIZE != 0)
	    panic("page_write_file");

	if (offset >= (fdp->fd_size << fdp->fd_bshift))
	    return FS_NOT_IN_FILE;

	off = fdir_blkoff(fdp, offset);
	file_block = fdir_lblkno(fdp, offset);

	if (file_is_device(fdp)) {
	    disk_block = file_block;
	} else {
	    disk_block = fdp->fd_blocks[file_block];
	    if (disk_block == 0)
		return FS_NOT_IN_FILE;
	}

	if (size > fdp->fd_bsize)
	    size = fdp->fd_bsize;

	/*
	 * Write the data.  Wait for completion to keep
	 * reads from getting ahead of writes and reading
	 * stale data.
	 */
	rc = device_write(
			fdp->fd_dev,
			0,
			(recnum_t) (fdir_fsbtodb(fdp, disk_block) + btodb(off)),
			(char *) addr,
			size,
			&num_written);
	*size_written = num_written;
	return rc;
}

