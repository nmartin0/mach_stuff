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
 * Revision 2.4  94/03/25  18:23:07  mrt
 * 	Split into block and char device tables to be
 * 	consistent with i386
 * 	[94/03/12            mrt]
 * 
 * Revision 2.3  91/12/19  20:28:47  mrt
 * 	Updated to new copyright
 * 
 * Revision 2.2  90/09/08  00:19:42  rwd
 * 	Created from bsd_special.c.
 * 	[90/07/20            rwd]
 * 
 */
/*
 *	File:	./mips/conf.h
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

/* special block devices - match the Mach UNIX server */

struct special_switch_struct block_spec[]=
{
{	enxio_special,	""		/*  0 */ },
{	enxio_special,	""		/*  1 */ },
{	enxio_special,	""		/*  2 */ },
{	enxio_special,	""		/*  3 */ },
{	bdev_special,	"rd"		/*  4 */ },
{	enxio_special,	""		/*  5 */ },
{	enxio_special,	""		/*  6 */ },
{	enxio_special,	""		/*  7 */ },
{	bdev_special,	"rz"		/*  8 */ },
{	enxio_special,	""		/*  9 */ },
{	bdev_special,	"fd"		/* 10 */ }
};


struct special_switch_struct char_spec[]=
{
{	tty_special,	"console"	/*  0 */ },
{	tty_special,	""		/*  1 */ },
{	mem_special,	""		/*  2 */ },
{	log_special,	""		/*  3 */ },
{	mem_special,	""		/*  4 */ },
{	tty_special,	""		/*  5 */ },
{	pty_special,	""		/*  6 */ },
{	pty_special,	""		/*  7 */ },
{	bdev_special,	"rz"		/*  8 */ },
{	enxio_special,	""		/*  9 */ },
{	enxio_special,	""		/* 10 */ },
{	enxio_special,	""		/* 11 */ },
{	enxio_special,	""		/* 12 */ },
{	cmupty_special,	""		/* 13 */ },
{	cmupty_special,	""		/* 14 */ },
{	enxio_special,	""		/* 15 */ },
{	enxio_special,	""		/* 16 */ },
{	enxio_special,	""		/* 17 */ },
{	enxio_special,	""		/* 18 */ },
{	bdev_special,	"fd"		/* 19 */ }
};


/* number of special block devices */
int nblkdev = sizeof(block_spec)/sizeof(block_spec[0]);

/* number of special character devices */
int nchardev = sizeof(char_spec)/sizeof(char_spec[0]);
