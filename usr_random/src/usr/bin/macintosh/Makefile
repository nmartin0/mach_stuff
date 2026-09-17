# MacMach Operating System
# Copyright (c) 1992 Carnegie Mellon University
# All Rights Reserved.
#
# MacMach was developed by CMU with support from Apple Computer, Inc.
# Use of this software is constrained by the MacMach End-User license.

# Makefile for MACH3.0+BSD4.3 /usr/bin/macintosh

USRBIN=	${INSTALL_TREE}/usr/bin
MAN1=	${INSTALL_TREE}/usr/man/man1

CFLAGS=	-O -MD

OBJS=	catch_exception.o debug.o main.o console_fix.o opset.o \
	pager_mappings.o pager_routines.o pager_support.o server.o \
	signal.o memory_object_change_completed.o macserver_user.o

GENS=	macserver_user.c macserver.h

compile: macintosh
	cd macpatches; make compile

macintosh: ${GENS} ${OBJS}
	cc ${OBJS} -o $@ -lcmucs -lthreads -lmach

macserver_user.c macserver.h: macserver.defs
	mig -user macserver_user.c -header macserver.h -server /dev/null $?

install:
	install -c -s -m 555 -o bin -g bin macintosh ${USRBIN}
	cd macpatches; make install

includes:
	cd macpatches; make includes

manpages:
	-install -c -m 444 -o bin -g bin macintosh.man ${MAN1}/macintosh.1
	cd macpatches; make manpages

clean:
	rm -f macintosh ${GENS} *.o *.d Makedep
	cd macpatches; make clean

.EXIT:
	@md -d -f -u Makedep *.d

-include Makedep
