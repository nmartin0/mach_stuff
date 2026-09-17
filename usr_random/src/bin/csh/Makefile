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

# Makefile for MACH3.0+BSD4.3 /bin/csh

# C Shell with process control; VM/UNIX VAX Makefile
# Bill Joy UC Berkeley; Jim Kulp IIASA, Austria

BIN=	${INSTALL_TREE}/bin
MAN1=	${INSTALL_TREE}/usr/man/man1
CONFIG=	${INSTALL_TREE}/etc/newconfig

#CFLAGS= -O -MD -DTELL -DVFORK -DFILEC -DPROF -pg
CFLAGS= -O -MD -DTELL -DVFORK -DFILEC

OBJS=	alloc.o doprnt.o sh.o sh.char.o sh.dir.o sh.dol.o sh.err.o \
	sh.exec.o sh.exp.o sh.file.o sh.func.o sh.glob.o sh.hist.o \
	sh.init.o sh.lex.o sh.misc.o sh.parse.o sh.print.o sh.proc.o \
	sh.sem.o sh.set.o sh.time.o

compile: csh

# strings.o must be last since it can change when previous files compile
csh:	${OBJS} strings.o
	cc ${CFLAGS} ${OBJS} strings.o -o $@

.c.o:
	cc -E ${CFLAGS} $*.c | xstr -c -
	cc -c ${CFLAGS} x.c 
	mv -f x.o $*.o
	rm -f x.c

strings.o: strings
	xstr
	cc -c -R xs.c
	mv -f xs.o strings.o
	rm -f xs.c

sh.char.o sh.init.o:
	cc -E ${CFLAGS} $*.c | xstr -c -
	cc ${CFLAGS} -c -R x.c
	mv -f x.o $*.o
	rm -f x.c

install:
	install -c -s -m 555 -o bin -g bin csh ${BIN}
	install -c -o bin -g bin -m 444 csh.login ${CONFIG}/.login
	install -c -o bin -g bin -m 444 csh.cshrc ${CONFIG}/.cshrc

includes:

manpages:
	install -c -m 444 -o bin -g bin csh.man ${MAN1}/csh.1
	rm -f ${MAN1}/limit.1; ln -s csh.1 ${MAN1}/limit.1

clean:
	rm -f csh *.o *.d strings strings.o x.c xs.c errs Makedep

.EXIT:
	@md -d -f -u Makedep *.d

-include Makedep
