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

# Makefile for MACH3.0+BSD4.3 /usr/bin/neqn

USRBIN=	${INSTALL_TREE}/usr/bin
MAN1=	${INSTALL_TREE}/usr/man/man1

COMMON=	../common_source

CFLAGS=	-O -MD -DNEQN -I. -I${COMMON}

YFLAGS=	-d

VPATH=	${COMMON}

OBJS=	e.o diacrit.o eqnbox.o font.o fromto.o funny.o glob.o integral.o \
	io.o lex.o lookup.o mark.o matrix.o move.o over.o paren.o pile.o \
	shift.o size.o sqrt.o text.o

compile: neqn

neqn:	${OBJS}
	cc ${OBJS} -o $@

e.c:	${COMMON}/e.y
	yacc ${YFLAGS} ${COMMON}/e.y
	mv y.tab.h e.def
	mv y.tab.c e.c

install:
	install -c -s -m 555 -o bin -g bin neqn ${USRBIN}

includes:

manpages:

clean:
	rm -f neqn e.c e.def *.o *.d Makedep

.EXIT:
	md -d -f -u Makedep *.d

-include Makedep
