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

# Makefile for MACH3.0+BSD4.3 /usr/ucb/expand and /usr/ucb/unexpand

USRUCB=	${INSTALL_TREE}/usr/ucb
MAN1=	${INSTALL_TREE}/usr/man/man1

CFLAGS=	-O -MD

compile: expand unexpand

expand:	expand.o
	cc $? -o $@

unexpand: unexpand.o
	cc $? -o $@

install:
	install -c -s -m 555 -o bin -g bin expand ${USRUCB}
	install -c -s -m 555 -o bin -g bin unexpand ${USRUCB}

includes:

manpages:
	install -c -m 444 -o bin -g bin expand.man ${MAN1}/expand.1
	rm -f ${MAN1}/unexpand.1; ln -s expand.1 ${MAN1}/unexpand.1

clean:
	rm -f expand unexpand *.o *.d Makedep

.EXIT:
	@md -d -f -u Makedep *.d

-include Makedep
