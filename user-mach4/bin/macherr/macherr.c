/* 
 * Mach Operating System
 * Copyright (c) 1992,1989,1988,1987 Carnegie Mellon University
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
 * $Log: macherr.c,v $
 * Revision 1.1.1.1  1995/05/04  06:56:50  sclawson
 * New files.
 *
 * Revision 2.2  92/04/01  19:09:45  rpd
 * 	Moved to the user collection.
 * 	[92/03/09            rpd]
 * 
 * Revision 1.3  89/05/05  18:22:26  mrt
 * 	Cleanup for Mach 2.5
 * 
 *
 *  11-May-88  Douglas Orr (dorr) at Carnegie Mellon
 *	Added ability to interpret hex error codes
 *
 *  25-Mar-87  Mary Thompson @ Carnegie Mellon
 *	Started
 *******************************************************/
/*
 * macherr.c
 *
 *  Usage: macherr error_number
 *	prints out a message for the given mach error code
 */

#include <stdio.h>
#include <mach_error.h>

#define true	(1)
#define false	(0)

main(argc,argv)
  int 	argc;
  char  * * argv;
{
	mach_error_t	err;

	if (argc == 1 ) {
		printf("Usage is %s [x]errnum ...\n", *argv);
		exit (1);
	}
	while( --argc ) {
		++argv;
		if( (*argv)[0] == 'x' ) {
		    if( !atox( *argv, &err ) )
			continue;
		}
		else
			err = atoi( *argv );
		printf("%s\n",mach_error_string(err));
	}
}


atox( a, val )
	char	* a;
	int	* val;
{
	char	* oa = a;
	int	x;

	x = 0;
	a++;
	while( *a ) {
		if ( (*a >= '0') && (*a <= '9') )
			x = (x << 4) | (*a - '0');
		else
		if ( (*a >= 'a') && (*a <= 'f') )
			x = (x << 4) | (*a - 'a' + 10);
		else
		if ( (*a >= 'A') && (*a <= 'F') )
			x = (x << 4) | (*a - 'A' + 10);
		else {
			fprintf( stderr, "invalid hex number %s\n", oa );
			return( false );
		}
		++a;
	}

	*val = x;
	return( true );
}
