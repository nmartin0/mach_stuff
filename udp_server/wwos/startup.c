/*
 * Simpleminded UDP Server
 *
 * Copyright (C) 1992  Chris Maeda
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * Chris Maeda (cmaeda@cs.cmu.edu)
 * School of Computer Science
 * Carnegie Mellon University
 * 5000 Forbes Ave
 * Pittsburgh PA  15213-3890
 * USA
 */
/*
 * startup.c -- starting up the server
 */

#include <stdio.h>
#include <mach.h>
#include <cthreads.h>
#include <servers/netname.h>

#define NAME_SERVER_SLOT 0	/* where the hell is this defined? */
#define MAX_SERVER_THREADS 10

mach_port_t name_srvr_port = MACH_PORT_NULL;
mach_port_t udp_service_port = MACH_PORT_NULL;

static char service_name[] = "UDPSERVER";
char netname[80];	/* netname service name */


int
init_env_ports()
{
	kern_return_t kr;
	mach_port_array_t well_known_ports;
	int port_count;

	kr = mach_ports_lookup(mach_task_self(), 
			       &well_known_ports,
			       &port_count);
	if (kr != KERN_SUCCESS) {
		fprintf(stderr, "mach_ports_lookup: error %d\n", kr);
		return;
	}

	printf("[ mach_ports_lookup returned %d ports ]\n", port_count);
	if (port_count <= NAME_SERVER_SLOT) {
		fprintf(stderr, "mach_ports_lookup: no name server port\n");
		return -1;
	}
	name_srvr_port = well_known_ports[NAME_SERVER_SLOT];
	
	/*
	 * Create a service port.
	 */
	kr = mach_port_allocate(mach_task_self(),
				MACH_PORT_RIGHT_RECEIVE,
				&udp_service_port);
	if (kr != KERN_SUCCESS) {
		fprintf(stderr, "mach_port_allocate: error %d\n", kr);
		return -1;
	}

	/*
	 * Now register udp service with name server.
	 */
	bzero(netname, sizeof(netname));
	bcopy(service_name, netname, sizeof(service_name));
	printf("[ checking in service name \"%s\" ]\n", netname);
	kr = netname_check_in(name_srvr_port,
			      netname,
			      MACH_PORT_NULL, /* no signature */
			      udp_service_port);
	if (kr != KERN_SUCCESS) {
		fprintf(stderr, "netname_check_in: error %d\n", kr);
		return -1;
	}
	return 0;
}

void
shutdown_env_ports()
{
	kern_return_t kr;

	printf("[ checking out service name \"%s\" ]\n", netname);
	kr = netname_check_out(name_srvr_port, netname, MACH_PORT_NULL);
	if (kr != KERN_SUCCESS) {
		fprintf(stderr, "netname_check_out: error %d\n", kr);
	}
}

void
shutdown(v)
	int v;
{
	shutdown_env_ports();
	exit(v);
}

    

main ()
{
    int i;
    extern void network_input_thread();
    extern void server_root();

    printf("\n");
    /*
     * register with snames as the udp server
     */
    if (init_env_ports()) {
	fprintf(stderr, "init_env_ports: failure\n");
	exit(1);
    }
	
    /*
     * get network ports
     */
    init_netstuff();

    /*
     * udp queues
     */
    uport_init();

    /*
     * fork off network thread
     */
    cthread_detach(cthread_fork(network_input_thread, 0));



    printf("[ %x : starting some server loops ]\n", cthread_self());
    for (i = 0; i < MAX_SERVER_THREADS; i++)  {
	cthread_detach(cthread_fork(server_root, 0));
    }
    sigpause(0);
}

