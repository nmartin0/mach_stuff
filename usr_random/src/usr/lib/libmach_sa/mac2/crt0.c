/* 
 * MacMach Operating System
 * Copyright (c) 1992 Carnegie Mellon University
 * All Rights Reserved.
 * 
 * MacMach was developed by CMU with support from Apple Computer, Inc.
 * Use of this software is constrained by the MacMach End-User license.
 */

/* VERSION is a identifying string to exist in all executables */
#ifdef VERSION
static char version[] = VERSION;
#endif

/* C start up routine for mac2. */

/*
  start() is jmp'ed to and the stack looks like:
	sp +0  [kargc]
	   +4  [kargv0]
  Since kargc is in the place that the compiler expects a return address,
  it can not be accessed as an argument to start().  The compiler sets up
  a stack frame for start().  The (argc, argv, envp) arguments to main()
  are allocated as local variables to start().  The stack then looks like:
	   -12  [argc]
	   -8   [argv]
	   -4   [envp]
	a6 +0   [old a6]
	   +4   [kargc]
	   +8   [kargv0]
*/

/* these function pointers are initialized non-zero by libmach */
void (*mach_init_routine)();
void (*_cthread_init_routine)();
int (*_cthread_exit_routine)();

char **environ = (char **)0;

#ifdef MCRT0
extern unsigned char etext;
extern unsigned char eprol; /* Not really extern, see asm() below. */
#endif

static start(char *kargv0)
{
  char **envp;
  char **argv;
  int argc;

  argc = *(int *)(&kargv0 - 1);
  argv = &kargv0;
  environ = envp = &argv[argc + 1];

  if (mach_init_routine) (*mach_init_routine)();

#ifdef MCRT0
  asm("_eprol:");
  monstartup(&eprol, &etext);
#endif MCRT0

  if (_cthread_init_routine) {
    (*_cthread_init_routine)();
    asm("tstl	d0; beq 0f; movl d0,sp; 0:");
  }
  (void)main(argc, argv, envp);
  if (_cthread_exit_routine) (*_cthread_exit_routine)(0);
  else exit(0);
}

#ifdef MCRT0
/* this version of exit() will call monitor() */
void exit(register int code)
{
  monitor(0);
  _cleanup();
  _exit(code);
}
#else
/*
 * null mcount and moncontrol,
 * just in case some routine is compiled for profiling
 */
moncontrol(int val)
{
}
asm("	.text");
asm("	.globl	mcount");
asm("mcount:	rts");
#endif

/* change _start to start */
asm(".set start, _start; .globl start");
