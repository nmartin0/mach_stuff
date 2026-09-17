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
 * $Log: test_netname.c,v $
 * Revision 1.1.1.1  1995/05/04  06:56:46  sclawson
 * New files.
 *
 * Revision 2.2  92/11/13  17:38:38  mrt
 * 	Updated for Mach 3.0
 * 	[92/11/13            mrt]
 * 
 *
 */
/*  Program to check if named is working correctly 
 *  and that netnameserver lookups are working
 */

#include <stdio.h>
#include <netdb.h>
#include <mach.h>
#include <servers/netname.h>
#include <mach_error.h>


main(argc,argv)
   int argc;
   char *argv[];
{
    char hostname[80];
    int namelen = 80;
    struct hostent *host_ent;
    char NMMonServName[120];
    char **namep;
    mach_port_t nm_port;
    kern_return_t gr;
    int exit_value = 0;

   if ( gethostname( hostname, namelen) == -1)
   {	printf("cannot get my host name\n");
	exit (1);
   }
   sprintf(NMMonServName, "%s", "NMMonitor");
   printf("port name is %s\n",NMMonServName);
   host_ent = gethostbyname(hostname);
   if (host_ent == 0)
   {	printf ("gethostbyname(%s) failed\n",hostname);
	exit_value = 2;
   }
   else
   {	printf("host_ent->h_name is %s\n",host_ent->h_name);
   	namep = host_ent->h_aliases;
	if (*namep == 0)
		printf("no aliases found\n");
	else
	{	while(*namep != 0)
		{  printf("Alias is %s\n",*namep);
		   namep++;
		}
	}
   }

   printf("Name_server_port is %x\n",(int)name_server_port);
   if (argc == 1)
   {
	printf("Doing local lookup of %s\n",NMMonServName);
	gr = netname_look_up(name_server_port,"",NMMonServName,&nm_port);
	if (gr != KERN_SUCCESS)
	{	printf("net_name_lookup failed - %s\n",mach_error_string(gr));
		exit_value = 3;
	}
	else printf("%s is %x\n",NMMonServName,(int)nm_port);

	printf("Doing broadcast lookup of %s\n",NMMonServName);
	gr = netname_look_up(name_server_port,"*",NMMonServName,&nm_port);
	if (gr != KERN_SUCCESS)
	{	printf("net_name_lookup failed - %s\n",mach_error_string(gr));
		exit_value = 3;
	}
	else printf("%s is %x\n",NMMonServName,(int)nm_port);

	printf("Doing host-specific lookup of %s on %s\n",NMMonServName,hostname);
	gr = netname_look_up(name_server_port,hostname,NMMonServName,&nm_port);
	if (gr != KERN_SUCCESS)
	{	printf("net_name_lookup failed - %s\n",mach_error_string(gr));
		exit_value = 3;
	}
	else printf("%s is %x\n",NMMonServName,(int)nm_port);
   }
   else 
   {    if (argc != 3)
	{	printf("usage is testn [hostname portname]\n");
		exit (1);
	}
	printf("Doing local lookup of %s\n",argv[2]);
	gr = netname_look_up(name_server_port,"",argv[2],&nm_port);
	if (gr != KERN_SUCCESS)
	{	printf("net_name_lookup failed - %s\n",mach_error_string(gr));
		exit_value = 3;
	}
	else printf("%s is %x\n",argv[2],(int)nm_port);

	printf("Doing broadcast lookup of %s\n",argv[2]);
	gr = netname_look_up(name_server_port,"*",argv[2],&nm_port);
	if (gr != KERN_SUCCESS)
	{	printf("net_name_lookup failed - %s\n",mach_error_string(gr));
		exit_value = 3;
	}
	else printf("%s is %x\n",argv[2],(int)nm_port);

	printf("Doing host-specific lookup of %s on %s\n",argv[2],argv[1]);
	gr = netname_look_up(name_server_port,argv[1],argv[2],&nm_port);
	if (gr != KERN_SUCCESS)
	{	printf("net_name_lookup failed - %s\n",mach_error_string(gr));
		exit_value = 3;
	}
	else {
		printf("%s is %x\n",argv[2],(int)nm_port);	
		printf("NOTE: this test is only valid if netmsgserver is running.\n");
		printf("      snames does local name lookups only.\n");
	}
   }
   exit (exit_value);
}
