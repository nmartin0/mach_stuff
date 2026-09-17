/* 
 * MacMach Operating System
 * Copyright (c) 1992 Carnegie Mellon University
 * All Rights Reserved.
 * 
 * MacMach was developed by CMU with support from Apple Computer, Inc.
 * Use of this software is constrained by the MacMach End-User license.
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
