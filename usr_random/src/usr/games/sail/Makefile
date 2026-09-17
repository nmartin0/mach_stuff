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

# Makefile for MACH3.0+BSD4.3 /usr/games/sail

USRG=	${INSTALL_TREE}/usr/games
USRGHD= ${INSTALL_TREE}/usr/games/hide
MAN6=	${INSTALL_TREE}/usr/man/man6

CFLAGS=	-O -MD

OBJS=	main.o pl_main.o pl_1.o pl_2.o pl_3.o pl_4.o pl_5.o pl_6.o pl_7.o \
	dr_main.o dr_1.o dr_2.o dr_3.o dr_4.o dr_5.o lo_main.o \
	assorted.o game.o globals.o misc.o parties.o sync.o version.o

compile: sail

sail:	${OBJS}
	cc ${OBJS} -o $@ -lcurses -ltermcap

install:
	install -c -s -m 540 -o games -g bin sail ${USRGHD}
	rm -f ${USRG}/sail; ln -s dm ${USRG}/sail

includes:

manpages:
	install -c -m 444 -o bin -g bin sail.man ${MAN6}/sail.6

clean:
	rm -f sail *.o *.d Makedep

.EXIT:
	@md -d -f -u Makedep *.d

-include Makedep
