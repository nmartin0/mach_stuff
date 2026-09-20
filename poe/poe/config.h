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
 * CARNEGIE MELLON ALLOWS FREE USE OF THIS SOFTWARE IN ITS "AS IS"
 * CONDITION.  CARNEGIE MELLON DISCLAIMS ANY LIABILITY OF ANY KIND FOR
 * ANY DAMAGES WHATSOEVER RESULTING FROM THE USE OF THIS SOFTWARE.
 * 
 * Carnegie Mellon requests users of this software to return to
 * 
 *  Software Distribution Coordinator   or   Software.Distribution@CS.CMU.EDU
 *  School of Computer Science
 *  Carnegie Mellon University
 *  Pittsburgh PA 15213-3890
 * 
 * any improvements or extensions that they made and grant Carnegie Mellon
 * the rights to redistribute these changes.
 */
/*
 * HISTORY
 * $Log:	config.h,v $
 * Revision 2.4  94/03/25  18:22:45  mrt
 * 	Allow STANDALONE to be a compilation flag.
 * 	[94/02/18            mrt]
 * 
 * Revision 2.3  91/12/19  20:28:00  mrt
 * 	Updated to new copyright
 * 
 * Revision 2.2  90/09/08  00:18:47  rwd
 * 	Create.  Define standalone parameter.
 * 	[90/07/13            rwd]
 * 
 */
/*
 *	File:	./config.h
 *	Author:	Joseph S. Barrera III, Randall W. Dean
 *
 *	Copyright (c) 1990 Joseph S. Barrera III, Randall W. Dean
 */
/*
 * STANDALONE = 1 means poe is running as the first mach task. 
 *	      = 0 means it is being run as a second server by
 *		  some other server e.g. UX. This is normally done
 *		  only for debugging.
 */

#ifndef STANDALONE
#define STANDALONE	1	/* This is the first mach task */
#endif
