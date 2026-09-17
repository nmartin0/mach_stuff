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

# Makefile for MACH3.0+BSD4.3 /bin/sh

BIN=	${INSTALL_TREE}/bin
CONFIG=	${INSTALL_TREE}/etc/newconfig
MAN1=	${INSTALL_TREE}/usr/man/man1

CFLAGS=	-O -MD -w

OBJS=	setbrk.o builtin.o blok.o stak.o cmd.o fault.o main.o word.o \
	string.o name.o args.o xec.o service.o error.o io.o print.o \
	macro.o expand.o ctype.o msg.o

compile: sh

sh:	${OBJS}
	cc ${CFLAGS} ${OBJS} -o $@

install:
	install -c -s -m 555 -o bin -g bin sh ${BIN}
	install -c -m 444 -o bin -g bin sh.profile ${CONFIG}/.profile

includes:

manpages:
	install -c -m 444 -o bin -g bin sh.man ${MAN1}/sh.1
	rm -f ${MAN1}/case.1; ln -s sh.w ${MAN1}/case.1
	rm -f ${MAN1}/for.1; ln -s sh.w ${MAN1}/for.1
	rm -f ${MAN1}/if.1; ln -s sh.w ${MAN1}/if.1
	rm -f ${MAN1}/while.1; ln -s sh.w ${MAN1}/while.1

clean:
	rm -f sh *.o *.d Makedep

.EXIT:
	@md -d -f -u Makedep *.d

-include Makedep
