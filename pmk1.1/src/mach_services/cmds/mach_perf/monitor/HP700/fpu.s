/*
 * Copyright 1996 1995 by Open Software Foundation, Inc.   
 *              All Rights Reserved 
 *  
 * Permission to use, copy, modify, and distribute this software and 
 * its documentation for any purpose and without fee is hereby granted, 
 * provided that the above copyright notice appears in all copies and 
 * that both the copyright notice and this permission notice appear in 
 * supporting documentation. 
 *  
 * OSF DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE 
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS 
 * FOR A PARTICULAR PURPOSE. 
 *  
 * IN NO EVENT SHALL OSF BE LIABLE FOR ANY SPECIAL, INDIRECT, OR 
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM 
 * LOSS OF USE, DATA OR PROFITS, WHETHER IN ACTION OF CONTRACT, 
 * NEGLIGENCE, OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION 
 * WITH THE USE OR PERFORMANCE OF THIS SOFTWARE. 
 */
/*
 *  (c) Copyright 1989 HEWLETT-PACKARD COMPANY
 * 
 *  To anyone who acknowledges that this file is provided "AS IS"
 *  without any express or implied warranty:
 *      permission to use, copy, modify, and distribute this file
 *  for any purpose is hereby granted without fee, provided that
 *  the above copyright notice and this notice appears in all
 *  copies, and that the name of Hewlett-Packard Company not be
 *  used in advertising or publicity pertaining to distribution
 *  of the software without specific, written prior permission.
 *  Hewlett-Packard Company makes no representations about the
 *  suitability of this software for any purpose.
 */
/*
 * pmk1.1
 */

/*
 *  The bit patterns and corresponding rounding modes as specified by ANSI are:
 *      ARG  PA  NAME 
 *	00   01	 RZ	Round toward zero
 *	01   00	 RN	Round toward nearest representable value
 *	10   10	 RP	Round toward positive infinity
 *	11   11	 RM	Round toward minus infinity
 *
 */

#include <hp_pa/asm.h>

	.space	$TEXT$
	.subspa $CODE$

	.export write_rnd,entry
	.proc   
	.callinfo no_calls

write_rnd
	.enter
	b		_set_rnd
	ldi		0,%arg1
	.leave
	.procend

	.export read_rnd,entry
	.proc   
	.callinfo no_calls

read_rnd
	.enter
	b		_set_rnd
	ldi		1,%arg1
	.leave
	.procend


	.export _set_rnd,entry
	.proc   
	.callinfo no_calls

_set_rnd
	.enter
	bl		L10,%r1		/* place address of table in r1 */
	nop
	.leave
	.procend

convert_table
	ldi		1,%r29
	nop

	ldi		0,%r29
	nop

	nop
	nop

	nop
	nop

L10
	ldo		-40(%sp),%r19
	fstws		%fr0,(%r19)	/* get fp status word */
	ldw		(%r19),%r20
	extru		%r20,22,2,%r29	/* get current rounding mode */
	bv		%r29(%r1)	/* execute one instruction from convert_table */
	b		L20
L20
	extru,=		%arg1,31,1,%r0
	bv		(%rp)		/* return if we are just read_rnd */
	copy		%r29,%r28
	copy		%arg0,%r29	/* get new mode */
	bv		%arg0(%r1)	/* execute one instruction from convert_table */
	b		L30
L30
	dep		%r29,22,2,%r20	/* stuff new round mode in fp status word */
	depi		0,20,21,%r20	/* get rid of junk bits */
	depi		0,26,4,%r20	/* get rid of junk bits */
	stw		%r20,(%r19)
	bv		(%rp)
	fldws		(%r19),%fr0
ENDP
