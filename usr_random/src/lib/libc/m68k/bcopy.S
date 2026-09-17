/* 
 * Mach Operating System
 * Copyright (c) 1991,1990,1989 Carnegie Mellon University
 * All Rights Reserved.
 * 
 * Permission to use, copy, modify and distribute this software and its
 * documentation is hereby granted, provided that both the copyright
 * notice and this permission notice appear in all copies of the
 * software, derivative works or modified versions, and any portions
 * thereof, and that both notices appear in supporting documentation.
 * 
 * CARNEGIE MELLON ALLOWS FREE USE OF THIS SOFTWARE IN ITS "AS IS"
 * CONDITION.  CARNEGIE MELLON DISCLAIMS ANY LIABILITY OF ANY KIND FOR
 * ANY DAMAGES WHATSOEVER RESULTING FROM THE USE OF THIS SOFTWARE.
 * 
 * Carnegie Mellon requests users of this software to return to
 * 
 *  Software Distribution Coordinator  or  Software.Distribution@CS.CMU.EDU
 *  School of Computer Science
 *  Carnegie Mellon University
 *  Pittsburgh PA 15213-3890
 * 
 * any improvements or extensions that they make and grant Carnegie Mellon
 * the rights to redistribute these changes.
 */

#include <mac2/SYS.h>

/* bcopy(unsigned char *src, unsigned char *dst, unsigned long length) */

ENTRY(bcopy)
	/* collect arguments */
	movl	sp@(4),a0
	movl	sp@(8),a1
	movl	sp@(12),d1

	/* get count of long words into d0 */
	movl	d1,d0
	lsrl	#2,d0

	/* check for direction of copy */
	cmpl	a0,a1
	bgt	backw

	/* check for count == 0 */    
	beq	2f
	bra	1f

	/* loop for moving lower 16 bits of count */
0:	movl	a0@+,a1@+
1:	dbf	d0,0b

	/* check upper 16 bits of count for more to do */    
	swap	d0
	tstw	d0
	beq	2f
	subqw	#1,d0
	swap	d0
	bra	0b

	/* check for residue bytes */
2:	andl	#3,d1
	bra	1f

	/* move residue */
0:	movb	a0@+,a1@+
1:	dbf	d1,0b

	rts

backw:
	/* start copy at end */
	addl	d1,a0
	addl	d1,a1

	/* check for count == 0 */    
	beq	2f
	bra	1f

	/* loop for moving lower 16 bits of count */
0:	movl	a0@-,a1@-
1:	dbf	d0,0b

	/* check upper 16 bits of count for more to do */    
	swap	d0
	tstw	d0
	beq	2f
	subqw	#1,d0
	swap	d0
	bra	0b

	/* check for residue bytes */
2:	andl	#3,d1
	bra	1f

	/* move residue */
0:	movb	a0@-,a1@-
1:	dbf	d1,0b

	rts
