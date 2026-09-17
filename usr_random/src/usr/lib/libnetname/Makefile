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

# Makefile for MACH3.0+BSD4.3 /usr/lib/libnetname.a

USRLIB=	${INSTALL_TREE}/usr/lib
INCLUDE=${INSTALL_TREE}/usr/include
MAN3=	${INSTALL_TREE}/usr/man/man3

CFLAGS=	-O -MD

SERVERS_INCS= netname.h netname_defs.h

OBJS=	netname_user.o netname_server.o

GENS=	netname_user.c netname_server.c netname.h

compile: libnetname.a

libnetname.a: ${GENS} ${OBJS}
	ar cu $@ ${OBJS}

netname_user.c netname_server.c netname.h: netname.defs
	mig \
	  -user netname_user.c \
	  -header netname.h \
	  -server netname_server.c \
	  $?

install:
	install -c -m 444 -o bin -g bin libnetname.a ${USRLIB}
	ranlib ${USRLIB}/libnetname.a

includes: ${INCLUDE}/servers/netname_defs.h ${SERVERS_INCS}
	@install -p -v -c -m 444 -o bin -g bin ${SERVERS_INCS} ${INCLUDE}/servers

${INCLUDE}/servers/netname_defs.h:
	@install -p -v -c -m 444 -o bin -g bin netname_defs.h ${INCLUDE}/servers

manpages:
	-install -c -m 444 -o bin -g bin libnetname.man ${MAN3}/libnetname.3

clean:
	rm -f ${GENS} ${OBJS} libnetname.a Makedep

.EXIT:
	@md -d -f -u Makedep *.d

-include Makedep
