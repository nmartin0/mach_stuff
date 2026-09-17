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

# Makefile for MACH3.0+BSD4.3 /usr/bin/lex

USRBIN=	${INSTALL_TREE}/usr/bin
USRLIB=	${INSTALL_TREE}/usr/lib
MAN1=	${INSTALL_TREE}/usr/man/man1

CFLAGS=	-O -MD

OBJS=	lmain.o parser.o sub1.o sub2.o header.o

GENS=	parser.c

compile: lex

lex:	${GENS} ${OBJS}
	cc ${OBJS} -o $@

install:
	-@[ -d ${USRLIB}/lex ] || mkdir ${USRLIB}/lex
	chown bin.bin ${USRLIB}/lex
	chmod 555 ${USRLIB}/lex
	install -c -s -m 555 -o bin -g bin lex ${USRBIN}
	install -c -m 444 -o bin -g bin ncform ${USRLIB}/lex
	install -c -m 444 -o bin -g bin nrform ${USRLIB}/lex

includes:

manpages:
	install -c -m 444 -o bin -g bin lex.man ${MAN1}/lex.1

clean:
	rm -f ${GENS} lex *.o *.d Makedep

.EXIT:
	@md -d -f -u Makedep *.d

-include Makedep
