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

# Makefile for MACH3.0+BSD4.3 /lib/crt0.o, /lib/mcrt0.o and /usr/lib/gcrt0.o

# This version string will end up in every executable program.
VERSION= ${target_machine} MACH3.0/BSD4.3 version 01

LIB=	${INSTALL_TREE}/lib
USRLIB=	${INSTALL_TREE}/usr/lib
MAN3=	${INSTALL_TREE}/usr/man/man3

VPATH=	${target_machine}

compile: crt0.o mcrt0.o gcrt0.o

crt0.o: crt0.c
	cc -MD -DVERSION="\"${VERSION}\"" crt0.c -c -o x.o
	ld -x -r -o crt0.o x.o
	rm -f x.o

moncrt0.o: crt0.c
	cc -MD -DVERSION="\"${VERSION}\"" -DMCRT0 -c crt0.c -o moncrt0.o

mon.o: mon.c
	cc -MD -c mon.c -o x.o
	ld -x -r -o mon.o x.o
	rm -f x.o

gmon.o: gmon.c
	cc -MD -c gmon.c

gcrt0.o: moncrt0.o gmon.o
	ld -x -r -o gcrt0.o moncrt0.o gmon.o

mcrt0.o: moncrt0.o mon.o
	ld -x -r -o mcrt0.o moncrt0.o mon.o

install:
	install -c -o bin -g bin -m 644 crt0.o ${LIB}/crt0.o
	install -c -o bin -g bin -m 644 mcrt0.o ${LIB}/mcrt0.o
	install -c -o bin -g bin -m 644 gcrt0.o ${USRLIB}/gcrt0.o

includes:

manpages:
	-install -c -m 644 -o bin -g bin crt0.man ${MAN3}/crt0.3
	rm -f ${MAN3}/mcrt0.3; ln -s crt0.3 ${MAN3}/mcrt0.3
	rm -f ${MAN3}/gcrt0.3; ln -s crt0.3 ${MAN3}/gcrt0.3

clean:
	rm -f *.o *.d Makedep

.EXIT:
	@md -d -f -u Makedep *.d

-include Makedep
