/*
 * Copyright (c) Open Software Foundation, Inc.   
 * 
 */
/*
 * pmk1.1
 */

#include <machine/asm.h>

	.space  $TEXT$
	.subspa $CODE$	

	.export	__delay,entry
	.proc
	.callinfo
__delay
	.entry
        combf,=,n	r0,arg0,__delay
        ldo             -1(arg0),arg0
        bv,n		0(rp)
	.exit
	.procend

	.export	abs,entry
	.proc
	.callinfo
abs
	.entry
	add,>	arg0,r0,ret0
	subi	0,arg0,ret0	
	bv,n	0(rp)
	.exit
	.procend

	.export	flush_cache,entry
	.proc
	.callinfo
flush_cache
	.entry
	comb,<<= arg2,arg1,$flush_end
	mtsp    arg0,sr1
	fdc     (sr1,arg1)
	fic     (sr1,arg1)
	b	flush_cache
	ldo	1(arg1),arg1
$flush_end
	bv	0(rp)
	sync
	.exit
	.procend
	.end




