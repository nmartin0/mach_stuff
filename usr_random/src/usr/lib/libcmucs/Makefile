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

# Makefile for MACH3.0+BSD4.3 /usr/lib/libcmucs.a

USRLIB=	${INSTALL_TREE}/usr/lib
MAN2=	${INSTALL_TREE}/usr/man/man2
MAN3=	${INSTALL_TREE}/usr/man/man3

ASFLAGS= -MD
CFLAGS=	-O -MD

OBJS=	table.o atoh.o getname.o openp.o searchp.o quit.o

VPATH=	.:${target_machine}

compile: libcmucs.a

libcmucs.a: ${OBJS}
	ar cu $@ ${OBJS}

install:
	install -c -m 755 -o bin -g bin libcmucs.a ${USRLIB}
	ranlib ${USRLIB}/libcmucs.a

includes:

manpages:
	install -c -m 644 -o bin -g bin table.man ${MAN2}/table.2
	install -c -m 644 -o bin -g bin atoh.man ${MAN3}/atoh.3
	install -c -m 644 -o bin -g bin getname.man ${MAN3}/getname.3
	install -c -m 644 -o bin -g bin openp.man ${MAN3}/openp.3
	install -c -m 644 -o bin -g bin quit.man ${MAN3}/quit.3
	install -c -m 644 -o bin -g bin searchp.man ${MAN3}/searchp.3

clean:
	rm -f libcmucs.a *.o *.d Makedep

.EXIT:
	@md -d -f -u Makedep *.d

-include Makedep
