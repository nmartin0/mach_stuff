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

# Makefile for MACH3.0+BSD4.3 /etc/dump and /etc/rdump

ETC=	${INSTALL_TREE}/etc
USRETC=	${INSTALL_TREE}/usr/etc
INCLUDE=${INSTALL_TREE}/usr/include
MAN8=	${INSTALL_TREE}/usr/man/man8

CFLAGS=	-O -MD

OBJS1=	dumpitime.o dumpmain.o dumpoptr.o dumptape.o dumptraverse.o \
	unctime.o

OBJS2=	dumpitime.o dumprmain.o dumpoptr.o dumprmt.o dumprtape.o \
	dumptraverse.o unctime.o

compile: dump rdump

dump:	${OBJS1}
	cc ${OBJS1} -o $@

rdump:	${OBJS2}
	cc ${OBJS2} -o $@

dumprmain.o: dumpmain.c
	cc -c -DRDUMP ${CFLAGS} dumpmain.c -o dumprmain.o

dumprtape.o: dumptape.c
	cc -c -DRDUMP ${CFLAGS} dumptape.c -o dumprtape.o

install:
	install -c -s -m 544 -o root -g bin dump ${USRETC}
	rm -f ${ETC}/dump; ln -s ../usr/etc/dump ${ETC}/dump
	install -c -s -m 544 -o root -g bin rdump ${USRETC}
	rm -f ${ETC}/rdump; ln -s ../usr/etc/rdump ${ETC}/rdump

includes:
	@install -p -v -c -m 444 -o bin -g bin dumprestore.h ${INCLUDE}/protocols

manpages:
	install -c -m 444 -o bin -g bin dump.man ${MAN8}/dump.8
	install -c -m 444 -o bin -g bin rdump.man ${MAN8}/rdump.8

clean:
	rm -f dump rdump *.o *.d Makedep

.EXIT:
	@md -d -f -u Makedep *.d

-include Makedep
