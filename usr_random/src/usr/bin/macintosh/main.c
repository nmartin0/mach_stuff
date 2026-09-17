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

task_t task;
static thread_t thread;

int file;
struct exec filehdr;

stop()
{
  task_suspend(task);
  thread_abort(thread);
  debugger(thread);
  task_resume(task);
}

int open_file(char *name)
{
  return ((file = open(name, 0)) < 0) ? -1 : 0;
}

main(argc, argv)
int argc;
char **argv;
{
  char buffer[1024];
  char *path;
  extern char *getenv();

  stdout->_flag |= _IONBF;

  argc--; argv++;
  if (argc > 1) {
    fprintf(stderr, "usage: [options] [ <patch file> ]\n");
    exit(0);
  }
  if (argc) {
    if (open_file(*argv)) unix_error_exit("open patches file");
  }
  else {
    if (!(path = getenv("LPATH"))) path = "/usr/lib";
    if (searchp(path, "macpatches", buffer, open_file)) {
      fprintf(stderr, "can not find macpatches on LPATH\n");
      exit(0);
    }
  }
  signal_init();
  pager_setup();
  setup_task();
  setup_thread();
  server_main();
  normal_exit();
}

setup_task()
{
  kern_return_t result;

  result = task_create(mach_task_self(), FALSE,  &task);
  if (result != KERN_SUCCESS) mach_error_exit("task_create", result);
  (void)task_suspend(task);
}

setup_thread()
{
  thread_state_regs_t regs;
  thread_state_frame_t frame;
  kern_return_t result;    

  result = thread_create(task, &thread);
  if (result != KERN_SUCCESS) mach_error_exit("thread create", result);
  regs.r_sp = EMUL_PATCHES_START+EMUL_PATCHES_SIZE;
  result = thread_set_state(thread,
			    THREAD_STATE_REGS,
			    (thread_state_t)&regs,
			    THREAD_STATE_REGS_COUNT);
  if (result != KERN_SUCCESS) mach_error_exit("thread set regs", result);
  frame.f_normal.f_fmt = STKFMT_NORMAL;
  frame.f_normal.f_sr = 0;
  frame.f_normal.f_pc = EMUL_PATCHES_START;
  result = thread_set_state(thread,
			    THREAD_STATE_FRAME,
			    (thread_state_t)&frame,
			    THREAD_STATE_FRAME_COUNT);
  if (result != KERN_SUCCESS) mach_error_exit("thread set frame", result);
  (void)thread_resume(thread);
}
