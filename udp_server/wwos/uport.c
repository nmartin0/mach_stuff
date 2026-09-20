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
 * uport.c -- handle udp ports
 */

/* Port numbers in this file are always in network byte order!!!! */

#include <assert.h>
#include <stdio.h>
#include <sys/time.h>
#include <mach.h>
#include <cthreads.h>
#include "uport.h"
#define DEBUG 0


mutex_t	uport_mutex;
uport_t *uport_list = (uport_t *) NULL;
unsigned short port_cnt = UPORT_BASE;

unsigned long	counters[5];
#define uport_drop_no_dest counters[0]
#define uport_drop_no_space counters[1]


void
uport_init()
{
	uport_mutex = mutex_alloc();
	if (uport_mutex == (mutex_t) NULL) {
		fprintf(stderr, "uport_init: mutex_alloc error\n");
		exit(1);
	}
}

/*
 * Called by mig server thread to allocate a new udp port.
 */
int
uport_allocate(portnum)
	short *portnum;
{
	uport_t *new;

	/*
	 * Initialize a new uport.  (Unsynchronized state only.)
	 */
	new = (uport_t *) malloc(sizeof(uport_t));
	if (new == (uport_t *) NULL) {
		perror("malloc");
		return -1;
	}
	new->up_mutex 		= mutex_alloc();
	new->up_condition 	= condition_alloc();
	new->up_wait		= 0;
	new->up_count		= 0;
	new->up_q 	        = 0;
	new->up_here.upk_data  = (char*)malloc(2048);	/* you get one for free */
	bzero(new->up_here.upk_data, 2048);

	/*
	 * Now grab the mutex and allocate the port number.
	 */
	mutex_lock(uport_mutex);

	new->up_next 		= (uport_t *) uport_list;
	new->up_lport		= htons(++port_cnt);
	uport_list 		= new;
	*portnum 		= new->up_lport;

	mutex_unlock(uport_mutex);
	return 0;		/* success */
}

/*
 * Find a port struct.  
 * Portnum is in network byte order.
 */
uport_t *
uport_find(portnum)
	short portnum;
{
	uport_t *up;

	mutex_lock(uport_mutex);
	for (up = uport_list; up; up = up->up_next)
		if (up->up_lport == portnum)
			break;
	mutex_unlock(uport_mutex);
	return up;
}

/*
 * Called by low level protocol code.
 * Queues incoming packets for delivery.
 */

  /*
   * packet is still in ipc buffer.  Copy it either into buffer queue or
   * receiving thread's stacks.
   */
void
uport_input(dport, saddr, sport, data, datalen)
	short dport;
        unsigned long saddr;
        short sport;
        char *data;
        int datalen;
{
    uport_t *up;
    struct upacket *recv_pkt;

    /*
     * Find destination queue.
     */
    up = uport_find(dport);
    if (up == (uport_t *) NULL) {
	uport_drop_no_dest++;
	return;
    }

    /*
     * Lock the uport so we can manipulate the queue.
     * Once we have the up mutex, we can release the list lock.
     */
    mutex_lock(up->up_mutex);

    /*
     * Queue the packet.
     */
    if (up->up_count > 0)  {
	/* queue is full. */
#if 1
	printf("dropping packet on the floor in uport_input\n");
#endif	
	uport_drop_no_space++;	/* unsynched.  so what */
	goto out;
    }

    recv_pkt = up->up_q;
#if DEBUG    
	printf("in uport input. Datalen is %d\n", datalen);
#endif    
    if (recv_pkt == 0)	{
#if DEBUG    	
	printf("no waiters waiting (0x%x)\n", up);
#endif	
	/* no waiters waiting */
	up->up_count++;
	up->up_here.upk_fromaddr = saddr;
	up->up_here.upk_fromport = sport;
	up->up_here.upk_len = datalen;
	/* now we actually copy the data out of the ipc buffer */
	bcopy(data, up->up_here.upk_data, datalen);
    } else  {
	/* threads are waiting.  Copy into their buffers */
	while (recv_pkt)  {
#if DEBUG	    
	 printf("Copying msg onto thread stack %x\n", recv_pkt);
#endif	 
	    recv_pkt->upk_fromaddr = saddr;
	    recv_pkt->upk_fromport = sport;
	    recv_pkt->upk_len = datalen;
	    bcopy(data, recv_pkt->upk_data, datalen);
	    recv_pkt = recv_pkt->upk_next;
	}
	up->up_q = 0; /* no more waiters */
#if DEBUG	
	printf("Waking up waiters\n");
#endif	
	condition_signal(up->up_condition); /* probably should be cond_broadcast */
    }
out:    
    mutex_unlock(up->up_mutex);
    return;
}
	
int
timeout(t1, t2)
	struct timeval *t1, *t2;
{
	long v1, v2;

	v1 = (t1->tv_sec * 1000) + (t1->tv_usec / 1000);
	v2 = (t2->tv_sec * 1000) + (t2->tv_usec / 1000);
	return (v2 - v1) > 100;	/* 100ms timeout */
}


uport_recv(portnum, pkt)
	short portnum;
	upacket_t *pkt;
{
	uport_t *up;

	up = uport_find(portnum);
	if (up == (uport_t *) NULL) {
#if DEBUG
		printf("[ uport_recv: no port %d ]\n", portnum);
#endif
		return -1;
	}

	
	mutex_lock(up->up_mutex);
	

	if (up->up_count)	{
	    register struct upacket *inpkt = &up->up_here;
#if DEBUG	    
	printf("something on queue. Grab it and return\n");
#endif	    
		/*
		 * Something on the queue.  Grab it and return.
		 */
	    pkt->upk_fromaddr = inpkt->upk_fromaddr;
	    pkt->upk_fromport = inpkt->upk_fromport;
	    pkt->upk_len = inpkt->upk_len;
	    bcopy(inpkt->upk_data, pkt->upk_data, pkt->upk_len);
	    up->up_count = 0;
	} else {
#if DEBUG	    
		printf("nothing on queue. thread is waiting (up=0x%x)\n", up);
#endif		
		/*
		 * Nothing on the queue.  Start waiting, Jack.
		 */
	    
		/* put our pkt at front of queue */
		pkt->upk_next = up->up_q;
		up->up_q = pkt;
		pkt->upk_fromaddr = 0;		/* nothing in this packet */
		while (pkt->upk_fromaddr == 0) {
#if DEBUG		    
		printf("thread is now waiting: pkt is %d\n", pkt->upk_fromaddr);
#endif	
			condition_wait(up->up_condition, up->up_mutex);
#if DEBUG		
		printf("thread has awoken: pkt is %d\n", pkt->upk_fromaddr);
#endif		
		}
	}
	mutex_unlock(up->up_mutex);
	return 0;
}
