; 
; Mach Operating System
; Copyright (c) 1992 Carnegie Mellon University
; All Rights Reserved.
; 
; Permission to use, copy, modify and distribute this software and its
; documentation is hereby granted, provided that both the copyright
; notice and this permission notice appear in all copies of the
; software, derivative works or modified versions, and any portions
; thereof, and that both notices appear in supporting documentation.
; 
; CARNEGIE MELLON ALLOWS FREE USE OF THIS SOFTWARE IN ITS "AS IS"
; CONDITION.  CARNEGIE MELLON DISCLAIMS ANY LIABILITY OF ANY KIND FOR
; ANY DAMAGES WHATSOEVER RESULTING FROM THE USE OF THIS SOFTWARE.
; 
; Carnegie Mellon requests users of this software to return to
; 
;  Software Distribution Coordinator  or  Software.Distribution@CS.CMU.EDU
;  School of Computer Science
;  Carnegie Mellon University
;  Pittsburgh PA 15213-3890
; 
; any improvements or extensions that they make and grant Carnegie Mellon 
; the rights to redistribute these changes.
;
;
;
; HISTORY
; $Log: table.ss,v $
# Revision 1.1.1.1  1995/05/04  06:56:39  sclawson
# New files.
#
; Revision 2.2  93/04/14  11:46:59  mrt
; 	Moved from luna88k
; 	[93/04/06  16:04:32  mrt]
; 
; Revision 2.2  92/02/20  17:08:40  danner
; 		Created.
; 	[92/02/20            danner]
;  
;
	text
	align   8
	
_table:	global	_table
	or.u	r9, r0, hi16(-6)
	or	r9, r9, lo16(-6)
	tb0	0, r0, 0x80
	br	cerror
	jmp	r1		
