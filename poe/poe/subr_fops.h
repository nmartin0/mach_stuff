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
 * $Log:	subr_fops.h,v $
 * Revision 2.4  94/03/27  17:53:26  mrt
 * 	Added some
 * 	 comments.
 * 	[94/03/27            mrt]
 * 
 * Revision 2.3  91/12/19  20:29:13  mrt
 * 	Updated to new copyright
 * 
 * Revision 2.2  90/09/08  00:20:23  rwd
 * 	First checkin
 * 	[90/08/31  13:56:05  rwd]
 * 
 */
/*
 *	File:	./subr_fops.h
 *	Author:	Joseph S. Barrera III, Randall W. Dean
 *
 *	Copyright (c) 1990 Joseph S. Barrera III, Randall W. Dean
 */
/*
 *	fnode for a special device, i.e. disk or tty
 */
struct snode {
	struct fnode	sn_fn;		/* common fnode part */
	struct fnode *	sn_entry;	/* ptr to the unode for a special device */
	char *		sn_private;	/* for block device ptr to port for mapped device */
					/* for tty ptr to ttystate structure */

};

extern int spec_make_special();
extern int enxio_special();

extern int dev_getstat();
extern int dev_setstat();

extern int spec_close();
extern int spec_getstat();
extern int spec_setstat();

extern int nil_ioctl();
extern int nil_getpager();
extern int nil_lookup();
extern int nil_create();
extern int nil_link();
extern int nil_unlink();

extern int null_open();
extern int null_read();
extern int null_write();
extern int null_select();

extern int enxio_op();
