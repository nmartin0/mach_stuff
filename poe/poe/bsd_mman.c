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
 * $Log:	bsd_mman.c,v $
 * Revision 2.3  91/12/19  20:27:30  mrt
 * 	Updated to new copyright
 * 
 * Revision 2.2  90/09/08  00:14:32  rwd
 * 	First Checkin
 * 
 */
/*
 *	File:	./bsd_mman.c
 *	Author:	Joseph S. Barrera III, Randall W. Dean
 *
 *	Copyright (c) 1990 Joseph S. Barrera III, Randall W. Dean
 */

#include <mach.h>
#include <errno.h>
#include <ux_user.h>

Bsd_sbrk(ut, rval, size)
	struct ux_task *ut;
	int rval[2];
	int size;
{
	int error;

	size = round_page(size);
	if (size > 0) {
		error = vm_allocate(ut->ut_task, &ut->ut_break, size, FALSE);
		if (error) {
			/* XXX should do a vm_region to see what's up */
			mach_error("sbrk: vm_allocate", error);
			return ENOMEM;
		}
	}
	rval[0] = ut->ut_break;
	ut->ut_break += size;
	return 0;

}

Bsd_obreak(ut, rval, new_addr)
	struct ux_task *ut;
	int rval[2];
	vm_offset_t new_addr;
{
	int error;

	new_addr = round_page(new_addr);
	if (new_addr <= ut->ut_break) {
		return 0;
	}
	error = vm_allocate(ut->ut_task, &ut->ut_break,
			    new_addr - ut->ut_break, FALSE);
	if (error) {
		/* XXX should do a vm_region to see what's up */
		mach_error("obreak: vm_allocate", error);
		return ENOMEM;
	}
	ut->ut_break = new_addr;
	return 0;
}

Bsd_getpagesize(ut, rval)
	struct ux_task *ut;
	int rval[2];
{
#if	vax
	rval[0] = 1024;
#else
	rval[0] = vm_page_size;
#endif
	return 0;
}
