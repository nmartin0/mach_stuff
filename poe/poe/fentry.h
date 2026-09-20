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
 * $Log:	fentry.h,v $
 * Revision 2.3  91/12/19  20:28:04  mrt
 * 	Updated to new copyright
 * 
 * Revision 2.2  90/09/08  00:18:55  rwd
 * 	First checkin
 * 	[90/08/31  13:52:03  rwd]
 * 
 */
/*
 *	File:	./fentry.h
 *	Author:	Joseph S. Barrera III, Randall W. Dean
 *
 *	Copyright (c) 1990 Joseph S. Barrera III, Randall W. Dean
 */

struct fentry {
	int		fe_refcount;
	struct fnode *	fe_fnode;
	unsigned long	fe_offset;
	boolean_t	fe_append;
	char		fe_rdwr;	/* subset of FREAD|FWRITE */
};

#define	FENTRY_NULL	((struct fentry *) 0)
