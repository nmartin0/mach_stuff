/* 
 * Mach Operating System
 * Copyright (c) 1991 Carnegie Mellon University
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
 * $Log: xmm_flush.c,v $
 * Revision 1.1.1.1  1995/05/04  06:56:42  sclawson
 * New files.
 *
 * Revision 2.3  92/01/22  22:54:17  rpd
 * 	Fixed includes to use "" when appropriate.
 * 	[92/01/18            rpd]
 * 
 * Revision 2.2  91/07/06  15:05:24  jsb
 * 	First checkin.
 * 
 */
/*
 *	File:	xmm_flush.c
 *	Author:	Joseph S. Barrera III
 *	Date:	1991
 *
 *	Xmm class which forces a flush when data is cleaned.
 */

#include <mach.h>
#include "xmm_obj.h"

kern_return_t k_flush_lock_request();

xmm_decl(flush_class,
	/* m_init		*/	m_interpose_init,
	/* m_terminate		*/	m_interpose_terminate,
	/* m_copy		*/	m_interpose_copy,
	/* m_data_request	*/	m_interpose_data_request,
	/* m_data_unlock	*/	m_interpose_data_unlock,
	/* m_data_write		*/	m_interpose_data_write,
	/* m_lock_completed	*/	m_interpose_lock_completed,
	/* m_supply_completed	*/	m_interpose_supply_completed,
	/* m_data_return	*/	m_interpose_data_return,

	/* k_data_provided	*/	k_interpose_data_provided,
	/* k_data_unavailable	*/	k_interpose_data_unavailable,
	/* k_get_attributes	*/	k_interpose_get_attributes,
	/* k_lock_request	*/	k_flush_lock_request,
	/* k_data_error		*/	k_interpose_data_error,
	/* k_set_attributes	*/	k_interpose_set_attributes,
	/* k_destroy		*/	k_interpose_destroy,
	/* k_data_supply	*/	k_interpose_data_supply,

	/* name			*/	"flush",
	/* size			*/	sizeof(struct xmm_obj)
);

kern_return_t
xmm_flush_create(old_mobj, new_mobj)
	xmm_obj_t old_mobj;
	xmm_obj_t *new_mobj;
{
	return xmm_obj_allocate(&flush_class, old_mobj, new_mobj);
}

k_flush_lock_request(kobj, offset, length, should_clean, should_flush,
		     lock_value, robj)
	xmm_obj_t kobj;
	vm_offset_t offset;
	vm_size_t length;
	boolean_t should_clean;
	boolean_t should_flush;
	vm_prot_t lock_value;
	xmm_obj_t robj;
{
	if (should_clean) {
		should_flush = TRUE;
	}
	K_LOCK_REQUEST(kobj, offset, length, should_clean, should_flush,
		       lock_value, robj);
}
