/* 
 * Mach Operating System
 * Copyright (c) 1993,1991,1990 Carnegie Mellon University
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
 * $Log:	db_task_thread.h,v $
 * Revision 2.3  93/11/17  16:25:51  dbg
 * 	Added ANSI function prototypes.
 * 	[93/10/07            dbg]
 * 
 * Revision 2.2  91/10/09  16:03:18  af
 * 	Created for task/thread handling.
 * 	[91/08/29            tak]
 * 
 */

#ifndef _DDB_DB_TASK_THREAD_H_
#define _DDB_DB_TASK_THREAD_H_

#include <kern/task.h>
#include <kern/thread.h>
#include <ddb/db_variables.h>


#define db_current_task()						\
		((current_thread())? current_thread()->task: TASK_NULL)
#define db_target_space(thread, user_space)				\
		((!(user_space))? TASK_NULL:				\
		(thread)? (thread)->task: db_current_task())
#define db_is_current_task(task) 					\
		((task) == TASK_NULL || (task) == db_current_task())

extern task_t	db_default_task;		/* default target task */
extern thread_t	db_default_thread;		/* default target thread */

extern int
db_lookup_task(
	task_t		target_task);

extern int
db_lookup_thread(
	thread_t	target_thread);

extern int
db_lookup_task_thread(
	task_t		task,
	thread_t	target_thread);

extern boolean_t
db_check_thread_address_valid(
	db_addr_t	address);

extern boolean_t
db_get_next_thread(
	thread_t	*threadp,
	int		position);

extern void
db_init_default_thread(void);

extern void
db_set_default_thread(
	struct db_variable	*vp,
	db_expr_t		*valuep,
	int			rw_flag,
	db_var_aux_param_t	ap);

extern void
db_get_task_thread(
	struct db_variable	*vp,
	db_expr_t		*valuep,
	int			rw_flag,
	db_var_aux_param_t	ap);

#endif	/* _DDB_DB_TASK_THREAD_H_ */
