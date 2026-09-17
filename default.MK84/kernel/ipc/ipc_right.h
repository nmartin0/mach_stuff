/* 
 * Mach Operating System
 * Copyright (c) 1993,1991,1990,1989 Carnegie Mellon University
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
 * $Log:	ipc_right.h,v $
 * Revision 2.5  93/11/17  17:01:29  dbg
 * 	Added ANSI function prototypes.
 * 	[93/09/23            dbg]
 * 
 * Revision 2.4  91/05/14  16:36:32  mrt
 * 	Correcting copyright
 * 
 * Revision 2.3  91/02/05  17:23:39  mrt
 * 	Changed to new Mach copyright
 * 	[91/02/01  15:51:08  mrt]
 * 
 * Revision 2.2  90/06/02  14:51:35  rpd
 * 	Created for new IPC.
 * 	[90/03/26  21:02:56  rpd]
 * 
 */
/*
 *	File:	ipc/ipc_right.h
 *	Author:	Rich Draves
 *	Date:	1989
 *
 *	Declarations of functions to manipulate IPC capabilities.
 */

#ifndef	_IPC_IPC_RIGHT_H_
#define	_IPC_IPC_RIGHT_H_

#include <mach_ipc_compat.h>

#include <mach/boolean.h>
#include <mach/kern_return.h>
#include <ipc/ipc_types.h>
#include <ipc/ipc_entry.h>
#include <ipc/ipc_port.h>

#define	ipc_right_lookup_read	ipc_right_lookup_write

extern kern_return_t
ipc_right_lookup_write(
	ipc_space_t	space,
	mach_port_t	name,
	ipc_entry_t	*entryp);

extern boolean_t
ipc_right_reverse(
	ipc_space_t	space,
	ipc_object_t	object,
	mach_port_t	*namep,
	ipc_entry_t	*entryp);

extern kern_return_t
ipc_right_dnrequest(
	ipc_space_t	space,
	mach_port_t	name,
	boolean_t	immediate,
	ipc_port_t	notify,
	ipc_port_t	*previousp);

extern ipc_port_t
ipc_right_dncancel(
	ipc_space_t	space,
	ipc_port_t	port,
	mach_port_t	name,
	ipc_entry_t	entry);

#define	ipc_right_dncancel_macro(space, port, name, entry)		\
		(((entry)->ie_request == 0) ? IP_NULL :			\
		 ipc_right_dncancel((space), (port), (name), (entry)))

extern boolean_t
ipc_right_inuse(
	ipc_space_t	space,
	mach_port_t	name,
	ipc_entry_t	entry);

extern boolean_t
ipc_right_check(
	ipc_space_t	space,
	ipc_port_t	port,
	mach_port_t	name,
	ipc_entry_t	entry);

extern void
ipc_right_clean(
	ipc_space_t	space,
	mach_port_t	name,
	ipc_entry_t	entry);

extern kern_return_t
ipc_right_destroy(
	ipc_space_t	space,
	mach_port_t	name,
	ipc_entry_t	entry);

extern kern_return_t
ipc_right_dealloc(
	ipc_space_t	space,
	mach_port_t	name,
	ipc_entry_t	entry);

extern kern_return_t
ipc_right_delta(
	ipc_space_t	space,
	mach_port_t	name,
	ipc_entry_t	entry,
	mach_port_right_t right,
	mach_port_delta_t delta);

extern kern_return_t
ipc_right_info(
	ipc_space_t	space,
	mach_port_t	name,
	ipc_entry_t	entry,
	mach_port_type_t *typep,
	mach_port_urefs_t *urefsp);

extern boolean_t
ipc_right_copyin_check(
	ipc_space_t	space,
	mach_port_t	name,
	ipc_entry_t	entry,
	mach_msg_type_name_t msgt_name);

extern kern_return_t
ipc_right_copyin(
	ipc_space_t	space,
	mach_port_t	name,
	ipc_entry_t	entry,
	mach_msg_type_name_t msgt_name,
	boolean_t	deadok,
	ipc_object_t	*objectp,
	ipc_port_t	*sorightp);

extern void
ipc_right_copyin_undo(
	ipc_space_t	space,
	mach_port_t	name,
	ipc_entry_t	entry,
	mach_msg_type_name_t msgt_name,
	ipc_object_t	object,
	ipc_port_t	soright);

extern kern_return_t
ipc_right_copyin_two(
	ipc_space_t	space,
	mach_port_t	name,
	ipc_entry_t	entry,
	ipc_object_t	*objectp,
	ipc_port_t	*sorightp);

extern kern_return_t
ipc_right_copyout(
	ipc_space_t	space,
	mach_port_t	name,
	ipc_entry_t	entry,
	mach_msg_type_name_t msgt_name,
	boolean_t	overflow,
	ipc_object_t	object);

extern kern_return_t
ipc_right_rename(
	ipc_space_t	space,
	mach_port_t	oname,
	ipc_entry_t	oentry,
	mach_port_t	nname,
	ipc_entry_t	nentry);

#if	MACH_IPC_COMPAT

extern kern_return_t
ipc_right_copyin_compat(ipc_space_t, mach_port_t, ipc_entry_t,
			mach_msg_type_name_t, boolean_t, ipc_object_t *);

extern kern_return_t
ipc_right_copyin_header(ipc_space_t, mach_port_t, ipc_entry_t,
			ipc_object_t *, mach_msg_type_name_t *);

#endif	/* MACH_IPC_COMPAT */
#endif	/* _IPC_IPC_RIGHT_H_ */
