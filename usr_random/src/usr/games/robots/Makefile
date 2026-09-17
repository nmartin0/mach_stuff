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

# Makefile for MACH3.0+BSD4.3 /usr/games/robots

USRG=	${INSTALL_TREE}/usr/games
USRGHD= ${INSTALL_TREE}/usr/games/hide
MAN6=	${INSTALL_TREE}/usr/man/man6

CFLAGS=	-O -MD -DMAX_PER_UID=5

OBJS=	extern.o init_field.o main.o make_level.o move.o move_robs.o \
	play_level.o query.o rnd_pos.o score.o flush_in.o

compile: robots

robots:	${OBJS}
	cc ${OBJS} -o $@ -lcurses -ltermcap

install:
	install -c -s -m 540 -o games -g bin robots ${USRGHD}
	rm -f ${USRG}/robots; ln -s dm ${USRG}/robots

includes:

manpages:
	install -c -m 444 -o bin -g bin robots.man ${MAN6}/robots.6

clean:
	rm -f robots *.o *.d Makedep

.EXIT:
	@md -d -f -u Makedep *.d

-include Makedep
