
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

# Makefile for MACH3.0+BSD4.3 /usr/lib/libmachid.a

USRLIB=	${INSTALL_TREE}/usr/lib
INCLUDE=${INSTALL_TREE}/usr/include
MAN3=	${INSTALL_TREE}/usr/man/man3

CFLAGS=	-O -MD

SERVERS_INCS= machid.h machid_types.h machid_debug.h

OBJS=	machid_user.o machid_server.o machid_lib.o \
	machid_debug_user.o machid_debug_server.o

GENS=	machid_user.c machid_server.c machid.h \
	machid_debug_user.c machid_debug_server.c machid_debug.h

compile: libmachid.a

libmachid.a: ${GENS} ${OBJS}
	ar cu $@ ${OBJS}

machid_user.c machid_server.c machid.h: machid.defs
	mig \
	  -user machid_user.c \
	  -header machid.h \
	  -server machid_server.c \
	  $?

machid_debug_user.c machid_debug_server.c machid_debug.h: machid_debug.defs
	mig \
	  -user machid_debug_user.c \
	  -header machid_debug.h \
	  -server machid_debug_server.c \
	  $?

install:
	install -c -m 444 -o bin -g bin libmachid.a ${USRLIB}
	ranlib ${USRLIB}/libmachid.a

includes: ${SERVERS_INCS}
	@install -p -v -c -m 444 -o bin -g bin ${SERVERS_INCS} ${INCLUDE}/servers

manpages:
	-install -c -m 444 -o bin -g bin libmachid.man ${MAN3}/libmachid.3

clean:
	rm -f ${GENS} ${OBJS} libmachid.a Makedep

.EXIT:
	@md -d -f -u Makedep *.d

-include Makedep
