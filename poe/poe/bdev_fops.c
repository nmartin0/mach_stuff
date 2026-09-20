/* 
 * Mach Operating System
 * Copyright (c) 1994-1990 Carnegie Mellon University
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
 *  Software Distribution Coordinator   or   Software.Distribution@CS.CMU.EDU
 *  School of Computer Science
 *  Carnegie Mellon University
 *  Pittsburgh PA 15213-3890
 * 
 * any improvements or extensions that they made and grant Carnegie Mellon
 * the rights to redistribute these changes.
 */
/*
 * HISTORY
 * $Log:	bdev_fops.c,v $
 * Revision 2.5  94/03/25  18:21:41  mrt
 * 	Added comments. Changed bdev_speical to use the split
 * 	block and char special device switches. Removed OLD_xxX
 * 	routine.
 * 	[94/03/09            mrt]
 * 
 * Revision 2.3  91/06/18  13:06:19  jjc
 * 	Made changes to enable reading/writing raw disk partitions:
 * 		1) Modified bdev_special() to handle root disk as a character 
 * 		   or a block device.
 * 		2) Added a mode argument to bdev_make_special() for 
 * 		   device_open().
 * 		3) Tossed definition of D_READ and included 
 * 		   <device/device_types.h> for device modes instead.
 * 		4) Implemented bdev_read().
 * 		5) Included <ufs_param.h> for DEV_BSIZE.
 * 	[91/04/16            jjc]
 * 
 * Revision 2.2  90/09/08  00:06:20  rwd
 * 	Include cthread_self in debugging output.
 * 	[90/09/04            rwd]
 * 	First checkin
 * 	[90/08/31  13:17:45  rwd]
 * 
 */
/*
 *	File:	./bdev_fops.c
 *	Author:	Joseph S. Barrera III, Randall W. Dean
 *
 *	Copyright (c) 1990 Joseph S. Barrera III, Randall W. Dean
 */
/*
 *  bdev_fops provide fops on block devices or raw disk (character
 *	devices)
 *	bdev_open, bdev_close do nothing but return success
 *	bdev_device_read read bytes of data from device port in the snode
 *		record number and bytes wanted are input, data is returned
 *		in buffer that device_read and vm_allocated
 *	bdev_read reads bytes of data from the device port in the snode
 *		offset is a character offset into the file and the data
 *		is copied into the  buffer provided by the caller.
 *	bdev_write writes data to offset using the device port in the snode
 *		offset is given in bytes, but must fall on a block boundary
 *		if not all the bytes are written the count of the left-over ones
 *		will be returned.
 *	bdev_select, bdev_getpager are not implemented and return EINVAL
 *	bdev_special creates and initializes an snode for a given device name.
 */

#include <mach.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <ux_user.h>
#include <fnode.h>
#include <sys/ioctl.h>
#include <bsd_ioctl.h>
#include <subr_fops.h>
#include <machine/spec.h>
#include <device/device_types.h>
#include <ufs_param.h>

/*
 *  It would be nice to get rid of the dinfo structure...
 */

struct dinfo {
	mach_port_t	di_port;	/* port to mapped device */
};

extern int bdev_open();
extern int bdev_close();
extern int bdev_read();
extern int bdev_write();
extern int bdev_select();
extern int bdev_getpager();

struct fops bdev_fops = {
	bdev_open,
	bdev_close,
	bdev_read,
	bdev_write,
	spec_getstat,
	spec_setstat,
	bdev_select,
	nil_ioctl,
	bdev_getpager,
	nil_lookup,
	nil_create,
	nil_link,
	nil_unlink,
};
	
struct fnfs bdev_fs = {
	0,		/* fs_mountpoint */
	0,		/* fs_root */
	&bdev_fops,	/* fs_fops */
	TRUE,		/* fs_mayseek */
	TRUE,		/* fs_maymap */
};

struct fnfs cdev_fs = {
	0,		/* fs_mountpoint */
	0,		/* fs_root */
	&bdev_fops,	/* fs_fops */
	TRUE,		/* fs_mayseek */
	FALSE,		/* fs_maymap */
};

int bdev_debug = 0;

bdev_open()
{
	return 0;
}

bdev_close()
{
	return 0;
}

/*	
 *	read when wanting data starting at a record boundry
 *	data is returned in a dynamically allocated data buffer
 */
bdev_device_read(sn, mode, recnum, bytes_wanted, data, data_size)
	struct snode *sn;
	int mode;
	int recnum;
	int bytes_wanted;
	vm_offset_t *data;
	vm_size_t *data_size;
{
	struct dinfo *di = (struct dinfo *) sn->sn_private;
	int status;

	if (bdev_debug) {
		printf("[%x]",cthread_self());
		printf("device_read(0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x)\r\n",di->di_port, mode, recnum, bytes_wanted,  data, data_size);
	}
	status =  device_read(di->di_port, mode, recnum, bytes_wanted,
				  data, data_size);
	if (status)
	    printf("device_read(0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x) 0x%x\r\n",di->di_port, mode, recnum, bytes_wanted,  data, data_size, status);
	return status;
}
/*
 *	Read when data offset in file may not be an integral record number.
 *	Data is copied into buffer supplied by caller.
 */
bdev_read(sn, offset, buffer, size, resid)
	struct snode		*sn;
	register vm_offset_t	offset;
	vm_offset_t		buffer;
	vm_size_t		size;
	vm_size_t		*resid;
{
	register int		bytes_wanted;
	vm_offset_t		data;
	vm_size_t		data_count;
	struct dinfo		*di = (struct dinfo *) sn->sn_private;
	register int		recnum;
	register int		status;

	if (bdev_debug) {
		printf("[%x]",cthread_self());
		printf("bdev_read(0x%x, 0x%x, 0x%x, 0x%x, 0x%x)\n",
			sn, offset, buffer, size, resid);
	}

	/*
	 *	Figure out which record to start reading from, the offset
	 *	into the record for the data, and how many bytes to read.
	 */
	recnum = offset / DEV_BSIZE;
	offset = offset % DEV_BSIZE;
	bytes_wanted = size + offset;

	status =  device_read(di->di_port, 0, recnum, bytes_wanted, &data,
			      &data_count);

	if (status == D_SUCCESS) {
		bcopy(data + offset, buffer, size);
		if (data_count >= bytes_wanted)
			*resid = 0;
		else
			*resid = bytes_wanted - data_count;
		(void)vm_deallocate(mach_task_self(), data, data_count);
	}
	else {
		printf("device_read(0x%x,0x%x,0x%x,0x%x,0x%x,0x%x) 0x%x\r\n",
			di->di_port, 0, recnum, bytes_wanted, data,
			data_count, status);
		buffer = 0;
		*resid = size;
	}

	return status;
}
/*
 *	Write from user buffer to  offset in file
 *	offset must be an integral number of records
 */
bdev_write(sn, offset, buffer, size, resid)
	struct snode		*sn;
	register vm_offset_t	offset;
	vm_offset_t		buffer;
	vm_size_t		size;
	vm_size_t		*resid;
{
	int			bytes_written;
	struct dinfo		*di = (struct dinfo *) sn->sn_private;
	register int		recnum;
	register int		status;

	if (bdev_debug) {
		printf("[%x]",cthread_self());
		printf("bdev_write(0x%x, 0x%x, 0x%x, 0x%x, 0x%x)\n",
			sn, offset, buffer, size, resid);
	}

	recnum = offset / DEV_BSIZE;
	offset = offset % DEV_BSIZE;
	if (offset != 0) {
		printf("bdev_write: offset % DEV_BSIZE != 0\n");
		return D_INVALID_RECNUM;
	}

	status =  device_write(di->di_port, 0, recnum, buffer, size,
			       &bytes_written);

	if (status == D_SUCCESS) {
		if (bytes_written >= size)
			*resid = 0;
		else
			*resid = bytes_written - size;
	}
	else {
		printf("device_write(0x%x,0x%x,0x%x,0x%x,0x%x,0x%x) 0x%x\r\n",
			di->di_port, 0, recnum, buffer, size, 
			bytes_written, status);
		buffer = 0;
		*resid = size;
	}

	return status;	
}

bdev_select()
{
	printf("bdev_select: called!\n");
	return EINVAL;
}

bdev_getpager()
{
	printf("bdev_getpager: called!\n");
	return EINVAL;
}

bdev_make_special(sn, devname, mode)
	struct snode *sn;
	char *devname;
	dev_mode_t mode;
{
	int error;
	extern mach_port_t device_server_port;
	struct dinfo *di;

	di = (struct dinfo *) malloc(sizeof(*di));
	error = device_open(device_server_port, mode, devname, &di->di_port);
	if (error) {
		printf("error opening device %s %d\n", devname, error);
		return error;
	}
	sn->sn_private = (char *) di;
	return 0;
}

bdev_special(fnp, fmt, dev)
	struct fnode **fnp;
	int fmt;
	dev_t dev;
{
	static char devname[8];/* XXX */
	int error;
	int major_num = major(dev);
	int minor_num = minor(dev);

	if ( fmt == S_IFBLK ) 
	    strcpy(devname,block_spec[major_num].name);
	else
	    strcpy(devname,char_spec[major_num].name);
	devname[2] = (minor_num / ROOT_DEVICE_DIVISOR) + '0';
	devname[3] = (minor_num % ROOT_DEVICE_DIVISOR) + 'a';
	devname[4] = '\0';
	if (fmt == S_IFBLK) {
		error = spec_make_special(fnp, fmt, dev, &bdev_fs);
		if (error) {
			return error;
		}
		return bdev_make_special(*fnp, devname, D_READ);
	}
	else if (fmt == S_IFCHR) {
		error = spec_make_special(fnp, fmt, dev, &cdev_fs);
		if (error) {
			return error;
		}
		return bdev_make_special(*fnp, devname,D_READ|D_WRITE);
	}
	return enxio_special(fnp, fmt, dev);
}
