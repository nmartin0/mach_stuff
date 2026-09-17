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

# Makefile for MACH3.0+BSD4.3 /usr/games/backgammon

USRG=	${INSTALL_TREE}/usr/games
USRGHD= ${INSTALL_TREE}/usr/games/hide
MAN6=	${INSTALL_TREE}/usr/man/man6

CFLAGS=	-O -MD -DV7

OBJS1=	allow.o board.o check.o extra.o fancy.o init.o main.o move.o \
	odds.o one.o save.o subs.o table.o text.o version.o

OBJS2=	allow.o board.o check.o data.o fancy.o init.o odds.o one.o save.o \
	subs.o table.o teach.o ttext1.o ttext2.o tutor.o

compile: backgammon teachgammon

backgammon: ${OBJS1}
	cc ${OBJS1} -o $@ -ltermlib

teachgammon: ${OBJS2}
	cc ${OBJS2} -o $@ -ltermlib

install:
	install -c -s -m 540 -o games -g bin backgammon ${USRGHD}
	rm -f ${USRG}/backgammon; ln -s dm ${USRG}/backgammon
	install -c -s -m 540 -o games -g bin teachgammon ${USRGHD}
	rm -f ${USRG}/teachgammon; ln -s dm ${USRG}/teachgammon

includes:

manpages:
	install -c -m 444 -o bin -g bin backgammon.man ${MAN6}/backgammon.6
	rm -f ${MAN6}/teachgammon.6; ln -s backgammon.6 ${MAN6}/teachgammon.6

clean:
	rm -f backgammon teachgammon *.o *.d Makedep

.EXIT:
	@md -d -f -u Makedep *.d

-include Makedep
