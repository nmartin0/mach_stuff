/*
 * Copyright (c) 1991 Carnegie-Mellon University
 *
 * This file is part of 'macpatches',
 * which is the emulation library used
 * for running macOS under MACH 3.0.
 *
 * Written by David Bohman in 1991
 */

	.data
	.space (8192)
	.text
	.globl	_start
_start:
	lea	_end,a0
	lea	_edata,a1
	subl	a1,a0
	movl	a0,sp@-
	pea	_edata
	jsr	_bzero
	addqw	#8,sp	

	movl	_mach_init_routine,a0
	jsr	a0@

	jsr	_setup
	jsr	_th_init
	movl	d0,sp

	jsr	_main

/*
 * Launch the startup.
 */
	clrl	0x110		| StkLowPt
	movl	0x10c,sp	| BufPtr
	clrl	0x964		| ScrapHandle
	.word	0xa02c		| InitApplZone
	subqw	#4,sp
	movl	#0x434f4445,sp@-
	movw	#0xbfaa,sp@-
	.word	0xa9a0		| GetResource
	movl	sp@+,d1
	beq	syserr
	movl	d1,a0
	movl	a0@,a0
	subl	a0@+,sp
	movl	sp,a5
	subl	a0@+,sp
	movl	a5,0x904	| CurrentA5
	movl	sp,0x908	| CurStackBase
	movl	sp,0x110	| StkLowPt
	movl	a0@+,d0
	movl	a0@+,a1
	movw	a1,0x934	| CurJTOffset
	addl	a5,a1
	pea	a1@(2)
	.word	0xa02e		| BlockMove
	movl	d1,sp@-
	.word	0xa9a3		| ReleaseResource
	movl	sp,a0
	subw	#0x2000,a0
	.word	0xa02d		| SetApplLimit
	rts

syserr:
	moveq	#0x63,d0
	.word	0xa9c9		| SysError
0:
	trap	#15
	bra	0b
