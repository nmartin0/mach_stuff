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

# Makefile for MACH3.0+BSD4.3 /etc/sony and /etc/eject

ETC=	${INSTALL_TREE}/etc
MAN8=	${INSTALL_TREE}/usr/man/man8

CFLAGS=	-O -MD

compile: sony

sony:	sony.o
	cc ${CFLAGS} $? -o $@

install:
	install -c -s -m 555 -o root -g bin sony ${ETC}
	rm -f ${ETC}/eject; ln -s sony ${ETC}/eject

includes:

manpages:
	-install -c -m 444 -o bin -g bin sony.man ${MAN8}/sony.8
	rm -f ${MAN8}/eject.8; ln -s sony.8 ${MAN8}/eject.8

clean:
	rm -f sony *.o *.d Makedep

.EXIT:
	@md -d -f -u Makedep *.d

-include Makedep
