/*
 * @OSF_COPYRIGHT@
 */
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
 * HISTORY
 * $Log: vm_sync.h,v $
 * Revision 1.2.7.1  1994/09/23  02:44:50  ezf
 * 	change marker to not FREE
 * 	[1994/09/22  21:43:54  ezf]
 *
 * Revision 1.2.3.3  1993/06/22  15:18:34  sp
 * 	Add definition of VM_SYNC_SYNCHRONOUS [david@gr.osf.org]
 * 	[1993/06/21  13:00:18  sp]
 * 
 * Revision 1.2.3.2  1993/06/09  02:44:33  gm
 * 	Added to OSF/1 R1.3 from NMK15.0.
 * 	[1993/06/02  21:18:58  jeffc]
 * 
 * Revision 1.2  1993/04/19  16:41:03  devrcs
 * 	Made compatible with other mach .h files.
 * 	[1993/03/15  17:34:44  david]
 * 
 * 	New for vm_sync definitions.
 * 	[1993/03/03  12:39:16  david]
 * 
 * $EndLog$
 */
/*
 *	File:	mach/vm_sync.h
 *
 *	Virtual memory synchronisation definitions.
 *
 */

#ifndef VM_SYNC_H_
#define VM_SYNC_H_

typedef unsigned		vm_sync_t;

/*
 *	Synchronization flags, defined as bits within the vm_sync_t type
 */

#define	VM_SYNC_ASYNCHRONOUS	((vm_sync_t) 0x01)
#define	VM_SYNC_SYNCHRONOUS	((vm_sync_t) 0x02)
#define VM_SYNC_INVALIDATE	((vm_sync_t) 0x04)

#endif  /* VM_SYNC_H_ */
