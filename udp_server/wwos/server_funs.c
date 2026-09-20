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
 * server_funs.c -- perform the service here
 */

#include <stdio.h>
#include <mach.h>
#include <cthreads.h>
#include "uport.h"

kern_return_t
udp_getsport(proc_port, ipaddr, portnum)
	mach_port_t proc_port;
	unsigned ipaddr;
	short *portnum;
{
	int retval;
	extern int uport_allocate();

	retval = uport_allocate(portnum);
	if (retval == 0) {
		printf("[ udp_getsport: allocating %d ]\n", ntohs(*portnum));
		return KERN_SUCCESS;
	}
	else {
		printf("udp_getsport: error %d\n", retval);
		return KERN_FAILURE;
	}
}

kern_return_t
udp_sendto(proc_port, d_addr, d_port, s_addr, s_port, dgram, d_len)
	mach_port_t 	proc_port;
	unsigned 	d_addr, s_addr;		/* ip addresses */
	short		d_port, s_port;		/* udp ports */
	char		*dgram;
	int		d_len;			/* size of dgram */
{
#if DEBUG
	printf("[ udp_sendto sending to port %d ]\n", ntohs(d_port));
#endif    
    if (d_len == 0) return KERN_SUCCESS;
    else return udp_output(dgram, d_len,
			   d_addr, d_port,
			   s_addr, s_port, 1);
}



kern_return_t
udp_sendto_n(proc_port, d_addr, d_port, s_addr, s_port, dgram, d_len, n)
	mach_port_t 	proc_port;
	unsigned 	d_addr, s_addr;		/* ip addresses */
	short		d_port, s_port;		/* udp ports */
	char		*dgram;
	int		d_len;			/* size of dgram */
	int 		n;			/* number of sends */
{
#if DEBUG
	printf("[ udp_sendto sending to port %d ]\n", ntohs(d_port));
#endif    
    if (d_len == 0) return KERN_SUCCESS;
    else return udp_output(dgram, d_len,
			   d_addr, d_port,
			   s_addr, s_port, n);
}

kern_return_t
udp_recvfrom(proc_port, d_addr, d_port, s_addr, s_port, dgram, d_len)
	mach_port_t 	proc_port;
	unsigned 	d_addr, *s_addr;	/* ip addresses */
	short		d_port, *s_port;	/* udp ports */
	char		*dgram;
	int		*d_len;			/* size of dgram */
{
	upacket_t pkt_data;

	pkt_data.upk_data = dgram;		/* where the data goes eventually */

#if DEBUG
	printf("[ udp_recvfrom receiving from port %d ]\n", ntohs(d_port));
#endif
	if (uport_recv((short)d_port, &pkt_data))
		return KERN_FAILURE;

	if (*d_len < pkt_data.upk_len) {
		return KERN_RESOURCE_SHORTAGE;
	}
	/*
	 * Set up return values.
	 */
	*d_len = pkt_data.upk_len;
	*s_addr = pkt_data.upk_fromaddr;
	*s_port = pkt_data.upk_fromport;
	return KERN_SUCCESS;
}

	
