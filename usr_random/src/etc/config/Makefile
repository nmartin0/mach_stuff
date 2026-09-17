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

# Makefile for MACH3.0+BSD4.3 /usr/bin/config

ETC=	${INSTALL_TREE}/etc
USRETC=	${INSTALL_TREE}/usr/etc
MAN8=	${INSTALL_TREE}/usr/man/man8

CFLAGS=	-O -MD

OBJS=	y.tab.o main.o lex.yy.o mkioconf.o mkmakefile.o \
	mkglue.o mkheaders.o mkswapconf.o

GENS=	lex.yy.c y.tab.c y.tab.h

compile: config

config:	${GENS} ${OBJS}
	cc ${OBJS} -o $@ -ll -lcmucs

lex.yy.c: config.l
	lex config.l

y.tab.h y.tab.c: config.y
	yacc -d config.y

install:
	install -c -s -m 555 -o bin -g bin config ${USRETC}
	rm -f ${ETC}/config; ln -s ../usr/etc/config ${ETC}/config

includes:

manpages:
	-install -c -m 444 -o bin -g bin config.man ${MAN8}/config.8

clean:
	rm -f ${GENS} config *.o *.d Makedep

.EXIT:
	@md -d -f -u Makedep *.d

-include Makedep
