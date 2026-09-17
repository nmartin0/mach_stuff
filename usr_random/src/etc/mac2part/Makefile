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

# Makefile for MACH3.0+BSD4.3 /etc/mac2part

ETC=	${INSTALL_TREE}/etc
MAN8=	${INSTALL_TREE}/usr/man/man8

CFLAGS=	-O -MD

compile: mac2part

mac2part: mac2part.o
	cc ${CFLAGS} $? -o $@ -lmach -lsolicit

install:
	install -c -s -m 544 -o root -g bin mac2part ${ETC}

includes:

manpages:
	install -c -m 444 -o bin -g bin mac2part.man ${MAN8}/mac2part.8

clean:
	rm -f mac2part *.o *.d Makedep

.EXIT:
	@md -d -f -u Makedep *.d

-include Makedep
