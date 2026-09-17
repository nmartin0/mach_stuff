/* 
 * MacMach Operating System
 * Copyright (c) 1992 Carnegie Mellon University
 * All Rights Reserved.
 * 
 * MacMach was developed by CMU with support from Apple Computer, Inc.
 * Use of this software is constrained by the MacMach End-User license.
 */

	.even
	.globl	_soundHdr
_soundHdr:
soundHdr:
	.word	0x4000
	.word	0x0000
	.word	0x0000
	.word	0x0000
	.word	soundCall-soundHdr
	.word	soundCall-soundHdr
	.word	soundCall-soundHdr
	.word	soundCall-soundHdr
	.word	soundCall-soundHdr
	.byte	0x6, 0x2e, 0x53, 0x6f, 0x75, 0x6e, 0x64	| .Sound

soundCall:
	moveml	d0-d1/a0-a1,sp@-
	trap	#15
	moveml	sp@+,d0-d1/a0-a1
	rts

	.even
	.globl	_scsiHdr
_scsiHdr:
scsiHdr:
	.word	0x4f00
	.word	0x0000
	.word	0x0000
	.word	0x0000
	.word	scsiCall-scsiHdr
	.word	scsiCall-scsiHdr
	.word	scsiCall-scsiHdr
	.word	scsiCall-scsiHdr
	.word	scsiCall-scsiHdr
	.byte	0x5, 0x2e, 0x53, 0x43, 0x53, 0x49	| .SCSI

scsiCall:
	moveml	d0-d1/a0-a1,sp@-
	jsr	_scsiCall
	moveml	sp@+,d0-d1/a0-a1
	rts

	.even
	.globl	_sonyHdr
_sonyHdr:
sonyHdr:
	.word	0x4f00
	.word	0x0000
	.word	0x0000
	.word	0x0000
	.word	sonyCall-sonyHdr
	.word	sonyCall-sonyHdr
	.word	sonyCall-sonyHdr
	.word	sonyCall-sonyHdr
	.word	sonyCall-sonyHdr
	.byte	0x5, 0x2e, 0x53, 0x4f, 0x4e, 0x59	| .SONY

sonyCall:
	moveml	d0-d1/a0-a1,sp@-
	jsr	_sonyCall
	moveml	sp@+,d0-d1/a0-a1
	rts

	.even
	.globl	_etherHdr
_etherHdr:
etherHdr:
	.word	0x4400
	.word	0x0000
	.word	0x0000
	.word	0x0000
	.word	etherCall-etherHdr
	.word	etherCall-etherHdr
	.word	etherCall-etherHdr
	.word	etherCall-etherHdr
	.word	etherCall-etherHdr
	.byte	0x5, 0x2e, 0x45, 0x4e, 0x45, 0x54	| .ENET

etherCall:
	moveml	d0-d1/a0-a1,sp@-
	jsr	_etherCall
	moveml	sp@+,d0-d1/a0-a1
	rts

	.globl	etherRPRR
etherRPRR:
	bra	etherRP
	bra	etherRR

etherRP:
	subw	d3,d1
	bccs	1f
	addw	d1,d3
	bra	1f
0:
	movb	a0@+,a3@+
1:
	dbf	d3,0b
	tstw	d1
	bmis	2f
	moveq	#0,d3
	rts
2:
	movw	d1,d3
	moveq	#0,d1
	rts
	
etherRR:
	subw	d3,d1
	bccs	1f
	addw	d1,d3
	bra	1f
0:
	movb	a0@+,a3@+
1:
	dbf	d3,0b
	movw	d1,d3
	negw	d3
	moveq	#0,d1
	rts
