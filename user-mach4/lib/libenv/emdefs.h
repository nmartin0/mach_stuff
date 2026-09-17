/*
 * Mach Operating System
 * Copyright (c) 1991,1990,1989,1988,1987 Carnegie Mellon University
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
 * $Log: emdefs.h,v $
 * Revision 1.2  1995/05/04  07:07:23  sclawson
 * changed #include from <errorlib.h> to <mach/errorlib.h>.
 *
 * Revision 2.2  92/04/01  19:10:01  rpd
 * 	Added one to all error codes in order to make them agree with libmach.
 * 	[92/02/21            jtp]
 * 
 * 	Created.
 * 	[92/02/21            jtp]
 * 
 */
/*  
 *  File: emdefs
 *	Exported definitions for Environment Manager
 *
 *  Author: Jukka Partanen, Helsinki University of Technology 1992
 *      based on the Mach 2.5 version by Mary Thompson
 *
 */

#ifndef _env_mgr_defs
#define _env_mgr_defs

#include <mach/errorlib.h>

#define env_name_size		(80)
#define env_val_size		(2048)

#define ENV_SUCCESS		(KERN_SUCCESS)

#define ENV_VAR_NOT_FOUND	(SERV_ENV_MOD | 1)
#define ENV_WRONG_VAR_TYPE	(SERV_ENV_MOD | 2)
#define ENV_UNKNOWN_PORT	(SERV_ENV_MOD | 3)
#define ENV_READ_ONLY		(SERV_ENV_MOD | 4)
#define ENV_NO_MORE_CONN	(SERV_ENV_MOD | 5)
#define ENV_PORT_TABLE_FULL	(SERV_ENV_MOD | 6)
#define ENV_PORT_NULL		(SERV_ENV_MOD | 7)

typedef char	env_name_t[env_name_size];

typedef char	env_str_val_t[env_val_size];

typedef char	*env_str_t;

typedef env_name_t *env_name_list; 	/* Variable sized array */

typedef env_str_val_t *env_str_list; 	/* Variable sized array */

#endif _env_mgr_defs
