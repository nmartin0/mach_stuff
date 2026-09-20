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
 * $Log:	ufs_alloc.c,v $
 * Revision 2.3  91/12/19  20:29:45  mrt
 * 	Updated to new copyright
 * 
 * Revision 2.2  90/09/08  00:21:19  rwd
 * 	First checkin
 * 	[90/08/31  13:58:57  rwd]
 * 
 */
/*
 *	File:	./ufs_alloc.c
 *	Author:	Joseph S. Barrera III, Randall W. Dean
 *
 *	Copyright (c) 1990 Joseph S. Barrera III, Randall W. Dean
 */

#include <mach.h>
#include <fnode.h>
#include <errno.h>
#include <ux_user.h>
#include <ufs_fops.h>
#include <device/device_types.h>
#include <sys/types.h>
#include <sys/stat.h>

/*
 * A unode has a uhash structure attached to it when it has dirty blocks.
 * The un->i_ic info will generally be invalid, in particular the size.
 * Mode and other things
 *
 * Hash table size should be dynamic.
 * When overwriting file, perhaps should take hint from old size.
 *
 * [ How does COW fit into all this? ]
 */
#define	UH_TABLESIZE	16
struct uhash {
	int		uh_size; /* current size of file */
	struct ubuf *	uh_table[UH_TABLESIZE];
};

/*
 *  ud_data is only valid as far as file is big.
 *  
 */
struct ubuf {
	char *		ub_data;	/* not nesc all valid */
	struct ubuf *	ub_next;	/* hash link table */
};

/*
 *  Returns ESPIPE if beyond end-of-file;
 *  Returns ENOENT if we should use old block (from inode).
 */
uh_read_file(un, offset, buf_p, size_p)
	register struct unode	*un;
	vm_offset_t		offset;
	vm_offset_t		*buf_p;		/* out */
	vm_size_t		*size_p;	/* out */
{
	
}
