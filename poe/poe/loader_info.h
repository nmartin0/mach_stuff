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
 * $Log:	loader_info.h,v $
 * Revision 2.3  91/12/19  20:28:28  mrt
 * 	Updated to new copyright
 * 
 * Revision 2.2  90/09/08  00:19:08  rwd
 * 	First checkin
 * 	[90/08/31  13:51:53  rwd]
 * 
 */
/*
 *	File:	./loader_info.h
 *	Author:	Joseph S. Barrera III, Randall W. Dean
 *
 *	Copyright (c) 1990 Joseph S. Barrera III, Randall W. Dean
 */

/*
 *	Data structures for bootstrap program loader.
 */

struct loader_info {
	int		format;		/* load format */
	vm_offset_t	text_start;	/* text start in memory */
	vm_size_t	text_size;
	vm_offset_t	text_offset;	/* text offset in file */
	vm_offset_t	data_start;	/* data+bss start in memory */
	vm_size_t	data_size;
	vm_offset_t	data_offset;	/* data offset in file */
	vm_size_t	bss_size;
	vm_offset_t	entry_1;	/* 2 words for entry address */
	vm_offset_t	entry_2;
} ;
/* values for format */
#define	EX_UNKNOWN	0		/* ? */
#define	EX_READIN	1		/* 0407 */
#define	EX_SHAREABLE	2		/* 0410 */
#define	EX_PAGEABLE	3		/* 0413 */

