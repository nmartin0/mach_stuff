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

# Makefile for MACH3.0+BSD4.3 /usr/ucb/gprof

USRUCB=	${INSTALL_TREE}/usr/ucb
MAN1=	${INSTALL_TREE}/usr/man/man1
CRT0=	../../../lib/crt0/${TARGET}

CFLAGS=	-O -MD

compile: gprof

OBJS=	gprof.o arcs.o dfn.o lookup.o ${TARGET}.o hertz.o \
	printgprof.o printlist.o

gprof:	gcrt0.h ${OBJS}
	cc ${OBJS} -o $@

gcrt0.h:
	ln -s ${CRT0}/gmon.h gcrt0.h

install:
	install -c -s -m 555 -o bin -g bin gprof ${USRUCB}
	install -c -m 444 -o bin -g bin gprof.flat gprof.callg ${USRLIB}

includes:

manpages:
	install -c -m 444 -o bin -g bin gprof.man ${MAN1}/gprof.1

clean:
	rm -f gprof gcrt0.h *.o *.d Makedep

.EXIT:
	@md -d -f -u Makedep *.d

-include Makedep
