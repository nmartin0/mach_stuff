/*
 * Copyright (c) 1991 Carnegie-Mellon University
 *
 * This file is part of 'macpatches',
 * which is the emulation library used
 * for running macOS under MACH 3.0.
 *
 * Written by David Bohman in 1991
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
