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
 *  Software Distribution Coordinator  or  Software.Distribution@CS.CMU.EDU
 *  School of Computer Science
 *  Carnegie Mellon University
 *  Pittsburgh PA 15213-3890
 *
 * any improvements or extensions that they make and grant Carnegie Mellon
 * the rights to redistribute these changes.
 */
/*
 * HISTORY
 * $Log: mach_getenv.h,v $
 * Revision 1.1.1.1  1995/05/04  06:56:48  sclawson
 * New files.
 *
 * Revision 2.2  92/04/01  19:09:38  rpd
 * 	Created.
 * 	[92/02/21            jtp]
 * 
 */
#ifndef _mach_getenv_h_
#define _mach_getenv_h_

#include <mach/machine/kern_return.h>

char *getenv();
int setenv();
void unsetenv();
int putenv();
char **copy_env_to_environ();
kern_return_t copy_environ_to_env();

#endif
