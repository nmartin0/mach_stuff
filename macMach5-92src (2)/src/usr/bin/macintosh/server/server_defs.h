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
 *	File: emul/server/server_defs.h
 *	Author: David E. Bohman II (CMU macmach)
 */

#include <mach.h>

#include <stdio.h>

#include <sys/param.h>
#include <sys/file.h>
#include <sys/exec.h>

#define EMUL_PATCHES_START	0x40880000
#define	EMUL_PATCHES_SIZE	0x00080000

/*
 * Terminate the emulator task if one
 * exists.
 */
#define task_kill()					\
{							\
    if (task != 0) {					\
	(void) task_terminate(task);			\
    }							\
}

/*
 * Code for exiting the server.
 */
#define normal_exit()	\
{			\
    task_kill();	\
    exit(0);		\
}
    
#define mach_error_exit(s, r)	\
{				\
    mach_error((s), (r));	\
    task_kill();		\
    exit(0);			\
}

#define unix_error_exit(s)	\
{				\
    perror((s));		\
    task_kill();		\
    exit(0);			\
}

#define other_error_exit(s)		\
{					\
    fprintf(stderr, "%s\n", (s));	\
    task_kill();			\
    exit(0);				\
}

/*
 * Assertion checking.
 */
#define	ASSERT(x)	\
{									\
    if (!(x)) {								\
	fprintf(stderr, "Assertion " #x " failed, %s, line %d\n",	\
		__FILE__, __LINE__);					\
	other_error_exit("assertion failed");				\
    }									\
}

extern task_t		task;
extern mach_port_t	master_device_port, host_priv_port;
extern memory_object_t	memory_object;
extern memory_object_control_t	memory_object_control;
extern vm_size_t	memory_object_page_size;
extern boolean_t	mac32bit;
extern int		file;
extern struct exec	filehdr;
