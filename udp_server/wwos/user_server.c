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
 * udp user
 */

#include <stdio.h>
#include <mach.h>
#include <servers/netname.h>

#define NAME_SERVER_SLOT 0

char udp_service[] = "UDPSERVER";

mach_port_t name_port = MACH_PORT_NULL;
mach_port_t udp_port = MACH_PORT_NULL;

extern int udp_getsport();	/* mig */

void
get_name_port ()
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
		return;
	}
	name_port = well_known_ports[NAME_SERVER_SLOT];
	printf("[ name port = 0x%x ]\n", name_port);
}

get_udp_port ()
{
	kern_return_t kr;
	char sname[80];

	bzero(sname, sizeof(sname));
	bcopy(udp_service, sname, sizeof(udp_service));

	printf("[ requesting service port for \"%s\" ]\n", sname);
	kr = netname_look_up(name_port, "", sname, &udp_port);
	if (kr != KERN_SUCCESS) {
		fprintf(stderr, "netname_look_up: error 0x%x\n", kr);
		exit (0);
	}
	printf("[ got service port 0x%x ]\n", udp_port);
}

main ()
{
	unsigned long saddr;
	short portname, sport;
	kern_return_t kr;
	char buf[2048];
	int i, buflen;


	get_name_port();
	get_udp_port();

	kr = udp_getsport(udp_port, 0, &portname);
	if (kr != KERN_SUCCESS) {
		fprintf(stderr, "udp_getsport: error 0x%x\n", kr);
		exit (0);
	}
	printf("[ udp_getsport returned port %d ]\n", ntohs(portname));

	while (1) {
		buflen = sizeof(buf);
		kr = udp_recvfrom(udp_port, 0, portname,
				  &saddr, &sport, buf, &buflen);
		if (kr != KERN_SUCCESS) {
			fprintf(stderr, "udp_recvfrom: error 0x%x\n", kr);
		}
		
#if DEBUG
		/* dump the packet */
		printf("[ from %x port %d ]\n", ntohl(saddr), ntohs(sport));
		for (i = 0; i < buflen; i++) {
			printf("%2x ", buf[i]);
			if ((i > 0) && ((i % 16) == 0))
				printf("\n");
		}
		printf("\n");
#endif

		/*
		 * Return the data.
		 */
		kr = udp_sendto(udp_port, saddr, sport, 0, portname, 
				buf, buflen);
		if (kr != KERN_SUCCESS) {
			fprintf(stderr, "udp_sendto: error 0x%x\n", kr);
		}
	}
}

	
