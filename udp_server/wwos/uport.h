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
 * uport.h -- handle udp ports
 */

struct upacket {
	struct upacket	*upk_next;
	unsigned long	upk_fromaddr;
	short		upk_fromport;
	int		upk_len;
	char		*upk_data;	/* where the data goes */
};
typedef struct upacket upacket_t;

/*
 * Struct that maps udp ports to mach ports.
 */
struct uport {
	struct uport 	*up_next;	
	mutex_t		up_mutex;
	condition_t	up_condition; 	/* wait for a packet */
	short		up_lport; 	/* udp port */
	short		up_wait; 	/* number of waiters */
	struct upacket	*up_q; 		/* packets for this port */
	short		up_count; 	/* size of queue */
        struct upacket  up_here;	/* packet arrives, no waiters */
};
typedef struct uport uport_t;

/*
 * To avoid conflicting with the unix server,
 * we allocate udp ports from a high range of numbers.
 * There is a more elegant solution, but it requires
 * a lot of unix server hacking.
 */
#define UPORT_BASE 0x4000	/* 16384 */

