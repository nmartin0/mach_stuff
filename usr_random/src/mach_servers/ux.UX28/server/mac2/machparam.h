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
 * $Log:	machparam.h,v $
 * Revision 2.2  90/08/30  18:23:58  bohman
 * 	Created.
 * 
 * 
 */

/*
 *	Apple Macintosh II Mach (macmach)
 *
 *	File: mac2/machparam.h
 */

/*
 * Copyright (c) 1982, 1986 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 *	@(#)machparam.h	7.1 (Berkeley) 6/5/86
 */

#ifndef _MAC2_MACHPARAM_H_
#define _MAC2_MACHPARAM_H_

/* Machine dependent constants for mac2. */

#include <mac2/vmparam.h>

#define	NBPG	PAGE_SIZE		/* bytes/page */
#define	PGOFSET	(NBPG-1)		/* byte offset into page */
#define	PGSHIFT	PAGE_SHIFT		/* LOG2(NBPG) */

/*
 * These segment definitions
 * are only defined to allow
 * sun exec format compatability.
 */
#define	NBSG	131072		/* bytes/segment */
#define	SGOFSET	(NBSG-1)	/* byte offset into segment */
#define	SGSHIFT	17		/* LOG2(NBSG) */
#define NSGVA	2048		/* segments per virtual address space */

#define	CLSIZE		1
#define	CLSIZELOG2	0

#define	SSIZE		1	/* initial stack size/NBPG */
#define	SINCR		1	/* increment of stack/NBPG */

/*
 * UPAGES is defined to provide berkeley debugger compatability.  The u-area
 * consists of the fake user structure plus the thread kernel_stack (which
 * includes the thread pcb).
 */
#define	UPAGES		2

/* clicks to bytes */
#define	ctob(x)	((x)<<PGSHIFT)

/* bytes to clicks */
#define	btoc(x)	((((unsigned)(x)+(NBPG-1))>>PGSHIFT))

/* the following are needed for BSD/TAHOE compatability */

#define MACHINE "mac2"

#ifndef ASSEMBLER
#ifdef KERNEL
int cpuspeed;
#define	DELAY(n)	{ register int N = (cpuspeed ? cpuspeed : 1) * (n); while (--N > 0); }
#else KERNEL
#define	DELAY(n)	{ register int N = (n); while (--N > 0); }
#endif KERNEL
#endif

#include <mac2/endian.h>

#define	CHAR_BIT	NBBY
#define	CHAR_MAX	0x7f
#define	CHAR_MIN	0x80
#define	CLK_TCK		60			/* for times() */
#define	INT_MAX		0x7fffffff
#define	INT_MIN		0x80000000
#define	LONG_MAX	0x7fffffff
#define	LONG_MIN	0x80000000
#define	SCHAR_MAX	0x7f
#define	SCHAR_MIN	0x80
#define	SHRT_MAX	0x7fff
#define	SHRT_MIN	0x8000
#define	UCHAR_MAX	0xff
#define	UINT_MAX	0xffffffff
#define	ULONG_MAX	0xffffffff
#define	USHRT_MAX	0xffff

#endif	_MAC2_MACHPARAM_H_
