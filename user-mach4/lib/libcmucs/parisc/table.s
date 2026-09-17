/* 
 * Copyright (c) 1994, The University of Utah and
 * the Computer Systems Laboratory at the University of Utah (CSL).
 * All rights reserved.
 *
 * Permission to use, copy, modify and distribute this software and its
 * documentation is hereby granted, provided that both the copyright
 * notice and this permission notice appear in all copies of the
 * software, derivative works or modified versions, and any portions
 * thereof, and that both notices appear in supporting documentation.
 *
 * THE UNIVERSITY OF UTAH AND CSL ALLOW FREE USE OF THIS SOFTWARE IN ITS "AS
 * IS" CONDITION.  THE UNIVERSITY OF UTAH AND CSL DISCLAIM ANY LIABILITY OF
 * ANY KIND FOR ANY DAMAGES WHATSOEVER RESULTING FROM THE USE OF THIS SOFTWARE.
 *
 * CSL requests users of this software to return to csl-dist@cs.utah.edu any
 * improvements that they make and grant CSL redistribution rights.
 *
 */

	.space	$TEXT$
	.subspa	$CODE$
	.export table,ENTRY
table:
	.proc
	.callinfo no_calls
	.entry
	ldil	L%0xc0000004,%r1
	ble	R%0xc0000004(%sr7,%r1)
	ldi	-6,%r22
	comb,=,n %r0,%r22,$noerror
	.import	errno,data
	addil	L%errno-$global$,%r27
	stw	%r28,R%errno-$global$(%r1)
	ldi	-1,%r28
$noerror
	bv,n	0(%r2)
	.exit
	.procend
