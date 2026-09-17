/* 
 * MacMach Operating System
 * Copyright (c) 1992 Carnegie Mellon University
 * All Rights Reserved.
 * 
 * MacMach was developed by CMU with support from Apple Computer, Inc.
 * Use of this software is constrained by the MacMach End-User license.
 */

| Mutex implementation for mac2.

|	int
|	spin_try_lock(m)
|		int m;	(= int *m for our purposes)

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
|		int m;	(= int *m for our purposes)

	.globl	_spin_unlock
_spin_unlock:
	movl	sp@(4),a0
	clrl	a0@
	rts
