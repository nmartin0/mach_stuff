/* 
 * MacMach Operating System
 * Copyright (c) 1992 Carnegie Mellon University
 * All Rights Reserved.
 * 
 * MacMach was developed by CMU with support from Apple Computer, Inc.
 * Use of this software is constrained by the MacMach End-User license.
 */

#include <mac2/SYS.h>

SYSCALL(fork) /* pid = fork() */
	tstl	d1
	beqs	skip	/* parent, since d1 == 0 in parent, 1 in child */
	link	a6,#0
	jsr	_mach_init
	unlk	a6
	clrl	d0
skip:	rts
