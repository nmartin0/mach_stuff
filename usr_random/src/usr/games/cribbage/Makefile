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

# Makefile for MACH3.0+BSD4.3 /usr/games/cribbage

USRG=	${INSTALL_TREE}/usr/games
USRGHD= ${INSTALL_TREE}/usr/games/hide
USRGLB= ${INSTALL_TREE}/usr/games/lib
MAN6=	${INSTALL_TREE}/usr/man/man6

CFLAGS=	-O -MD

OBJS1=	extern.o crib.o support.o cards.o score.o io.o

OBJS2=	test.o cards.o score.o io.o extern.o

compile: cribbage crib.instr

cribbage: ${OBJS1}
	cc ${OBJS1} -o $@ -lcurses -ltermcap

crib.instr: cribbage.n macro
	nroff cribbage.n > crib.instr

crib.test: ${OBJS2}
	cc ${OBJS2} -o $@ -lcurses -ltermcap

install:
	install -c -s -m 540 -o games -g bin cribbage ${USRGHD}
	rm -f ${USRG}/cribbage; ln -s dm ${USRG}/cribbage
	install -c -m 440 -o games -g bin crib.instr ${USRGLB}

includes:

manpages:
	install -c -m 444 -o bin -g bin cribbage.man ${MAN6}/cribbage.6

clean:
	rm -f cribbage *.o *.d Makedep

.EXIT:
	@md -d -f -u Makedep *.d

-include Makedep
