/* 
 * Mach Operating System
 * Copyright (c) 1991,1990,1989 Carnegie Mellon University
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
 * $Log:	emul_cache.c,v $
 * Revision 2.3  91/12/19  20:20:05  mrt
 * 	Updated to new copyright
 * 
 * Revision 2.2  90/11/16  11:39:40  rwd
 * 	Taken from UX21
 * 	[90/11/14  15:19:50  rwd]
 * 
 * Revision 2.2  89/10/17  11:23:57  rwd
 * 	Removed reference to ERROR macro.
 * 	[89/09/26            rwd]
 * 
 */
/*
 * Routines which can cache values for performance.
 */

#ifdef ECACHE
#include <bsd_msg.h>

#define ROUTINE(routine) int routine(serv_port, syscode, argp, rvalp)\
	int serv_port;\
	int syscode;\
	int *argp;\
	int *rvalp;\
{\
	register int rv=0;\
	if (emul_cache.routine[0] == 0) {\
	    rv = emul_generic(serv_port, syscode, argp, rvalp);\
	    if (rv) return (rv);\
	    emul_cache.routine[0] = rvalp[0];\
	    emul_cache.routine[1] = rvalp[1];\
	} else {\
	    rvalp[0] = emul_cache.routine[0];\
	    rvalp[1] = emul_cache.routine[1];\
	}\
	return (rv);\
}

struct emul_cache_struct {
	int e_getpid[2];
	int e_getdtablesize[2];
	int e_getpagesize[2];
} emul_cache;

emul_cache_init()
{
	bzero(&emul_cache,sizeof(struct emul_cache_struct));
}

ROUTINE(e_getpid)
ROUTINE(e_getdtablesize)
ROUTINE(e_getpagesize)
#endif ECACHE
