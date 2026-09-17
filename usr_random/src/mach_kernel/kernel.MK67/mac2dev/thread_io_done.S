/* 
 * MacMach Operating System
 * Copyright (c) 1992 Carnegie Mellon University
 * All Rights Reserved.
 * 
 * MacMach was developed by CMU with support from Apple Computer, Inc.
 * Use of this software is constrained by the MacMach End-User license.
 */

/*
 * HISTORY
 * $Log:	thread_io_done.s,v $
 * Revision 2.2  91/09/12  16:48:17  bohman
 * 	Created.
 * 	[91/09/11  16:07:27  bohman]
 * 
 */

/*
 *	Apple Macintosh II Mach (macmach)
 *
 *	File: mac2dev/thread_io_done.s
 *	Author: David E. Bohman II (CMU macmach)
 */

#include <mac2/asm.h>

#include <mac2os/Globals.h>

ENTRY(thread_io_done)
	movl	a0,d2
#ifdef MODE24
	movb	G_MMU32bit,sp@-
	bne	0f
	moveq	#1,d0
	movl	G_JSwapMMU,a0
	jsr	a0@
0:
#endif /* MODE24 */
	movl	d2,sp@-
	jsr	_thread_io_complete
	addqw	#4,sp

#ifdef MODE24
	movb	sp@+,d0
	bne	0f
	movl	G_JSwapMMU,a0
	jsr	a0@
0:
#endif /* MODE24 */
	rts
