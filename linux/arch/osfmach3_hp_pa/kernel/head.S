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
	
	.proc
	.callinfo SAVE_SP,FRAME=128

	.export	stext,entry
	.export	_stext,entry
	.entry
_stext
stext
	.exit
	.procend

	.end


