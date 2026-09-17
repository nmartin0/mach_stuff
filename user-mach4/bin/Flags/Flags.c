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
 * $Log: Flags.c,v $
 * Revision 1.1.1.1  1995/05/04  06:57:06  sclawson
 * New files.
 *
 * Revision 2.2  93/04/14  11:25:05  mrt
 * 	Changed from I_flags to take a flag name on the argument line so
 * 	that it can be used for loaded as well as includes. Also added a
 * 	usage message.
 * 	[92/11/30            jeffreyh]
 * 
 * 	Created.
 * 	[92/06/02            af]
 * 
 */
/*
 * Tired of too many disgusting hackeries I wrote a program.
 */
#include <strings.h>

main(argc,argv)
	char	**argv;
{
	char	*path, *getenv(const char *), *flag_name;

	if (argc < 3){
		printf("Usage:%s FLAG_NAME(I or L) PATH1 PATH2 ... \n",*argv);
		exit (-1);
	}
	argc--, argv++;
	flag_name = *argv;
	while (argc > 1) {
		argc--, argv++;

		path = getenv(*argv);
		if (path == 0) continue;

		while (*path) {
			char	*colon = index(path, ':');
			char	*next;

			/* skip leading separators */
			if (colon == path) {
				path++;
				continue;
			}

			/* where does it terminate, and who is next */
			if (colon) {
				next = colon + 1;
				*colon = 0;
			} else
				next = path + strlen(path);

			/* print it now */
			printf("-%s%s ", flag_name, path);

			/* and we are done */
			path = next;
		}

	}
	printf("\n");
	return 0;
}

