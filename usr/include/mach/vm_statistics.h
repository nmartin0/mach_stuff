/*
 * @OSF_COPYRIGHT@
 */
/*
 * HISTORY
 * $Log: vm_statistics.h,v $
 * Revision 1.2.12.2  1995/01/06  19:52:25  devrcs
 * 	mk6 CR668 - 1.3b26 merge
 * 	added vm stats
 * 	[1994/10/14  03:43:30  dwm]
 *
 * Revision 1.2.12.1  1994/09/23  02:44:40  ezf
 * 	change marker to not FREE
 * 	[1994/09/22  21:43:49  ezf]
 * 
 * Revision 1.2.4.4  1993/08/03  18:30:01  gm
 * 	CR9596: Change KERNEL to MACH_KERNEL.
 * 	[1993/08/02  18:58:32  gm]
 * 
 * Revision 1.2.4.3  1993/06/15  20:28:27  brezak
 * 	Make xxx_vm_statistic for now.
 * 	[1993/06/14  14:11:07  brezak]
 * 
 * Revision 1.2.2.2  1993/06/08  19:03:02  brezak
 * 	Remove page_size from vm_statistics.
 * 
 * Revision 1.1.4.2  1993/06/02  23:49:41  jeffc
 * 	Added to OSF/1 R1.3 from NMK15.0.
 * 	[1993/06/02  21:18:54  jeffc]
 * 
 * Revision 1.2  1993/04/19  16:40:55  devrcs
 * 	ansi C conformance changes
 * 	[1993/02/02  18:55:38  david]
 * 
 * Revision 1.1  1992/09/30  02:32:30  robert
 * 	Initial revision
 * 
 * $EndLog$
 */
/* CMU_HIST */
/*
 * Revision 2.3  91/05/14  17:03:07  mrt
 * 	Correcting copyright
 * 
 * Revision 2.2  91/02/05  17:37:41  mrt
 * 	Changed to new Mach copyright
 * 	[91/02/01  17:22:49  mrt]
 * 
 * Revision 2.1  89/08/03  16:06:55  rwd
 * Created.
 * 
 * Revision 2.4  89/02/25  18:42:35  gm0w
 * 	Changes for cleanup.
 * 
 * Revision 2.3  89/02/07  00:54:39  mwyoung
 * Relocated from sys/vm_statistics.h
 * 
 * Revision 2.2  89/01/30  22:08:54  rpd
 * 	Made variable declarations use "extern".
 * 	[89/01/25  15:26:30  rpd]
 * 
 * 30-Sep-86  Avadis Tevanian (avie) at Carnegie-Mellon University
 *	Changed "reclaim" to "inactive."
 *
 * 22-Aug-86  Michael Young (mwyoung) at Carnegie-Mellon University
 *	Made vm_stat structure kernel-only.
 *
 * 22-May-86  Avadis Tevanian (avie) at Carnegie-Mellon University
 *	Defined vm_statistics_data_t as a real typedef so that
 *	MatchMaker can deal with it.
 *
 * 14-Feb-86  Avadis Tevanian (avie) at Carnegie-Mellon University
 *	Created.
 *
 */
/* CMU_ENDHIST */
/* 
 * Mach Operating System
 * Copyright (c) 1991,1990,1989,1988,1987 Carnegie Mellon University
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
 *  Software Distribution Coordinator  or  Software.Distribution@CS.CMU.EDU
 *  School of Computer Science
 *  Carnegie Mellon University
 *  Pittsburgh PA 15213-3890
 * 
 * any improvements or extensions that they make and grant Carnegie Mellon
 * the rights to redistribute these changes.
 */
/*
 */
/*
 *	File:	mach/vm_statistics.h
 *	Author:	Avadis Tevanian, Jr., Michael Wayne Young, David Golub
 *
 *	Virtual memory statistics structure.
 *
 */

#ifndef	VM_STATISTICS_H_
#define	VM_STATISTICS_H_

#include <mach/machine/vm_types.h>

struct vm_statistics {
	integer_t	free_count;		/* # of pages free */
	integer_t	active_count;		/* # of pages active */
	integer_t	inactive_count;		/* # of pages inactive */
	integer_t	wire_count;		/* # of pages wired down */
	integer_t	zero_fill_count;	/* # of zero fill pages */
	integer_t	reactivations;		/* # of pages reactivated */
	integer_t	pageins;		/* # of pageins */
	integer_t	pageouts;		/* # of pageouts */
	integer_t	faults;			/* # of faults */
	integer_t	cow_faults;		/* # of copy-on-writes */
	integer_t	lookups;		/* object cache lookups */
	integer_t	hits;			/* object cache hits */
};

typedef struct vm_statistics	*vm_statistics_t;
typedef struct vm_statistics	vm_statistics_data_t;

/*
 *	Each machine dependent implementation is expected to
 *	keep certain statistics.  They may do this anyway they
 *	so choose, but are expected to return the statistics
 *	in the following structure.
 */

struct pmap_statistics {
	integer_t	resident_count;	/* # of pages mapped (total)*/
	integer_t	wired_count;	/* # of pages wired */
};

typedef struct pmap_statistics	*pmap_statistics_t;
#endif	/* VM_STATISTICS_H_ */
