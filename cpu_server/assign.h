/*
 * Mach Operating System
 * Copyright (c) 1991,1990 Carnegie Mellon University
 * All Rights Reserved.
 * 
 * Permission to use, copy, modify and distribute this software and its
 * documentation is hereby granted, provided that both the copyright
 * notice and this permission notice appear in all copies of the
 * software, derivative works or modified versions, and any portions
 * thereof, and that both notices appear in supporting documentation.
 * 
 * CARNEGIE MELLON ALLOWS FREE USE OF THIS SOFTWARE IN ITS 
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
 * any improvements or extensions that they make and grant Carnegie the
 * rights to redistribute these changes.
 */
/*
 * HISTORY
 * $Log:	assign.h,v $
 * Revision 2.3  91/02/14  14:32:45  mrt
 * 	Changed to new Mach copyright
 * 
 * Revision 2.2  90/01/24  23:33:30  mrt
 * 	Created
 * 	[90/01/18            dlb]
 * 
 */
/*
 *	File: 	assign.h
 *	Author:	David Black, Carnegie Mellon University
 *	Date:	Jan 1990
 *
 *	assign.h - Header file for processor assignment management.
 */

processor_set_t		default_set;

processor_t		*processors;
int			processor_count;

struct run_processor {
    	struct run_processor	*next;
	processor_t		this_processor;
	int			slot_num;
	int			status;		/* FREE or BUSY */
};

typedef	struct run_processor	run_processor_data_t;
typedef struct run_processor	*run_processor_t;

#define	FREE	0
#define	BUSY	1

#define RP_NULL	(run_processor_t)0

#define	MIN_PROCESSORS			0
#define MIN_TIME			0

run_processor_t		free_processors;

int	max_time;
int	max_total_time;
int	max_processors;
int	available_processors;

