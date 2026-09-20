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
 * $Log:	bsd_ioctl.h,v $
 * Revision 2.3  91/12/19  20:27:22  mrt
 * 	Updated to new copyright
 * 
 * Revision 2.2  90/09/08  00:07:22  rwd
 * 	First checkin
 * 	[90/08/31  13:31:40  rwd]
 * 
 */
/*
 *	File:	bsd_ioctl.h
 *	Author:	Joseph S. Barrera III, Randall W. Dean
 *
 *	Copyright (c) 1990 Joseph S. Barrera III, Randall W. Dean
 */

/*
 * Get command byte from an ioctl definition (eg icmd(TIOCGETP) = 8))
 */
#define	icmd(xxx)	((xxx) & 0xff)

/*
 * Copy server structure in or out.
 * Should be invoked inside xxx_ioctl fnode op with
 * first argument declared as rw and
 * last two arguments declared as param and psize.
 */
#define	PCOPY(server)	pcopy(rw, server, param, psize)
