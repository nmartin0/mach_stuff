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
 * $Log:	subr_uerror.c,v $
 * Revision 2.3  91/12/19  20:29:17  mrt
 * 	Updated to new copyright
 * 
 * Revision 2.2  90/09/08  00:20:30  rwd
 * 	First checkin
 * 	[90/08/31  13:56:37  rwd]
 * 
 */
/*
 *	File:	./subr_uerror.c
 *	Author:	Joseph S. Barrera III, Randall W. Dean
 *
 *	Copyright (c) 1990 Joseph S. Barrera III, Randall W. Dean
 */

char *unix_errors[] = {
	"Error 0",				/*  0 */
	"Not owner",
	"No such file or directory",
	"No such process",
	"Interrupted system call",
	"I/O error",				/*  5 */
	"No such device or address",
	"Arg list too long",
	"Exec format error",
	"Bad file number",
	"No children",				/* 10 */
	"No more processes",
	"Not enough memory",
	"Permission denied",
	"Bad address",
	"Block device required",		/* 15 */
	"File or device busy",
	"File exists",
	"Cross-device link",
	"No such device",
	"Not a directory",			/* 20 */
	"Is a directory",
	"Invalid argument",
	"File table overflow",
	"Too many open files",
	"Inappropriate ioctl for device",	/* 25 */
	"Text file busy",
	"File too large",
	"No space left on device",
	"Illegal seek",
	"Read-only file system",		/* 30 */
	"Too many links",
	"Broken pipe",
	"Argument too large",
	"Result too large",
	"Operation would block",		/* 35 */
	"Operation now in progress",
	"Operation already in progress",
	"Socket operation on non-socket",
	"Destination address required",
	"Message too long",			/* 40 */
	"Protocol wrong type for socket",
	"Option not supported by protocol",
	"Protocol not supported",
	"Socket type not supported",
	"Operation not supported on socket",	/* 45 */
	"Protocol family not supported",
	"Address family not supported by protocol family",
	"Address already in use",
	"Can't assign requested address",
	"Network is down",			/* 50 */
	"Network is unreachable",
	"Network dropped connection on reset",
	"Software caused connection abort",
	"Connection reset by peer",
	"No buffer space available",		/* 55 */
	"Socket is already connected",
	"Socket is not connected",
	"Can't send after socket shutdown",
	"Too many references: can't splice",
	"Connection timed out",			/* 60 */
	"Connection refused",
	"Too many levels of symbolic links",
	"File name too long",
	"Host is down",
	"Host is unreachable",			/* 65 */
	"Directory not empty",
	"Too many processes",
	"Too many users",
	"Disc quota exceeded",
};

unix_error(s, error)
	char *s;
	unsigned int error;
{
	if (error < sizeof(unix_errors) / sizeof(*unix_errors)) {
		printf("%s: %s\n", s, unix_errors[error]);
	} else {
		printf("%s: Unknown error\n", s);
	}
}
