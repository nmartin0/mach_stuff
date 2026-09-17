/*
 * Copyright (c) 1991 Carnegie-Mellon University
 *
 * This file is part of 'macpatches',
 * which is the emulation library used
 * for running macOS under MACH 3.0.
 *
 * Written by David Bohman in 1991
 */

	.globl	_VBL_tick
0:	rts
_VBL_tick:
	addql	#1,0x16a
	bset	#6,0x160
	bne	0b
	moveml	a2-a3/d2-d3,sp@-
	lea	0x160,a1
	lea	a1@(2),a0
0:	movl	a0@,d0
	beq	1f
	movl	d0,a0
	subqw	#1,a0@(0xa)
	bne	0b
	movl	a0,sp@-
	movl	a1,sp@-
	movl	a0@(6),a1
	jsr	a1@
	movl	sp@+,a1
	movl	sp@+,a0
	tstw	a0@(0xa)
	bne	0b
	moveml	a0-a1,sp@-
	jsr	_dequeue
	moveml	sp@+,a0-a1
	bra	0b
1:
	moveml	sp@+,d2-d3/a2-a3
	bclr	#6,0x160
	rts

	.globl	_slow_tick
_slow_tick:
	addql	#1,0x20c
	bclr	#5,0x21f
	tstb	0x208
	bpl	0f
	movl	0x200,d0
	beq	0f
	cmpl	0x20c,d0
	bhi	0f
	bclr	#1,0x21f
0:	rts

	.globl	_update_cursor
_update_cursor:
	moveml	a2-a3/d2-d3,sp@-
	movl	0x8ee,a0
	jsr	a0@
	moveml	sp@+,a2-a3/d2-d3
	rts

	.globl	_flush_icache
_flush_icache:
	moveml	d0-d1/a0-a1,sp@-
	jsr	_FlushICache
	moveml	sp@+,d0-d1/a0-a1
	rts

	.globl	_LogicalPageSize
_LogicalPageSize:
	movl	sp@+,a1
	movl	sp@+,a0
	addqw	#4,sp
	clrw	sp@
	movl	_vm_page_size,a0@
	jmp	a1@

	.globl	_LogicalRAMSize
_LogicalRAMSize:
	movl	sp@+,a1
	movl	sp@+,a0
	addqw	#4,sp
	clrw	sp@
	movl	#0x02000000,a0@
	jmp	a1@

	.globl	_VMAttr
_VMAttr:
	movl	sp@+,a1
	movl	sp@+,a0
	addqw	#4,sp
	clrw	sp@
	movl	#0x00000001,a0@
	jmp	a1@

	.globl	_NewPtrSysClear
_NewPtrSysClear:
	movl	sp@(4),d0
	.word	0xa71e
	movw	d0,0x220
	movl	a0,d0
	rts

	.globl	_DisposPtr
_DisposPtr:
	movl	sp@(4),a0
	.word	0xa01f
	movw	d0,0x220
	extl	d0
	rts

	.globl	_PostEvent
_PostEvent:
	movl	sp@(8),d0
	movl	sp@(4),a0
	.word	0xa02f
	extl	d0
	rts

	.globl	_ReplaceGestalt
_ReplaceGestalt:
	movl	sp@(8),a0
	movl	sp@(4),d0
	.word	0xa5ad
	movl	a0,sp@(12)@(0)
	extl	d0
	rts

	.globl	_GetADBInfo
_GetADBInfo:
	movl	sp@(4),a0
	movl	sp@(8),d0
	.word	0xa079
	rts

	.globl	_stop
_stop:
	movl	sp@(8),d0
	movl	sp@(4),a0
	trap	#15
	rts

#define	emul_os(name)	\
	.globl	__##name;		\
__##name:	;			\
	moveml	d0-d1/a0-a1,sp@-;	\
	jsr	_##name;		\
	moveml	sp@+,d0-d1/a0-a1;	\
	rts

emul_os(WriteParam)
emul_os(ReadDateTime)
emul_os(SetDateTime)
emul_os(InitUtil)
emul_os(ReadXPRam)
emul_os(WriteXPRam)
emul_os(InsTime)
emul_os(RmvTime)
emul_os(PrimeTime)
emul_os(MemoryDispatch)

#define emul_tbox(name)	\
	.globl	__##name;		\
__##name:	;			\
	pea	sp@;			\
	moveml	d0-d1/a0-a1,sp@-;	\
	jsr	_##name;		\
	moveml	sp@+,d0-d1/a0-a1;	\
	movl	sp@,sp;			\
	rts

emul_tbox(SoundDispatch)
emul_tbox(SndDisposeChannel)
emul_tbox(SndAddModifier)
emul_tbox(SndDoCommand)
emul_tbox(SndDoImmediate)
emul_tbox(SndPlay)
emul_tbox(SndControl)
emul_tbox(SndNewChannel)
emul_tbox(Dequeue)
emul_tbox(Enqueue)	

	.globl	__trap1
__trap1:
	pea	sp@(8)
	moveml	a0-a6/d0-d7,sp@-
	jsr	_patched_inst
	moveml	sp@+,a0-a6/d0-d7
	movl	sp@(6),sp@(0)@(-4)
	movw	sp@(4),sp@(0)@(-6)
	lea	sp@(0)@(-6),sp
	rtr
