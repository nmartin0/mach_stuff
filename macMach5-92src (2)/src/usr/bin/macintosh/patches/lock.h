/*
 * Copyright (c) 1991 Carnegie-Mellon University
 *
 * This file is part of 'macpatches',
 * which is the emulation library used
 * for running macOS under MACH 3.0.
 *
 * Written by David Bohman in 1991
 */

typedef unsigned	lock_t[2];

extern boolean_t	lock_try(lock_t);

extern void	lock(lock_t), unlock(lock_t);
