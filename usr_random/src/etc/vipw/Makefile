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

# Makefile for MACH3.0+BSD4.3 /etc/vipw

ETC=	${INSTALL_TREE}/etc
USRETC=	${INSTALL_TREE}/usr/etc
MAN8=	${INSTALL_TREE}/usr/man/man8

CFLAGS=	-O -MD

compile: vipw

vipw:	vipw.o
	cc $? -o $@

install:
	install -c -s -m 544 -o root -g bin vipw ${USRETC}
	rm -f ${ETC}/vipw; ln -s ../usr/etc/vipw ${ETC}/vipw

includes:

manpages:
	install -c -m 444 -o bin -g bin vipw.man ${MAN8}/vipw.8

clean:
	rm -f vipw *.o *.d Makedep

.EXIT:
	@md -d -f -u Makedep *.d

-include Makedep
