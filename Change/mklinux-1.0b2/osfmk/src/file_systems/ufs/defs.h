/*
 * @OSF_COPYRIGHT@
 */
/*
 * HISTORY
 * $Log: defs.h,v $
 * Revision 1.1.4.1  1996/02/27  16:20:57  barbou
 * 	Merged MkLinux changes.
 * 	[1996/02/27  16:12:16  barbou]
 *
 * Revision 1.1.2.1  1995/12/28  16:19:07  barbou
 * 	Self-Contained Mach Distribution:
 * 	created.
 * 	[95/12/28            barbou]
 * 
 * $EndLog$
 */
/* 
 * Mach Operating System
 * Copyright (c) 1991,1990,1989,1988 Carnegie Mellon University
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

#include <sys/types.h>

/* from sys/syslimits.h */
#define	NAME_MAX	255	/* max number of bytes in a file name */
#define	PATH_MAX	1023	/* max number of bytes in pathname */

/* from sys/dirent.h */
/*
 * The POSIX standard way of returning directory entries is in directory entry
 * structures, which are of variable length.  Each directory entry has
 * a struct direct at the front of it, containing its inode number,
 * the length of the entry, and the length of the name contained in
 * the entry.  These are followed by the name padded to a 4 byte boundary
 * with null bytes.  All names are guaranteed null terminated.
 * The maximum length of a name in a directory is returned by
 * pathconf(directory_pathname, _PC_NAME_MAX).  The type struct dirent is
 * a template for the beginning of the actual structure, since the
 * length is variable, and so should only be used to declare
 * struct dirent * pointer types.
 */

struct  dirent {
        ino_t    d_ino;               /* file number of entry */
        ushort_t d_reclen;            /* length of this record */
#ifndef ykpark
        uchar_t d_type;            /* file type */
        uchar_t d_namlen;            /* length of string in d_name */
#else /* ykpark */
        ushort_t d_namlen;            /* length of string in d_name */
#endif /* ykpark */
        char    d_name[256];		/* DUMMY NAME LENGTH */
					/* the real maximum length is */
					/* returned by pathconf() */
					/* At this time, this MUST */
					/* be 256 -- the kernel */
					/* requires it */
};
