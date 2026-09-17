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

# Makefile for MACH3.0+BSD4.3 /usr/lib/libservice.a

USRLIB=	${INSTALL_TREE}/usr/lib
INCLUDE=${INSTALL_TREE}/usr/include
MAN3=	${INSTALL_TREE}/usr/man/man3

CFLAGS=	-O -MD

SERVERS_INCS= service.h

OBJS=	service_user.o service_server.o

GENS=	service_user.c service_server.c service.h

compile: libservice.a

libservice.a: ${GENS} ${OBJS}
	ar cu $@ ${OBJS}

service_user.c service_server.c service.h: service.defs
	mig \
	  -user service_user.c \
	  -header service.h \
	  -server service_server.c \
	  $?

install:
	install -c -m 444 -o bin -g bin libservice.a ${USRLIB}
	ranlib ${USRLIB}/libservice.a

includes: ${SERVERS_INCS}
	@install -p -v -c -m 444 -o bin -g bin ${SERVERS_INCS} ${INCLUDE}/servers

manpages:
	-install -c -m 444 -o bin -g bin libservice.man ${MAN3}/libservice.3

clean:
	rm -f ${GENS} ${OBJS} libservice.a Makedep

.EXIT:
	@md -d -f -u Makedep *.d

-include Makedep
