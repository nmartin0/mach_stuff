# Mach Operating System
# Copyright (c) 1992 Carnegie Mellon University
# All Rights Reserved.
# 
# Permission to use, copy, modify and distribute this software and its
# documentation is hereby granted, provided that both the copyright
# notice and this permission notice appear in all copies of the
# software, derivative works or modified versions, and any portions
# thereof, and that both notices appear in supporting documentation.
# 
# CARNEGIE MELLON ALLOWS FREE USE OF THIS SOFTWARE IN ITS "AS IS"
# CONDITION.  CARNEGIE MELLON DISCLAIMS ANY LIABILITY OF ANY KIND FOR
# ANY DAMAGES WHATSOEVER RESULTING FROM THE USE OF THIS SOFTWARE.
# 
# Carnegie Mellon requests users of this software to return to
# 
#  Software Distribution Coordinator  or  Software.Distribution@CS.CMU.EDU
#  School of Computer Science
#  Carnegie Mellon University
#  Pittsburgh PA 15213-3890
# 
# any improvements or extensions that they make and grant Carnegie Mellon 
# the rights to redistribute these changes.

# Makefile for MACH3.0+BSD4.3 /usr/src/bin/adb/scripts

SCRIPTDIR= ${INSTALL_TREE}/usr/lib/adb

SCRIPTS= buf callout callout.next clist clist.nxt dino dir dir.nxt \
	dmap file filsys findinode findinode.nxt findproc host.nxt hosts \
	hosts.nxt hosttable idp ifaddr ifnet ifrw imp in_ifaddr inode inpcb \
	iovec ipreass ipreass.nxt mact mact.nxt mbstat mbuf mbuf.nxt \
	mbufchain mbufs mbufs.nxt mount nameidata nspcb nsrtentry packetchain \
	protosw quota rawcb route rtentry rusage setproc setproc.done socket \
	spp sppcb stat tcpcb tcpip tcpreass tcpreass.nxt text traceall \
	traceall.nxt u u.all ubadev uio un unpcb urppcb vtimes

compile:

install:
	-[ -d ${SCRIPTDIR} ] || mkdir ${SCRIPTDIR}
	chown bin.bin ${SCRIPTDIR}
	chmod 555 ${SCRIPTDIR}
	rm -rf ${SCRIPTDIR}/*
	install -c -o bin -g bin -m 444 ${SCRIPTS} ${SCRIPTDIR}

includes:

manpages:

clean:
