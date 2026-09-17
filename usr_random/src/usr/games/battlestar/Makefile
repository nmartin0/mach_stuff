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

# Makefile for MACH3.0+BSD4.3 /usr/games/battlestar

USRG=	${INSTALL_TREE}/usr/games
USRGHD= ${INSTALL_TREE}/usr/games/hide
MAN6=	${INSTALL_TREE}/usr/man/man6

CFLAGS=	-O -MD

OBJS=	battlestar.o com1.o com2.o com3.o com4.o com5.o com6.o com7.o \
	init.o cypher.o getcom.o parse.o room.o save.o fly.o misc.o \
	globals.o dayfile.o nightfile.o dayobjs.o nightobjs.o words.o

compile: battlestar

battlestar: ${OBJS}
	cc ${OBJS} -o $@ -lcurses -ltermcap

install:
	install -c -s -m 540 -o games -g bin battlestar ${USRGHD}
	rm -f ${USRG}/battlestar; ln -s dm ${USRG}/battlestar

includes:

manpages:
	install -c -m 444 -o bin -g bin battlestar.man ${MAN6}/battlestar.6

clean:
	rm -f battlestar *.o *.d Makedep

.EXIT:
	@md -d -f -u Makedep *.d

-include Makedep
