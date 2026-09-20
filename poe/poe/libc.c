/* 
 * Mach Operating System
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
 * $Log:	libc.c,v $
 * Revision 2.2  92/02/02  13:08:51  rpd
 * 	Created.
 * 	[92/01/31            rpd]
 * 
 */
/*
 *	Functions which should be in a stand-alone libc of some kind,
 *	but aren't yet.
 */

#include <mach/kern_return.h>

extern char *malloc();
extern void bzero();

char *
calloc(nelem, elsize)
    unsigned int nelem, elsize;
{
    unsigned int size = nelem * elsize;
    char *mem = malloc(size);
    if (mem != 0)
	bzero(mem, size);
    return mem;
}

char *
index(s, c)
    char *s, c;
{
    while (*s != c)
	if (*s++ == '\0')
	    return 0;
    return s;
}

/*
 *	This is in libmach_sa, but the version there isn't usable
 *	because it needs a full stdio (fprintf, _iob).
 */

void
mach_error(mess, code)
    char *mess;
    kern_return_t code;
{
    printf("poe: %s: %d\n", mess, code);
}
