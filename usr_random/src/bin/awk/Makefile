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

# Makefile for MACH3.0+BSD4.3 /bin/awk

BIN=	${INSTALL_TREE}/bin
MAN1=	${INSTALL_TREE}/usr/man/man1

CFLAGS=	-O -MD

OBJS=	awk.g.o awk.lx.o b.o main.o token.o tran.o lib.o run.o parse.o \
	proctab.o freeze.o

GENS=	awk.h awk.g.c awk.lx.c proctab.c token.c

compile: awk

awk:	${GENS} ${OBJS}
	cc ${OBJS} -o $@ -lm

awk.g.c awk.h: awk.g.y
	yacc -d awk.g.y && mv y.tab.c awk.g.c && mv y.tab.h awk.h

token.c: awk.h tokenscript
	ex - < tokenscript

proctab.c: proc
	./proc > proctab.c

proc:	awk.h proc.o token.o
	cc -o $@ proc.o token.o

install:
	install -c -s -m 555 -o bin -g bin awk ${BIN}/awk

includes:

manpages:
	install -c -m 444 -o bin -g bin awk.man ${MAN1}/awk.1

clean:
	rm -f awk proc ${GENS} temp *.o *.d Makedep

.EXIT:
	@md -d -f -u Makedep *.d

-include Makedep
