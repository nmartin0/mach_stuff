#
# Copyright (c) 1991 The Regents of the University of California.
# All rights reserved.
#
# Redistribution and use in source and binary forms are permitted provided
# that: (1) source distributions retain this entire copyright notice and
# comment, and (2) distributions including binaries display the following
# acknowledgement:  ``This product includes software developed by the
# University of California, Berkeley and its contributors'' in the
# documentation or other materials provided with the distribution and in
# all advertising materials mentioning features or use of this software.
# Neither the name of the University nor the names of its contributors may
# be used to endorse or promote products derived from this software without
# specific prior written permission.
# THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR IMPLIED
# WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTIES OF
# MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
#
#	@(#)Makefile.generic	5.5 (Berkeley) 3/1/91
#

SRCS=	commands.c main.c network.c ring.c \
	sys_bsd.c telnet.c terminal.c \
	utilities.c genget.c ${GETOPT_SRC}

CC = gcc 
CFLAGS = -g -O ${INCLUDES} ${DEFINES} -DTERMCAP 

ALLHC=	${SRCS} \
	defines.h externs.h fdset.h general.h \
	ring.h types.h

OBJS=	authenc.o commands.o main.o network.o ring.o sys_bsd.o \
	telnet.o terminal.o utilities.o genget.o ${GETOPT_OBJ}
MAN=	telnet.0

LIBS = -ltermcap -lsockets -lthreads -lmach

#
# These next three lines are not needed in 4.4BSD
#
.SUFFIXES: .0 .1
.1.0:
	nroff -man -h $< > $@

all: telnet

telnet:	${OBJS} ${LIBPATH}
	${CC} -o $@ ${CFLAGS} ${OBJS} ${LIBS}

clean: FRC
	rm -f ${OBJS} core errs l.errs telnet netstats* *~

cleandir: clean
	rm -f ${MAN} tags .depend

clist:	FRC ${SRCS}
	@for i in ${SRCS} ; \
		do (echo ${DIRPATH}$$i); done

hclist:	FRC ${ALLHC}
	@for i in ${ALLHC} ; \
		do (echo ${DIRPATH}$$i); done

depend: FRC ${SRCS}
	mkdep ${CFLAGS} `make clist`

install: ${MAN} FRC
	install -s -o bin -g bin -m 755 telnet ${DEST}
	install -c -o bin -g bin -m 444 telnet.0 ${DESTDIR}/usr/man/cat1

lint: FRC ${SRCS}
	lint ${CFLAGS} `make clist`

tags: FRC ${SRCS}
	etags `make hclist`

FRC:

#.EXIT:
#	md -d -u Makedep *.d

-include Makedep

