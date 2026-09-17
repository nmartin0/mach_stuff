/*
 * @OSF_COPYRIGHT@
 */
/*
 * HISTORY
 * $Log: thread_status.h,v $
 * Revision 1.1.14.4  1996/07/31  09:56:55  paire
 * 	Merged with nmk20b7_shared (1.1.22.1)
 * 	[96/06/10            paire]
 *
 * Revision 1.1.22.1  1996/03/22  16:05:59  bruel
 * 	register renaming cleanup.
 * 	[96/03/22            bruel]
 * 
 * Revision 1.1.14.3  1996/02/02  12:19:41  emcmanus
 * 	Copied from nmk20b5_shared.
 * 	[1996/02/01  16:59:37  emcmanus]
 * 
 * Revision 1.1.20.1  1996/01/11  15:17:01  bruel
 * 	Added fpu flag in thread_state.
 * 	Fixed THREAD_MACHINE_STATE_MAX since we have a 32 doubles state.
 * 	[96/01/11            bruel]
 * 
 * Revision 1.1.14.2  1995/08/21  20:45:48  devrcs
 * 	removed unecessary fields in thread_saved_state.
 * 	[95/07/03            bruel]
 * 
 * 	removed floats from saved_state, their place is in pcb.
 * 	[95/06/23            bruel]
 * 
 * Revision 1.1.14.1  1995/03/15  17:47:22  bruel
 * 	hppa merge
 * 	[1995/03/15  09:45:31  bruel]
 * 
 * Revision 1.1.6.3  1994/11/07  17:36:38  bruel
 * 	removed unused prevssp.
 * 	[94/11/04            bruel]
 * 
 * Revision 1.1.6.2  1994/06/13  08:04:34  bernadat
 * 	Remove HP700_SAVED_STATE. This is not a thread_state flavor
 * 	passed via thread_get_state.
 * 	[94/06/08            bernadat]
 * 
 * Revision 1.1.6.1  1994/05/25  13:16:04  bruel
 * 	moved hp700_saved_state here. Added some defines.
 * 	[94/05/19            bruel]
 * 
 * Revision 1.1.2.4  1994/03/24  19:45:05  bruel
 * 	filled up thread_syscall_state.
 * 	[94/03/24            bruel]
 * 
 * Revision 1.1.2.3  1994/03/21  16:39:16  bruel
 * 	Added thread_float_state. Removed floating points from hp700_thread_state.
 * 	[94/03/21            bruel]
 * 
 * Revision 1.1.2.2  1994/03/16  14:37:15  bruel
 * 	hp800->hp700. Added thread_syscall_state.
 * 	[94/02/21            bruel]
 * 
 * Revision 1.1.2.1  1994/02/11  13:28:28  bruel
 * 	Created from Utah.
 * 	[93/11/19            bruel]
 * 
 * $EndLog$
 */
/* 
 * Copyright (c) 1990, 1991, 1992, The University of Utah and
 * the Center for Software Science at the University of Utah (CSS).
 * All rights reserved.
 *
 * Permission to use, copy, modify and distribute this software is hereby
 * granted provided that (1) source code retains these copyright, permission,
 * and disclaimer notices, and (2) redistributions including binaries
 * reproduce the notices in supporting documentation, and (3) all advertising
 * materials mentioning features or use of this software display the following
 * acknowledgement: ``This product includes software developed by the Center
 * for Software Science at the University of Utah.''
 *
 * THE UNIVERSITY OF UTAH AND CSS ALLOW FREE USE OF THIS SOFTWARE IN ITS "AS
 * IS" CONDITION.  THE UNIVERSITY OF UTAH AND CSS DISCLAIM ANY LIABILITY OF
 * ANY KIND FOR ANY DAMAGES WHATSOEVER RESULTING FROM THE USE OF THIS SOFTWARE.
 *
 * CSS requests users of this software to return to css-dist@cs.utah.edu any
 * improvements that they make and grant CSS redistribution rights.
 *
 * 	Utah $Hdr: $
 */

#ifndef	_MACH_HP700_THREAD_STATUS_H_
#define _MACH_HP700_THREAD_STATUS_H_

/*
 * hp700_thread_state is the structure that is exported to user threads for 
 * use in status/mutate calls.  This structure should never change.
 */

#define HP700_THREAD_STATE      1
#define HP700_FLOAT_STATE       2
#define THREAD_SYSCALL_STATE	6
#define THREAD_STATE_NONE	7

struct hp700_thread_state {
	unsigned flags;
	unsigned r1;
	unsigned rp;          /* r2 */
	unsigned r3;          /* frame pointer when -g */
	unsigned r4;
	unsigned r5;
	unsigned r6;
	unsigned r7;
	unsigned r8;
	unsigned r9;
	unsigned r10;
	unsigned r11;
	unsigned r12;
	unsigned r13;
	unsigned r14;
	unsigned r15;
	unsigned r16;
	unsigned r17;
	unsigned r18;
	unsigned t4;          /* r19 */
	unsigned t3;          /* r20 */
	unsigned t2;          /* r21 */
	unsigned t1;          /* r22 */
	unsigned arg3;        /* r23 */
	unsigned arg2;        /* r24 */
	unsigned arg1;	      /* r25 */
	unsigned arg0;	      /* r26 */
	unsigned dp;	      /* r27 */
	unsigned ret0;	      /* r28 */
	unsigned ret1;	      /* r29 */
	unsigned sp;	      /* r30 */
	unsigned r31;
	unsigned sar;	      /* cr11 */
	unsigned iioq_head;
	unsigned iisq_head;
	unsigned iioq_tail;
	unsigned iisq_tail;
	unsigned eiem;        /* cr15 */
	unsigned iir;         /* cr19 */
	unsigned isr;         /* cr20 */
	unsigned ior;         /* cr21 */
	unsigned ipsw;	      /* cr22 */
	unsigned sr4;
	unsigned sr0;
	unsigned sr1;
	unsigned sr2;
	unsigned sr3;
	unsigned sr5;
	unsigned sr6;
	unsigned sr7;
	unsigned rctr;         /* cr0 */
	unsigned pidr1;        /* cr8 */
	unsigned pidr2;        /* cr9 */
	unsigned ccr;	       /* cr10 */
	unsigned pidr3;        /* cr12 */
	unsigned pidr4;        /* cr13 */
	unsigned ptov;         /* cr24 */
	unsigned tr1;          /* cr25 */
	unsigned tr2;          /* cr26 */
	unsigned fpu; 
};

/*
 * saved state structure
 *
 * This structure corresponds to the state of the user registers as saved
 * on the stack upon kernel entry (saved in pcb). On interrupts and exceptions
 * we save all registers. On system calls we only save the registers not
 * saved by the caller.
 *
 * NB: If you change this structure you must also change the KGDB stub as well.
 */

/*
 * in order to avoid the mess of converting between HP-UX/BSD and Mach for 
 * debuggers we'll just adopt the same HP-UX layout.
 */
struct hp700_saved_state {
	unsigned flags;
	unsigned r1;
	unsigned rp;          /* r2 */
	unsigned r3;          /* frame pointer when -g */
	unsigned r4;
	unsigned r5;
	unsigned r6;
	unsigned r7;
	unsigned r8;
	unsigned r9;
	unsigned r10;
	unsigned r11;
	unsigned r12;
	unsigned r13;
	unsigned r14;
	unsigned r15;
	unsigned r16;
	unsigned r17;
	unsigned r18;
	unsigned t4;          /* r19 */
	unsigned t3;          /* r20 */
	unsigned t2;          /* r21 */
	unsigned t1;          /* r22 */
	unsigned arg3;        /* r23 */
	unsigned arg2;        /* r24 */
	unsigned arg1;	      /* r25 */
	unsigned arg0;	      /* r26 */
	unsigned dp;	      /* r27 */
	unsigned ret0;	      /* r28 */
	unsigned ret1;	      /* r29 */
	unsigned sp;	      /* r30 */
	unsigned r31;
	unsigned sar;	      /* cr11 */
	unsigned iioq_head;
	unsigned iisq_head;
	unsigned iioq_tail;
	unsigned iisq_tail;
	unsigned eiem;        /* cr15 */
	unsigned iir;         /* cr19 */
	unsigned isr;         /* cr20 */
	unsigned ior;         /* cr21 */
	unsigned ipsw;	      /* cr22 */
	unsigned sr4;
	unsigned sr0;
	unsigned sr1;
	unsigned sr2;
	unsigned sr3;
	unsigned sr5;
	unsigned sr6;
	unsigned sr7;
	unsigned rctr;         /* cr0 */
	unsigned pidr1;        /* cr8 */
	unsigned pidr2;        /* cr9 */
	unsigned ccr;	       /* cr10 */
	unsigned pidr3;        /* cr12 */
	unsigned pidr4;        /* cr13 */
	unsigned ptov;         /* cr24 */
	unsigned tr1;          /* cr25 */
	unsigned tr2;          /* cr26 */
	unsigned fpu; 
};

struct hp700_float_state {
	double	fr0;
	double	fr1;
	double	fr2;
	double	fr3;
	double	farg0;         /* fr4,fret */
	double	farg1;         /* fr5 */
	double	farg2;         /* fr6 */
	double	farg3;         /* fr7 */
	double	fr8;
	double	fr9;
	double	fr10;
	double	fr11;
	double	fr12;
	double	fr13;
	double	fr14;
	double	fr15;
	double	fr16;
	double	fr17;
	double	fr18;
	double	fr19;
	double	fr20;
	double	fr21;
	double	fr22;
	double	fr23;
	double	fr24;
	double	fr25;
	double	fr26;
	double	fr27;
	double	fr28;
	double	fr29;
	double	fr30;
	double	fr31;
};

struct thread_syscall_state {
	unsigned r1;
	unsigned rp;		
	unsigned r3;                    /* frame pointer when -g */
	unsigned t1;
	unsigned arg3;			/* r23 */
	unsigned arg2;			/* r24 */
	unsigned arg1;			/* r25 */
	unsigned arg0;			/* r26 */
	unsigned dp;			/* r27 */
	unsigned ret0;			/* r28 */
	unsigned ret1;			/* r29 */
	unsigned sp;			/* r30 */
	unsigned iioq_head;
	unsigned iioq_tail;
};

/*
 * Save State Flags
 */
#define	SS_INTRAP	0x01	/* State saved from trap */
#define	SS_INSYSCALL	0x02	/* State saved from system call */
#define SS_ININT	0x04	/* On the interrupt stack */
#define	SS_PSPKERNEL	0x08	/* Previous context stack pointer kernel */
#define	SS_RFIRETURN	0x10	/* Must RFI from syscall to restore
				   complete user state (i.e. sigreturn) */
#define	SS_INTRAP_POS	 31	/* State saved from trap */
#define	SS_INSYSCALL_POS 30	/* State saved from system call */
#define SS_ININT_POS	 29	/* On the interrupt stack */
#define	SS_PSPKERNEL_POS 28	/* Previous context stack pointer kernel */
#define	SS_RFIRETURN_POS 27	/* RFI from syscall */

#define HP700_THREAD_STATE_COUNT \
   (sizeof(struct hp700_thread_state) / sizeof(int))

#define HP700_FLOAT_STATE_COUNT \
   (sizeof(struct hp700_float_state) / sizeof(int))

#define HP700_SYSCALL_STATE_COUNT \
   (sizeof(struct thread_syscall_state) / sizeof(int))

/*
 * Machine-independent way for servers and Mach's exception mechanism to
 * choose the most efficient state flavor for exception RPC's:
 */
#define MACHINE_THREAD_STATE		HP700_THREAD_STATE
#define MACHINE_THREAD_STATE_COUNT	HP700_THREAD_STATE_COUNT

/*
 * Largest state on this machine:
 */
#define THREAD_MACHINE_STATE_MAX	HP700_FLOAT_STATE_COUNT

#endif /* _MACH_HP700_THREAD_STATUS_H_ */
