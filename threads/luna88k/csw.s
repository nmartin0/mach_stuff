;
;
;  Mach Operating System
;  Copyright (c) 1991 Carnegie Mellon University
;  All Rights Reserved.
;  
;  Permission to use, copy, modify and distribute this software and its
;  documentation is hereby granted, provided that both the copyright
;  notice and this permission notice appear in all copies of the
;  software, derivative works or modified versions, and any portions
;  thereof, and that both notices appear in supporting documentation.
;  
;  CARNEGIE MELLON ALLOWS FREE USE OF THIS SOFTWARE IN ITS "AS IS"
;  CONDITION.  CARNEGIE MELLON DISCLAIMS ANY LIABILITY OF ANY KIND FOR
;  ANY DAMAGES WHATSOEVER RESULTING FROM THE USE OF THIS SOFTWARE.
;  
;  Carnegie Mellon requests users of this software to return to
;  
;   Software Distribution Coordinator  or  Software.Distribution@CS.CMU.EDU
;   School of Computer Science
;   Carnegie Mellon University
;   Pittsburgh PA 15213-3890
;  
;  any improvements or extensions that they make and grant Carnegie Mellon 
;  the rights to redistribute these changes.
; 
;
; HISTORY
; 14-Dec-92  Randall Dean (rwd) at Carnegie-Mellon University
;	Modify for continuation based version.
;
; $Log:	csw.s,v $
; Revision 2.3.1.1  92/06/22  11:50:12  rwd
; 	Changed for one lock version of cproc
; 	[92/05/27            rwd]
; 
; Revision 2.3  92/04/01  10:56:39  rpd
; 	[92/03/21  12:04:28  danner]
; 
;	Added cthread_sp 
; 
; Revision 2.2  91/08/24  12:33:20  af
;	Comment leader fix
;	[91/07/19  20:03:34  danner]
;	Created by rwd
;	[91/07/19  18:58:17  danner]
;
 
;
; luna/csw.s
;
; Context switch and cproc startup for LUNA88K COROUTINE implementation.
;

	text
	align	4

;
; cthread_filter(con, type, a1, a2, a3, a4)
;

	global	_cthread_filter
_cthread_filter:

	bcnd	ne0,r3,1f

; IN
	subu	r31,r31,0x48	; 4 bytes * 16, for r30, r1, r14-25,
				; fcr62, and fcr63 and CONT_PC and CONT_ARG
	st	r30,r31,0x00	; old frame pointer
	st	r1, r31,0x04	; return address
	st	r14,r31,0x08
	st	r15,r31,0x0c
	st	r16,r31,0x10
	st	r17,r31,0x14
	st	r18,r31,0x18
	st	r19,r31,0x1c
	st	r20,r31,0x20
	st	r21,r31,0x24
	st	r22,r31,0x28
	st	r23,r31,0x2c
	st	r24,r31,0x30
	st	r25,r31,0x34
	fldcr	r11,fcr62
	st	r11,r31,0x38
	fldcr	r11,fcr63
	st	r11,r31,0x3c
	st	r31,r2,0	; save current fp

	or	r2,r5,0		; set arg #1
	jmp.n	r4
	or	r3,r6,0		; set arg #2

1:	subu	r3,r3,1
	bcnd	ne0,r3,2f
; COMPRESS

	ld	r11,r2,0	; get context
	st	r4,r11,0x40	; save routine
	st	r5,r11,0x44	; and arg

	ld	r31,r7,0	; new context

	st	r0,r6,0		; clear lock

	ld	r2,r31,0x44	; get arg
	ld	r12,r31,0x40	; routine
	jmp	r12		; invoke

2:	subu	r3,r3,1
	bcnd	ne0,r3,3f
; DECOMPRESS


3:	subu	r3,r3,1
	bcnd	ne0,r3,4f
; OUT
	ld	r31 ,r2,0	; restore next fp
	ld	r30,r31,0x00	; frame pointer
	ld	r1, r31,0x04	; return address
	ld	r14,r31,0x08
	ld	r15,r31,0x0c
	ld	r16,r31,0x10
	ld	r17,r31,0x14
	ld	r18,r31,0x18
	ld	r19,r31,0x1c
	ld	r20,r31,0x20
	ld	r21,r31,0x24
	ld	r22,r31,0x28
	ld	r23,r31,0x2c
	ld	r24,r31,0x30
	ld	r25,r31,0x34
	ld	r11,r31,0x38
	fstcr	r11,fcr62
	ld	r11,r31,0x3c
	fstcr	r11,fcr63
	jmp.n	r1		; return to next thread
	addu	r31,r31,0x48	; restore sp

4:
; PREPARE

	ld	r11,r2,0	; get context
	st	r4,r11,0x40	; save routine
	st	r5,r11,0x44	; and arg
	jmp	r1		; and return

;
; get current stack pointer 
;
	global	_cthread_sp
_cthread_sp:	
	jmp.n 	r1
	or	r2, r0, r31


