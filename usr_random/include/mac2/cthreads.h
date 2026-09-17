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
 * $Log:	cthreads.h,v $
 * Revision 2.4  91/05/14  17:59:25  mrt
 * 	Correcting copyright
 * 
 * Revision 2.3  91/02/14  14:21:26  mrt
 * 	Added new Mach copyright
 * 	[91/02/13  12:39:26  mrt]
 * 
 * Revision 2.2  90/11/05  14:38:00  rpd
 * 	Created.
 * 	[90/11/01            rwd]
 * 
 */

#ifndef _MACHINE_CTHREADS_H_
#define _MACHINE_CTHREADS_H_

typedef int spin_lock_t;
#define SPIN_LOCK_INITIALIZER 0
#define spin_lock_init(s) *(s)=0
#define spin_lock_locked(s) (*(s) != 0)

#endif _MACHINE_CTHREADS_H_
