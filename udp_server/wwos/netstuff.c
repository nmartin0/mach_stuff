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

#include <assert.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>

#include <mach.h>
#include <device/net_status.h>

#include <netinet/in.h>
#include <netinet/in_systm.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <netdb.h>
#include <cthreads.h>
#include "uport.h"

#define DEBUG 0

struct	ether_header {
	u_char	ether_dhost[6];
	u_char	ether_shost[6];
	u_short	ether_type;
};

int use_device_syscall = 0;

int need_ip_checksum = 1;
int need_udp_checksum = 1;

unsigned long ip_counters[5];
#define ip_packets_in		ip_counters[0]
#define ip_bad_checksum 	ip_counters[1]
#define udp_nospace_drops	ip_counters[2]

/*
 * mig stubs
 */
extern int bsd_arp_resolve();
extern int bsd_udp_get_ports();
extern int bsd_ether_device_port();

/*
 * networking ports and stuff
 */
mach_port_t	boot_port = MACH_PORT_NULL; 	/* for config info */
mach_port_t	ether_port = MACH_PORT_NULL; 	/* for sending */
mach_port_t	recv_port = MACH_PORT_NULL;	/* for receiving */
char ether_addr[16];
struct net_status ether_stat;
struct in_addr hostaddr;

void
init_hostaddr()
{
	char hostname[80];
	u_long hostid;
	struct hostent *host;

	gethostname(hostname, sizeof(hostname));
	host = gethostbyname(hostname);
	if (host == (struct hostent *) NULL) {
		extern int h_errno;

		printf ("gethostbyname[1]: error %d\n", h_errno);
		exit (1);
	}
	bcopy(host->h_addr, &hostaddr.s_addr, sizeof(u_long));
	printf("[ host address 0x%x ]\n", hostaddr.s_addr);
}

/*
 * Initialize ether_port.
 */
void
get_ether_port(name)
	struct sockaddr_in *name;
{
	kern_return_t kr;
	boolean_t interrupt;
	int i, status_count;
	u_long *addrp;

	if (boot_port == MACH_PORT_NULL) {
		kr = task_get_bootstrap_port(mach_task_self(), &boot_port);
		if (kr != KERN_SUCCESS) {
			printf("Error (%d) getting bootstrap port!\n", kr);
			exit(1);
		}
	}

	/*
	 * NB: This is an experimental ux message.
	 */
	kr = bsd_ether_device_port(boot_port, &interrupt,
				   name, sizeof(struct sockaddr_in),
				   &ether_port);
	if (kr != KERN_SUCCESS) {
		printf("Error (%d) getting port for address %x!\n",
		       kr, name->sin_addr.s_addr);
		exit(1);
	}

	/*
	 * Get local ethernet address.
	 */
	status_count = sizeof(ether_addr)/sizeof(u_long);
	kr = device_get_status(ether_port, NET_ADDRESS,
			       ether_addr, &status_count);
	if (kr != KERN_SUCCESS) {
		printf("Error %d getting ethernet address!\n", kr);
		exit(1);
	}
	printf("[ ethernet address: ");
	addrp = (u_long *) ether_addr;
	for (i = 0; i < status_count; i++) {
		printf("%lx", *addrp);
		*addrp = htonl(*addrp);
		addrp++;
	}
	printf(" ]\n");

	/*
	 * Get random device status in case we need it.
	 */
	status_count = sizeof(ether_stat)/sizeof(u_long);
	kr = device_get_status(ether_port, NET_STATUS,
			       &ether_stat, &status_count);
	if (kr != D_SUCCESS) {
		printf("Error %d getting device status!\n", kr);
		exit(1);
	}
}

void
set_packet_filter(name)
     struct sockaddr_in *name;
{
  kern_return_t kr;
  filter_t filter[32];
  int idx;

  /*
   * First allocate a port for the new filter.
   */
  recv_port = mach_reply_port();
  if (recv_port == MACH_PORT_NULL) {
	  printf("set_packet_filter: error allocating new port!\n", kr);
	  exit(1);
  }
  
  /*
   * Build the filter.
   */
  idx = 0;

  /*
   * See if this udp packet is addressed to the right port.
   * Note that we make some assumptions about ip header
   * length which are verified later.
   */
  filter[idx++] = NETF_PUSHWORD+13; 	  	/* udp dest port */
  filter[idx++] = NETF_PUSHLIT | NETF_AND;
  filter[idx++] = htons(UPORT_BASE); 		/* our type bit for ports */
  filter[idx++] = NETF_PUSHLIT | NETF_CAND;
  filter[idx++] = htons(UPORT_BASE); 		/* is it set? */
  /*
   * Look for the UDP protocol number in the IP header.
   */
  filter[idx++] = NETF_PUSHWORD+6;		/* get ttl/prot */
  filter[idx++] = NETF_PUSHLIT | NETF_AND;  	/* mask off ttl */
  filter[idx++] = htons(0x00ff);
  filter[idx++] = NETF_PUSHLIT | NETF_CAND;	/* compare to udp */
  filter[idx++] = htons(0x0011);                /* protocol number (17) */
  /*
   * Punt on packets that have header options since
   * we can't handle variable length headers.
   * Incidentally verify the IP version number.
   */
  filter[idx++] = NETF_PUSHWORD+2; 	  	/* get ip ver/ihl/tos */
  filter[idx++] = NETF_PUSHLIT | NETF_AND;
  filter[idx++] = htons(0xff00);		/* mask off type-o-service */
  filter[idx++] = NETF_PUSHLIT | NETF_CAND;
  filter[idx++] = htons(0x4500);		/* ip v4, 20 byte hdr */
  /*
   * Punt on fragmented ip datagrams.
   */
  filter[idx++] = NETF_PUSHWORD+5;		/* frag flags and offset */
  /*
   * Note that we want all but one bit of this word --
   * The don't-fragment (DF) bit may be 0 or 1.  Sigh.
   */
  filter[idx++] = NETF_PUSHLIT | NETF_AND;
  filter[idx++] = htons(0xbfff);
  filter[idx++] = NETF_PUSHZERO | NETF_CAND;
  /*
   * Verify that this is an IP packet/
   */
  filter[idx++] = NETF_PUSHWORD+1;		/* get type word */
  filter[idx++] = NETF_PUSHLIT | NETF_CAND;	/* compare to ethernet */
  filter[idx++] = htons(0x0800);		/* IP type number */

  /*
   * Install the filter.
   */
  kr = device_set_filter(ether_port,
			 recv_port,
			 MACH_MSG_TYPE_MAKE_SEND,
			 NET_HI_PRI,	/* high priority */
			 filter,
			 idx);
  if (kr != KERN_SUCCESS) {
	  (void) mach_port_deallocate(mach_task_self(), recv_port);
	  recv_port = MACH_PORT_NULL;
	  printf("Error %d setting packet filter!\n", kr);
	  exit(1);
  }
}

void
dump_packet(msg, f)
	struct net_rcv_msg *msg;
	FILE *f;
{
	long i;
	struct packet_header *ph = (struct packet_header *) msg->packet;
	char *data = (char *) (ph + 1);

	for (i = 0; i < ph->length; i++) {
		fprintf(f, "%02x ", (data[i] & 255));
		if ((i > 0) && ((i % 16) == 0))
			fprintf(f, "\n");
	}
	fprintf(f, "\n");
}

/*
 * A trip up the ip stack.  Network input functions.
 */

u_short
ip_checksum(data, len)
     char *data;
     int len;
{
  u_long ck;

  ck = in_checksum(data, len, 0);
  return (~ck & 0xFFFF);
}  

struct ip_phdr {		/* ip pseudo header for udp checksum */
	u_long ipp_saddr;
	u_long ipp_daddr;
	u_char ipp_zero;
	u_char ipp_prot;
	u_short ipp_length;
};

void
udp_input(msg, data, saddr, daddr)
	struct net_rcv_msg *msg;
	char *data;
        u_long saddr, daddr;
{
	struct udphdr *uh = (struct udphdr *)data;
	int datalen;

	if (need_udp_checksum && uh->uh_sum) {
	  struct ip_phdr *ipp;
	  u_short sum;

	  /*
	   * Compute udp checksum.  Write ip pseudo header over ip header.
	   */
	  ipp = (struct ip_phdr *)(data - sizeof(struct ip_phdr));
	  ipp->ipp_saddr 	= saddr;
	  ipp->ipp_daddr 	= daddr;
	  ipp->ipp_zero 	= (u_char) 0x00;
	  ipp->ipp_prot 	= (u_char) 0x11;
	  ipp->ipp_length	= uh->uh_ulen;

	  if (sum = ip_checksum(ipp, (sizeof(struct ip_phdr) + ntohs(uh->uh_ulen)), 0)) {
#if DEBUG
	    printf("[ checksum failed %x recv %x ]\n", sum, uh->uh_sum);
	    dump_packet(msg, stdout);
#endif
	    return;
	  }
	}

	/*
	 * checksum ok
	 * packet data is still in mach ipc message
	 */
#if DEBUG	
	printf("udp input. dest port is %d\n", ntohs(uh->uh_dport));
#endif	
	datalen = ntohs(uh->uh_ulen) - sizeof(struct udphdr);
	uport_input(uh->uh_dport, saddr, uh->uh_sport, uh + 1,  datalen);
}

void
ip_input(msg)
	struct net_rcv_msg *msg;
{
	kern_return_t kr;
	u_short sum;

	unsigned long checksum, recv_checksum, i;	
	unsigned long saddr, daddr;
	struct packet_header *ph;
	struct ip *ip;

	ip_packets_in++;
	ph = (struct packet_header *) msg->packet;
	ip = (struct ip *) (ph + 1);

	if (sum = ip_checksum(ip, (ip->ip_hl << 2))) {
	  ip_bad_checksum++;
#if DEBUG
	  printf("[ bad checksum %x, dropping packet ]\n, sum");
	  dump_packet(msg, stdout);
#endif
	  return;
	}
	  
	/*
	 * Copy source and destination addresses for later.
	 */
	bcopy(&ip->ip_src.s_addr, &saddr, sizeof(saddr));
	bcopy(&ip->ip_dst.s_addr, &daddr, sizeof(daddr));

	/*
	 * We don't do options, frags, or forwarding.
	 * Nothing left to do but pass it up to the next layer.
	 */
	switch (ip->ip_p) {
	    case 0x11:		/* UDP */
		udp_input(msg, (char *)(ip + 1), saddr, daddr);
		break;
	    default:
		printf("[ unknown protocol 0x%x ]\n", ip->ip_p);
		dump_packet(msg, stdout);
	}
}

/*
 * network output functions
 */

kern_return_t
ether_output(buf, data, dlen, daddr, saddr, ethertype, n)
	char *buf, *data;
	int dlen;
	u_long daddr, saddr;
	u_short ethertype;
	int	n;
{
	boolean_t intr;
	struct ether_header *eh;
	struct sockaddr_in sin;
	static char arp_eaddr[6];
	static u_long arp_iaddr;
	int i;

	assert((buf + sizeof(struct ether_header)) == data);
	eh = (struct ether_header *) buf;
	eh->ether_type = htons(ethertype);
	bcopy(ether_addr, eh->ether_shost, 6);
	
	/*
	 * Arp for dest address.  But first check the cache.
	 */

	if (arp_iaddr != daddr) {
		sin.sin_family = AF_INET;
		sin.sin_addr.s_addr = hostaddr.s_addr;

		/*
		 * NB: bsd_arp_resolve is an experimental UX message.
		 */
		while(bsd_arp_resolve(boot_port,
				      &intr, 
				      &sin, sizeof(sin),
				      daddr,
				      arp_eaddr))
			printf("arp cache: double miss on %x\n", daddr);
		arp_iaddr = daddr;
	}
	bcopy(arp_eaddr, eh->ether_dhost, 6);

	dlen += sizeof(struct ether_header);
#if DEBUG
	{
		int i;
		printf("[ outgoing packet ]\n");
		for (i = 0; i < dlen; i++) {
			printf("%2x ", buf[i] & 255);
			if ((i > 0) && ((i % 16) == 0))
				printf("\n");
		}
		printf("\n");
	}
#endif	

#ifdef notdef
	/*
	 * If you've hacked your kernel to have a device_write trap,
	 * then you can use this path.
	 */
	for (i = 0; i < n; i++)	{
        	syscall_device_write_request(ether_port, 0, 0, buf, dlen);
	}
#endif

#if 1
	/*
	 * Otherwise, you have to use mach ipc.
	 */
	if (dlen <= IO_INBAND_MAX)
	
		device_write_request_inband(ether_port,
					    MACH_PORT_NULL,
					    0, 0,
					    buf, dlen);

	else
		device_write_request(ether_port,
				     MACH_PORT_NULL,
				     0, 0,
				     buf, dlen);	
#endif	
	return KERN_SUCCESS;
}

unsigned short ip_id_counter = 1;
		 
kern_return_t
ip_output(buf, data, dlen, daddr, saddr, prot, n)
	char *buf, *data;
	int dlen;
	u_long daddr, saddr;
	u_char prot;
	int n;
{
    struct ip *ih;
    register unsigned long checksum, i;
    u_short len;

    ih = (struct ip *) (data - sizeof(struct ip));

    ih->ip_v	= 4;
    ih->ip_hl	= 5; /* 20 octets (default) */
    ih->ip_tos	= 0;

    len = (ih->ip_hl << 2) + dlen;
    ih->ip_len	= htons(len);
    ih->ip_id	= htons(ip_id_counter++);
    ih->ip_off	= 0;
    ih->ip_ttl	= 255;
    ih->ip_p	= prot;
    ih->ip_sum	= 0;
    bcopy(&saddr, &ih->ip_src, 4);
    bcopy(&daddr, &ih->ip_dst, 4);

    ih->ip_sum = ip_checksum(ih, (ih->ip_hl << 2), 0);

    return ether_output(buf, ih, len, daddr, saddr,
			0x0800, n); /* ethertype = IP */
}

/*
 * Send a udp packet down the protocol stack.
 * Called from a mig server function.
 */
kern_return_t
udp_output(data, dlen, daddr, dport, saddr, sport, n)
	char *data;
	int dlen;
	u_long daddr, saddr;
	short dport, sport;
	int n;
{
	char pkt[2048];
	char *off;
	int pktlen;
	struct udphdr *uh;
	
	if (saddr == 0)
		saddr = hostaddr.s_addr;
	
	/*
	 * Allocate packet buffer.
	 */
	pktlen = (dlen % 2 ? dlen + 1 : dlen)
		 + sizeof(struct udphdr)
		 + sizeof(struct ip)
		 + sizeof(struct ether_header);
	
	off = pkt + sizeof(struct ether_header)
		  + sizeof(struct ip)
		  + sizeof(struct udphdr);
	
	bcopy(data, off, dlen);
	
	uh = (struct udphdr *) (off - sizeof(struct udphdr));
	uh->uh_sport 	= sport;
	uh->uh_dport 	= dport;
	uh->uh_ulen  	= htons(dlen + sizeof(struct udphdr));

	/*
	 * Compute the UDP checksum.
	 */
	if (need_udp_checksum)
	  {
	    struct ip_phdr ipp;
	    u_long ck;

	    ipp.ipp_saddr  = saddr;
	    ipp.ipp_daddr  = daddr;
	    ipp.ipp_zero   = 0;
	    ipp.ipp_prot   = 0x11;
	    ipp.ipp_length = uh->uh_ulen;

	    ck = in_checksum(&ipp, sizeof(ipp), 0);
	    ck = in_checksum(&uh, dlen + sizeof(struct udphdr), ck);
	    uh->uh_sum = (~ck & 0xFFFF);
	  }
	else
	  uh->uh_sum = 0;

	return ip_output(pkt,(char *)uh, dlen+sizeof(struct udphdr),
			 daddr, saddr,
			 0x11, n);	/* protocol = UDP */
}
	


void
init_netstuff()
{
	struct sockaddr_in name;

	init_hostaddr();

	name.sin_family = AF_INET;
	name.sin_port = 0;
	bcopy(&hostaddr.s_addr, &name.sin_addr.s_addr, sizeof(u_long));
	
	get_ether_port(&name);
	set_packet_filter(&name);
}

void
network_input_thread(x)
	int x;
{
	struct net_rcv_msg msg;
	kern_return_t kr;
	
	/*cthread_wire();*/

	while(1) {
#if DEBUG
		printf("[ net thread %x : waiting for message ]\n",
		       cthread_self());
#endif
		kr = mach_msg(&msg.msg_hdr, MACH_RCV_MSG,
			      0, sizeof(msg), recv_port,
			      MACH_MSG_TIMEOUT_NONE, MACH_PORT_NULL);
		if (kr != KERN_SUCCESS) {
			fprintf(stderr, "mach_msg: error 0x%x\n", kr);
			exit(1);
		}
#if DEBUG
		printf("[ net thread %x : received packet ]\n",
		       cthread_self());
		dump_packet(&msg, stdout);
#endif
		ip_input(&msg);
		/*cthread_yield(); /* yield to delivery thread */
	}
}

