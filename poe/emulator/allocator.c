/* 
 * Mach Operating System
 * Copyright (c) 1991,1990,1989 Carnegie Mellon University
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
 * $Log:	allocator.c,v $
 * Revision 2.5  94/03/25  18:21:10  mrt
 * 	Corrected include of mach/mach.h to mach.h.
 * 	[93/11/15            mrt]
 * 
 * Revision 2.4  92/02/02  13:00:11  rpd
 * 	Renamed vm_allocate and vm_map with my_ prefix to avoid
 * 	conflicting with prototypes for the real functions.
 * 	Removed special mips definition of VM_PROT_DEFAULT.
 * 	[92/01/30            rpd]
 * 
 * Revision 2.3  91/12/19  20:19:57  mrt
 * 	Updated to new copyright
 * 
 * Revision 2.2  90/11/16  11:39:18  rwd
 * 	Taken from UX21
 * 	[90/11/14  15:13:09  rwd]
 * 
 * Revision 2.4  90/06/02  15:20:11  rpd
 * 	Removed vm_allocate_with_pager.
 * 	Added reply_port argument to vm_map.
 * 	[90/04/08            rpd]
 * 
 * Revision 2.3  90/05/29  20:22:22  rwd
 * 	if anywhere in vm_allocate set addr to EMULATOR_BASE
 * 	[90/05/29            rwd]
 * 
 * Revision 2.2  89/11/29  15:26:01  af
 * 	RCS-ed.
 * 	[89/11/29            af]
 * 
 */
/*
 * Replacement for vm_allocate, to keep memory within emulator address space.
 */

#include <mach.h>
#include <machine/vmparam.h>

extern mach_port_t mig_get_reply_port();

kern_return_t
my_vm_allocate(task, addr, size, anywhere)
	task_t		task;
	vm_offset_t	*addr;
	vm_size_t	size;
	boolean_t	anywhere;
{
	if (anywhere)
	    *addr = EMULATOR_BASE;

	return (htg_vm_map(task, mig_get_reply_port(),
			addr, size, (vm_offset_t)0, anywhere,
			MEMORY_OBJECT_NULL, (vm_offset_t)0, FALSE,
			VM_PROT_DEFAULT, VM_PROT_ALL, VM_INHERIT_COPY));
}

kern_return_t
my_vm_map(task, reply_port, address, size, mask, anywhere,
	memory_object, offset, copy,
	cur_protection, max_protection, inheritance)

	vm_task_t	task;
	mach_port_t	reply_port;
	vm_address_t	*address;
	vm_size_t	size;
	vm_address_t	mask;
	boolean_t	anywhere;
	memory_object_t	memory_object;
	vm_offset_t	offset;
	boolean_t	copy;
	vm_prot_t	cur_protection;
	vm_prot_t	max_protection;
	vm_inherit_t	inheritance;
{
	if (anywhere && *address < EMULATOR_BASE)
	    *address = EMULATOR_BASE;

	return (htg_vm_map(task, reply_port, address, size, mask, anywhere,
			memory_object, offset, copy,
			cur_protection, max_protection, inheritance));
}
