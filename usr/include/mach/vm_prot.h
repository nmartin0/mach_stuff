/*
 * @OSF_COPYRIGHT@
 */
/*
 * HISTORY
 * $Log: vm_prot.h,v $
 * Revision 1.3.8.1  1994/09/23  02:44:31  ezf
 * 	change marker to not FREE
 * 	[1994/09/22  21:43:46  ezf]
 *
 * Revision 1.3.2.2  1993/06/09  02:44:23  gm
 * 	Added to OSF/1 R1.3 from NMK15.0.
 * 	[1993/06/02  21:18:51  jeffc]
 * 
 * Revision 1.3  1993/04/19  16:40:48  devrcs
 * 	make endif tags ansi compliant/include files
 * 	[1993/02/20  21:45:25  david]
 * 
 * Revision 1.2  1992/12/07  21:29:28  robert
 * 	integrate any changes below for 14.0 (branch from 13.16 base)
 * 
 * 	Joseph Barrera (jsb) at Carnegie-Mellon University 05-Aug-92
 * 	Added VM_PROT_WANTS_COPY to solve copy-call race condition.
 * 	[1992/12/06  20:25:47  robert]
 * 
 * Revision 1.1  1992/09/30  02:32:28  robert
 * 	Initial revision
 * 
 * $EndLog$
 */
/* CMU_HIST */
/*
 * Revision 2.4.3.1  92/03/03  16:22:41  jeffreyh
 * 	[David L. Black 92/02/22  17:03:43  dlb@osf.org]
 * 	   Add no change protection value for memory_object_lock_request.
 * 
 * Revision 2.4  91/05/14  17:03:00  mrt
 * 	Correcting copyright
 * 
 * Revision 2.3  91/02/05  17:37:38  mrt
 * 	Changed to new Mach copyright
 * 	[91/02/01  17:22:39  mrt]
 * 
 * Revision 2.2  90/01/22  23:05:57  af
 * 	Removed execute permission from default protection.
 * 	On the only machine that cares for execute permission (mips)
 * 	this is an expensive liability: it requires keeping
 * 	Icache consistent memory that never contains code.
 * 	[89/12/15            af]
 * 
 * Revision 2.1  89/08/03  16:06:47  rwd
 * Created.
 * 
 * Revision 2.3  89/02/25  18:42:29  gm0w
 * 	Changes for cleanup.
 * 
 *  6-Jun-85  Avadis Tevanian (avie) at Carnegie-Mellon University
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
 *	File:	mach/vm_prot.h
 *	Author:	Avadis Tevanian, Jr., Michael Wayne Young
 *
 *	Virtual memory protection definitions.
 *
 */

#ifndef	VM_PROT_H_
#define	VM_PROT_H_

/*
 *	Types defined:
 *
 *	vm_prot_t		VM protection values.
 */

typedef int		vm_prot_t;

/*
 *	Protection values, defined as bits within the vm_prot_t type
 */

#define	VM_PROT_NONE	((vm_prot_t) 0x00)

#define VM_PROT_READ	((vm_prot_t) 0x01)	/* read permission */
#define VM_PROT_WRITE	((vm_prot_t) 0x02)	/* write permission */
#define VM_PROT_EXECUTE	((vm_prot_t) 0x04)	/* execute permission */

/*
 *	The default protection for newly-created virtual memory
 */

#define VM_PROT_DEFAULT	(VM_PROT_READ|VM_PROT_WRITE)

/*
 *	The maximum privileges possible, for parameter checking.
 */

#define VM_PROT_ALL	(VM_PROT_READ|VM_PROT_WRITE|VM_PROT_EXECUTE)

/*
 *	An invalid protection value.
 *	Used only by memory_object_lock_request to indicate no change
 *	to page locks.  Using -1 here is a bad idea because it
 *	looks like VM_PROT_ALL and then some.
 */
#define VM_PROT_NO_CHANGE	((vm_prot_t) 0x08)

/*
 *	Another invalid protection value.
 *	Used only by memory_object_data_request upon an object
 *	which has specified a copy_call copy strategy. It is used
 *	when the kernel wants a page belonging to a copy of the
 *	object, and is only asking the object as a result of
 *	following a shadow chain. This solves the race between pages
 *	being pushed up by the memory manager and the kernel
 *	walking down the shadow chain.
 */
#define VM_PROT_WANTS_COPY	((vm_prot_t) 0x10)

#endif	/* VM_PROT_H_ */
