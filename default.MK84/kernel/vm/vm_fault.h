/* 
 * Mach Operating System
 * Copyright (c) 1993-1987 Carnegie Mellon University
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
 * HISTORY
 * $Log:	vm_fault.h,v $
 * Revision 2.7  93/11/17  18:53:57  dbg
 * 	Added vm_fault_noreturn.
 * 	[93/10/21            dbg]
 * 
 * 	Use no_return type for continuations.
 * 	[93/05/04            dbg]
 * 
 * 	Added ANSI function prototypes.
 * 	[92/12/30            dbg]
 * 
 * Revision 2.6  91/05/18  14:40:15  rpd
 * 	Added VM_FAULT_FICTITIOUS_SHORTAGE.
 * 	[91/03/29            rpd]
 * 
 * Revision 2.5  91/05/14  17:48:59  mrt
 * 	Correcting copyright
 * 
 * Revision 2.4  91/03/16  15:05:03  rpd
 * 	Added vm_fault_init.
 * 	[91/02/16            rpd]
 * 
 * Revision 2.3  91/02/05  17:58:12  mrt
 * 	Changed to new Mach copyright
 * 	[91/02/01  16:32:04  mrt]
 * 
 * Revision 2.2  90/02/22  20:05:32  dbg
 * 	Add vm_fault_copy(), vm_fault_cleanup().  Remove
 * 	vm_fault_copy_entry().
 * 	[90/01/25            dbg]
 * 
 * Revision 2.1  89/08/03  16:44:57  rwd
 * Created.
 * 
 * Revision 2.6  89/04/18  21:25:22  mwyoung
 * 	Reset history.
 * 	[89/04/18            mwyoung]
 * 
 */
/*
 *	File:	vm/vm_fault.h
 *
 *	Page fault handling module declarations.
 */

#ifndef	_VM_VM_FAULT_H_
#define _VM_VM_FAULT_H_

#include <mach/kern_return.h>
#include <mach/vm_prot.h>
#include <mach/machine/vm_types.h>
#include <kern/kern_types.h>
#include <vm/vm_map.h>
#include <vm/vm_object.h>
#include <vm/vm_page.h>


/*
 *	Page fault handling based on vm_object only.
 */

typedef	kern_return_t	vm_fault_return_t;
#define VM_FAULT_SUCCESS		0
#define VM_FAULT_RETRY			1
#define VM_FAULT_INTERRUPTED		2
#define VM_FAULT_MEMORY_SHORTAGE 	3
#define VM_FAULT_FICTITIOUS_SHORTAGE 	4
#define VM_FAULT_MEMORY_ERROR		5

extern void vm_fault_init(void);

extern vm_fault_return_t vm_fault_page(
	vm_object_t	first_object,
	vm_offset_t	first_offset,
	vm_prot_t	fault_type,
	boolean_t	must_be_resident,
	boolean_t	interruptible,
	vm_prot_t	*protection,	/* in/out */
	vm_page_t	*result_page,	/* out */
	vm_page_t	*top_page,	/* out */
	boolean_t	resume,
	continuation_t	continuation);

extern void		vm_fault_cleanup(
	vm_object_t	object,
	vm_page_t	top_page);

/*
 *	Page fault handling based on vm_map (or entries therein)
 */

extern kern_return_t	vm_fault(
	vm_map_t	map,
	vm_offset_t	vaddr,
	vm_prot_t	fault_type,
	boolean_t	change_wiring,
	boolean_t	resume,
	no_return	(*continuation)(kern_return_t));

extern void		vm_fault_wire(
	vm_map_t	map,
	vm_map_entry_t	entry);

extern void		vm_fault_unwire(
	vm_map_t	map,
	vm_map_entry_t	entry);

extern kern_return_t	vm_fault_copy(
	vm_object_t	src_object,
	vm_offset_t	src_offset,
	vm_size_t	*src_size,		/* in/out */
	vm_object_t	dst_object,
	vm_offset_t	dst_offset,
	vm_map_t	dst_map,
	vm_map_version_t *dst_version,
	boolean_t	interruptible);		/* Copy pages from
						 * one object to another
						 */
/*
 *	Continuation routine in fault sequence.
 */
extern no_return vm_fault_continue(void);

/*
 *	Version of vm_fault to call when we want to
 *	make it clear that it will not return.
 */
#define	vm_fault_noreturn(m,v,ft,cw,r,c)				\
	((*(no_return (*)(vm_map_t, vm_offset_t, vm_prot_t,		\
			  boolean_t, boolean_t,				\
			  no_return (*)(kern_return_t))) vm_fault	\
	  )((m),(v),(ft),(cw),(r),(c)) )

#endif	/* _VM_VM_FAULT_H_ */
