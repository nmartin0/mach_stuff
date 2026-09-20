# 
# Mach Operating System
# Copyright (c) 1991,1990,1989 Carnegie Mellon University
# All Rights Reserved.
# 
# Permission to use, copy, modify and distribute this software and its
# documentation is hereby granted, provided that both the copyright
# notice and this permission notice appear in all copies of the
# software, derivative works or modified versions, and any portions
# thereof, and that both notices appear in supporting documentation.
# 
# CARNEGIE MELLON ALLOWS FREE USE OF THIS SOFTWARE IN ITS "AS IS"
# CONDITION.  CARNEGIE MELLON DISCLAIMS ANY LIABILITY OF ANY KIND FOR
# ANY DAMAGES WHATSOEVER RESULTING FROM THE USE OF THIS SOFTWARE.
# 
# Carnegie Mellon requests users of this software to return to
# 
#  Software Distribution Coordinator  or  Software.Distribution@CS.CMU.EDU
#  School of Computer Science
#  Carnegie Mellon University
#  Pittsburgh PA 15213-3890
# 
# any improvements or extensions that they make and grant Carnegie Mellon
# the rights to redistribute these changes.
#
#
# HISTORY
# $Log:	cthread_inline.awk,v $
# Revision 2.4  91/05/14  17:59:16  mrt
# 	Correcting copyright
# 
# Revision 2.3  91/02/14  14:21:19  mrt
# 	Added new Mach copyright
# 	[91/02/13  12:39:22  mrt]
# 
# Revision 2.2  89/12/08  19:54:30  rwd
# 	Inlines are now spins instead of mutexes.
# 	[89/10/23            rwd]
# 
# Revision 2.1  89/08/04  15:15:14  rwd
# Created.
# 
# Revision 1.3  89/05/05  19:00:33  mrt
# 	Cleanup for Mach 2.5
# 

# sun/cthread_inline.awk
#
# Awk script to inline critical C Threads primitives on Sun.

NF == 2 && $1 == "jbsr" && $2 == "_spin_try_lock" {
	print	"|	BEGIN INLINE spin_try_lock"
	print	"	movl	sp@,a0"
	print	"	tas	a0@"
	print	"	beq	1f"
	print	"	clrl	d0		| no"
	print	"	jra	2f"
	print	"1:	moveq	#1,d0		| yes"
	print	"2:"
	print	"|	END INLINE spin_try_lock"
	continue
}
NF == 2 && $1 == "jbsr" && $2 == "_spin_unlock" {
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
