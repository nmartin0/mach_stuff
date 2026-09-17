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

# Makefile for MACH3.0+BSD4.3 /etc/named

ETC=	${INSTALL_TREE}/etc
USRETC=	${INSTALL_TREE}/usr/etc
INCLUDE=${INSTALL_TREE}/usr/include
MAN8=	${INSTALL_TREE}/usr/man/man8

CFLAGS=	-O -MD

OBJS=	db_dump.o db_load.o db_lookup.o db_reload.o db_save.o db_update.o \
	ns_forw.o ns_init.o ns_main.o ns_maint.o ns_req.o ns_resp.o \
	ns_sort.o ns_stats.o version.o

GENS=	version.c

compile: named

named:	${OBJS}
	cc ${OBJS} -o $@

version.c:
	-sh newvers.sh

install:
	install -c -s -m 544 -o root -g bin named ${USRETC}
	rm -f ${ETC}/named; ln -s ../usr/etc/named ${ETC}/named

includes:
	@install -p -v -c -m 444 -o bin -g bin nameser.h ${INCLUDE}/arpa

manpages:
	install -c -m 444 -o bin -g bin named.man ${MAN8}/named.8

clean:
	rm -f ${GENS} named *.o *.d Makedep

.EXIT:
	@md -d -f -u Makedep *.d

-include Makedep
