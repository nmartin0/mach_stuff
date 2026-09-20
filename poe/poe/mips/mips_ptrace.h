/* 
 * Mach Operating System
 * Copyright (c) 1991,1990 Carnegie Mellon University
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
 * HISTORY
 * $Log:	mips_ptrace.h,v $
 * Revision 2.3  91/12/19  20:28:55  mrt
 * 	Updated to new copyright
 * 
 * Revision 2.2  90/09/08  00:19:50  rwd
 * 	Taken from XUX22 version by af.
 * 	[90/07/14            rwd]
 * 
 *	Created.
 */
/*
 *	File:	./mips/mips_ptrace.h
 *	Author:	Joseph S. Barrera III, Randall W. Dean
 *
 *	Copyright (c) 1990 Joseph S. Barrera III, Randall W. Dean
 */


/*
 * register number definitions for PT_READ_U's and PT_WRITE_U's
 */
#define GPR_BASE	0			/* general purpose regs */
#define	NGP_REGS	32			/* number of gpr's */

#define FPR_BASE	(GPR_BASE+NGP_REGS)	/* fp regs */
#define	NFP_REGS	32			/* number of fp regs */

#define	SIG_BASE	(FPR_BASE+NFP_REGS)	/* sig handler addresses */
#define	NSIG_HNDLRS	32			/* number of signal handlers */

#define SPEC_BASE	(SIG_BASE+NSIG_HNDLRS)	/* base of spec purpose regs */
#define PC		SPEC_BASE+0		/* program counter */
#define	CAUSE		SPEC_BASE+1		/* cp0 cause register */
#define MMHI		SPEC_BASE+2		/* multiply high */
#define MMLO		SPEC_BASE+3		/* multiply low */
#define FPC_CSR		SPEC_BASE+4		/* fp csr register */
#define FPC_EIR		SPEC_BASE+5		/* fp eir register */
#define NSPEC_REGS	8			/* number of spec registers */
#define NPTRC_REGS	(SPEC_BASE + NSPEC_REGS)

