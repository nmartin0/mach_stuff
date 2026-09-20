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
 * $Log:	conf.h,v $
 * Revision 2.4  94/04/12  10:41:11  mrt
 * 	Split into block and char device tables since device
 * 	3 is sd for block and mem for char. added zd for Sequent
 * 	[94/04/12            mrt]
 * 
 * Revision 2.3  91/12/19  20:28:10  mrt
 * 	Updated to new copyright
 * 
 * Revision 2.2  90/09/27  13:54:23  rwd
 * 	First Checkin
 * 	[90/09/10  19:05:40  rwd]
 * 
 */
/*
 *	File:	./i386/conf.h
 *	Author:	Joseph S. Barrera III, Randall W. Dean
 *
 *	Copyright (c) 1990 Joseph S. Barrera III, Randall W. Dean
 */


#include <machine/spec.h>

extern int tty_special();
extern int mem_special();
extern int log_special();
extern int pty_special();
extern int bdev_special();
extern int cmupty_special();
extern int enxio_special();


/* 
 *  _NET_BSD_ case is compatible with NetBSD 0.9 
 *  the other case is compatible with CMU's Mach i386 
 */

#ifndef _NET_BSD_

/* special block devices - match the Mach UNIX server */

struct special_switch_struct block_spec[]=
{
{	bdev_special,	"hd"		/*  0 */ },
{	bdev_special,	"fd"		/*  1 */ },
{	enxio_special,	"wt"		/*  2 */ },
{	bdev_special,	"sd"		/*  3 */ }, /* also mem for char */
{	bdev_special,	"zd"		/*  4 */ }
};


/* special character devices - match the Mach UNIX server */

struct special_switch_struct char_spec[]=
{
{	bdev_special,	"com"		/*  0 */ },
{	tty_special,	"console"	/*  1 */ },
{	tty_special,	""		/*  2 */ },
{	mem_special,	"mem"		/*  3 */ }, 
{	bdev_special,	"hd"		/*  4 */ },
{	bdev_special,	"fd"		/*  5 */ },
{	enxio_special,	""		/*  6 */ },
{	enxio_special,	""		/*  7 */ },
{	enxio_special,	""		/*  8 */ },
{	pty_special,	""		/*  9 */ },
{	pty_special,	""		/* 10 */ },
{	enxio_special,	""		/* 11 */ },
{	enxio_special,	""		/* 12 */ },
{	enxio_special,	""		/* 13 */ },
{	enxio_special,	""		/* 14 */ },
{	enxio_special,	""		/* 15 */ },
{	enxio_special,	""		/* 16 */ },
{	enxio_special,	""		/* 17 */ },
{	enxio_special,	""		/* 18 */ },
{	enxio_special,	""		/* 19 */ },
{	enxio_special,	""		/* 20 */ },
{	enxio_special,	""		/* 21 */ },
{	enxio_special,	""		/* 22 */ },
{	bdev_special,	"sd"		/* 23 */ },
{	bdev_special,	"zd"		/* 24 */ }
};

#else /* _NET_BSD_ */

/* special block devices - match NetBSD */

struct special_switch_struct block_spec[]=
{
{	bdev_special,	"hd"		/*  0 */ },
{	enxio_special,	""		/*  1 */ },
{	bdev_special,	"fd"		/*  2 */ },
{	enxio_special,	"wt"		/*  3 */ },
{	bdev_special,	"sd"		/*  4 */ },
{	enxio_special,	"st"		/*  5 */ },
{	enxio_special,	"cd"		/*  6 */ }
};


/* special character devices - match the NetBSD */

struct special_switch_struct char_spec[]=
{
{	tty_special,	"console"	/*  0 */ },
{	enxio_special,	""		/*  1 */ },
{	mem_special,	"mem"		/*  2 */ },
{	bdev_special,	"hd"		/*  3 */ }, /* also mem for char */
{	enxio_special,	""		/*  4 */ },
{	enxio_special,	""		/*  5 */ },
{	enxio_special,	""		/*  6 */ },
{	enxio_special,	""		/*  7 */ },
{	tty_special,	"com"		/*  8 */ },
{	bdev_special,	"fd"		/*  9 */ },
{	enxio_special,	""		/* 10 */ },
{	enxio_special,	""		/* 11 */ },
{	enxio_special,	""		/* 12 */ },
{	bdev_special,	"sd"		/* 13 */ },
{	enxio_special,	"st"		/* 14 */ },
{	enxio_special,	"cd"		/* 15 */ },
{	enxio_special,	""		/* 16 */ },
{	enxio_special,	""		/* 17 */ },
{	enxio_special,	""		/* 18 */ },
{	enxio_special,	""		/* 19 */ },
{	enxio_special,	""		/* 20 */ },
{	enxio_special,	""		/* 21 */ },
{	enxio_special,	"iopl"		/* 22 */ },
{	enxio_special,	"kbd"		/* 23 */ },
{	enxio_special,	"mouse"		/* 24 */ }
};

#endif /* _NET_BSD_ */

/* number of special block devices */
int nblkdev = sizeof(block_spec)/sizeof(block_spec[0]);

/* number of special character devices */
int nchardev = sizeof(char_spec)/sizeof(char_spec[0]);
