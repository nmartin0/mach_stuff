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
 * $Log:	bsd_types.h,v $
 * Revision 2.4  92/02/02  13:02:18  rpd
 * 	Removed <sys/param.h>; use defines from <bsd_types_gen.h> instead.
 * 	[92/01/31            rpd]
 * 
 * Revision 2.3  91/12/19  20:27:55  mrt
 * 	Updated to new copyright
 * 
 * Revision 2.2  90/09/08  00:14:41  rwd
 * 	Taken from XUX22.
 * 	[90/09/08            rwd]
 * 
 */
/*
 *	File:	./bsd_types.h
 *	Author:	Joseph S. Barrera III, Randall W. Dean
 *
 *	Copyright (c) 1990 Joseph S. Barrera III, Randall W. Dean
 */


#ifndef	_UXKERN_BSD_TYPES_H_
#define	_UXKERN_BSD_TYPES_H_

/*
 * Types for BSD kernel interface.
 */

#include <sys/types.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/signal.h>

#include <bsd_types_gen.h>

typedef	char		*char_array;
typedef char		small_char_array[SMALL_ARRAY_LIMIT];
typedef	char		path_name_t[PATH_LENGTH];
typedef struct timeval	timeval_t;
typedef	struct timeval	timeval_2_t[2];
typedef	struct timeval	timeval_3_t[3];
typedef
struct statb_t {
	int	s_dev;
	int	s_ino;
	int	s_mode;
	int	s_nlink;
	int	s_uid;
	int	s_gid;
	int	s_rdev;
	int	s_size;
	int	s_atime;
	int	s_mtime;
	int	s_ctime;
	int	s_blksize;
	int	s_blocks;
} statb_t;

typedef	struct rusage	rusage_t;
typedef	char		sockarg_t[128];
typedef	int		entry_array[16];
typedef	int		gidset_t[GROUPS_LIMIT];
typedef	struct rlimit	rlimit_t[1];
typedef	struct sigvec	sigvec_t;
typedef	struct sigstack	sigstack_t;
typedef struct timezone	timezone_t;
typedef	struct itimerval itimerval_t;
typedef	char		hostname_t[HOST_NAME_LIMIT];
typedef	char		cfname_t[64];

#endif	_UXKERN_BSD_TYPES_H_
