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

# Makefile for MACH3.0+BSD4.3 /usr/bin/xsend

USRBIN=	${INSTALL_TREE}/usr/bin
MAN1=	${INSTALL_TREE}/usr/man/man1

CFLAGS=	-O -MD

SRCS=	enroll.c lib.c xget.c xsend.c
OBJS=	enroll.o lib.o xget.o xsend.o

compile: enroll xsend xget

enroll: enroll.o lib.o
	cc enroll.o lib.o -lmp -o $@

xget: xget.o lib.o
	cc xget.o lib.o -lmp -o $@

xsend: xsend.o lib.o
	cc xsend.o lib.o -lmp -o $@

install:
	install -c -s -o bin -g bin -m 755 enroll ${USRBIN}/enroll
	install -c -s -o bin -g bin -m 755 xget ${USRBIN}/xget
	install -c -s -o bin -g bin -m 755 xsend ${USRBIN}/xsend

includes:

manpages:
	install -c -o bin -g bin -m 444 xsend.man ${MAN1}/xsend.1
	rm -f ${MAN1}/enroll.1; ln -s xsend.1 ${MAN1}/enroll.1
	rm -f ${MAN1}/xget.1; ln -s xsend.1 ${MAN1}/xget.1

clean:
	rm -f ${OBJS} enroll xsend xget Makedep

.EXIT:
	@md -d -f -u Makedep *.d

-include Makedep
