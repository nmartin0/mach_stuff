/*
 * Copyright (c) 1991 Carnegie-Mellon University
 *
 * This file is part of 'macpatches',
 * which is the emulation library used
 * for running macOS under MACH 3.0.
 *
 * Written by David Bohman in 1991
 */

/*
 * Registers are passed by value to emulation
 * routines.
 */
typedef struct {
    unsigned long	d_0;
    unsigned long	d_1;
    unsigned long	a_0;
    unsigned long	a_1;
} os_reg_t;

typedef struct {
    unsigned long	d_0;
    unsigned long	d_1;
    unsigned long	a_0;
    unsigned long	a_1;
    unsigned long	a_7;
} tbox_reg_t;
