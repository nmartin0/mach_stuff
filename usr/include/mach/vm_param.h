/*
 * @OSF_COPYRIGHT@
 */
/*
 * HISTORY
 * $Log: vm_param.h,v $
 * Revision 1.2.13.2  1995/01/06  19:52:21  devrcs
 * 	mk6 CR668 - 1.3b26 merge
 * 	64bit natural_t
 * 	[1994/10/14  03:43:27  dwm]
 *
 * Revision 1.2.13.1  1994/09/23  02:44:22  ezf
 * 	change marker to not FREE
 * 	[1994/09/22  21:43:42  ezf]
 * 
 * Revision 1.2.4.4  1993/08/20  20:46:54  collins
 * 	CR9651: Fixed PAGE_SIZE_FIXED implementation.
 * 	[1993/08/12  14:36:10  collins]
 * 
 * Revision 1.2.4.3  1993/08/03  18:29:59  gm
 * 	CR9596: Change KERNEL to MACH_KERNEL.
 * 	[1993/08/02  18:58:10  gm]
 * 
 * Revision 1.2.4.2  1993/06/09  02:44:18  gm
 * 	Added to OSF/1 R1.3 from NMK15.0.
 * 	[1993/06/02  21:18:48  jeffc]
 * 
 * Revision 1.2  1993/04/19  16:40:40  devrcs
 * 	Fixes for ANSI C
 * 	[1993/02/26  13:30:31  sp]
 * 
 * Revision 1.1  1992/09/30  02:32:26  robert
 * 	Initial revision
 * 
 * $EndLog$
 */
/* CMU_HIST */
/*
 * Revision 2.4  91/05/14  17:02:53  mrt
 * 	Correcting copyright
 * 
 * Revision 2.3  91/02/05  17:37:34  mrt
 * 	Changed to new Mach copyright
 * 	[91/02/01  17:22:32  mrt]
 * 
 * Revision 2.2  90/06/02  15:00:23  rpd
 * 	Removed user definitions; added page_aligned.
 * 	[90/03/26  22:42:42  rpd]
 * 
 * Revision 2.1  89/08/03  16:06:40  rwd
 * Created.
 * 
 * Revision 2.3  89/02/25  18:42:23  gm0w
 * 	Changes for cleanup.
 * 
 * 15-May-87  Avadis Tevanian (avie) at Carnegie-Mellon University
 *	Coalesced includes, documented use of PAGE_SIZE and PAGE_SHIFT.
 *
 * 15-Nov-86  Avadis Tevanian (avie) at Carnegie-Mellon University
 *	Removed include of <mach_init.h> before non-kernel definitions
 *	of trunc/round page macros.  When this is included, it makes it
 *	impossible for user programs to include system header files to
 *	retrieve kernel data structures.  (For example, the user level
 *	mach_init will cause a task to be a port, making it impossible
 *	to get the real kernel definition of a task).
 *
 *  7-Nov-86  Michael Young (mwyoung) at Carnegie-Mellon University
 *	Added non-KERNEL definitions of round_page, trunc_page.
 *
 * 21-May-86  David Golub (dbg) at Carnegie-Mellon University
 *	Added page_mask.  Changed types of page_size and page_mask
 *	to vm_size_t.
 *
 *  9-Jun-85  Avadis Tevanian (avie) at Carnegie-Mellon University
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
 *	File:	mach/vm_param.h
 *	Author:	Avadis Tevanian, Jr., Michael Wayne Young
 *	Date:	1985
 *
 *	Machine independent virtual memory parameters.
 *
 */

#ifndef	_MACH_VM_PARAM_H_
#define _MACH_VM_PARAM_H_

#ifndef	MACH_KERNEL

;YOU HAVE MADE A MISTAKE BY INCLUDING THIS FILE;

THIS FILE SHOULD NOT BE VISIBLE TO USER PROGRAMS.

USE <mach/machine/vm_param.h> TO GET MACHINE-DEPENDENT ADDRESS
SPACE AND PAGE SIZE ITEMS.

USE <mach/machine/vm_types.h> TO GET TYPE DECLARATIONS USED IN
THE MACH KERNEL INTERFACE.

IN ALL PROBABILITY, YOU SHOULD GET ALL OF THE TYPES USED IN THE
INTERFACE FROM <mach/mach_types.h>

#endif	/* MACH_KERNEL */

#include <mach/machine/vm_param.h>
#include <mach/machine/vm_types.h>

/*
 *	The machine independent pages are refered to as PAGES.  A page
 *	is some number of hardware pages, depending on the target machine.
 */

/*
 *	All references to the size of a page should be done with PAGE_SIZE
 *	or PAGE_SHIFT.  The fact they are variables is hidden here so that
 *	we can easily make them constant if we so desire.
 */

/*
 *	Regardless whether it is implemented with a constant or a variable,
 *	the PAGE_SIZE is assumed to be a power of two throughout the
 *	virtual memory system implementation.
 */

#ifndef	PAGE_SIZE_FIXED
extern vm_size_t	page_size;
extern vm_size_t	page_mask;
extern int		page_shift;

#define PAGE_SIZE	page_size	/* size of page in addressible units */
#define PAGE_SHIFT	page_shift	/* number of bits to shift for pages */
#define PAGE_MASK	page_mask	/* mask for offset within page */
#else	/* PAGE_SIZE_FIXED */
#define PAGE_SIZE	4096
#define PAGE_SHIFT	12
#define	PAGE_MASK	(PAGE_SIZE-1)
#endif	/* PAGE_SIZE_FIXED */

#ifndef	ASSEMBLER
/*
 *	Convert addresses to pages and vice versa.
 *	No rounding is used.
 */

#define atop(x)		(((natural_t)(x)) >> PAGE_SHIFT)
#define ptoa(x)		((vm_offset_t)((x) << PAGE_SHIFT))

/*
 *	Round off or truncate to the nearest page.  These will work
 *	for either addresses or counts.  (i.e. 1 byte rounds to 1 page
 *	bytes.
 */

#define round_page(x)	((vm_offset_t)((((vm_offset_t)(x)) + PAGE_MASK) & ~PAGE_MASK))
#define trunc_page(x)	((vm_offset_t)(((vm_offset_t)(x)) & ~PAGE_MASK))

/*
 *	Determine whether an address is page-aligned, or a count is
 *	an exact page multiple.
 */

#define	page_aligned(x)	((((vm_offset_t) (x)) & PAGE_MASK) == 0)

extern vm_size_t	mem_size;	/* size of physical memory (bytes) */

#endif	/* ASSEMBLER */
#endif	/* _MACH_VM_PARAM_H_ */
