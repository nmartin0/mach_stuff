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

# Makefile for MACH3.0+BSD4.3 /usr/games/rogue

USRG=	${INSTALL_TREE}/usr/games
USRGHD= ${INSTALL_TREE}/usr/games/hide
MAN6=	${INSTALL_TREE}/usr/man/man6

CFLAGS=	-O -MD -DUNIX -DUNIX_BSD4_2

OBJS=	curses.o hit.o init.o inventory.o level.o machdep.o main.o \
	message.o monster.o move.o object.o pack.o play.o random.o ring.o \
	room.o save.o score.o spec_hit.o throw.o trap.o use.o zap.o

compile: rogue

rogue:	${OBJS}
	cc ${OBJS} -o $@ -lcurses -ltermcap

install:
	install -c -s -m 540 -o games -g bin rogue ${USRGHD}
	rm -f ${USRG}/rogue; ln -s dm ${USRG}/rogue

includes:

manpages:
	install -c -m 444 -o bin -g bin rogue.man ${MAN6}/rogue.6

clean:
	rm -f rogue *.o *.d Makedep

.EXIT:
	@md -d -f -u Makedep *.d

-include Makedep
