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

# Makefile for MACH3.0+BSD4.3 /usr/games/hangman

USRG=	${INSTALL_TREE}/usr/games
USRGHD= ${INSTALL_TREE}/usr/games/hide
MAN6=	${INSTALL_TREE}/usr/man/man6

CFLAGS=	-O -MD

OBJS=	endgame.o extern.o getguess.o getword.o main.o playgame.o \
	prdata.o prman.o prword.o setup.o

compile: hangman

hangman: ${OBJS}
	cc ${OBJS} -o $@ -lcurses -ltermcap

install:
	install -c -s -m 540 -o games -g bin hangman ${USRGHD}
	rm -f ${USRG}/hangman; ln -s dm ${USRG}/hangman

includes:

manpages:
	install -c -m 444 -o bin -g bin hangman.man ${MAN6}/hangman.6

clean:
	rm -f hangman *.o *.d Makedep

.EXIT:
	@md -d -f -u Makedep *.d

-include Makedep
