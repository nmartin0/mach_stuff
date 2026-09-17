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
 * CARNEGIE MELLON ALLOWS FREE USE OF THIS SOFTWARE IN ITS 
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
 * any improvements or extensions that they make and grant Carnegie the
 * rights to redistribute these changes.
 */
/*
 * HISTORY
 * $Log: swapon.c,v $
 * Revision 1.1.1.1  1995/05/04  06:57:06  sclawson
 * New files.
 *
 * Revision 2.3  93/04/14  11:45:05  mrt
 * 	default_pager_paging_file has changed.
 * 	[92/03/08            af]
 * 
 * Revision 2.1.1.1  92/02/19  18:32:20  af
 * 	Created.
 * 	[92/02/17            af]
 * 
 */

#include <stdio.h>
#include <strings.h>
#include <mach.h>
#include <mach_error.h>
#include <mach/default_pager_types.h>

#define streql(a, b)	(strcmp((a), (b)) == 0)

static int ask_default_pager();


static void
usage()
{
    quit(1, "usage: swapon [-v] [-add|-remove] filename\n");
}

static boolean_t Verbose = FALSE;

main(argc, argv)
    int argc;
    char *argv[];
{
    boolean_t       add_file = TRUE;
    char           *file_name;
    int             i;

    for (i = 1; i < argc; i++)
	if (streql(argv[i], "-v"))
	    Verbose = TRUE;
	else if (streql(argv[i], "-add"))
	    add_file = TRUE;
	else if (streql(argv[i], "-remove"))
	    add_file = FALSE;
	else if (streql(argv[i], "--")) {
	    i++;
	    break;
	} else if (argv[i][0] == '-')
	    usage();
	else
	    break;

    argv += i;
    argc -= i;

    if (argc < 1)
	usage();

    /* xxx maybe do path searching xxx */
    file_name = *argv;

    exit(ask_default_pager(add_file, file_name));

}


static int
ask_default_pager(op, file_name)
    boolean_t	op;
    default_pager_filename_t	file_name;
{
    kern_return_t   kr;
    mach_port_t     auth_port;
    mach_port_t     default_pager_port;

    auth_port = mach_host_priv_self();
    if (auth_port == MACH_PORT_NULL) {
	if (Verbose)
	    printf("Could not get privileged host port\n");
	auth_port = mach_task_self();
    }
    default_pager_port = MACH_PORT_NULL;
    kr = vm_set_default_memory_manager(auth_port, &default_pager_port);
    if (Verbose && kr != KERN_SUCCESS)
	printf("Could not get default pager port: %s\n", mach_error_string(kr));

    if (kr == KERN_SUCCESS) {
	kr = default_pager_paging_file(default_pager_port, MACH_PORT_NULL, file_name, op);
	if (Verbose && kr != KERN_SUCCESS)
	    printf("Operation refused: %s\n", mach_error_string(kr));
    }
    return (kr == KERN_SUCCESS) ? 0 : 1;
}

