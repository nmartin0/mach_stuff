| 
| Mach Operating System
| Copyright (c) 1991,1990,1989 Carnegie Mellon University
| All Rights Reserved.
| 
| Permission to use, copy, modify and distribute this software and its
| documentation is hereby granted, provided that both the copyright
| notice and this permission notice appear in all copies of the
| software, derivative works or modified versions, and any portions
| thereof, and that both notices appear in supporting documentation.
| 
| CARNEGIE MELLON ALLOWS FREE USE OF THIS SOFTWARE IN ITS "AS IS"
| CONDITION.  CARNEGIE MELLON DISCLAIMS ANY LIABILITY OF ANY KIND FOR
| ANY DAMAGES WHATSOEVER RESULTING FROM THE USE OF THIS SOFTWARE.
| 
| Carnegie Mellon requests users of this software to return to
| 
|  Software Distribution Coordinator  or  Software.Distribution@CS.CMU.EDU
|  School of Computer Science
|  Carnegie Mellon University
|  Pittsburgh PA 15213-3890
| 
| any improvements or extensions that they make and grant Carnegie Mellon
| the rights to redistribute these changes.
|
|
| HISTORY
| $Log:	lock.s,v $
|Revision 2.4  91/05/14  17:59:33  mrt
|	Correcting copyright
|
|Revision 2.3  91/02/14  14:21:30  mrt
|	Added new Mach copyright
|	[91/02/13  12:39:34  mrt]
|
|Revision 2.2  89/12/08  19:54:39  rwd
|		The only simple locks are now spins.
|	[89/10/23            rwd]
|
|Revision 2.1  89/08/03  17:11:10  rwd
|Created.
|
| Revision 1.3  89/05/05  19:00:44  mrt
| 	Cleanup for Mach 2.5
| 

| sun/lock.s
|
| Mutex implementation for Sun.

|	int
|	spin_try_lock(m)
|		int m;		(= int *m for our purposes)

	.globl	_spin_try_lock
_spin_try_lock:
	movl	sp@(4),a0
	tas	a0@
	bne	1f
	moveq	#1,d0		| yes
	rts
1:
	clrl	d0		| no
	rts

|	void
|	spin_unlock(m)
|		int m;		(= int *m for our purposes)

	.globl	_spin_unlock
_spin_unlock:
	movl	sp@(4),a0
	clrl	a0@
	rts
