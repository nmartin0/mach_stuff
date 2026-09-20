# 
#
#  Mach Operating System
#  Copyright (c) 1991 Carnegie Mellon University
#  All Rights Reserved.
#  
#  Permission to use, copy, modify and distribute this software and its
#  documentation is hereby granted, provided that both the copyright
#  notice and this permission notice appear in all copies of the
#  software, derivative works or modified versions, and any portions
#  thereof, and that both notices appear in supporting documentation.
#  
#  CARNEGIE MELLON ALLOWS FREE USE OF THIS SOFTWARE IN ITS "AS IS"
#  CONDITION.  CARNEGIE MELLON DISCLAIMS ANY LIABILITY OF ANY KIND FOR
#  ANY DAMAGES WHATSOEVER RESULTING FROM THE USE OF THIS SOFTWARE.
#  
#  Carnegie Mellon requests users of this software to return to
#  
#   Software Distribution Coordinator  or  Software.Distribution@CS.CMU.EDU
#   School of Computer Science
#   Carnegie Mellon University
#   Pittsburgh PA 15213-3890
#  
#  any improvements or extensions that they make and grant Carnegie Mellon 
#  the rights to redistribute these changes.
# 

#

# HISTORY
# $Log:	cthread_inline.awk,v $
# Revision 2.2  91/08/24  12:33:53  af
# 	Revision 2.1  91/07/11  17:24:50  danner
# 	Created.
# 
# Revision 2.1.3.1  91/08/19  13:49:32  danner
# 	From rwd, with updated copyright.
# 
# 	              90/09/11            ohm@astem.or.jp (MORISHIMA Akitoshi)
# 	      Re-ported to luna88k, taking dalayed-branch-subroutine instruction
# 	      (bsr.n) in mind.
# 	[91/07/20  15:47:52  danner]
# 
# Revision 1.3  89/05/05  19:01:53  mrt
# 	Cleanup for Mach 2.5
# 

# luna/cthread_inline.awk
#
# Awk script to inline critical C Threads primitives on LUNA88K.

NF == 2 && $1 == "bsr" && $2 == "_spin_try_lock" {
	print	"#	BEGIN INLINE spin_try_lock"
	print	"	ld	r3,r2,0"
	print	"	bcnd	ne0,r3,1f"
	print	"	or	r3,r0,1"
	print	"	xmem	r3,r2,0"
	print	"	bcnd	ne0,r3,1f"
	print	"	br.n	2f"
	print	"	or	r2,r0,1		# yes"
	print	"1:"
	print	"	or	r2,r0,0		# no"
	print	"2:"
	print	"#	END INLINE spin_try_lock"
	continue
}
NF == 2 && $1 == "bsr" && $2 == "_spin_unlock" {
	print	"#	BEGIN INLINE " $2
	print	"	st	r0,r2,0"
	print	"#	END INLINE " $2
	continue
}
NF == 2 && $1 == "bsr" && $2 == "_cthread_sp" {
	print	"#	BEGIN INLINE cthread_sp"
	print	"	or	r2,r0,r31"
	print	"#	END INLINE cthread_sp"
	continue
}
NF == 2 && $1 == "bsr.n" && ($2 == "_spin_try_lock" || $2 == "_spin_unlock" || $2 == "_cthread_sp") {
	split($0, prev)
	continue
}
# default:
{
	print
	if (prev[1] == "bsr.n") {
		prev[1] = ""
		if (prev[2] == "_spin_try_lock") {
			print	"#	BEGIN INLINE(d) spin_try_lock"
			print	"	ld	r3,r2,0"
			print	"	bcnd	ne0,r3,1f"
			print	"	or	r3,r0,1"
			print	"	xmem	r3,r2,0"
			print	"	bcnd	ne0,r3,1f"
			print	"	br.n	2f"
			print	"	or	r2,r0,1		# yes"
			print	"1:"
			print	"	or	r2,r0,0		# no"
			print	"2:"
			print	"#	END INLINE(d) spin_try_lock"
		} else if (prev[2] == "_spin_unlock" ) {
			print	"#	BEGIN INLINE(d) " prev[2]
			print	"	st	r0,r2,0"
			print	"#	END INLINE(d) " prev[2]
		} else if (prev[2] == "_cthread_sp") {
			print	"#	BEGIN INLINE(d) cthread_sp"
			print	"	or	r2,r0,r31"
			print	"#	END INLINE(d) cthread_sp"
		} else {
			print "Sorry, this condition shouldn't happen"
		}
	}
}

