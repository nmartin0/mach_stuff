/*
 * Copyright (c) Open Software Foundation, Inc.   
 * 
 */
/*
 * pmk1.1
 */



#include <linux/autoconf.h>

#include <osfmach3/device_utils.h>
#include <osfmach3/mach3_debug.h>
#include <osfmach3/assert.h>
#include <osfmach3/server_thread.h>
#include <osfmach3/block_dev.h>

#include <linux/fd.h>
#include <linux/kernel.h>
#include <linux/errno.h>

#include "mach_ioctl.h"

#if	CONFIG_OSFMACH3_DEBUG
#define FLOPPY_DEBUG	1
#endif	/* CONFIG_OSFMACH3_DEBUG */

#ifdef	FLOPPY_DEBUG
extern int floppy_debug;
#endif	/* FLOPPY_DEBUG */

/*
 * Mach floppy device naming convention:
 * 	fd0c:	3.50" 1.44 MB
 *
 * This array should reflect what's in the 'floppy_type' array above.
 */
char floppy_mach_minor[2] = {
	'?',	/* 0 no testing		*/
	'c',	/* 1.44MB 3.5"          */
};
	
int machine_floppy_format(kdev_t device, struct format_descr *tmp_format_req)
{
	printk("machine_floppy_format: not implemented");
	return -EOPNOTSUPP;
}
