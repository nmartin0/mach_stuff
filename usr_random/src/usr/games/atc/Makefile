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

# Makefile for MACH3.0+BSD4.3 /usr/games/atc

USRG=	${INSTALL_TREE}/usr/games
USRGHD= ${INSTALL_TREE}/usr/games/hide
USRGLB= ${INSTALL_TREE}/usr/games/lib
MAN6=	${INSTALL_TREE}/usr/man/man6

CFLAGS=	-O -MD -DBSD -DDEST=\"${USRGLB}/atc/\"

OBJS=	extern.o grammar.o input.o lex.o list.o log.o main.o tunable.o \
	graphics.o update.o

GENS=	grammar.c lex.c y.tab.h

compile: atc

atc:	${OBJS}
	cc ${OBJS} -o $@ -ll -lm -lcurses -ltermcap

grammar.c: grammar.y
	yacc -d grammar.y; mv y.tab.c grammar.c

install:
	install -c -s -m 540 -o games -g bin atc ${USRGHD}
	rm -f ${USRG}/atc; ln -s dm ${USRG}/atc
	-@[ -d ${USRGLB}/atc ] || mkdir ${USRGLB}/atc
	chown games.bin ${USRGLB}/atc
	chmod 750 ${USRGLB}/atc
	install -c -m 440 -o games -g bin games/* ${USRGLB}/atc

includes:

manpages:
	install -c -m 444 -o bin -g bin atc.man ${MAN6}/atc.6

clean:
	rm -f atc ${GENS} *.o *.d Makedep

.EXIT:
	@md -d -f -u Makedep *.d

-include Makedep
