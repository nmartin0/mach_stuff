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
 * $Log:	if_enproto.s,v $
 * Revision 2.2  91/09/12  16:49:42  bohman
 * 	Created.
 * 	[91/09/11  16:19:02  bohman]
 * 
 */

/*
 *	Apple Macintosh II Mach (macmach)
 *
 *	File: mac2if/if_enproto.s
 *	Author: David E. Bohman II (CMU macmach)
 */

#include <mac2/asm.h>

#include <assym.s>

#include <mac2os/Globals.h>

#define SHORT_BRANCH(loc) \
	.word (0x6000)+(loc)-(.+2)	| stupid assembler
#define PROTO(u) \
	moveq	\#u,d0; SHORT_BRANCH(enproto)
ENTRY(enproto)
	PROTO(0)
	PROTO(1)
	PROTO(2)
	PROTO(3)
	PROTO(4)
	PROTO(5)
	nop
enproto:
#ifdef MODE24
	movb	G_MMU32bit,sp@-
	bne	0f
	moveml	a0-a1/d0-d1,sp@-
	moveq	#1,d0
	movl	G_JSwapMMU,a0
	jsr	a0@
	moveml	sp@+,a0-a1/d0-d1
0:
#endif /* MODE24 */
	moveml	a0-a1,sp@-
	pea	a4@
	extl	d1
	movl	d1,sp@-
	pea	a3@(-14)
	movl	d0,sp@-
	jsr	_eninput
	addw	#24,sp

#ifdef MODE24
	movb	sp@+,d0
	bne	0f
	movl	G_JSwapMMU,a0
	jsr	a0@
0:
#endif /* MODE24 */
	rts

ENTRY(endone)
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
	jsr	_encomplete
	addqw	#4,sp

#ifdef MODE24
	movb	sp@+,d0
	bne	0f
	movl	G_JSwapMMU,a0
	jsr	a0@
0:
#endif /* MODE24 */
	rts
