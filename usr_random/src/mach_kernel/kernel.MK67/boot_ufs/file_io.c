/* 
 * Mach Operating System
 * Copyright (c) 1991,1990 Carnegie Mellon University
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

#include <kern/kalloc.h>
#include <kern/lock.h>
#include <boot_ufs/file_io.h>
#include <boot_ufs/dir.h>

#include <device/device_types.h>
#include <device/device.h>

#include <mach/mach_traps.h>
#include <mach/mach_interface.h>
#include <mach/mach_user_internal.h>
#include <mach/mach_port_internal.h>
#include <mach/mach_host_interface.h>
#include <mach/mach_host_internal.h>

#include <boot_ufs/boot_printf.h>

#include <sys/varargs.h>

#ifdef mac2

#include <sys/types.h>
#include <sys/disklabel.h>

#define NDISK 2

/* disk information per disk */
static struct disk {
  mach_port_t      disk_port;
  int              partition;
  struct disklabel disklabel;
} disk[NDISK];

/* adjust recnum and bytes_wanted to be within the Unix partition */
/* returns non-zero if error */
static int unix_partition(mach_port_t disk_port,
                          recnum_t *recnum,
                          int *bytes_wanted)
{
  struct disk *d;
  int bytes_available;

  /* get a pointer to the disk[] entry */
  if (disk[0].disk_port == disk_port) d = &disk[0];
  else if (disk[1].disk_port == disk_port) d = &disk[1];
  else return -1;

  /* make sure recnum is within bounds */
  if (*recnum >= d->disklabel.d_partitions[d->partition].p_size) return -1;

  /* limit bytes_wanted to be less or equal to bytes_available */
  bytes_available =
    (d->disklabel.d_partitions[d->partition].p_size - *recnum) *
      d->disklabel.d_secsize;
  if (*bytes_wanted > bytes_available) *bytes_wanted = bytes_available;

  /* add partition offset to recnum */
  (*recnum) += d->disklabel.d_partitions[d->partition].p_offset;

  /* all done */
  return 0;

} /* unix_partition() */

/* open a Unix disk, setup disklabel parameters */
static kern_return_t disk_open(mach_port_t master_disk_port,
                        int flags,
                        char *name,
                        mach_port_t *disk_port)
{
  struct disk *d;
  char mach_device[32];
  kern_return_t rc;
  int len;
  mach_msg_type_number_t count;
  io_buf_ptr_t buffer;

  /* pick a free disk[] */
  d = 0;
  if (!disk[0].disk_port) d = &disk[0];
  else if (!disk[1].disk_port) d = &disk[1];
  if (!d) return D_IO_ERROR;

  /* seperate the name into <mach device><unix partition> */
  len = strlen(name);
  if (len < 1) return D_IO_ERROR;
  d->partition = name[len - 1] - 'a';
  strcpy(mach_device, name);
  mach_device[len - 1] = 0;

  rc = device_open(master_disk_port, flags, mach_device, &d->disk_port);
  if (rc != D_SUCCESS) return rc;

  /* read the LABELSECTOR into a buffer */
  rc = device_read(d->disk_port,
                   0,
                   LABELSECTOR,
                   512,
                   &buffer,
                   &count);
  if (rc != D_SUCCESS) return rc;

  /* copy the disklabel into disk[] */
  if (count < sizeof(struct disklabel)) return D_IO_ERROR;
  bcopy(&buffer[LABELOFFSET], &d->disklabel, sizeof(struct disklabel));

  /* let go of the buffer allocated by device_read() */
  (void)vm_deallocate(mach_task_self(), (vm_address_t)buffer, (vm_size_t)count);

  /* make sure that this is a valid disklabel */
  if (d->disklabel.d_magic != DISKMAGIC) return D_IO_ERROR;
  if (d->disklabel.d_magic2 != DISKMAGIC) return D_IO_ERROR;

  /* make sure that the specified partition exists */
  if (d->partition < 0) return D_IO_ERROR;
  if (d->partition >= d->disklabel.d_npartitions) return D_IO_ERROR;
  if (d->disklabel.d_partitions[d->partition].p_size <= 0) return D_IO_ERROR;

  /* all done */
  *disk_port = d->disk_port;

  return D_SUCCESS;

} /* disk_open() */

/* read from a Unix disk */
static kern_return_t disk_read(mach_port_t disk_port,
                        dev_mode_t mode,
                        recnum_t recnum,
                        int bytes_wanted,
                        io_buf_ptr_t *data,
                        mach_msg_type_number_t *dataCnt)
{

  if (unix_partition(disk_port, &recnum, &bytes_wanted)) return D_IO_ERROR;

  return device_read(disk_port, mode, recnum, bytes_wanted, data, dataCnt);

} /* disk_read() */

/* write to a Unix disk */
static kern_return_t disk_write(mach_port_t disk_port,
                         dev_mode_t mode,
                         recnum_t recnum,
                         io_buf_ptr_t data,
                         mach_msg_type_number_t dataCnt,
                         int *bytes_written)
{

  if (unix_partition(disk_port, &recnum, (int *)&dataCnt)) return D_IO_ERROR;

  return device_write(disk_port, mode, recnum, data, dataCnt, bytes_written);

} /* disk_write() */

#endif

/*
 * Read a new inode into a file structure.
 */
int
read_inode(inumber, fp)
	ino_t			inumber;
	register struct file	*fp;
{
	vm_offset_t		buf;
	vm_size_t		buf_size;
	register struct fs	*fs;
	daddr_t			disk_block;
	kern_return_t		rc;

	fs = fp->f_fs;
	disk_block = itod(fs, inumber);

#ifdef mac2
	rc = disk_read(fp->f_dev,
		       0,
		       (recnum_t) fsbtodb(fp->f_fs, disk_block),
		       (int) fs->fs_bsize,
		       (char **)&buf,
		       &buf_size);
#else
	rc = device_read(fp->f_dev,
			 0,
			 (recnum_t) fsbtodb(fp->f_fs, disk_block),
			 (int) fs->fs_bsize,
			 (char **)&buf,
			 &buf_size);
#endif
	if (rc != KERN_SUCCESS)
	    return (rc);

	{
	    register struct dinode *dp;

	    dp = (struct dinode *)buf;
	    dp += itoo(fs, inumber);
	    fp->i_ic = dp->di_ic;
	}

	(void) vm_deallocate(mach_task_self(), buf, buf_size);

	/*
	 * Clear out the old buffers
	 */
	{
	    register int level;

	    for (level = 0; level < NIADDR; level++) {
		if (fp->f_blk[level] != 0) {
		    (void) vm_deallocate(mach_task_self(),
					 fp->f_blk[level],
					 fp->f_blksize[level]);
		    fp->f_blk[level] = 0;
		}
		fp->f_blkno[level] = -1;
	    }

	    if (fp->f_buf != 0) {
		(void) vm_deallocate(mach_task_self(),
				     fp->f_buf,
				     fp->f_buf_size);
		fp->f_buf = 0;
	    }
	    fp->f_buf_blkno = -1;
	}
	return (0);	 
}

/*
 * Given an offset in a file, find the disk block number that
 * contains that block.
 */
int
block_map(fp, file_block, disk_block_p)
	struct file	*fp;
	daddr_t		file_block;
	daddr_t		*disk_block_p;	/* out */
{
	int		level;
	int		idx;
	daddr_t		ind_block_num;
	kern_return_t	rc;

	/*
	 * Index structure of an inode:
	 *
	 * i_db[0..NDADDR-1]	hold block numbers for blocks
	 *			0..NDADDR-1
	 *
	 * i_ib[0]		index block 0 is the single indirect
	 *			block
	 *			holds block numbers for blocks
	 *			NDADDR .. NDADDR + NINDIR(fs)-1
	 *
	 * i_ib[1]		index block 1 is the double indirect
	 *			block
	 *			holds block numbers for INDEX blocks
	 *			for blocks
	 *			NDADDR + NINDIR(fs) ..
	 *			NDADDR + NINDIR(fs) + NINDIR(fs)**2 - 1
	 *
	 * i_ib[2]		index block 2 is the triple indirect
	 *			block
	 *			holds block numbers for double-indirect
	 *			blocks for blocks
	 *			NDADDR + NINDIR(fs) + NINDIR(fs)**2 ..
	 *			NDADDR + NINDIR(fs) + NINDIR(fs)**2
	 *				+ NINDIR(fs)**3 - 1
	 */

	lock_read(&fp->f_lock);

	if (file_block < NDADDR) {
	    /* Direct block. */
	    *disk_block_p = fp->i_db[file_block];
	    lock_done(&fp->f_lock);
	    return (0);
	}

	file_block -= NDADDR;

	/*
	 * nindir[0] = NINDIR
	 * nindir[1] = NINDIR**2
	 * nindir[2] = NINDIR**3
	 *	etc
	 */
	for (level = 0; level < NIADDR; level++) {
	    if (file_block < fp->f_nindir[level])
		break;
	    file_block -= fp->f_nindir[level];
	}
	if (level == NIADDR) {
	    /* Block number too high */
	    lock_done(&fp->f_lock);
	    return (FS_NOT_IN_FILE);
	}

	ind_block_num = fp->i_ib[level];

	for (; level >= 0; level--) {
	    vm_offset_t data;
	    vm_size_t size;

	    if (ind_block_num == 0) {
		lock_done(&fp->f_lock);
		*disk_block_p = 0;	/* missing */
		return (0);
	    }

	    if (fp->f_blkno[level] == ind_block_num) {
		/*
		 *	Cache hit.  Just pick up the data.
		 */

		data = fp->f_blk[level];
		size = 0;
	    } else {
		/*
		 *	Drop our lock while doing the read.
		 *	(The f_dev and f_fs fields don't change.)
		 */
		lock_done(&fp->f_lock);

#ifdef mac2
		rc = disk_read(fp->f_dev,
				0,
				(recnum_t) fsbtodb(fp->f_fs, ind_block_num),
				fp->f_fs->fs_bsize,
				(char **)&data,
				&size);
#else
		rc = device_read(fp->f_dev,
				0,
				(recnum_t) fsbtodb(fp->f_fs, ind_block_num),
				fp->f_fs->fs_bsize,
				(char **)&data,
				&size);
#endif
		if (rc != KERN_SUCCESS)
		    return (rc);

		/*
		 *	See if we can cache the data.  Need a write lock to
		 *	do this.  While we hold the write lock, we can't do
		 *	*anything* which might block for memory.  Otherwise
		 *	a non-privileged thread might deadlock with the
		 *	privileged threads.  We can't block while taking the
		 *	write lock.  Otherwise a non-privileged thread
		 *	blocked in the vm_deallocate (while holding a read
		 *	lock) will block a privileged thread.  For the same
		 *	reason, we can't take a read lock and then use
		 *	lock_read_to_write.
		 */

		if (lock_try_write(&fp->f_lock)) {
		    vm_offset_t olddata;
		    vm_size_t oldsize;

		    olddata = fp->f_blk[level];
		    oldsize = fp->f_blksize[level];

		    fp->f_blkno[level] = ind_block_num;
		    fp->f_blk[level] = data;
		    fp->f_blksize[level] = size;

		    /*
		     *	Return to holding a read lock, and
		     *	dispose of old data.
		     */

		    lock_write_to_read(&fp->f_lock);

		    if (oldsize != 0)
			(void) vm_deallocate(mach_task_self(),
					     olddata, oldsize);
		    size = 0;
		} else
			lock_read(&fp->f_lock);
	    }

	    if (level > 0) {
		idx = file_block / fp->f_nindir[level-1];
		file_block %= fp->f_nindir[level-1];
	    } else
		idx = file_block;

	    ind_block_num = ((daddr_t *)data)[idx];

	    if (size != 0)
		(void) vm_deallocate(mach_task_self(), data, size);
	}

	lock_done(&fp->f_lock);
	*disk_block_p = ind_block_num;
	return (0);
}

/*
 * Read a portion of a file into an internal buffer.  Return
 * the location in the buffer and the amount in the buffer.
 */
int
buf_read_file(fp, offset, buf_p, size_p)
	register struct file	*fp;
	vm_offset_t		offset;
	vm_offset_t		*buf_p;		/* out */
	vm_size_t		*size_p;	/* out */
{
	register struct fs	*fs;
	vm_offset_t		off;
	register daddr_t	file_block;
	daddr_t			disk_block;
	int			rc;
	vm_offset_t		block_size;

	if (offset >= fp->i_size)
	    return (FS_NOT_IN_FILE);

	fs = fp->f_fs;

	off = blkoff(fs, offset);
	file_block = lblkno(fs, offset);
	block_size = blksize(fs, fp, file_block);

	if (file_block != fp->f_buf_blkno) {
	    rc = block_map(fp, file_block, &disk_block);
	    if (rc != 0)
		return (rc);

	    if (fp->f_buf)
		(void)vm_deallocate(mach_task_self(),
				    fp->f_buf,
				    fp->f_buf_size);

	    if (disk_block == 0) {
		(void)vm_allocate(mach_task_self(),
				  &fp->f_buf,
				  block_size,
				  TRUE);
		fp->f_buf_size = block_size;
	    }
	    else {
#ifdef mac2
		rc = disk_read(fp->f_dev,
				0,
				(recnum_t) fsbtodb(fs, disk_block),
				(int) block_size,
				(char **) &fp->f_buf,
				&fp->f_buf_size);
#else
		rc = device_read(fp->f_dev,
				0,
				(recnum_t) fsbtodb(fs, disk_block),
				(int) block_size,
				(char **) &fp->f_buf,
				&fp->f_buf_size);
#endif
	    }
	    if (rc)
		return (rc);

	    fp->f_buf_blkno = file_block;
	}

	/*
	 * Return address of byte in buffer corresponding to
	 * offset, and size of remainder of buffer after that
	 * byte.
	 */
	*buf_p = fp->f_buf + off;
	*size_p = block_size - off;

	/*
	 * But truncate buffer at end of file.
	 */
	if (*size_p > fp->i_size - offset)
	    *size_p = fp->i_size - offset;

	return (0);
}

/*
 * Search a directory for a name and return its
 * i_number.
 */
int
search_directory(name, fp, inumber_p)
	char *		name;
	register struct file *fp;
	ino_t		*inumber_p;	/* out */
{
	vm_offset_t	buf;
	vm_size_t	buf_size;
	vm_offset_t	offset;
	register struct direct *dp;
	int		length;
	kern_return_t	rc;

	length = strlen(name);

	offset = 0;
	while (offset < fp->i_size) {
	    rc = buf_read_file(fp, offset, &buf, &buf_size);
	    if (rc != KERN_SUCCESS)
		return (rc);

	    dp = (struct direct *)buf;
	    if (dp->d_ino != 0) {
		if (dp->d_namlen == length &&
		    !strcmp(name, dp->d_name))
	    	{
		    /* found entry */
		    *inumber_p = dp->d_ino;
		    return (0);
		}
	    }
	    offset += dp->d_reclen;
	}
	return (FS_NO_ENTRY);
}

int
read_fs(dev, fsp)
	mach_port_t dev;
	struct fs **fsp;
{
	register struct fs *fs;
	vm_offset_t	buf;
	vm_size_t	buf_size;
	int		error;

#ifdef mac2
	error = disk_read(dev, 0, (recnum_t) SBLOCK, SBSIZE,
			    (char **) &buf, &buf_size);
#else
	error = device_read(dev, 0, (recnum_t) SBLOCK, SBSIZE,
			    (char **) &buf, &buf_size);
#endif
	if (error)
	    return (error);

	fs = (struct fs *)buf;
	if (fs->fs_magic != FS_MAGIC ||
	    fs->fs_bsize > MAXBSIZE ||
	    fs->fs_bsize < sizeof(struct fs)) {
		(void) vm_deallocate(mach_task_self(), buf, buf_size);
		return (FS_INVALID_FS);
	}
	/* don't read cylinder groups - we aren't modifying anything */

	*fsp = fs;
	return 0;
}

int
mount_fs(fp)
	register struct file	*fp;
{
	register struct fs *fs;
	int error;

	error = read_fs(fp->f_dev, &fp->f_fs);
	if (error)
	    return (error);
	fs = fp->f_fs;

	/*
	 * Calculate indirect block levels.
	 */
	{
	    register int	mult;
	    register int	level;

	    mult = 1;
	    for (level = 0; level < NIADDR; level++) {
		mult *= NINDIR(fs);
		fp->f_nindir[level] = mult;
	    }
	}

	return (0);
}

/*
 * Open a file.
 */
int
open_file(master_device_port, path, fp)
	mach_port_t	master_device_port;
	char *		path;
	struct file	*fp;
{
	register char	*cp, *ncp;
	register int	c;	/* char */
	register int	rc;
	ino_t		inumber, parent_inumber;
	int		nlinks = 0;

	char	*namebuf = 0;
	char	*component = 0;

	if (path == 0 || *path == '\0') {
	    rc = FS_NO_ENTRY;
	    goto exit;
	}

	/*
	 * Allocate buffers.
	 */
	namebuf = (char *) kalloc(MAXPATHLEN+1);
	component = (char *) kalloc(MAXPATHLEN+1);

	/*
	 * Copy name into buffer to allow modifying it.
	 */
	bcopy(path, namebuf, (unsigned)(strlen(path) + 1));

	/*
	 * Look for '/dev/xxx' at start of path, for
	 * root device.
	 */
	if (!strprefix(namebuf, "/dev/")) {
	    printf("no device name\n");
	    rc = FS_NO_ENTRY;
	    goto exit;
	}

	cp = namebuf + 5;	/* device */
	ncp = component;
	while ((c = *cp) != '\0' && c != '/') {
	    *ncp++ = c;
	    cp++;
	}
	*ncp = 0;

#ifdef mac2
	rc = disk_open(master_device_port,
		       D_READ|D_WRITE,
		       component,
		       &fp->f_dev);
#else
	rc = device_open(master_device_port,
			D_READ|D_WRITE,
			component,
			&fp->f_dev);
#endif
	if (rc)
	    goto exit;

	rc = mount_fs(fp);
	if (rc)
	    goto exit;

	inumber = (ino_t) ROOTINO;
	if ((rc = read_inode(inumber, fp)) != 0) {
	    printf("can't read root inode\n");
	    goto exit;
	}

	while (*cp) {

	    /*
	     * Check that current node is a directory.
	     */
	    if ((fp->i_mode & IFMT) != IFDIR) {
		rc = FS_NOT_DIRECTORY;
		goto exit;
	    }

	    /*
	     * Remove extra separators
	     */
	    while (*cp == '/')
		cp++;

	    /*
	     * Get next component of path name.
	     */
	    {
		register int	len = 0;

		ncp = component;
		while ((c = *cp) != '\0' && c != '/') {
		    if (len++ > MAXNAMLEN) {
			rc = FS_NAME_TOO_LONG;
			goto exit;
		    }
		    if (c & 0200) {
			rc = FS_INVALID_PARAMETER;
			goto exit;
		    }

		    *ncp++ = c;
		    cp++;
		}
		*ncp = 0;
	    }

	    /*
	     * Look up component in current directory.
	     * Save directory inumber in case we find a
	     * symbolic link.
	     */
	    parent_inumber = inumber;
	    rc = search_directory(component, fp, &inumber);
	    if (rc) {
		printf("%s: not found\n", path);
		goto exit;
	    }

	    /*
	     * Open next component.
	     */
	    if ((rc = read_inode(inumber, fp)) != 0)
		goto exit;

	    /*
	     * Check for symbolic link.
	     */
	    if ((fp->i_mode & IFMT) == IFLNK) {

		int	link_len = fp->i_size;
		int	len;

		len = strlen(cp) + 1;

		if (link_len + len >= MAXPATHLEN - 1) {
		    rc = FS_NAME_TOO_LONG;
		    goto exit;
		}

		if (++nlinks > MAXSYMLINKS) {
		    rc = FS_SYMLINK_LOOP;
		    goto exit;
		}

		ovbcopy(cp, &namebuf[link_len], len);

#ifdef	IC_FASTLINK
		if ((fp->i_flags & IC_FASTLINK) != 0) {
		    bcopy(fp->i_symlink, namebuf, (unsigned) link_len);
		}
		else
#endif	IC_FASTLINK
		{
		    /*
		     * Read file for symbolic link
		     */
		    vm_offset_t	buf;
		    vm_size_t	buf_size;
		    daddr_t	disk_block;
		    register struct fs *fs = fp->f_fs;

		    (void) block_map(fp, (daddr_t)0, &disk_block);
#ifdef mac2
		    rc = disk_read(fp->f_dev,
				     0,
				     (recnum_t) fsbtodb(fs, disk_block),
				     (int) blksize(fs, fp, 0),
				     (char **) &buf,
				     &buf_size);
#else
		    rc = device_read(fp->f_dev,
				     0,
				     (recnum_t) fsbtodb(fs, disk_block),
				     (int) blksize(fs, fp, 0),
				     (char **) &buf,
				     &buf_size);
#endif
		    if (rc)
			goto exit;

		    bcopy((char *)buf, namebuf, (unsigned)link_len);
		    (void) vm_deallocate(mach_task_self(), buf, buf_size);
		}

		/*
		 * If relative pathname, restart at parent directory.
		 * If absolute pathname, restart at root.
		 * If pathname begins '/dev/<device>/',
		 *	restart at root of that device.
		 */
		cp = namebuf;
		if (*cp != '/') {
		    inumber = parent_inumber;
		}
		else if (!strprefix(cp, "/dev/")) {
		    inumber = (ino_t)ROOTINO;
		}
		else {
		    ncp = component;
		    cp += 5;
		    while ((c = *cp) != '\0' && c != '/') {
			*ncp++ = c;
			cp++;
		    }
		    *ncp = '\0';

#ifdef mac2
		    rc = disk_open(master_device_port,
				   D_READ,
				   component,
				   &fp->f_dev);
#else
		    rc = device_open(master_device_port,
				D_READ,
				component,
				&fp->f_dev);
#endif
		    if (rc)
			goto exit;

		    rc = mount_fs(fp);
		    if (rc)
			goto exit;

		    inumber = (ino_t)ROOTINO;
		}
		if ((rc = read_inode(inumber, fp)) != 0)
		    goto exit;
	    }
	}

	/*
	 * Found terminal component.
	 */

	lock_init(&fp->f_lock, TRUE);
	rc = 0;
      exit:
	if (namebuf) kfree((vm_offset_t)namebuf, MAXPATHLEN+1);
	if (component) kfree((vm_offset_t)component, MAXPATHLEN+1);
	return (rc);
}

/*
 * Copy a portion of a file into kernel memory.
 * Cross block boundaries when necessary.
 */
int
read_file(fp, offset, start, size, resid)
	register struct file	*fp;
	vm_offset_t		offset;
	vm_offset_t		start;
	vm_size_t		size;
	vm_size_t		*resid;	/* out */
{
	int			rc;
	register vm_size_t	csize;
	vm_offset_t		buf;
	vm_size_t		buf_size;

	while (size != 0) {
	    rc = buf_read_file(fp, offset, &buf, &buf_size);
	    if (rc)
		return (rc);

	    csize = size;
	    if (csize > buf_size)
		csize = buf_size;
	    if (csize == 0)
		break;

	    bcopy((char *)buf, (char *)start, csize);

	    offset += csize;
	    start  += csize;
	    size   -= csize;
	}
	if (resid)
	    *resid = size;

	return (0);
}

#if	0
/*
 * Special read and write routines for default pager.
 * Assume that all offsets and sizes are multiples
 * of DEV_BSIZE.
 */

/*
 * Read all or part of a data block, and
 * return a pointer to the appropriate part.
 * Caller must deallocate the block when done.
 */
int
page_read_file(fp, offset, size, addr, size_read)
	register struct file	*fp;
	vm_offset_t		offset;
	vm_size_t		size;
	vm_offset_t		*addr;		/* out */
	vm_size_t		*size_read;	/* out */
{
	register struct fs	*fs;
	vm_offset_t		off;
	register daddr_t	file_block;
	daddr_t			disk_block;
	register int		rc;
	vm_size_t		block_size;

	if (offset % DEV_BSIZE != 0 ||
	    size % DEV_BSIZE != 0)
	    panic("page_read_file");

	if (offset >= fp->i_size)
	    return (FS_NOT_IN_FILE);

	fs = fp->f_fs;

	off = blkoff(fs, offset);
	file_block = lblkno(fs, offset);
	block_size = blksize(fs, fp, file_block);

	rc = block_map(fp, file_block, &disk_block);
	if (rc != 0)
	    return (rc);
	if (disk_block == 0)
	    return (FS_NOT_IN_FILE);

	if (size > block_size)
	    size = block_size;

#ifdef mac2
	return (disk_read(fp->f_dev,
			0,
			(recnum_t) (fsbtodb(fs, disk_block) + btodb(off)),
			(int) size,
			(char **) addr,
			size_read));
#else
	return (device_read(fp->f_dev,
			0,
			(recnum_t) (fsbtodb(fs, disk_block) + btodb(off)),
			(int) size,
			(char **) addr,
			size_read));
#endif
}

/*
 * Write all or part of a data block, and
 * return the amount written.
 */
int
page_write_file(fp, offset, addr, size, size_written)
	register struct file	*fp;
	vm_offset_t		offset;
	vm_offset_t		addr;
	vm_size_t		size;
	vm_offset_t		*size_written;	/* out */
{
	register struct fs	*fs;
	vm_offset_t		off;
	register daddr_t	file_block;
	daddr_t			disk_block;
	int			rc;
	vm_offset_t		block_size;

	if (offset % DEV_BSIZE != 0 ||
	    size % DEV_BSIZE != 0)
	    panic("page_write_file");

	if (offset >= fp->i_size)
	    return (FS_NOT_IN_FILE);

	fs = fp->f_fs;

	off = blkoff(fs, offset);
	file_block = lblkno(fs, offset);
	block_size = blksize(fs, fp, file_block);

	rc = block_map(fp, file_block, &disk_block);
	if (rc != 0)
	    return (rc);
	if (disk_block == 0)
	    return (FS_NOT_IN_FILE);

	if (size > block_size)
	    size = block_size;

	/*
	 * Write the data.  Wait for completion to keep
	 * reads from getting ahead of writes and reading
	 * stale data.
	 */
#ifdef mac2
	return (disk_write(
			fp->f_dev,
			0,
			(recnum_t) (fsbtodb(fs, disk_block) + btodb(off)),
			(char *) addr,
			size,
			(int *)size_written));
#else
	return (device_write(
			fp->f_dev,
			0,
			(recnum_t) (fsbtodb(fs, disk_block) + btodb(off)),
			(char *) addr,
			size,
			(int *)size_written));
#endif
}

/*
 * Wire down file buffers if caller needs wired memory.
 */
int
file_wire(master_host_port, fp)
	mach_port_t	master_host_port;
	register struct file	*fp;
{
	register int	rc;
	register int	i;

	/*
	 * The disk super-block...
	 */
	rc = vm_wire(master_host_port,
			mach_task_self(),
			(vm_offset_t) fp->f_fs,
			SBSIZE,
			VM_PROT_READ|VM_PROT_WRITE);
	if (rc != KERN_SUCCESS)
	    return (rc);

	/*
	 * The inode buffer
	 */
	if (fp->f_buf) {
	    rc = vm_wire(master_host_port,
			mach_task_self(),
			fp->f_buf,
			fp->f_buf_size,
			VM_PROT_READ|VM_PROT_WRITE);
	    if (rc != KERN_SUCCESS)
		return (rc);
	}

	/*
	 * And the data blocks.
	 */
	for (i = 0; i < NIADDR; i++) {
	    if (fp->f_blk[i]) {
		rc = vm_wire(master_host_port,
			mach_task_self(),
			fp->f_blk[i],
			fp->f_blksize[i],
			VM_PROT_READ|VM_PROT_WRITE);
		if (rc != KERN_SUCCESS)
		    return (rc);
	    }
	}
	return (0);
}
#endif	0

/*
 * Make an empty file_direct for a device.
 */
int
open_file_direct(dev, fdp)
	mach_port_t dev;
	register struct file_direct *fdp;
{
	struct fs *fs;
	int rc;

	rc = read_fs(dev, &fs);
	if (rc)
		return rc;

	fdp->fd_dev = dev;
	fdp->fd_fs = fs;
	fdp->fd_blocks = (daddr_t *) 0;
	fdp->fd_size = 0;
	return 0;
}

/*
 * Add blocks from a file to a file_direct.
 */
int
add_file_direct(fdp, fp)
	register struct file_direct *fdp;
	register struct file *fp;
{
	register struct fs *fs;
	long num_blocks, i;
	vm_offset_t buffer;
	vm_size_t size;
	int rc;

	/* the file must be on the same device */

	if (fdp->fd_dev != fp->f_dev)
		return FS_INVALID_FS;
	fs = fdp->fd_fs;

	/* calculate number of blocks in the file, ignoring fragments */

	num_blocks = lblkno(fs, fp->i_size);

	/* allocate memory for a bigger array */

	size = (num_blocks + fdp->fd_size) * sizeof(daddr_t);
	rc = vm_allocate(mach_task_self(), &buffer, size, TRUE);
	if (rc != KERN_SUCCESS)
		return rc;

	/* lookup new block addresses */

	for (i = 0; i < num_blocks; i++) {
		daddr_t disk_block;

		rc = block_map(fp, (daddr_t) i, &disk_block);
		if (rc != 0) {
			(void) vm_deallocate(mach_task_self(), buffer, size);
			return rc;
		}

		((daddr_t *) buffer)[fdp->fd_size + i] = disk_block;
	}

	/* copy old addresses and install the new array */

	if (fdp->fd_blocks != 0) {
		bcopy((char *) fdp->fd_blocks, (char *) buffer,
		      fdp->fd_size * sizeof(daddr_t));

		(void) vm_deallocate(mach_task_self(),
				(vm_offset_t) fdp->fd_blocks,
				(vm_size_t) (fdp->fd_size * sizeof(daddr_t)));
	}
	fdp->fd_blocks = (daddr_t *) buffer;
	fdp->fd_size += num_blocks;

	/* deallocate cached blocks */

	if (fp->f_buf != 0) {
		(void) vm_deallocate(mach_task_self(),
				     fp->f_buf,
				     fp->f_buf_size);
		fp->f_buf = 0;
	}

	for (i = 0; i < NIADDR; i++) {
		if (fp->f_blk[i] != 0) {
			(void) vm_deallocate(mach_task_self(),
					     fp->f_blk[i],
					     fp->f_blksize[i]);
			fp->f_blk[i] = 0;
		}
	}

	return 0;
}

/*
 * Wire down the disk address array for callers that need to use wired memory.
 */
int
file_wire_direct(master_host_port, fdp)
	mach_port_t master_host_port;
	register struct file_direct *fdp;
{
	int rc;

	/* wire the file system */

	rc = vm_wire(master_host_port, mach_task_self(),
		     (vm_offset_t) fdp->fd_fs, SBSIZE,
		     VM_PROT_READ|VM_PROT_WRITE);
	if (rc != KERN_SUCCESS)
	    return (rc);

	/* wire the block addresses */

	rc = vm_wire(master_host_port, mach_task_self(),
		     (vm_offset_t) fdp->fd_blocks,
		     (vm_size_t) (fdp->fd_size * sizeof(daddr_t)),
		     VM_PROT_READ|VM_PROT_WRITE);
	return (rc);
}

/*
 * Special read and write routines for default pager.
 * Assume that all offsets and sizes are multiples
 * of DEV_BSIZE.
 */

/*
 * Read all or part of a data block, and
 * return a pointer to the appropriate part.
 * Caller must deallocate the block when done.
 */
int
page_read_file_direct(fdp, offset, size, addr, size_read)
	register struct file_direct *fdp;
	vm_offset_t offset;
	vm_size_t size;
	vm_offset_t *addr;	/* out */
	vm_size_t *size_read;	/* out */
{
	register struct fs	*fs;
	vm_offset_t		off;
	register daddr_t	file_block;
	daddr_t			disk_block;

	if (offset % DEV_BSIZE != 0 ||
	    size % DEV_BSIZE != 0)
	    panic("page_read_file_direct");

	fs = fdp->fd_fs;

	if (offset >= (fdp->fd_size << fs->fs_bshift))
	    return (FS_NOT_IN_FILE);

	off = blkoff(fs, offset);
	file_block = lblkno(fs, offset);

	disk_block = fdp->fd_blocks[file_block];
	if (disk_block == 0)
	    return (FS_NOT_IN_FILE);

	if (size > fs->fs_bsize)
	    size = fs->fs_bsize;

#ifdef mac2
	return (disk_read(fdp->fd_dev,
			0,
			(recnum_t) (fsbtodb(fs, disk_block) + btodb(off)),
			(int) size,
			(char **) addr,
			size_read));
#else
	return (device_read(fdp->fd_dev,
			0,
			(recnum_t) (fsbtodb(fs, disk_block) + btodb(off)),
			(int) size,
			(char **) addr,
			size_read));
#endif
}

/*
 * Write all or part of a data block, and
 * return the amount written.
 */
int
page_write_file_direct(fdp, offset, addr, size, size_written)
	register struct file_direct *fdp;
	vm_offset_t offset;
	vm_offset_t addr;
	vm_size_t size;
	vm_offset_t *size_written;	/* out */
{
	register struct fs	*fs;
	vm_offset_t		off;
	register daddr_t	file_block;
	daddr_t			disk_block;
	int			rc;
	vm_offset_t		block_size;

	if (offset % DEV_BSIZE != 0 ||
	    size % DEV_BSIZE != 0)
	    panic("page_write_file");

	fs = fdp->fd_fs;

	if (offset >= (fdp->fd_size << fs->fs_bshift))
	    return (FS_NOT_IN_FILE);

	off = blkoff(fs, offset);
	file_block = lblkno(fs, offset);

	disk_block = fdp->fd_blocks[file_block];
	if (disk_block == 0)
	    return (FS_NOT_IN_FILE);

	if (size > fs->fs_bsize)
	    size = fs->fs_bsize;

	/*
	 * Write the data.  Wait for completion to keep
	 * reads from getting ahead of writes and reading
	 * stale data.
	 */
#ifdef mac2
	return (disk_write(
			fdp->fd_dev,
			0,
			(recnum_t) (fsbtodb(fs, disk_block) + btodb(off)),
			(char *) addr,
			size,
			(int *)size_written));
#else
	return (device_write(
			fdp->fd_dev,
			0,
			(recnum_t) (fsbtodb(fs, disk_block) + btodb(off)),
			(char *) addr,
			size,
			(int *)size_written));
#endif
}

/*
 * Character subroutines
 */

/*
 * Concatenate a group of strings together into a buffer.
 * Return a pointer to the trailing '\0' character in
 * the result string.
 * The list of strings ends with a '(char *)0'.
 */
/*VARARGS1*/
char *
strbuild(dest, va_alist)
	register char *	dest;
	va_dcl
{
	va_list	argptr;
	register char *	src;
	register int	c;

	va_start(argptr);
	while ((src = va_arg(argptr, char *)) != (char *)0) {

	    while ((c = *src++) != '\0')
		*dest++ = c;
	}
	*dest = '\0';
	return (dest);
}


/*
 * Return TRUE if string 2 is a prefix of string 1.
 */
boolean_t
strprefix(s1, s2)
	register char *s1, *s2;
{
	register int	c;

	while ((c = *s2++) != '\0') {
	    if (c != *s1++)
		return (FALSE);
	}
	return (TRUE);
}
