/* 
 * Mach Operating System
 * Copyright (c) 1993,1991 Carnegie Mellon University
 * All Rights Reserved.
 * 
 * Permission to use, copy, modify and distribute this software and its
 * documentation is hereby granted, provided that both the copyright
 * notice and this permission notice appear in all copies of the
 * software, derivative works or modified versions, and any portions
 * thereof, and that both notices appear in supporting documentation.
 * 
 * CARNEGIE MELLON ALLOWS FREE USE OF THIS SOFTWARE IN ITS 
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
 * any improvements or extensions that they make and grant Carnegie the
 * rights to redistribute these changes.
 */
/*
 * HISTORY
 * $Log:	memory_object.h,v $
 * Revision 2.4  93/11/17  18:52:30  dbg
 * 	Added ANSI function prototypes.
 * 	[93/09/29            dbg]
 * 
 * Revision 2.3  91/06/25  10:33:21  rpd
 * 	Changed memory_object_t to ipc_port_t where appropriate.
 * 	[91/05/28            rpd]
 * 
 * Revision 2.2  91/05/18  14:39:45  rpd
 * 	Created.
 * 	[91/03/22            rpd]
 * 
 */

#ifndef	_VM_MEMORY_OBJECT_H_
#define	_VM_MEMORY_OBJECT_H_

#include <mach/boolean.h>
#include <ipc/ipc_types.h>	/* ipc_port_t */

extern ipc_port_t	memory_manager_default_reference(void);
extern boolean_t	memory_manager_default_port(ipc_port_t);
extern void		memory_manager_default_init(void);

extern ipc_port_t	memory_manager_default;

#endif	/* _VM_MEMORY_OBJECT_H_ */
