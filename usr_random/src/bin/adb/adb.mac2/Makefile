# MacMach Operating System
# Copyright (c) 1992 Carnegie Mellon University
# All Rights Reserved.
#
# MacMach was developed by CMU with support from Apple Computer, Inc.
# Use of this software is constrained by the MacMach End-User license.

# Makefile for MACH3.0+BSD4.3 /usr/src/bin/adb

BIN=	${INSTALL_TREE}/bin
MAN1=	${INSTALL_TREE}/usr/man/man1

CFLAGS= -O -MD -DM68881 -DVFORK -Dmach3

OBJS=	access.o command.o expr.o format.o input.o main.o message.o \
	opset.o optab.o output.o pcs.o print.o runpcs.o setup.o sym.o trace.o

compile: adb

adb:	${OBJS}
	cc ${OBJS} -o $@

message.o:
	cc -R ${CFLAGS} -c message.c

optab.o:
	cc -R ${CFLAGS} -c optab.c

install:
	install -c -s -m 555 -o bin -g bin adb ${BIN}/adb

includes:

manpages:
	install -c -o bin -g bin -m 444 adb.man ${MAN1}/adb.1

clean:
	rm -f adb *.o *.d Makedep

.EXIT:
	@md -d -f -u Makedep *.d

-include Makedep
