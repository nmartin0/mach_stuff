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

# Makefile for MACH3.0+BSD4.3 /usr/bin/find

USRBIN=	${INSTALL_TREE}/usr/bin
USRLIB=	${INSTALL_TREE}/usr/lib
MAN1=	${INSTALL_TREE}/usr/man/man1

CFLAGS=	-O -MD

compile: find bigram code

find:	find.o
	cc $? -o $@

bigram:	bigram.o
	cc $? -o $@

code:	code.o
	cc $? -o $@

install:
	-@[ -d ${USRLIB}/find ] || mkdir ${USRLIB}/find
	@chown bin.bin ${USRLIB}/find
	@chmod 555 ${USRLIB}/find
	install -c -s -m 555 -o bin -g bin find ${USRBIN}
	install -c -s -m 555 -o bin -g bin bigram ${USRLIB}/find
	install -c -s -m 555 -o bin -g bin code ${USRLIB}/find

includes:

manpages:
	install -c -m 444 -o bin -g bin find.man ${MAN1}/find.1

clean:
	rm -f find bigram code *.o *.d Makedep

.EXIT:
	@md -d -f -u Makedep *.d

-include Makedep
