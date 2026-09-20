/* 
 * Mach Operating System
 * Copyright (c) 1991,1990,1989,1988,1987 Carnegie Mellon University
 * All Rights Reserved.
 * 
 * Permission to use, copy, modify and distribute this software and its
 * documentation is hereby granted, provided that both the copyright
 * notice and this permission notice appear in all copies of the
 * software, derivative works or modified versions, and any portions
 * thereof, and that both notices appear in supporting documentation.
 * 
 * CARNEGIE MELLON ALLOWS FREE USE OF THIS SOFTWARE IN ITS "AS IS"
 * CONDITION.  CARNEGIE MELLON DISCLAIMS ANY LIABILITY OF ANY KIND FOR
 * ANY DAMAGES WHATSOEVER RESULTING FROM THE USE OF THIS SOFTWARE.
 * 
 * Carnegie Mellon requests users of this software to return to
 * 
 *  Software Distribution Coordinator   or   Software.Distribution@CS.CMU.EDU
 *  School of Computer Science
 *  Carnegie Mellon University
 *  Pittsburgh PA 15213-3890
 * 
 * any improvements or extensions that they made and grant Carnegie Mellon
 * the rights to redistribute these changes.
 */
/*
 * Author:  Alessandro Forin (af) at Carnegie-Mellon University
 *
 * HISTORY
 * $Log:	emul_vector.s,v $
 * Revision 2.5  94/03/25  18:21:36  mrt
 * 	Changed comments to standard c style.
 * 	[93/11/16            mrt]
 * 
 * Revision 2.4  91/12/19  20:25:37  mrt
 * 	Updated to new copyright
 * 
 * Revision 2.3  91/03/09  15:13:32  rpd
 * 	Changed EF_ prefix to MSS_.
 * 	[91/03/09            rpd]
 * 
 * Revision 2.2  90/11/16  11:41:17  rwd
 * 	Taken from UX21
 * 	[90/11/14  15:07:29  rwd]
 * 
 * Revision 2.3  90/06/19  23:06:57  rpd
 * 	Added emul_pid_by_task.
 * 	[90/06/14            rpd]
 * 
 * Revision 2.2  89/11/29  15:26:55  af
 * 	Fixed sigreturn to reload the S registers.
 * 	Adjust the sp ourselves when taking signals.
 * 	[89/11/26  11:13:52  af]
 * 
 * 	Created, from Vax version by David Golub who took it from the
 * 	one I designed with Doug Orr and Rich Sanzi, who got the idea
 * 	from Rick Rashid who came up with it in a meeting I called to
 * 	talk about debugging... and that's where the story ends.
 * 	I think.
 * 	[89/10/26            af]
 * 
 */

#include <sys/syscall.h>
#include <sys/errno.h>
#include <mach/mips/asm.h>
#include <mach/mips/mips_instruction.h>

/*
 * Emulator vector entry - allocate new stack
 */
	.data
	.globl	emul_stack_lock
emul_stack_lock:
	.word	0
	.text

/*
 * Stack frame definitions.
 *
 * In order to optimize signal handling, this
 * structure is laid down in accordance with
 * the sigcontext structure defined by MIPS.
 * This saves extra copies in case we have to
 * take a signal on exit from a syscall.
 * [Note that this way we could define an even
 *  faster sigtramp, because the regstate got
 *  saved in user space to begin with..]
 */
#define SC_ONSTACK	0
#define SC_MASK		4
#define SC_PC		8
#define SC_ZERO		12
#define SC_AT		16
#define	SC_V0		20
#define	SC_V1		24
#define SC_A0		28
#define SC_A1		32
#define SC_A2		36
#define SC_A3		40
#define	SC_T0		44
#define	SC_T1		48
#define	SC_T2		52
#define	SC_T3		56
#define	SC_T4		60
#define	SC_T5		64
#define	SC_T6		68
#define	SC_T7		72
#define	SC_S0		76
#define	SC_S1		80
#define	SC_S2		84
#define	SC_S3		88
#define	SC_S4		92
#define	SC_S5		96
#define	SC_S6		100
#define	SC_S7		104
#define	SC_T8		108
#define	SC_T9		112
#define SC_K0		116
#define SC_K1		120
#define	SC_GP		124
#define	SC_SP		128
#define	SC_FP		132
#define	SC_RA		136
#define SC_LO		140
#define SC_HI		144

#define SC_USE_FPA	148
#define SC_FPA_REGS	152
#define SC_FPA_CSR	280			/*(152+(32*4))*/
#define SC_FPA_EIR	284

#define SC_CS		288
#define SC_BADV		292
#define SC_BADP		296

#define SC_SIZE		300
			/* make VECTOR macro happy */
#define MSS_SIZE	SC_SIZE
#define MSS_RA		SC_RA

/*
 * Special extra frame for signal delivery
 */
#define SF_PC 0
#define SF_A0 4
#define SF_A1 8
#define SF_A2 12
#define SF_A3 16

/*
 * Generic emulator entry
 */
#define	EE(n)		sw a0,(SC_A0-SC_SIZE)(sp); j emul_common; li a0,(n)
#define	EE_S(n)		sw a0,(SC_A0-SC_SIZE)(sp); j emul_s_common; li a0,(n)

/*
 * Generic call
 */
#define ONLY_S		0x40ff0000
#define ALL_BUT_K_S	(0xffffffff & (~(M_K0|M_K1|ONLY_S)))

	.set	noreorder
VECTOR(emul_common,ALL_BUT_K_S)
	.set	noat
	sw	sp,(SC_SP-SC_SIZE)(sp)
	subu	sp,SC_SIZE
	sw	AT,SC_AT(sp)
	.set	at
	.set	reorder
emul_common_1:
	sw	a0,SC_V0(sp)
	sw	v0,SC_PC(sp)
	sw	v1,SC_V1(sp)
	sw	a1,SC_A1(sp)
	sw	a2,SC_A2(sp)
	sw	a3,SC_A3(sp)
	sw	ra,SC_RA(sp)
	sw	gp,SC_GP(sp)
	la	gp,_gp
	sw	t0,SC_T0(sp)
	mflo	t0
	sw	t1,SC_T1(sp)
	mfhi	t1
	sw	t2,SC_T2(sp)
	sw	t3,SC_T3(sp)
	sw	t4,SC_T4(sp)
	sw	t5,SC_T5(sp)
	sw	t6,SC_T6(sp)
	sw	t7,SC_T7(sp)
	sw	t8,SC_T8(sp)
	sw	t9,SC_T9(sp)
	sw	t0,SC_LO(sp)
	sw	t1,SC_HI(sp)
	.set	noreorder
					/* get a stack */
1:	la	a0,emul_stack_lock	/* XXX	syscall(swtch) */
	.set	noat
	lw	AT,0(a0)
	nop
	bne	AT,zero,1b
	nop
	.set	at
	.word	op_tas
	bne	a0,zero,1b
	nop

	lw	v0,emul_stack_list
	nop
	bne	v0,zero,2f
	nop
					/* must allocate a stack */
	sw	zero,emul_stack_lock
	jal	emul_stack_alloc
	subu	sp,4*4
	b	3f
	addu	sp,4*4

2:	lw	a0,0(v0)		/* dequeue from stack list */
	sw	a0,emul_stack_list
	sw	zero,emul_stack_lock
3:					/* save user's SP */
	subu	a0,v0,20		/* new SP */
	move	a1,sp
	sw	sp,16(a0)
	jal	emul_syscall		/* off to C code, after.. */
	move	sp,a0			/* .. switching to emulator's stack */
/*
 * Return here when syscall completes
 * If the return value is 1 its a sigreturn
 * and we must reload the S registers,
 * If the return value is other non-zero we
 * must take a special exit path to
 * the user's signal handler.
 */
	addu	a1,sp,20
	lw	sp,16(sp)		/* back to user's stack */
1:	la	a0,emul_stack_lock	/* put back emulator's stack */
	.set	noat
	lw	AT,0(a0)
	nop
	bne	AT,zero,1b
	nop
	.set	at
	.word	op_tas
	bne	a0,zero,1b
	nop

	lw	a0,emul_stack_list	/* .. onto stack list */
	nop
	sw	a0,0(a1)
	sw	a1,emul_stack_list
	sw	zero,emul_stack_lock
					/* restore user's registers */
	bne	v0,1,1f			/* sigreturn? */
	nop
	lw	s0,SC_S0(sp)
	lw	s1,SC_S1(sp)
	lw	s2,SC_S2(sp)
	lw	s3,SC_S3(sp)
	lw	s4,SC_S4(sp)
	lw	s5,SC_S5(sp)
	lw	s6,SC_S6(sp)
	lw	s7,SC_S7(sp)
	lw	fp,SC_FP(sp)
	b	restore_regs
	move	v0,zero

1:	beq	v0,zero,restore_regs	/* signals? */
	nop
	lw	a0,SF_A0(v0)		/* sig */
	lw	a1,SF_A1(v0)		/* code */
	lw	a2,SF_A2(v0)		/* scp */
	lw	a3,SF_A3(v0)		/* handler */
	lw	v0,SF_PC(v0)		/* sigtramp */
	move	sp,a2
					/* v0==0 still means no signals */
restore_regs:
	lw	t0,SC_LO(sp)
	lw	t1,SC_HI(sp)
	mtlo	t0
	mthi	t1
	lw	t0,SC_T0(sp)
	lw	t1,SC_T1(sp)
	lw	t2,SC_T2(sp)
	lw	t3,SC_T3(sp)
	lw	t4,SC_T4(sp)
	lw	t5,SC_T5(sp)
	lw	t6,SC_T6(sp)
	lw	t7,SC_T7(sp)
	lw	t8,SC_T8(sp)
	lw	t9,SC_T9(sp)

	lw	v1,SC_V1(sp)
	lw	ra,SC_RA(sp)
	beq	v0,zero,1f			/* signals ? */
	lw	gp,SC_GP(sp)
	.set	noat
	lw	AT,SC_AT(sp)
	.set	at
	j	v0
	lw	v0,SC_V0(sp)
1:					/* no signals */
	lw	v0,SC_V0(sp)
	lw	a0,SC_A0(sp)
	lw	a1,SC_A1(sp)
	lw	a2,SC_A2(sp)
	lw	a3,SC_A3(sp)
	.set	noat
	lw	AT,SC_PC(sp)
	lw	sp,SC_SP(sp)
	j	AT
	lw	AT,SC_AT-SC_SIZE(sp)
	.set	at
	END(emul_common)

/*
 * Declare the trap vectors
 */
EE(-10);EE(-9); EE(-8); EE(-7); EE(-6); EE(-5); EE(-4); EE(-3); EE(-2); EE(-1)
	EXPORT(emul_vector_base)
EE(  0);EE(  1);EE(  2);EE(  3);EE(  4);EE(  5);EE(  6);EE(  7);EE(  8);EE(  9)
EE( 10);EE( 11);EE( 12);EE( 13);EE( 14);EE( 15);EE( 16);EE( 17);EE( 18);EE( 19)
EE( 20);EE( 21);EE( 22);EE( 23);EE( 24);EE( 25);EE( 26);EE( 27);EE( 28);EE( 29)
EE( 30);EE( 31);EE( 32);EE( 33);EE( 34);EE( 35);EE( 36);EE( 37);EE( 38);EE( 39)
EE( 40);EE( 41);EE( 42);EE( 43);EE( 44);EE( 45);EE( 46);EE( 47);EE( 48);EE( 49)
EE( 50);EE( 51);EE( 52);EE( 53);EE( 54);EE( 55);EE( 56);EE( 57);EE( 58);EE( 59)
EE( 60);EE( 61);EE( 62);EE( 63);EE( 64);EE( 65);EE( 66);EE( 67);EE( 68);EE( 69)
EE( 70);EE( 71);EE( 72);EE( 73);EE( 74);EE( 75);EE( 76);EE( 77);EE( 78);EE( 79)
EE( 80);EE( 81);EE( 82);EE( 83);EE( 84);EE( 85);EE( 86);EE( 87);EE( 88);EE( 89)
EE( 90);EE( 91);EE( 92);EE( 93);EE( 94);EE( 95);EE( 96);EE( 97);EE( 98);EE( 99)
EE(100);EE(101);EE(102);EE(103);EE(104);EE(105);EE(106);EE(107);EE(108);EE(109)
EE(110);EE(111);EE(112);EE(113);EE(114);EE(115);EE(116);EE(117);EE(118);EE(119)
EE(120);EE(121);EE(122);EE(123);EE(124);EE(125);EE(126);EE(127);EE(128);EE(129)
EE(130);EE(131);EE(132);EE(133);EE(134);EE(135);EE(136);EE(137);EE(138);EE(139)
EE(140);EE(141);EE(142);EE(143);EE(144);EE(145);EE(146);EE(147);EE(148);EE(149)
EE(150);EE(151);EE(152);EE(153);EE(154);EE(155);EE(156);EE(157);EE(158);EE(159)
EE(160);EE(161);EE(162);EE(163);EE(164);EE(165);EE(166);EE(167);EE(168);EE(169)
EE(170);EE(171);EE(172);EE(173);EE(174);EE(175);EE(176);EE(177);EE(178);EE(179)
EE(180);EE(181);EE(182);EE(183);EE(184);EE(185);EE(186);EE(187);EE(188);EE(189)
EE(190);EE(191);EE(192);EE(193);EE(194);EE(195);EE(196);EE(197);EE(198);EE(199)
EE(200);EE(201);EE(202);EE(203);EE(204);EE(205);EE(206);EE(207);EE(208);EE(209)
EE(210);EE(211);EE(212);EE(213);EE(214);EE(215);EE(216);EE(217);EE(218);EE(219)
EE(220);EE(221);EE(222);EE(223);EE(224);EE(225);EE(226);EE(227);EE(228);EE(229)
EE(230);EE(231);EE(232);EE(233);EE(234);EE(235);EE(236);EE(237);EE(238);EE(239)
EE(240);EE(241);EE(242);EE(243);EE(244);EE(245);EE(246);EE(247);EE(248);EE(249)
EE(250);EE(251);EE(252);EE(253);EE(254);EE(255);EE(256);EE(257);EE(258);EE(259)

/*
 * Calls that do not fit in the table:
 */
LEAF(emul_task_by_pid)
	EE( -33)
	END(emul_task_by_pid)
LEAF(emul_pid_by_task)
	EE( -34)
	END(emul_pid_by_task)
LEAF(emul_init_process)
	EE(-41)
	END(emul_init_process)
LEAF(emul_htg_syscall)
	EE( -59)
	END(emul_htg_syscall)

/*
 * When forking, need to save the S registers.
 * Since we could fork indirectly, indirect too.
 */
LEAF(emul_fork)
	EE_S(SYS_fork)
	END(emul_fork)
LEAF(emul_indirect)
	EE_S(0)
	END(emul_indirect)

VECTOR(emul_s_common,ONLY_S|M_AT|M_A0)
	sw	sp,(SC_SP-SC_SIZE)(sp)
	subu	sp,SC_SIZE
	.set	noat
	sw	AT,SC_AT(sp)
	.set	at
	sw	s0,SC_S0(sp)
	sw	s1,SC_S1(sp)
	sw	s2,SC_S2(sp)
	sw	s3,SC_S3(sp)
	sw	s4,SC_S4(sp)
	sw	s5,SC_S5(sp)
	sw	s6,SC_S6(sp)
	sw	s7,SC_S7(sp)
	j	emul_common_1
	sw	fp,SC_FP(sp)
	END(emul_s_common)

/*
 * Amazingly enough, the easiest way to exec /etc/init is to take
 * an emulator trap!
 */
LEAF(emul_execve)
	li	v0,SYS_execve
	syscall
	/* we only get here on failure */
	j	ra
	li	v0,-1
	END(emul_execve)

/*
 * Reinitialize emulator before getting back to child
 * We get here with the stack pointing to our copy of
 * the saved state, and the relevant regs modified.
 */
LEAF(child_fork)
				/* Our state right now is the one
				 * that the user will see.  But we
				 * need to reinit some things before
				 * we get back to the user. */
	subu	sp,SC_SIZE
	la	gp,_gp		/* too messy doing this elsewhere */
	sw	v1,SC_V1(sp)
	sw	v0,SC_V0(sp)
	jal	child_init
	sw	a3,SC_A3(sp)
	b	restore_regs
	move	v0,zero
	END(child_fork)
