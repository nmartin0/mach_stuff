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

# Makefile for MACH3.0+BSD4.3 /usr/bin/at, /usr/bin/atq, /usr/bin/atrm

USRBIN=	${INSTALL_TREE}/usr/bin
USRLIB=	${INSTALL_TREE}/usr/lib
MAN1=	${INSTALL_TREE}/usr/man/man1
SPOOLDIRS= ${INSTALL_TREE}/usr/spool/at ${INSTALL_TREE}/usr/spool/at/past
SPOOLFILS= ${INSTALL_TREE}/usr/spool/at/lasttimedone

CFLAGS=	-O -MD

compile: atq atrm at atrun

atq:	atq.o
	cc $? -o $@

atrm:	atrm.o
	cc $? -o $@

at:	at.o
	cc $? -o $@

atrun:	atrun.o
	cc $? -o $@

install:
	-for i in ${SPOOLDIRS}; do \
	  [ -d $$i ] || mkdir $$i; \
	  chown bin.daemon $$i; \
	  chmod 575 $$i; \
	done
	-@for i in ${SPOOLFILS}; do \
	  [ -f $$i ] || echo 0 > $$i; \
	  chown bin.daemon $$i; \
	  chown 464 $$i; \
	done
	install -c -s -m 555 -o bin -g bin atrun ${USRLIB}
	install -c -s -m 2555 -o bin -g daemon atq ${USRBIN}
	install -c -s -m 2555 -o bin -g daemon at ${USRBIN}
	install -c -s -m 2555 -o bin -g daemon atrm ${USRBIN}

includes:

manpages:
	install -c -m 444 -o bin -g bin at.man ${MAN1}/at.1

clean:
	rm -f atq atrm at atrun  *.o *.d Makedep

.EXIT:
	@md -d -f -u Makedep *.d

-include Makedep
