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
 * $Log:	bsd_special.c,v $
 * Revision 2.5  94/03/27  17:51:03  mrt
 * 		Changed to differentiate between block and char special
 * 		devices. Added comments.
 * 	[94/03/27            mrt]
 * 
 * Revision 2.4  91/12/19  20:27:50  mrt
 * 	Updated to new copyright
 * 
 * Revision 2.3  90/09/27  13:54:07  rwd
 * 	Declare enxio_special.
 * 	[90/09/08            rwd]
 * 
 * Revision 2.2  90/09/08  00:15:53  rwd
 * 	Spilt out partially to machine dependant.
 * 	[90/07/20            rwd]
 * 
 */
/*
 *	File:	./bsd_special.c
 *	Author:	Joseph S. Barrera III, Randall W. Dean
 *
 *	Copyright (c) 1990 Joseph S. Barrera III, Randall W. Dean
 */

/* dev_special is called by usf_fops:ufs_lookup to create an
 *	snode when it finds an inode for a special device
 *   dev_special uses major device number to dispatch through
 *   special_switch to the routine provided for this type of device,
 *   The possible routines are:
 *	bdev_special(structs fnode **fnp,int fmt, dev_t dev): 
 *		in bdev_fops finds the device name, calls spec_make_speecial
 *		to make an snode with a bdev_fs fnfs structure init,
 *		calls bdev_make_special to open a device and save port in snode
 *	tty_special(struct fnode **fnp, int fmt, dev_t dev, char *name;)
 *		in tty_fops: calls spec_make_special to make an snode
 *		allocs and inits ttstate struct and puts pointer to it in snode
 *		if name is non-zero, store it in ttstate
 *	mem_special( struct fnode **fnp, int fmt, dev_t dev)
 *		in mem_fops if the dev is /dev/null calls spec_make_special
 *		with a null fnfs struct, otherwise call enxio_special
 *	pty_special: in bsd_special calls enxio_special
 *	enxio_special: in subr_fops which calls spec_make_special 
 *		to alloc and init a snode with an
 *		enxio_fs fnfs structure
*/
#include <mach.h>
#include <fnode.h>
#include <errno.h>
#include <ux_user.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <bsd_ioctl.h>
#include <machine/conf.h>


log_special(a,b,c)	{ return enxio_special(a,b,c); } /* not currently used */
pty_special(a,b,c)	{ return enxio_special(a,b,c); }
cmupty_special(a,b,c)	{ return enxio_special(a,b,c); }/* not currently used */

dev_special(fnp, fmt, dev)
	struct fnode **fnp;
	int fmt;
	dev_t dev;
{
	int maj = major(dev);

	if ( fmt == S_IFBLK ) {
	    if (maj < 0 || maj >= nblkdev) {
		return enxio_special(fnp, fmt, dev);
	    } else {
		register struct special_switch_struct *sw;
		sw = &block_spec[maj];
		return (*(sw->routine))(fnp, fmt, dev, sw->name);
	    }
	}
	else
	if (maj < 0 || maj >= nchardev) {
		return enxio_special(fnp, fmt, dev);
	} else {
		register struct special_switch_struct *sw;
		sw = &char_spec[maj];
		return (*(sw->routine))(fnp, fmt, dev, sw->name);
	}
}
