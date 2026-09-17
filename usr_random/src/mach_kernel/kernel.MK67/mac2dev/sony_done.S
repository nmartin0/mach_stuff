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
 * $Log:	sony_done.s,v $
 * Revision 2.2  91/09/12  16:48:00  bohman
 * 	Created.
 * 	[91/09/11  16:05:27  bohman]
 * 
 */

/*
 *	Apple Macintosh II Mach (macmach)
 *
 *	File: mac2dev/sony_done.s
 *	Author: David E. Bohman II (CMU macmach)
 */

#ifdef MODE24

#include <mac2/asm.h>

#include <mac2os/Globals.h>

/*
 * IOCompletion routine for
 * asynchronous floppy requests.
 */

ENTRY(sony_done)
	movb	G_MMU32bit,sp@-
	bne	0f
	moveq	#1,d0
	movl	G_JSwapMMU,a0
	jsr	a0@
0:
	jsr	_sony_complete

	movb	sp@+,d0
	bne	0f
	movl	G_JSwapMMU,a0
	jsr	a0@
0:
	rts

#endif /* MODE24 */
