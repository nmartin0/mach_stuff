# 
# Mach Operating System
# Copyright (c) 1991,1990 Carnegie Mellon University
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
# Revision 2.2  91/09/04  11:29:15  jsb
# 	First checkin. Contributed by Intel/SSD.
# 
# 	i860/cthread_inline.awk
# 	[91/09/02  08:55:28  jsb]
# 
#
# Awk script to inline critical C Threads primitives
#
#
#  NB: Must do a getline & print in each script because of the delayed
#      branch feature of the i860.  The parameter does not get loaded into
#      r16 until after the call in the asm source.
#
NF == 2 && $1 == "call" && $2 == "_spin_try_lock" {
	print	"#	BEGIN INLINE spin_try_lock"
	getline
	print
	print	"	nop"
	print	"	lock"
	print	"	ld.l	0(r16),r17"
	print	"	mov	1,r18"
	print	"	unlock"
	print	"	st.l	r18,0(r16)"
	print	"	xor	1,r17,r16"
	print	"#	END INLINE spin_try_lock"
	continue
}


NF == 2 && $1 == "call" && ($2 == "_spin_unlock") {
	print	"#	BEGIN INLINE spin_unlock"
	getline
	print
	print	"	st.l	r0,0(r16)"
	print	"#	END INLINE spin_unlock"
	continue
}


NF == 2 && $1 == "call" && $2 == "_cthread_sp" {
	print	"#	BEGIN INLINE cthread_sp"
	getline
	print
	print	"	mov	sp, r16"
	print	"#	END INLINE cthread_sp"
	continue
}


# default:
{
	print
}
