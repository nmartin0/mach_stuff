/* 
 * Mach Operating System
 * Copyright (c) 1993,1991 Carnegie Mellon University
 * All Rights Reserved.
 * 
 * Permission to use, copy, modify and distribute this software and its
 * documentation is hereby granted, provided that both the copyright
 * notice and this permission notice appear in all copies of the
 * software, derivative works or modified versions, and any portions
 * thereof, and that both notices appear in supporting documentation.
 * 
 * CARNEGIE MELLON ALLOWS FREE USE OF THIS SOFTWARE IN ITS "AS IS"
 * CONDITION.  CARNEGIE MELLON DISCLAIMS ANY LIABILITY OF ANY KIND FOR
 * ANY DAMAGES WHATSOEVER RESULTING FROM THE USE OF THIS SOFTWARE.
 * 
 * Carnegie Mellon requests users of this software to return to
 * 
 *  Software Distribution Coordinator  or  Software.Distribution@CS.CMU.EDU
 *  School of Computer Science
 *  Carnegie Mellon University
 *  Pittsburgh PA 15213-3890
 * 
 * any improvements or extensions that they make and grant Carnegie Mellon 
 * the rights to redistribute these changes.
 */

/*
 * HISTORY
 * $Log:	db_run.h,v $
 * Revision 2.4  93/11/17  16:24:52  dbg
 * 	Added function prototypes for db_run.c.
 * 	[93/10/11            dbg]
 * 
 * Revision 2.3  91/07/11  11:00:35  danner
 * 	Copyright Fixes
 * 
 * Revision 2.2  91/07/09  23:16:05  danner
 * 	Broken out of db_run.c because I needed access to the step mode in the 
 * 	machine dependent luna88k debugger code.
 * 	[91/07/09  00:47:31  danner]
 * 
 * Revision 2.2  91/04/11  00:13:39  mbj
 * 	Added to support luna debugger interface. Broken out of db_run.c
 * 	[91/03/18            danner]
 * 
 */

#ifndef	_DDB_DB_RUN_H_
#define	_DDB_DB_RUN_H_

#include <mach/boolean.h>
#include <kern/kern_types.h>
#include <machine/db_machdep.h>

extern int db_run_mode;

/* modes the system may be running in */

#define	STEP_NONE	0
#define	STEP_ONCE	1
#define	STEP_RETURN	2
#define	STEP_CALLT	3
#define	STEP_CONTINUE	4
#define STEP_INVISIBLE	5
#define	STEP_COUNT	6

extern boolean_t
db_stop_at_pc(
	boolean_t *is_breakpoint,
	task_t	  task);

extern void
db_restart_at_pc(
	boolean_t watchpt,
	task_t	  task);

extern void
db_single_step(
	db_regs_t *regs,
	task_t	  task);

extern boolean_t
db_in_single_step(void);

extern void
db_single_step_cmd(
	db_expr_t	addr,
	int		have_addr,
	db_expr_t	count,
	char *		modif);

extern void
db_trace_until_call_cmd(
	db_expr_t	addr,
	int		have_addr,
	db_expr_t	count,
	char *		modif);

extern void
db_trace_until_matching_cmd(
	db_expr_t	addr,
	int		have_addr,
	db_expr_t	count,
	char *		modif);

extern void
db_continue_cmd(
	db_expr_t	addr,
	int		have_addr,
	db_expr_t	count,
	char *		modif);

#endif	/* _DDB_DB_RUN_H_ */

