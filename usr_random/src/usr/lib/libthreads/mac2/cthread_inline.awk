# MacMach Operating System
# Copyright (c) 1992 Carnegie Mellon University
# All Rights Reserved.
#
# MacMach was developed by CMU with support from Apple Computer, Inc.
# Use of this software is constrained by the MacMach End-User license.

# Awk script to inline critical C Threads primitives on Mac2.

NF == 2 && $1 == "jbsr" && $2 == "_mutex_try_lock" {
	print	"|	BEGIN INLINE mutex_try_lock"
	print	"	movl	sp@,a0"
	print	"	tas	a0@"
	print	"	beq	1f"
	print	"	clrl	d0		| no"
	print	"	jra	2f"
	print	"1:	moveq	#1,d0		| yes"
	print	"2:"
	print	"|	END INLINE mutex_try_lock"
	continue
}
NF == 2 && $1 == "jbsr" && ($2 == "_spin_unlock" || $2 == "_mutex_unlock") {
	print	"|	BEGIN INLINE " $2
	print	"	movl	sp@,a0"
	print	"	clrl	a0@"
	print	"|	END INLINE " $2
	continue
}
NF == 2 && $1 == "jbsr" && $2 == "_cthread_sp" {
	print	"|	BEGIN INLINE cthread_sp"
	print	"	movl	sp,d0"
	print	"|	END INLINE cthread_sp"
	continue
}
# default:
{
	print
}
