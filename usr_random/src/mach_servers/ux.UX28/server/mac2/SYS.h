/* 
 * MacMach Operating System
 * Copyright (c) 1992 Carnegie Mellon University
 * All Rights Reserved.
 * 
 * MacMach was developed by CMU with support from Apple Computer, Inc.
 * Use of this software is constrained by the MacMach End-User license.
 */

#include <sys/syscall.h>

#ifdef __STDC__

#ifdef PROF
		    .globl  mcount
#define _MCOUNT	    link a6,\#0; lea mcntr,a0; \
		    .data; .even; mcntr: .long 0; .text; jsr mcount; unlk a6

#define _MCOUNT2    link a6,\#0; lea mcntr2,a0; \
		    .data; .even; mcntr2: .long 0; .text; jsr mcount; unlk a6
#else /* PROF */

#define _MCOUNT
#define _MCOUNT2

#endif /* PROF */

#define	ENTRY(x)    .text; .even; .globl _##x; _##x: _MCOUNT
#define	ENTRY2(x)    .text; .even; .globl _##x; _##x: _MCOUNT2

#else /* __STDC__ */

#ifdef PROF
		    .globl  mcount
#define _MCOUNT	    link a6,#0; lea mcntr,a0; \
		    .data; .even; mcntr: .long 0; .text; jsr mcount; unlk a6
#define _MCOUNT2    link a6,#0; lea mcntr2,a0; \
		    .data; .even; mcntr2: .long 0; .text; jsr mcount; unlk a6
#else /* PROF */

#define _MCOUNT
#define _MCOUNT2

#endif /* PROF */

#define	ENTRY(x)    .text; .even; .globl _/**/x; _/**/x: _MCOUNT
#define	ENTRY2(x)    .text; .even; .globl _/**/x; _/**/x: _MCOUNT2

#endif

#ifdef __STDC__

#define	SYSCALL(x)  .globl cerror; err: jmp cerror; ENTRY(x); pea SYS_##x; trap \#0; jcs err
#define	PSEUDO(x,y) ENTRY(x); pea SYS_##y; trap \#0

#else

#define	SYSCALL(x)  .globl cerror; err: jmp cerror; ENTRY(x); pea SYS_/**/x; trap #0; jcs err
#define	PSEUDO(x,y) ENTRY(x); pea SYS_/**/y; trap #0

#endif

