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
 * udp user client
 */

#include <stdio.h>
#include <sys/time.h>
#include <mach.h>
#include <servers/netname.h>
#include <netdb.h>

#define DEBUG 0
#define NAME_SERVER_SLOT 0

int first_global;

char udp_service[] = "UDPSERVER";

mach_port_t name_port = MACH_PORT_NULL;
mach_port_t udp_port = MACH_PORT_NULL;

extern int udp_getsport();	/* mig */
extern int udp_sendto();
extern int udp_recvfrom();

int last_global;

void cache_flush();		/* forward */

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

#if DEBUG
	printf("[ mach_ports_lookup returned %d ports ]\n", port_count);
#endif
	if (port_count <= NAME_SERVER_SLOT) {
		fprintf(stderr, "mach_ports_lookup: no name server port\n");
		return;
	}
	name_port = well_known_ports[NAME_SERVER_SLOT];
#if DEBUG
	printf("[ name port = 0x%x ]\n", name_port);
#endif
}

get_udp_port ()
{
	kern_return_t kr;
	char sname[80];

	bzero(sname, sizeof(sname));
	bcopy(udp_service, sname, sizeof(udp_service));

#if DEBUG
	printf("[ requesting service port for \"%s\" ]\n", sname);
#endif
	kr = netname_look_up(name_port, "", sname, &udp_port);
	if (kr != KERN_SUCCESS) {
		fprintf(stderr, "netname_look_up: error 0x%x\n", kr);
		exit (0);
	}
#if DEBUG
	printf("[ got service port 0x%x ]\n", udp_port);
#endif
}

unsigned long
lookup_hostaddr(hostname)
	char *hostname;
{
	struct hostent *host;
	unsigned long v;

	host = gethostbyname(hostname);
	if (host == (struct hostent *) NULL) {
		extern int h_errno;
		printf("error %d looking up host %s\n", h_errno, hostname);
		exit(1);
	}
	bcopy((char *)host->h_addr, (char *)&v, sizeof(v));
	return v;
}


int send_only = 0;

main (argc, argv)
	int argc;
	char **argv;
{
	unsigned long saddr;
	short portname, sport, destport;
	kern_return_t kr;
	char buf[2048];
	int i, j, buflen, pkt_cnt, pkt_len, repeat;
	unsigned long destaddr, flush_time;
	struct timeval now, then;
	

	if (argc < 5) {
		printf("usage: %s host port count data [repeat]\n", argv[0]);
		exit(0);
	}

	destaddr = lookup_hostaddr(argv[1]);
	destport = atoi(argv[2]);
	pkt_cnt = atoi(argv[3]);
	pkt_len = atoi(argv[4]);
        if (argc > 5) send_only = atoi(argv[5]);
	repeat = 1;

	get_name_port();
	get_udp_port();
	
	kr = udp_getsport(udp_port, 0, &portname);
	if (kr != KERN_SUCCESS) {
		fprintf(stderr, "udp_getsport: error 0x%x\n", kr);
		exit (0);
	}
#if DEBUG
	printf("[ udp_getsport returned port %d ]\n", ntohs(portname));
	printf("sendonly is %d\n", send_only);
	printf("[ sending %d %d byte packets to %x port %d ]\n",
	       pkt_cnt, pkt_len, destaddr, destport);
#endif

	bzero(buf, pkt_len);
	strcpy(buf,"ABCDEF");

	/*
	 * Compute cache flush overhead.
	 */
	gettimeofday(&then, 0);
	for (i = 0; i < pkt_cnt; i++)
	  cache_flush();
	gettimeofday(&now, 0);
	{
	  long v1, v2;

	  v1 = (then.tv_sec * 1000) + (then.tv_usec / 1000);
	  v2 = (now.tv_sec * 1000) + (now.tv_usec / 1000);
	  flush_time = v2 - v1;
	}

#if 1
	/* unstick the guy on the other end.... */
	kr = udp_sendto(udp_port, destaddr, htons(destport),
				0, portname, buf, pkt_len);
		if (kr != KERN_SUCCESS) {
			fprintf(stderr, "udp_sendto: error 0x%x\n", kr);
			exit (0);
	}	
#endif
	for (i = 0; i < repeat; i++) {
		long timediff;

		gettimeofday(&then, 0);

		for (j = 0; j < pkt_cnt; j++) {
#if DEBUG					    
			printf("client is sending\n");
#endif			
			cache_flush();

			if (send_only)	{
				kr = udp_sendto_n(udp_port, destaddr, 
					htons(destport),
					0, portname, buf, pkt_len, send_only);
			} else	{
				kr = udp_sendto(udp_port, destaddr, 
					htons(destport),
					0, portname, buf, pkt_len);
			}
			if (kr != KERN_SUCCESS) {
				fprintf(stderr, "udp_sendto: error 0x%x\n", kr);
				exit (0);
			}
#if DEBUG			
			printf("client is receiving\n");
#endif			
			if (!send_only)	{
				buflen = sizeof(buf);
				kr = udp_recvfrom(udp_port, 0, portname,
					  &saddr, &sport, buf, &buflen);
				if (kr != KERN_SUCCESS) {
					fprintf(stderr, "udp_recvfrom: error 0x%x\n", kr);
					exit (0);
				}
			}
		}

		gettimeofday(&now,0);
		{
			long v1, v2;

			v1 = (then.tv_sec * 1000) + (then.tv_usec / 1000);
			v2 = (now.tv_sec * 1000) + (now.tv_usec / 1000);
			timediff = v2 - v1;
		}
		timediff -= flush_time;
		printf("[%3d] %d packets %ld ms\n", i, pkt_cnt, timediff);
	}
	exit(0);
}

post_main()
{
  return;
}

void
cache_flush()
{
  int flush_len, value;  

  flush_len = (int)&last_global - (int)&first_global;
  value = MATTR_VAL_CACHE_FLUSH;
  vm_machine_attribute(mach_task_self(), &first_global, flush_len,
		       MATTR_CACHE, &value);
  flush_len = (int)post_main - (int)main;
  value = MATTR_VAL_CACHE_FLUSH;
  vm_machine_attribute(mach_task_self(), (vm_address_t)main, flush_len,
		       MATTR_CACHE, &value);
}

