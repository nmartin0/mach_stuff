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

# Makefile for MACH3.0+BSD4.3 /etc/inetd

ETC=	${INSTALL_TREE}/etc
USRETC=	${INSTALL_TREE}/usr/etc
INCLUDE=${INSTALL_TREE}/usr/include
CONFIG= ${INSTALL_TREE}/etc/newconfig
MAN8=	${INSTALL_TREE}/usr/man/man8

compile: inetd

inetd:	inetd.o
	cc $? -o $@

install:
	install -c -s -m 755 -o bin -g bin inetd ${USRETC}
	rm -f ${ETC}/inetd; ln -s ../usr/etc/inetd ${ETC}/inetd
	install -c -o bin -g bin -m 444 inetd.conf ${CONFIG}

includes:
	@install -p -v -c -m 444 -o bin -g bin inet.h ${INCLUDE}/arpa

manpages:
	install -c -o bin -g bin -m 444 inetd.man ${MAN8}/inetd.8

clean:
	rm -f inetd *.o *.d Makedep

.EXIT:
	@md -d -f -u Makedep *.d

-include Makedep
