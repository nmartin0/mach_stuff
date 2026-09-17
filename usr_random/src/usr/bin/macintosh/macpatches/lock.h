/* 
 * MacMach Operating System
 * Copyright (c) 1992 Carnegie Mellon University
 * All Rights Reserved.
 * 
 * MacMach was developed by CMU with support from Apple Computer, Inc.
 * Use of this software is constrained by the MacMach End-User license.
 */

typedef unsigned	lock_t[2];

extern boolean_t	lock_try(lock_t);

extern void	lock(lock_t), unlock(lock_t);
