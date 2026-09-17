/*
 * Copyright (c) 1992 Carnegie Mellon University
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
 *  Software Distribution Coordinator  or  Software_Distribution@CS.CMU.EDU
 *  School of Computer Science
 *  Carnegie Mellon University
 *  Pittsburgh PA 15213-3890
 *
 * any improvements or extensions that they make and grant Carnegie Mellon
 * the rights to redistribute these changes.
 */
/*
 *  HISTORY
 *  $Log: mkmajorvers.c,v $
 * Revision 1.2  1995/06/30  19:43:26  sclawson
 * Various cleanups for building on the x86.
 *
 * Revision 1.1.1.1  1995/05/04  06:56:39  sclawson
 * New files.
 *
 * Revision 2.2  92/11/13  17:17:03  mrt
 * 	Copied here from Mach 2.5 libsys.a
 * 
 * 
 * Revision 2.3  92/11/09  16:38:05  mrt
 * 	Save and restore any existing signal handler.
 * 	[92/11/06            mrt]
 * 
 * Revision 2.2  92/07/23  14:01:25  mrt
 * 	Used to tell if we are running Mach 2.5 or Mach 3.0
 * 	[92/07/23  13:34:02  mrt]
 * 
 */

/* 
 * At this point we can only assume that we're running mach 3.0. =)
 * This is only kept for those old stupid programs that need it!
 */

int
mk_major_version()
{

  return(3);

}
