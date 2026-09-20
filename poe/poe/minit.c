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
 * $Log:	minit.c,v $
 * Revision 2.4  94/03/25  18:23:05  mrt
 * 	Fixed up the use of the input args.
 * 	[94/03/15            mrt]
 * 
 * Revision 2.3  91/12/19  20:28:41  mrt
 * 	Updated to new copyright
 * 
 * Revision 2.2  90/09/08  00:19:31  rwd
 * 	Fix STANDALONE logic.
 * 	[90/09/04            rwd]
 * 	First Checkin
 * 	[90/09/03  17:44:29  rwd]
 * 
 */
/*
 *	File:	./minit.c
 *	Author:	Joseph S. Barrera III, Randall W. Dean
 *
 *	Copyright (c) 1990 Joseph S. Barrera III, Randall W. Dean
 */

#include <config.h>

extern char *rindex();

#if STANDALONE
#define TTY "/dev/console"
#else STANDALONE
#define TTY "/dev/tty"
#endif STANDALONE
char *cshargv[] = {
	"-csh",
	0,
};
char *cshenvp[] = {
	"HOME=/mach_servers",
	"SHELL=/bin/csh",
	"TERM=unknown",
	"USER=root",
	"PATH=/usr/cs/bin:/usr/ucb:/bin:/usr/bin:/etc:/mach_servers",
	"CPATH=:/usr/cs/include:/usr/include",
	"LPATH=:/usr/cs/lib:/lib:/usr/lib",
	"MPATH=:/usr/cs/man:/usr/man",
	"EPATH=:/usr/cs/maclib",
	0,
};
 

main(argc, argv)
int argc;
char *argv[];
{
/*	argv[0] is program name
 *	argv[1] is boot flags
 *	argv[2] is root partition name
 */
	int fd, nfds, error,len;
	char *cp;
	char init_dir [256];
	char init_rc [256];
	char run_rc [256];

	chdir("/");
	nfds = getdtablesize();
	if (nfds == -1) {
		perror("getdtablesize");
		exit(1);
	}
	for (fd = 0; fd < nfds; fd++) {
		close(fd);
	}
	error = open(TTY, 2);
	if (error == -1) {
		perror("open TTY");
		exit(1);
	}
	error = dup(0);
	if (error == -1) {
		perror("dup 0");
		exit(1);
	}
	error = dup(1);
	if (error == -1) {
		perror("dup 1");
		exit(1);
	}
	if (argc > 2)
		printf("poe_init: Root Device %s\n",argv[2]);

	/* get name of the directory in which this program was found */
	cp = rindex(argv[0],'/');
	len = (int)cp - (int)argv[0];
	strncpy(init_dir,argv[0],len); 
	init_dir[len] = '\0';
	printf("poe_init: home directory is %s\n", init_dir);

#if	STANDALONE
	if ( index(argv[1],'s') == 0 ){
	    strcpy(init_rc,init_dir);
	    strcat(init_rc,"/rc");
	    sprintf(run_rc,"bin/csh -f < %s", init_rc);
	    printf("poe_init: executing %s\n",init_rc);
	    system(run_rc);
	}
#endif	STANDALONE

	system("/etc/mount -a");
	chdir(init_dir);
	execve("/bin/csh", cshargv, cshenvp);
	perror("/bin/csh");
	exit(1);
}
