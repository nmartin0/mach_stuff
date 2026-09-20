/*
 * Copyright (c) Open Software Foundation, Inc.   
 * 
 */
/*
 * pmk1.1
 */


#include <linux/autoconf.h>

#include <mach/std_types.h>
#include <device/device.h>
#include <linux/types.h> /* for daddr_t */
#include <device/disk_status.h>

#include <osfmach3/device_utils.h>
#include <osfmach3/mach3_debug.h>
#include <osfmach3/uniproc.h>
#include <osfmach3/block_dev.h>

#include <linux/hdreg.h>
#include <linux/major.h>
#include <linux/errno.h>
#include <linux/fs.h>
#include <linux/ioctl.h>

#include <asm/segment.h>

#include "mach_ioctl.h"

#if	CONFIG_OSFMACH3_DEBUG
#define GENDISK_DEBUG	1
#endif	/* CONFIG_OSFMACH3_DEBUG */

#ifdef	GENDISK_DEBUG
extern int gendisk_debug;
#endif	/* GENDISK_DEBUG */

int
machine_disk_get_params(
	kdev_t			dev,
	mach_port_t		device_port,
	struct hd_geometry	*loc,
	boolean_t		partition_only)
{
	printk("machine_disk_get_parms: not implemented");
	return -EOPNOTSUPP;
}

kern_return_t
machine_disk_read_absolute(
	kdev_t		dev,
	mach_port_t	device_port,
	unsigned long	recnum,
	char		*data,
	unsigned long	size)
{
	printk("machine_disk_read_absolute: not implemented");
	return KERN_FAILURE;
}

kern_return_t
machine_disk_write_absolute(
	kdev_t		dev,
	mach_port_t	device_port,
	unsigned long	recnum,
	char		*data,
	unsigned long	size)
{
	printk("machine_disk_write_absolute: not implemented");
	return KERN_FAILURE;
}
