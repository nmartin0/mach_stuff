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
 * $Log:	mach_param.h,v $
 * Revision 2.2  91/09/12  16:41:20  bohman
 * 	Created.
 * 	[91/09/11  14:47:35  bohman]
 * 
 */

#ifndef _MAC2_MACH_PARAM_H_
#define _MAC2_MACH_PARAM_H_

/* Machine dependent constants for mac2. */

/* define SMALLPAGE to use 4K pages instead of 8K */
#undef SMALLPAGE

#ifdef KERNEL
#include <machine/cpu_inline.c>
#endif

#define HZ (60) /* XXX how to determine current clock rate? */

/* Macros to decode processor status word. */
#define	USERMODE(ps)	(((ps) & SR_SUPR) == 0)
#define	BASEPRI(ps)	(((ps) & SR_IPL) == 0)

/* The user stack can't start at the top page due to the way that MACH
 * handles ranges.  *** WHY IS THIS? ***
 */
#define USER_STACK_TOP	(0 - (16*1024*1024))

/* network byte orders */
#define	ntohl(x) (x)
#define	ntohs(x) (x)
#define	htonl(x) (x)
#define	htons(x) (x)

#endif /* _MAC2_MACH_PARAM_H_ */
