/* 
 * Mach Operating System
 * Copyright (c) 1993-1987 Carnegie Mellon University
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
 * $Log:	time_out.h,v $
 * Revision 2.6  93/11/17  17:32:10  dbg
 * 	Moved time_out functions to kern/mach_timer.h.  This file
 * 	now contains only the compatibility calls for old device
 * 	drivers.
 * 	[93/04/09            dbg]
 * 
 * Revision 2.5  91/07/31  17:51:15  dbg
 * 	Fix race condition.
 * 	[91/07/30  17:08:00  dbg]
 * 
 * Revision 2.4  91/05/14  16:49:27  mrt
 * 	Correcting copyright
 * 
 * Revision 2.3  91/02/05  17:30:51  mrt
 * 	Changed to new Mach copyright
 * 	[91/02/01  16:20:31  mrt]
 * 
 * Revision 2.2  90/11/05  14:32:00  rpd
 * 	Changed untimeout to return boolean.
 * 	[90/10/29            rpd]
 * 
 * Revision 2.1  89/08/03  15:57:24  rwd
 * Created.
 * 
 * 14-Jun-88  David Golub (dbg) at Carnegie-Mellon University
 *	Created.
 *
 */

#ifndef	_KERN_TIME_OUT_H_
#define	_KERN_TIME_OUT_H_

#include <mach/boolean.h>

/*
 * Anonymous timer requests for device drivers.
 */

extern int		hz;		/* number of 'ticks' per second -
					   compatibility */

/*
 *	Set a timeout.  Partial prototype for 'fcn' is
 *	for the benefit of old device drivers.
 */
extern void		timeout(
	void		(*fcn)(/* void * */),
	void *		param,
	int		interval);	/* in milliseconds */

/*
 *	Remove a timeout, returning whether it was still set.
 */
extern boolean_t	untimeout(
	void		(*fcn)(/* void * */),
	void *		param);

#endif	/* _KERN_TIME_OUT_H_ */
