/* 
 * MacMach Operating System
 * Copyright (c) 1992 Carnegie Mellon University
 * All Rights Reserved.
 * 
 * MacMach was developed by CMU with support from Apple Computer, Inc.
 * Use of this software is constrained by the MacMach End-User license.
 */

#define PRIO_NONE	0
#define PRIO_LOW	1
#define PRIO_HIGH	2
#define PRIO_MAX	3

extern void	prio_init(void);
extern int	prio_get(void), prio_get_thread(thread_t);
extern int	prio_set(int), prio_set_thread(thread_t, int);
extern int	prio_enter(int);
extern void	prio_exit(int);
