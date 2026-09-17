/*
 * Copyright (c) 1991 Carnegie-Mellon University
 *
 * This file is part of 'macpatches',
 * which is the emulation library used
 * for running macOS under MACH 3.0.
 *
 * Written by David Bohman in 1991
 */

typedef struct {
    thread_t		thread;
    mach_port_t		reply_port;
} *th_t;

/*
 * Must be a power of 2.
 */
#define TH_STACK_SIZE	(16*1024)

extern th_t	th_alloc(void (*)(), int), th_self(void);
