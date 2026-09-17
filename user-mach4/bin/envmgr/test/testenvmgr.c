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
 * $Log: testenvmgr.c,v $
 * Revision 1.1.1.1  1995/05/04  06:56:48  sclawson
 * New files.
 *
 * Revision 2.2  92/04/01  19:09:41  rpd
 * 	Created.
 * 	[92/02/21            jtp]
 * 
 *
 */
/*
 *  File: testenvmgr.c
 *      A program for testing the Environment Manager
 *
 *  Author: Jukka Partanen, Helsinki University of Technology 1992
 */

#include <stdio.h>
#define MACH_INIT_SLOTS 1
#include <mach.h>
#include <mach_init.h>
#include <mach/message.h>
#include <mach_error.h>
#include <servers/env_mgr.h>
#include <sys/wait.h>
#include <strings.h>
#include "mach_getenv.h"

extern char **environ;
char *progname;

void set_string(), set_port(), list_strings(), list_ports(), 
    delete_string(), delete_port(), get_string(), get_port(), exec_shell();

void usage(exitcode)
    int exitcode;
{
    fprintf(stderr, "Usage: %s [-l ports|strings] [-s name=value] [-p name] [-g name] [-d name] [-i]\n",
	    progname);
    exit(exitcode);
}

char *env_error_string(retcode)
    kern_return_t retcode;
{
    switch(retcode)
    {
    case ENV_SUCCESS:
	break;
    case ENV_VAR_NOT_FOUND:
	return "(server/envmgr) environment variable not found";
	break;
    case ENV_WRONG_VAR_TYPE:
	return "(server/envmgr) environment variable is of wrong type";
	break;
    case ENV_UNKNOWN_PORT:
	return "(server/envmgr) unknown port";
	break;
    case ENV_READ_ONLY:
	return "(server/envmgr) environment is read-only";
	break;
    case ENV_NO_MORE_CONN:
	return "(server/envmgr) too many connections";
	break;
    case ENV_PORT_TABLE_FULL:
	return "(server/envmgr) environment is full";
	break;
    case ENV_PORT_NULL:
	return "(server/envmgr) invalid right (MACH_PORT_NULL)";
	break;
    default:
	return mach_error_string(retcode);
    }
}

char *unix_error_string(errno)
int errno;
{
    extern char *sys_errlist[];

    return sys_errlist[errno];
}

typedef enum { cmd_set, cmd_get, cmd_delete, cmd_list,
		   cmd_copy, cmd_quit } cmd_t;

struct cmds {
    char *name;
    cmd_t cmd;
} cmds[] = {
    "set", cmd_set,
    "get", cmd_get,
    "delete", cmd_delete,
    "list", cmd_list,
    "copy", cmd_copy,
    "quit", cmd_quit,
    NULL, -1 };

cmd_t decode_cmd(cmd)
char *cmd;
{
    struct cmds *c;

    for (c = cmds; c->name && strncmp(cmd, c->name, strlen(c->name)); c++)
	;
    return c->cmd;
}

void print_commands()
{
    struct cmds *c;

    printf("commands:\n");
    for (c = cmds; c->name; c++)
	printf("  %s\n", c->name);
}

main(argc, argv, envp)
    int argc;
    char **argv, **envp;
{
    int ch, interactive = 0;
    char *rindex();
    extern int optind, opterr;
    extern char *optarg;
    
    progname = rindex(argv[0], '/');
    if (progname == NULL)
	progname = argv[0];
    else
	progname++;
    
    opterr = 0;
    copy_environ_to_env(environ);
    while ((ch = getopt(argc, argv, "il:s:p:d:g:r:w:")) != EOF)
	switch(ch)
	{
	case 'i':
	    interactive++;
	    break;
	case 'l':
	    if (optarg[0] == 'p')
		list_ports();
	    else
		list_strings();
	    break;
	case 'p':
	    set_port(optarg);
	    break;
	case 'w':
	case 'r':
	    exec_shell(optarg, NULL, ch == 'r' ? 1 : 0);
	    break;
	case 's':
	    {
		int i;
		char *name, *value, *s;
		
		name = optarg;
		for (s = optarg; *s && *s != '='; s++);
		*s = 0;
		value = ++s;
		if (strlen(value) == 0)
		{
		    fprintf(stderr, "%s: value must be specified\n", progname);
		    usage(1);
		}
		set_string(name, value);
		break;
	    }
	case 'd':
	    delete_string(optarg);
	    break;
	case 'g':
	    get_string(optarg);
	    break;
	default:
	    usage(1);
	}
    if (argc - optind || argc == 1)
	usage(1);
    while (interactive) {
	char cmd[256], *type, *name, *arg;
	
	printf("cmd: ");
	fflush(stdout);
	if (fgets(cmd, sizeof(cmd), stdin) == NULL) {
	    interactive = 0;
	    continue;
	}
	cmd[strlen(cmd)-1] = 0;
	for (type = index(cmd, ' '); type && *type && *type == ' '; type++)
	    ;
	if (type)
	    for (name = index(type, ' '); name && *name && *name == ' '; name++)
		;
	else
	    name = NULL;
	if (name) {
	    if (arg = index(name, ' '))
		for (*arg++ = 0; *arg && *arg == ' '; arg++)
		    ;
	} else 
	    arg = NULL;

	switch (decode_cmd(cmd)) {
	case cmd_set:
	    if (!type || !*type || !name || !*name) {
	    set_error:
		printf("use: set [port|string] name [#portno in hex|value]\n");
		continue;
	    }
	    if (*type == 'p') {	/* set a port */
		if (!arg || !*arg) /* set a new port */
		    set_port(name);
		else {		/* set a named port (must exist) */
		    mach_port_t portno;
		    
		    sscanf(arg, "%x", (int *)&portno);
		    do_set_port(name, portno);
		}
	    } else {		/* set a string */
		if (!arg)
		    goto set_error;
		set_string(name, arg);
	    }
	    break;
	case cmd_get:
	    if (!type || !*type || !name || !*name) {
	    get_error:
		printf("use: get [port|string] name\n");
		continue;
	    }
	    if (*type == 'p')
		get_port(name);
	    else 
		get_string(name);
	    break;
	case cmd_delete:
	    if (!type || !*type || !name || !*name) {
	    delete_error:
		printf("use: delete [port|string] name\n");
		continue;
	    }
	    if (*type == 'p')
		delete_port(name);
	    else
		delete_string(name);
	    break;
	case cmd_list:
	    if (!type || !*type) {
	    list_error:
		printf("use: list [ports|strings]\n");
		continue;
	    }
	    if (*type == 'p')
		list_ports();
	    else
		list_strings();
	    break;
	case cmd_copy:
	    if (!type || !*type || !name || !*name) {
	    copy_error:
		printf("use: copy [r|w|n] prog [args ...]\n");
		continue;
	    }
	    exec_shell(name, arg, *type == 'w' ? 0 : *type == 'r' ? 1 : 2);
	    break;
	case cmd_quit:
	    interactive = 0;
	    break;
	default:
	    print_commands();
	}
    }
    exit(0);
}

void list_strings()
{
    env_name_list namelist;
    env_str_list valuelist;
    kern_return_t kr;
    int namecount, valuecount, i;
    
    kr = env_list_strings(environment_port, &namelist, &namecount, &valuelist,
			  &valuecount);
    
    if (kr != ENV_SUCCESS)
    {
	fprintf(stderr, "%s: env_list_strings: %s\n", progname, 
		env_error_string(kr));
	exit(1);
    }
    printf("Strings in environment:\n");
    for (i=0; i < namecount; i++)
	if (i < valuecount)
	    printf("%s = %s\n", namelist[i], valuelist[i]);
}

void list_ports()
{
    int namecount, valuecount;
    env_name_list namelist;
    mach_port_array_t p_array;
    kern_return_t kr;
    int i;
    
    kr = env_list_ports(environment_port, &namelist, &namecount, &p_array,
			&valuecount);
    
    if (kr != ENV_SUCCESS) {
	fprintf(stderr, "%s: env_list_ports: %s\n", progname, 
		env_error_string(kr));
	exit(1);
    }
    printf("Ports in environment:\n");
    for (i = 0; i < namecount; i++)
	if (i < valuecount) {
	    printf("%s = 0x%x\n", namelist[i], (int)p_array[i]);
	    (void)mach_port_deallocate(mach_task_self(), p_array[i]);
	}
}

void set_string(name, value)
    char *name, *value;
{
    env_name_t ename;
    env_str_val_t evalue;
    kern_return_t kr;
    
    strncpy(ename, name, sizeof(ename)-1);
    strncpy(evalue, value, sizeof(evalue)-1);
    kr = env_set_string(environment_port, ename, evalue);
    if (kr != ENV_SUCCESS)
    {
	quit(1, "%s: env_set_string: %s\n", progname,
	     env_error_string(kr));
    }
}

kern_return_t do_set_port(name, port)
    char *name;
    mach_port_t port;
{
    env_name_t ename;
    kern_return_t kr;
    
    strncpy(ename, name, sizeof(ename)-1);
    return env_set_port(environment_port, ename, port);
}

void set_port(name)
    char *name;
{
    mach_port_t port;
    kern_return_t kr;
    
    kr = mach_port_allocate(mach_task_self(), MACH_PORT_RIGHT_RECEIVE, &port);
    if (kr != KERN_SUCCESS)
	quit(1, "%s: mach_port_allocate: %s.\n", progname, mach_error_string(kr));
    kr = mach_port_insert_right(mach_task_self(), port, port,
				MACH_MSG_TYPE_MAKE_SEND);
    if (kr != KERN_SUCCESS)
	quit(1, "%s: mach_port_insert_right: %s\n", progname, 
	     mach_error_string(kr));
    kr = do_set_port(name, port);
    if (kr != KERN_SUCCESS)
	quit(1, "%s: env_set_port: %s\n", progname, env_error_string(kr));
    printf("inserted port 0x%x\n", port);
}

void get_string(name)
    char *name;
{
    env_name_t ename;
    env_str_val_t evalue;
    kern_return_t kr;
    
    strncpy(ename, name, sizeof(ename)-1);
    kr = env_get_string(environment_port, ename, evalue);
    if (kr != ENV_SUCCESS) {
	fprintf(stderr, "%s: env_get_string: %s\n", progname, 
		env_error_string(kr));
	return;
    }
    printf("%s = %s\n", ename, evalue);
}

void get_port(name)
    char *name;
{
    env_name_t ename;
    mach_port_t port;
    kern_return_t kr;

    strncpy(ename, name, sizeof(ename)-1);
    kr = env_get_port(environment_port, ename, &port);
    if (kr != ENV_SUCCESS) {
	fprintf(stderr, "%s: env_get_port: %s\n", progname, 
		env_error_string(kr));
	return;
    }
    printf("%s = %d\n", ename, port);
    (void)mach_port_deallocate(mach_task_self(), port);	/* consume the port */
}

void delete_string(name)
    char *name;
{
    env_name_t ename;
    env_str_val_t evalue;
    kern_return_t kr;
    
    strncpy(ename, name, sizeof(ename)-1);
    kr = env_del_string(environment_port, ename);
    if (kr != ENV_SUCCESS) {
	fprintf(stderr, "%s: env_del_string(%s): %s\n", progname, name,
		env_error_string(kr));
    }
}

void delete_port(name)
    char *name;
{
    env_name_t ename;
    kern_return_t kr;

    strncpy(ename, name, sizeof(ename)-1);
    kr = env_del_port(environment_port, ename);
    if (kr != ENV_SUCCESS) {
	fprintf(stderr, "%s: env_del_port(%s): %s\n", progname, name,
		env_error_string(kr));
    }
}

void exec_shell(name, args, type)
    char *name, *args;
    int type;			/* is the environment r/w, r only or new */
{
    int count, pid;
    mach_port_t new_env_port;
    mach_port_array_t ports;
    kern_return_t kr;
    task_t child_task;
    union wait status;
    extern int errno;
    
    pid = fork();
    if (pid == 0)
    {
	int argc;
	char *argv[100], *arg;

	if (type == 0)
	{
	    kr = env_copy_conn(environment_port, &new_env_port);
	    if (kr != ENV_SUCCESS) {
		quit(-1, "%s: env_copy_conn: %s\n", progname,
		     env_error_string(kr));
	    }
	} else if (type == 1) {
	    kr = env_restrict_conn(environment_port, &new_env_port);
	    if (kr != ENV_SUCCESS) {
		quit(-1, "%s: env_restrict_conn: %s\n", progname,
		     env_error_string(kr));
	    }
	} else {
	    kr = env_new_conn(environment_port, &new_env_port);
	    if (kr != ENV_SUCCESS) {
		quit(-1, "%s: env_new_conn: %s\n", progname, 
		     env_error_string(kr));
	    }
	}
	kr = mach_ports_lookup(mach_task_self(), &ports, &count);
	if (kr != KERN_SUCCESS) {
	    quit(1, "%s: mach_ports_lookup: %s\n", progname,
		 mach_error_string(kr));
	}
	ports[ENVIRONMENT_SLOT] = new_env_port;
	kr = mach_ports_register(mach_task_self(), ports, count);
	if (kr != KERN_SUCCESS) {
	    quit(1, "%s: mach_ports_register: %s\n",
		 progname, mach_error_string(kr));
	}
	kr = vm_deallocate(mach_task_self(), (vm_address_t)ports,
			   (vm_size_t) (count * sizeof(mach_port_t)));
	if (kr != KERN_SUCCESS)	{
	    quit(1, "%s: vm_deallocate: %s\n",
		 progname, mach_error_string(kr));
	}
	printf("%s[child]: execve(%s)\n", progname, name);
	argv[0] = name;
	arg = argv[1] = args;
	for (argc = 2; argc < 99 && arg; argv[argc] = arg, argc++) {
	    arg = index(argv[argc-1], ' ');
	    if (arg)
		for (*arg++ = 0; *arg && *arg == ' '; arg++)
		    ;
	}
	argv[argc] = NULL;
	execve(argv[0], argv, NULL);
	quit(-1, "%s[child]: %s: %s\n",
	     progname, name, unix_error_string(errno));
    }
    if (pid == -1)
	quit(1, "%s: fork: %s\n", progname, unix_error_string(errno));
    printf("%s: waiting for child to finish\n", progname);
    if (wait(&status) != pid) {
	fprintf(stderr, "%s: wait != %d\n", progname, pid);
    } else 
	printf("%s: child exited with code %d\n",
	       progname, status.w_retcode);
}
