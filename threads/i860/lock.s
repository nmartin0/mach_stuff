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
 * $Log:	lock.s,v $
 * Revision 2.2  91/09/04  11:29:19  jsb
 * 	First checkin. Contributed by Intel/SSD.
 * 	[91/09/02  08:56:39  jsb]
 * 
 */
/*
 * Mutex implementation for i860.  Note that lock variables must be
 * stored in non-cacheable memory to lock the bus.
 */

	.text
/*
 *	int
 *	spin_try_lock(m)
 *		mutex_t m;	(= int *m for our purposes)
 */
	.align	4
_spin_try_lock::
	nop
	lock
	ld.l	0(r16),r17
	mov	1,r18
	unlock
	st.l	r18,0(r16)	// must be AFTER unlock inst (wlb)
	bri	r1
	xor	1,r17,r16


/*
 *	void
 *	spin_unlock(m)
 *		mutex_t m;	(= int *m for our purposes)
 */
_spin_unlock::
	bri	r1
	st.l	r0,0(r16)

