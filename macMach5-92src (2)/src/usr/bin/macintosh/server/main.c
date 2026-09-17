/* 
 * Mach Operating System
 * Copyright (c) 1991 Carnegie-Mellon University
 * Copyright (c) 1990 Carnegie-Mellon University
 * Copyright (c) 1989 Carnegie-Mellon University
 * Copyright (c) 1988 Carnegie-Mellon University
 * All rights reserved.  The CMU software License Agreement specifies
 * the terms and conditions for use and redistribution.
 */
/*
 * HISTORY
 */

/*
 *	Apple Macintosh II Mach (macmach)
 *
 *	File: emul/server/main.c
 *	Author: David E. Bohman II (CMU macmach)
 */

#include "server_defs.h"
#include "pager_defs.h"

#include <sys/errno.h>
#include <sys/resource.h>

task_t		task;
static thread_t	thread;

int		file;
struct exec	filehdr;

stop()
{
    task_suspend(task);
    thread_abort(thread);

    debugger(thread);

    task_resume(task);
}

main(argc, argv)
int argc;
char **argv;
{
    stdout->_flag |= _IONBF;

    argc--; argv++;
    while (argc > 0 && (*argv)[0] == '-') {
	switch ((*argv)[1]) {
	}
	argv++; argc--;
    }

    if (argc < 1) {
	fprintf(stderr, "usage: [options] <patch file>\n");
	fprintf(stderr, "options:\n");
	exit(0);
    }
    file = open(*argv, 0);
    if (file < 0)
	unix_error_exit("open patches file");

    argc--; argv++;

    signal_init();

    pager_setup();
  
    setup_task();

    setup_thread();

    server_main();

    normal_exit();
}

setup_task()
{
    kern_return_t	result;

    result = task_create(mach_task_self(),
			 FALSE,
			 &task);
    if (result != KERN_SUCCESS)
	mach_error_exit("task_create", result);

    (void) task_suspend(task);
}

setup_thread()
{
    thread_state_regs_t		regs;
    thread_state_frame_t	frame;
    kern_return_t		result;    

    result = thread_create(task, &thread);
    if (result != KERN_SUCCESS)
	mach_error_exit("thread create", result);

    regs.r_sp = EMUL_PATCHES_START+EMUL_PATCHES_SIZE;
    result = thread_set_state(thread,
			      THREAD_STATE_REGS,
			      (thread_state_t)&regs,
			      THREAD_STATE_REGS_COUNT);
    if (result != KERN_SUCCESS)
	mach_error_exit("thread set regs", result);

    frame.f_normal.f_fmt = STKFMT_NORMAL;
    frame.f_normal.f_sr = 0;
    frame.f_normal.f_pc = EMUL_PATCHES_START;
    result = thread_set_state(thread,
			      THREAD_STATE_FRAME,
			      (thread_state_t)&frame,
			      THREAD_STATE_FRAME_COUNT);
    if (result != KERN_SUCCESS)
	mach_error_exit("thread set frame", result);

    (void) thread_resume(thread);
}
