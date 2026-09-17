/* 
 * MacMach Operating System
 * Copyright (c) 1992 Carnegie Mellon University
 * All Rights Reserved.
 * 
 * MacMach was developed by CMU with support from Apple Computer, Inc.
 * Use of this software is constrained by the MacMach End-User license.
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
