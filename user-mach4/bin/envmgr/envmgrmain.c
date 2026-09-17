/*
 * Mach Operating System
 * Copyright (c) 1991,1990,1989,1988,1987 Carnegie Mellon University
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
 * $Log: envmgrmain.c,v $
 * Revision 1.1.1.1  1995/05/04  06:56:47  sclawson
 * New files.
 *
 * Revision 2.2  92/04/01  19:09:19  rpd
 * 	Created.
 * 	[92/02/21            jtp]
 * 
 *
 */
/*
 *  File: envmgrmain.c
 *      Environment Manager startup
 *
 *  Author: Jukka Partanen, Helsinki University of Technology 1992
 *      based on the Mach 2.5 version by Mary Thompson
 *
 */

#include <stdio.h>

#include <mach.h>
#include <mach/mach_types.h>
#include <mach/message.h>
#include <mach/notify.h>
#include <servers/netname.h>
#include <servers/emdefs.h>

#define USAGE(s)       printf("Usage: %s [-d] [-N hashtablesize]\n", (s))
#define PORT_HASH_TABLE_SIZE	500

extern void 		initial_conn();
extern void		env_init();
extern boolean_t	env_mgr_server();
extern boolean_t	notify_server();

static boolean_t	env_mgr_demux();

mach_port_t		service;	/* our service port */
mach_port_t		notify;		/* notification port for dead names */
mach_port_t		pset;		/* port set we are listening to */

int	debug = 0;
char	*progname;

main(argc, argv)
    int argc;
    char **argv;
{
    kern_return_t kr;
    int c, hashtablesize;
    extern int optind, opterr;
    extern char *optarg, *rindex();
    
    progname = rindex(argv[0], '/');
    if (progname == NULL)
	progname = argv[0];
    else
	progname++;

    hashtablesize = PORT_HASH_TABLE_SIZE;
    opterr = 0;
    while ((c = getopt(argc, argv, "dN:")) != EOF)
	switch(c) {
	case 'N':
	    hashtablesize = atoi(optarg);
	    if (hashtablesize < 1) {
		fprintf(stderr,
"%s: hash table cannot be smaller than 1, using the default value\n",
			progname);
		hashtablesize = PORT_HASH_TABLE_SIZE;
	    }
	    break;
	case 'd':
	    debug = 1;
	    break;
	default:
	    USAGE(progname);
	    exit(-1);
	}
    
    if (argc - optind) {
	USAGE(progname);
	exit(-1);
    }
    
    env_init(&service, hashtablesize);
    
    if (debug)
	fprintf(stderr, "%s: initialized, service port is 0x%x\n",
		progname, service);

    if (service == MACH_PORT_NULL)
	quit(1,
	     "%s: invalid service port (maybe an envmgr is already running).\n",
	     progname);

    kr = netname_check_in(name_server_port, "EnvMgrPort",
			  mach_task_self(), service);
    if (kr != KERN_SUCCESS)
	fprintf(stderr,
		"%s: Warning: netname_check_in(EnvMgrPort, 0x%x, 0x%x): %s\n",
		progname, mach_task_self(), service, mach_error_string(kr));
    
    kr = mach_port_allocate(mach_task_self(),
			    MACH_PORT_RIGHT_PORT_SET, &pset);
    if (kr != KERN_SUCCESS)
	quit(1, "%s: mach_port_allocate(pset): %s\n",
	     progname, mach_error_string(kr));
    
    kr = mach_port_allocate(mach_task_self(),
			    MACH_PORT_RIGHT_RECEIVE, &notify);
    if (kr != KERN_SUCCESS)
	quit(1, "%s: mach_port_allocate(notify): %s\n", progname);
    
    kr = mach_port_move_member(mach_task_self(), notify, pset);
    if (kr != KERN_SUCCESS)
	quit(1, "%s: mach_port_move_member(notify, pset): %s\n",
	     progname, mach_error_string(kr));

    kr = mach_port_move_member(mach_task_self(), service, pset);
    if (kr != KERN_SUCCESS)
	quit(1, "%s: mach_port_move_member(service, pset): %s\n",
	     progname, mach_error_string(kr));
    
    kr = mach_msg_server(env_mgr_demux,
			 round_page(env_name_size + env_val_size),
			 pset);
    fprintf(stderr, "%s: server loop exited: %s\n",
	    progname, mach_error_string(kr));
    exit(1);
}

static boolean_t env_mgr_demux(request, reply)
    mach_msg_header_t *request, *reply;
{
    if (MACH_MSGH_BITS_LOCAL(request->msgh_bits) ==
	MACH_MSG_TYPE_MOVE_SEND_ONCE) {
	if (debug) {
	    fprintf(stderr, 
	    "%s: notification: msgh_local_port 0x%x msgh_remote_port 0x%x\n",
		    progname,
		    request->msgh_local_port,
		    request->msgh_remote_port);
	}
	return notify_server(request, reply);
    }
    return env_mgr_server(request, reply);
}
