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

# Makefile for MACH3.0+BSD4.3 /usr/ucb/lpr, etc...

# Ownerships {see note at the top of lpd.c}
# ROOT		a user that can access any and all files
# DAEMON	someone special
# SPGRP		the group id of the spooling programs

ETC=	${INSTALL_TREE}/etc
USRETC=	${INSTALL_TREE}/usr/etc
USRLIB=	${INSTALL_TREE}/usr/lib
USRUCB=	${INSTALL_TREE}/usr/ucb
USRSPL=	${INSTALL_TREE}/usr/spool

CFLAGS=	-O -MD

ROOT=	root
DAEMON=	daemon
SPGRP=	daemon

L1SRCS=	lpd.c printjob.c recvjob.c displayq.c rmjob.c startdaemon.c \
	lpdchar.c common.c printcap.c
L1OBJS=	lpd.o printjob.o recvjob.o displayq.o rmjob.o startdaemon.o \
	lpdchar.o common.o printcap.o

L2SRCS=	lpr.c startdaemon.c printcap.c
L2OBJS=	lpr.o startdaemon.o printcap.o

L3SRCS=	lpq.c displayq.c common.c printcap.c
L3OBJS=	lpq.o displayq.o common.o printcap.o

L4SRCS=	lprm.c rmjob.c startdaemon.c common.c printcap.c
L4OBJS=	lprm.o rmjob.o startdaemon.o common.o printcap.o

L5SRCS=	lpc.c cmds.c cmdtab.c startdaemon.c common.c printcap.c
L5OBJS=	lpc.o cmds.o cmdtab.o startdaemon.o common.o printcap.o

L6SRCS=	lptest.c
L6OBJS=	lptest.o

L7SRCS=	pac.c printcap.c
L7OBJS=	pac.o printcap.o

OBJS=	${L1OBJS} ${L2OBJS} ${L3OBJS} ${L4OBJS} ${L5OBJS} ${L6OBJS} ${L7OBJS}

STD=	lpd lpr lpq lprm lpc lptest pac

compile: ${STD} FILTERS VFILTERS

lpd:	${L1OBJS}
	cc -o $@ ${L1OBJS}

lpr:	${L2OBJS}
	cc -o $@ ${L2OBJS}

lpq:	${L3OBJS}
	cc -o $@ ${L3OBJS}

lprm:	${L4OBJS}
	cc -o $@ ${L4OBJS}

lpc:	${L5OBJS}
	cc -o $@ ${L5OBJS}

lptest: ${L6OBJS}
	cc ${CFLAGS} -o $@ ${L6OBJS}

pac:	${L7OBJS}
	cc -o $@ ${L7OBJS}

FILTERS:
	cd filters; make

VFILTERS:
	cd vfilters; make

install:
	install -c -s -o ${ROOT} -g ${SPGRP} -m 6755 lpd ${USRLIB}
	install -c -s -o ${ROOT} -g ${SPGRP} -m 6755 lpr ${USRUCB}
	install -c -s -o ${ROOT} -g ${SPGRP} -m 6755 lpq ${USRUCB}
	install -c -s -o ${ROOT} -g ${SPGRP} -m 6755 lprm ${USRUCB}
	install -c -s -o bin -g ${SPGRP} -m 2755 lpc ${USRETC}
	rm -f ${ETC}/lpc; ln -s ../usr/etc/lpc ${ETC}/lpc
	install -c -s -o bin -g bin -m 755 lptest ${USRUCB}
	install -c -s -o bin -g bin -m 755 pac ${USRETC}
	rm -f ${ETC}/pac; ln -s ../usr/etc/pac ${ETC}/pac
	-[ -d ${USRSPL}/lpd ] || mkdir ${USRSPL}/lpd
	chown ${DAEMON} ${USRSPL}/lpd
	chgrp ${SPGRP} ${USRSPL}/lpd
	chmod 775 ${USRSPL}/lpd
	cd filters; make ${MFLAGS} DESTDIR=${INSTALL_TREE} install
	cd vfilters; make ${MFLAGS} DESTDIR=${INSTALL_TREE} install

includes:

manpages:

clean:
	rm -f ${OBJS} core ${STD} Makedep
	cd filters; make clean
	cd vfilters; make clean

tags:
	ctags ${L1SRCS}
	ctags -a ${L2SRCS}
	ctags -a ${L3SRCS}
	ctags -a ${L4SRCS}
	ctags -a ${L5SRCS}
	ctags -a ${L6SRCS}
	ctags -a ${L7SRCS}
	sort -o tags tags

.EXIT:
	@md -d -f -u Makedep *.d

-include Makedep
