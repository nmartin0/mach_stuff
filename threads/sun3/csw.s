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
| $Log:	csw.s,v $
| Revision 2.4.1.1  92/06/22  11:50:36  rwd
| 		Update for single lock cproc version.
| 	[92/05/27            rwd]
| 
| Revision 2.4  91/05/14  17:59:05  mrt
| 	Correcting copyright
| 
| Revision 2.3  91/02/14  14:21:14  mrt
| 	Added new Mach copyright
| 	[91/02/13  12:39:17  mrt]
| 
| Revision 2.2  89/12/08  19:54:21  rwd
| 	Added cproc_prepare
| 	[89/11/21            rwd]
| 	Changed cproc_start to cproc_start_wait.  Added unlock param
| 	[89/10/23            rwd]
| 
| Revision 2.1  89/08/03  17:10:49  rwd
| Created.
| 
| Revision 1.2  89/05/05  19:00:07  mrt
| 	Cleanup for Mach 2.5
| 

| sun/csw.s
|
| Context switch and cproc startup for Sun COROUTINE implementation.

	.text

| Suspend the current thread and resume the next one.
|
|	void
|	cproc_switch(cur, next)
|		int *cur;
|		int *next;

	.globl	_cproc_switch
_cproc_switch:
	movl	sp,a1		| keep sp
	movl	a1@(4),a0	| a0 = cur
	cmpl	a0,#0		| dont bother saving when 0
	beq	2f
	moveml	#Lpush,sp@-	| save current registers
	movl	sp,a0@		| save current sp
2:	movl	a1@(8),a3	| a3 = next
	movl	a3@,sp		| restore next sp
	moveml	sp@+,#Lpop	| restore next registers
	rts			| return to next thread

|	void
|	cproc_start_wait(parent_context, child, stackp, lock)
|		int *parent_context;
|		cproc_t child;
|		int stackp;

	.globl	_cproc_start_wait
_cproc_start_wait:
	movl	sp,a1		| keep sp
	moveml	#Lpush,sp@-	| save parent registers
	movl	a1@(4),a0	| a0 = parent
	movl	sp,a0@		| save parent sp
	movl	a1@(12),a3	| a3 = stackp
	movl	a1@(8),a3@-	| push child onto stackp
	movl	a1@(16),a2	| get lock pointer
	movl	a3,sp		| child sp = stackp
	clrl	a2@		| clear lock
	jbsr	_cproc_waiting


|	void
|	cproc_prepare(child, child_context, stack, lock)
|		int *child_context;
|		int *stack;
|		int *lock;

	.globl	_cproc_prepare
_cproc_prepare:
	movl	sp@(12),a0	| get stack
	movl	sp@(4),a0@-	| child is arg to cthread_body
	clrl	a0@-		| pretend return address from cthread_body
	movl	sp@(16),a0@-	| save lock address
	movl	@load_args,a0@-	| push return address
	moveml	#Lpush,a0@-	| push random regs
	movl	sp@(8),a1	| get context
	movl	a0,a1@		| save context
	rts


load_args:
	movl	sp@+,a0		| get lock
	clrl	a0@		| clear
	jmp	_cthread_body

	| Register save masks for moveml instruction.
	| The pop mask is for post-increment mode.
	| The push mask is for pre-decrement mode.
	Lpop	= 0x7cfc	| A6 .. A2, D7 .. D2
	Lpush	= 0x3f3e	| D2 .. D7, A2 .. A6
