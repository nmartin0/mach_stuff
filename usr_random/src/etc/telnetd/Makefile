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

# Makefile for MACH3.0+BSD4.3 /etc/telnetd

ETC=	${INSTALL_TREE}/etc
USRETC=	${INSTALL_TREE}/usr/etc
INCLUDE=${INSTALL_TREE}/usr/include
MAN8=	${INSTALL_TREE}/usr/man/man8

CFLAGS=	-O -MD

VPATH=	.:../getty

OBJS=	telnetd.o get_date.o gettytab.o

compile: telnetd

telnetd: ${OBJS}
	cc ${OBJS} -o $@

install:
	install -c -s -m 544 -o root -g bin telnetd ${USRETC}
	rm -f ${ETC}/telnetd; ln -s ../usr/etc/telnetd ${ETC}/telnetd

includes:
	@install -p -v -c -m 444 -o bin -g bin telnet.h ${INCLUDE}/arpa

manpages:
	install -c -m 444 -o bin -g bin telnetd.man ${MAN8}/telnetd.8

clean:
	rm -f telnetd *.o *.d Makedep

.EXIT:
	@md -d -f -u Makedep *.d

-include Makedep
