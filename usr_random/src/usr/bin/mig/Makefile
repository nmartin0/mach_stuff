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

# Makefile for MACH3.0+BSD4.3 /usr/bin/mig

USRBIN=	${INSTALL_TREE}/usr/bin
USRLIB=	${INSTALL_TREE}/usr/lib
MAN1=	${INSTALL_TREE}/usr/man/man1

CFLAGS=	-O -MD -traditional

OBJS= 	parser.o lexxer.o mig.o error.o string.o type.o routine.o \
	statement.o global.o header.o user.o server.o utils.o

GENS=	parser.c parser.h lexxer.c

compile: migcom

migcom:	${GENS} ${OBJS}
	cc ${OBJS} -o $@ -ll

parser.c parser.h: parser.y
	yacc -d parser.y && mv y.tab.c parser.c && mv y.tab.h parser.h

lexxer.c: lexxer.l
	lex lexxer.l &&	mv lex.yy.c lexxer.c

install:
	install -c -m 555 -o bin -g bin mig.sh ${USRBIN}/mig
	install -c -s -m 555 -o bin -g bin migcom ${USRLIB}/migcom

includes:

manpages:
	-install -c -m 444 -o bin -g bin mig.man ${MAN1}/mig.1

clean:
	rm -f ${GENS} migcom *.o *.d Makedep

.EXIT:
	@md -d -f -u Makedep *.d

-include Makedep
