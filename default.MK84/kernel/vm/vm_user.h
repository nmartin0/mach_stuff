/* 
 * Mach Operating System
 * Copyright (c) 1993,1991,1990,1989,1988,1987 Carnegie Mellon University
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
 * $Log:	vm_user.h,v $
 * Revision 2.4  93/11/17  18:58:04  dbg
 * 	Added ANSI function prototypes.
 * 	[93/09/28            dbg]
 * 
 * Revision 2.3  91/05/14  17:51:44  mrt
 * 	Correcting copyright
 * 
 * Revision 2.2  91/02/05  18:00:43  mrt
 * 	Changed to new Mach copyright
 * 	[91/02/01  16:35:10  mrt]
 * 
 * Revision 2.1  89/08/03  16:46:17  rwd
 * Created.
 * 
 * Revision 2.5  89/04/18  21:31:31  mwyoung
 * 	Reset history.
 * 
 */
/*
 *	File:	vm/vm_user.h
 *	Author:	Avadis Tevanian, Jr., Michael Wayne Young
 *	Date:	1986
 *
 *	Declarations of user-visible virtual address space
 *	management functionality.
 */

#ifndef	_VM_VM_USER_H_
#define _VM_VM_USER_H_

#include <mach/kern_return.h>
#include <mach/mach_types.h>
#include <mach/std_types.h>		/* pointer_t */
#include <mach/vm_statistics.h>

#include <ipc/ipc_types.h>		/* ipc_port_t */
#include <vm/vm_map.h>

extern kern_return_t	vm_allocate(
		vm_map_t	map,
		vm_offset_t	*addr,	/* in/out */
		vm_size_t	size,
		boolean_t	anywhere);

extern kern_return_t	vm_deallocate(
		vm_map_t	map,
		vm_offset_t	start,
		vm_size_t	size);

extern kern_return_t	vm_inherit(
		vm_map_t	map,
		vm_offset_t	start,
		vm_size_t	size,
		vm_inherit_t	new_inheritance);

extern kern_return_t	vm_protect(
		vm_map_t	map,
		vm_offset_t	start,
		vm_size_t	size,
		boolean_t	set_maximum,
		vm_prot_t	new_protection);

extern kern_return_t	vm_statistics(
		vm_map_t	map,
		vm_statistics_data_t *stat);	/* out */

extern kern_return_t	vm_read(
		vm_map_t	map,
		vm_address_t	address,
		vm_size_t	size,
		pointer_t	*data,		/* out */
		vm_size_t	*data_size);	/* out */

extern kern_return_t	vm_write(
		vm_map_t	map,
		vm_address_t	address,
		pointer_t	data,
		vm_size_t	size);

extern kern_return_t	vm_copy(
		vm_map_t	map,
		vm_address_t	source_address,
		vm_size_t	size,
		vm_address_t	dest_address);

extern kern_return_t	vm_map(
		vm_map_t	target_map,
		vm_offset_t	*address,	/* in/out */
		vm_size_t	size,
		vm_offset_t	mask,
		boolean_t	anywhere,
		ipc_port_t	memory_object,
		vm_offset_t	offset,
		boolean_t	copy,
		vm_prot_t	cur_protection,
		vm_prot_t	max_protection,
		vm_inherit_t	inheritance);

#endif	/* _VM_VM_USER_H_ */
