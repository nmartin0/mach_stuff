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

# Makefile for MACH3.0+BSD4.3 /usr/ucb/Mail and /usr/ucb/fmt

USRUCB=	${INSTALL_TREE}/usr/ucb
USRLIB=	${INSTALL_TREE}/usr/lib
MAN1=	${INSTALL_TREE}/usr/man/man1

CFLAGS=	-O -MD

OBJS=	version.o aux.o cmd1.o cmd2.o cmd3.o cmdtab.o collect.o config.o \
	edit.o fio.o getname.o head.o v7.local.o lex.o list.o main.o \
	names.o optim.o popen.o quit.o send.o strings.o temp.o tty.o \
	vars.o str.o flsbuf.o

compile: Mail fmt

Mail:	${OBJS}
	cc ${OBJS} -o $@

fmt:	fmt.o xhead.o
	cc fmt.o xhead.o -o $@

.c.o:
	cc -E ${CFLAGS} $*.c | xstr -c -
	cc -c -o $*.o ${CFLAGS} x.c
	@rm -f x.c

version.o: version.c
	cc -c -R version.c

xhead.c:
	ln -s head.c xhead.c

xhead.o:
	cc -c ${CFLAGS} xhead.c

fmt.o:
	cc -c ${CFLAGS} fmt.c

str.o: strings
	xstr
	cc -R -c -o str.o xs.c
	rm -f xs.c

cmdtab.o: cmdtab.c
	cc -R -c ${CFLAGS} cmdtab.c

install:
	install -c -s -m 555 -o bin -g bin Mail ${USRUCB}
	install -c -s -m 555 -o bin -g bin fmt ${USRUCB}
	install -c -m 444 -o bin -g bin misc/Mail.* ${USRLIB}
	@rm -f ${USRUCB}/mail
	ln -s Mail ${USRUCB}/mail

includes:

manpages:
	install -c -m 444 -o bin -g bin fmt.man ${MAN1}/fmt.1
	install -c -m 444 -o bin -g bin mail.man ${MAN1}/mail.1
	@rm -f ${MAN1}/Mail.1
	ln -s mail.1 ${MAN1}/Mail.1

clean:
	rm -f Mail fmt strings *.o *.d xhead.c Makedep

.EXIT:
	@md -d -f -u Makedep *.d

-include Makedep
