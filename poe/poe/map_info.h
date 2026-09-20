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
 * $Log:	map_info.h,v $
 * Revision 2.3  91/12/19  20:28:34  mrt
 * 	Updated to new copyright
 * 
 * Revision 2.2  90/09/08  00:19:20  rwd
 * 	First checkin
 * 	[90/08/31  13:42:53  rwd]
 * 
 *
 */
/*
 *	File:	./map_info.h
 *	Author:	Joseph S. Barrera III, Randall W. Dean
 *
 *	Copyright (c) 1990 Joseph S. Barrera III, Randall W. Dean
 */


#ifndef	_MAP_INFO_
#define	_MAP_INFO_

#include <cthreads.h>

typedef	struct map_info {
	mach_port_t	mi_pager;
	struct mutex	mi_lock;
	vm_offset_t	mi_address;
	vm_size_t	mi_map_size;
	vm_offset_t	mi_offset;
	vm_size_t	mi_size;
	int		mi_read:1,
			mi_write:1;
} *map_info_t;
#endif	_MAP_INFO_
