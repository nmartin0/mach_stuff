/*
 * Copyright (c) 1991 Carnegie Mellon University
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
 * Revision 2.2  91/12/05  16:42:56  grm
 * 	Created.
 * 	[91/05/28  15:20:56  grm]
 * 
 */
/*
 * at386/lock.s
 *
 * Mutex implementation for AT386.
 */

	.text
/*
 *	int
 *	mutex_try_lock(m)
 *		mutex_t m;	(= int *m for our purposes)
 */
	.align	2
	.globl	_mutex_try_lock
_mutex_try_lock:
	movl	4(%esp), %eax
   lock;bts	$0, 0(%eax)
	jb	1f
	movl	$1, %eax		/* yes */
	ret
1:
	xorl	%eax, %eax		/* no */
	ret

/*
 *	void
 *	mutex_unlock(m)
 *		mutex_t m;	(= int *m for our purposes)
 */
	.align	2
	.globl	_mutex_unlock
_mutex_unlock:
	movl	4(%esp), %eax
   lock;btr	$0, 0(%eax)
	ret
