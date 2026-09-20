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
 * $Log:	mem_fops.c,v $
 * Revision 2.4  91/12/19  20:28:36  mrt
 * 	Updated to new copyright
 * 
 * Revision 2.3  91/06/18  13:06:44  jjc
 * 	Fixed mem_special() to get /dev/null's major device number right 
 * 	for non-MIPS machines.
 * 	[91/04/16            jjc]
 * 
 * Revision 2.2  90/09/08  00:19:23  rwd
 * 	First checkin
 * 	[90/08/31  13:44:35  rwd]
 * 
 * 	[90/08/31  13:35:41  rwd]
 * 
 *	First checkin
 *
 */
/*
 *	File:	./mem_fops.c
 *	Author:	Joseph S. Barrera III, Randall W. Dean
 *
 *	Copyright (c) 1990 Joseph S. Barrera III, Randall W. Dean
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

struct fops null_fops = {
	null_open,
	spec_close,
	null_read,
	null_write,
	spec_getstat,
	spec_setstat,
	null_select,
	nil_ioctl,
	nil_getpager,
	nil_lookup,
	nil_create,
	nil_link,
	nil_unlink,
};
	
struct fnfs null_fs = {
	0,		/* fs_mountpoint */
	0,		/* fs_root */
	&null_fops,	/* fs_fops */
	TRUE,		/* fs_mayseek */
	FALSE,		/* fs_maymap */
};

mem_special(fnp, fmt, dev)
	struct fnode **fnp;
	int fmt;
	dev_t dev;
{
	switch (dev) {
#if	mips
	case makedev(2, 2):	/*	 /dev/null	*/
#else	mips
	case makedev(3, 2):	/*	 /dev/null	*/
#endif	mips
		return spec_make_special(fnp, fmt, dev, &null_fs);

	default:
		return enxio_special(fnp, fmt, dev);
	}
}
