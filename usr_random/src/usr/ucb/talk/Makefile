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

# Makefile for MACH3.0+BSD4.3 /usr/ucb/talk

USRUCB=	${INSTALL_TREE}/usr/ucb
MAN1=	${INSTALL_TREE}/usr/man/man1

CFLAGS=	-O -MD

OBJS=	talk.o get_names.o display.o io.o ctl.o init_disp.o \
	msgs.o get_addrs.o ctl_transact.o invite.o look_up.o

compile: talk

talk:	${OBJS}
	cc ${OBJS} -o $@ -lcurses -ltermcap

install:
	install -c -s -m 555 -o bin -g bin talk ${USRUCB}

includes:

manpages:
	install -c -m 444 -o bin -g bin talk.man ${MAN1}/talk.1

clean:
	rm -f talk *.o *.d Makedep

.EXIT:
	@md -d -f -u Makedep *.d

-include Makedep
