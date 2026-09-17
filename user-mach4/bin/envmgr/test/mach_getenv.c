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
 * $Log: mach_getenv.c,v $
 * Revision 1.1.1.1  1995/05/04  06:56:48  sclawson
 * New files.
 *
 * Revision 2.2  92/04/01  19:09:34  rpd
 * 	Created.
 * 	[92/04/01  18:53:43  rpd]
 * 
 */
/*
 *  File: mach_getenv.c
 *      A sample implementation of libc env routines
 *
 *  Author: Jukka Partanen, Helsinki University of Technology 1992
 */

#include <mach.h>
#include <mach_init.h>
#include <mach/message.h>
#include <servers/env_mgr.h>
#include <strings.h>
#include <stdio.h>

extern char *malloc();

char *getenv (name)
    char *name;
{
    kern_return_t kr;
    env_name_t ename;
    env_str_val_t evalue;
    char *value;

    strncpy(ename, name, sizeof(ename)-1);
    ename[sizeof(ename)-1] = '\0';
    if (value = index(ename, '='))
	*value = '\0';
    kr = env_get_string(environment_port, ename, evalue);
    if (kr != KERN_SUCCESS) {
	return NULL;
    }
    value = malloc(strlen(evalue) + 1);
    if (value != NULL)
	strcpy(value, evalue);
    return value;
}

int setenv (name, value, overwrite)
    char *name, *value;
    int overwrite;
{
    kern_return_t kr;
    env_name_t ename;
    env_str_val_t evalue;
    char *s;

    strncpy(ename, name, sizeof(ename)-1);
    ename[sizeof(ename)-1] = '\0';
    if (s = index(ename, '='))
	*s = '\0';
    kr = env_get_string(environment_port, ename, evalue);
    strncpy(evalue, value, sizeof(evalue)-1);
    evalue[sizeof(evalue)-1] - 1;
    switch (kr)
    {
    case ENV_VAR_NOT_FOUND:
	kr = env_set_string(environment_port, evalue, ename);
	break;
    case ENV_SUCCESS:
	if (overwrite)
	    kr = env_set_string(environment_port, evalue, ename);
	break;
    case ENV_WRONG_VAR_TYPE:	/* should we delete the port and set string? */
    default:
	break;
    }
    if (kr != KERN_SUCCESS)
	return -1;
    return 0;
}

void unsetenv (name)
    char *name;
{
    env_name_t ename;
    kern_return_t kr;
    char *s;

    strncpy(ename, name, sizeof(ename)-1);
    ename[sizeof(ename)-1] = '\0';
    if (s = index(ename, '='))
	*s = '\0';
    kr = env_del_string(environment_port, ename);
    if (kr == ENV_WRONG_VAR_TYPE)
	kr = env_del_port(environment_port, ename);
}

int putenv (string)
    char *string;
{
    char *value;

    value = index(string, '=');
    if (!value)
	return -1;
    /* NOTE! This relies on the fact that setenv will index(name, '=')
       and place a NUL in its copy of name */
    return setenv(string, ++value, 1);
}

/* 
 * These routines convert from UN*X-style environment to Mach environment
 * and back.
 */
char **copy_env_to_environ ()
{
    char **environ;
    kern_return_t kr;
    env_name_list namelist;
    env_str_list valuelist;
    mach_msg_type_number_t namecnt, valuecnt;

    kr = env_list_strings(environment_port, &namelist, &namecnt,
			  &valuelist, &valuecnt);
    if (kr != KERN_SUCCESS)
	return NULL;
    environ = (char **)malloc(namecnt * sizeof(char *));
    if (environ != NULL) {
	int i;
	char *s;

	for (i = 0; i < namecnt; i++) {
	    s = malloc(strlen(namelist[i]) + strlen(valuelist[i]) + 2);
	    if (s == NULL) {
		while (--i >= 0)
		    free(environ[i]);
		free(environ);
		environ = NULL;
		goto deallocate;
	    }
	    strcpy(s, namelist[i]);
	    strcat(s, "=");
	    strcat(s, valuelist[i]);
	    environ[i] = s;
	}
    }
 deallocate:
    (void)vm_deallocate(mach_task_self(), (vm_address_t)namelist,
			namecnt * sizeof(env_name_t));
    (void)vm_deallocate(mach_task_self(), (vm_address_t)valuelist,
			valuecnt * sizeof(env_str_val_t));
    return environ;
}

kern_return_t copy_environ_to_env (environ)
    char **environ;
{
    char *name, *value, *s;
    env_name_list namelist;
    env_str_list valuelist;
    kern_return_t kr;
    mach_msg_type_number_t namecnt, valuecnt;

    for (namecnt = 0; environ[namecnt]; namecnt++)
	;
    kr = vm_allocate(mach_task_self(), (vm_address_t *)&namelist,
		     namecnt * sizeof(env_name_t), TRUE);
    if (kr != KERN_SUCCESS) {
	return kr;
    }
    kr = vm_allocate(mach_task_self(), (vm_address_t *)&valuelist,
		     namecnt * sizeof(env_str_val_t), TRUE);
    if (kr != KERN_SUCCESS) {
	(void)vm_deallocate(mach_task_self(), (vm_address_t)namelist,
			    namecnt * sizeof(env_name_t));
	return kr;
    }
    
    for (valuecnt = 0; valuecnt < namecnt; valuecnt++) {
	strncpy(namelist[valuecnt], environ[valuecnt], sizeof(env_name_t)-1);
	namelist[valuecnt][sizeof(env_name_t)-1] = '\0';
	if (s = index(namelist[valuecnt], '='))
	    *s = '\0';
	if (s = index(environ[valuecnt], '='))
	    strncpy(valuelist[valuecnt], ++s, sizeof(env_str_val_t)-1);
	else 
	    strcpy(valuelist[valuecnt], ""); 
	valuelist[valuecnt][sizeof(env_str_val_t)-1] = '\0';
    }
    kr = env_set_stlist(environment_port, namelist, namecnt, valuelist, namecnt);
    (void)vm_deallocate(mach_task_self(), (vm_address_t)namelist,
			namecnt * sizeof(env_name_t));
    (void)vm_deallocate(mach_task_self(), (vm_address_t)valuelist,
			namecnt * sizeof(env_str_val_t));
    if (kr != KERN_SUCCESS) {
	return kr;
    }
    return KERN_SUCCESS;
}
    
