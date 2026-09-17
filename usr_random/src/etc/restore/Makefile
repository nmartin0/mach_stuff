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

# Makefile for MACH3.0+BSD4.3 /etc/restore and /etc/rrestore

ETC=	${INSTALL_TREE}/etc
USRETC=	${INSTALL_TREE}/usr/etc
MAN8=	${INSTALL_TREE}/usr/man/man8

CFLAGS=	-O -MD

VPATH=	.:../dump

OBJS1=	main.o interactive.o restore.o dirs.o symtab.o utilities.o tape.o

OBJS2=	main.o interactive.o restore.o dirs.o symtab.o utilities.o rtape.o \
	dumprmt.o

compile: restore rrestore

restore: ${OBJS1}
	cc ${OBJS1} -o $@

rrestore: ${OBJS2}
	cc ${OBJS2} -o $@

rtape.o: tape.c
	cc ${CFLAGS} -c -DRRESTORE tape.c -o rtape.o

install:
	install -c -s -m 544 -o root -g bin restore ${USRETC}
	rm -f ${ETC}/restore; ln -s ../usr/etc/restore ${ETC}/restore
	install -c -s -m 544 -o root -g bin rrestore ${USRETC}
	rm -f ${ETC}/rrestore; ln -s ../usr/etc/rrestore ${ETC}/rrestore

includes:

manpages:
	install -c -m 444 -o bin -g bin restore.man ${MAN8}/restore.8
	install -c -m 444 -o bin -g bin rrestore.man ${MAN8}/rrestore.8

clean:
	rm -f restore rrestore *.o *.d Makedep

.EXIT:
	@md -d -f -u Makedep *.d

-include Makedep
