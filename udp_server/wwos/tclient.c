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
 * tclient.c -- bounce packets off a server
 */

#include <stdio.h>
#include <sys/types.h>
#include <netdb.h>
#include <mach.h>
#include <cthreads.h>
#include "timing.h"

int cache_flush();

int
main (argc, argv)
     int argc;
     char **argv;
{
    kern_return_t kr;
    int i, pkt_len, pkt_cnt, sendonly, buflen;
    short dport, sport, s_port;
    u_long hostaddr, s_addr;
    struct hostent *host;
    ElapsedTimeT t;
    char buf[2048];
    extern void network_input_thread();

    if (argc < 5) {
      printf("usage: %s host port pkts datalen [sendonly?]\n", argv[0]);
      exit(0);
    }
    if (argc > 5)
      sendonly = 1;
    else
      sendonly = 0;
    
    /* parse args now that we know they're there */
    host = gethostbyname(argv[1]);
    if (host == (struct hostent *) NULL) {
      printf("gethostbyname: bad host %s\n", argv[1]);
      exit(1);
    }
    bcopy(host->h_addr, (char *)&hostaddr, sizeof(hostaddr));
    dport = atoi(argv[2]);
    pkt_cnt = atoi(argv[3]);
    pkt_len = atoi(argv[4]);
    printf("sending %d %d packets to %s (port %d)\n",
	   pkt_cnt, pkt_len, host->h_name, dport);
    dport = htons(dport);
        	
    /*
     * Initialization:
     */
    init_netstuff();		/* get network ports */
    uport_init();		/* udp queues */
    cthread_detach(cthread_fork(network_input_thread, 42));
    cthread_yield();

    udp_getsport(MACH_PORT_NULL, 0, &sport);
    printf("got local port %d\n", ntohs(sport));
    if ((kr = udp_sendto(MACH_PORT_NULL,
			 hostaddr, dport,
			 0, sport,
			 buf, pkt_len)) != KERN_SUCCESS) {
      printf("udp_sendto 0x%x\n", kr);
      exit(1);
    }

    /* compute cache flush overhead */
    t = NewTiming("cache flush", 0);
    StartTiming(t);
    for (i = 0; i < pkt_cnt; i++)
      cache_flush();
    EndTiming(t);

    t = NewTiming("udp client", t);
    StartTiming(t);

    for (i = 0; i < pkt_cnt; i++) {
      cache_flush();

      if ((kr = udp_sendto(MACH_PORT_NULL,
			   hostaddr, dport,
			   0, sport,
			   buf, pkt_len)) != KERN_SUCCESS) {
	printf("udp_sendto 0x%x\n", kr);
	exit(1);
      }
      
      if (! sendonly) {
	buflen = pkt_len;
	if ((kr = udp_recvfrom(MACH_PORT_NULL,
			       0, sport,
			       &s_addr, &s_port,
			       buf, &buflen)) != KERN_SUCCESS) {
	  printf("udp_recvfrom 0x%x\n", kr);
	  exit(1);
	}
      }
    }

    EndTiming(t, pkt_cnt);
    DumpTimings(t, stdout);
    sigpause(0);
}

cache_flush()
{
  int flush_len, value;  

  flush_len = (int)cache_flush - (int)main;
  value = MATTR_VAL_CACHE_FLUSH;
  vm_machine_attribute(mach_task_self(), (vm_address_t)main, flush_len,
		       MATTR_CACHE, &value);
}

