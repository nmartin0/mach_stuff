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

# Makefile for MACH3.0+BSD4.3 /bin/passwd

BIN=	${INSTALL_TREE}/bin
MAN1=	${INSTALL_TREE}/usr/man/man1

CFLAGS=	-O -MD

compile: passwd

passwd:	passwd.o
	cc $? -o $@

install:
	install -c -s -m 4555 -o root -g bin passwd ${BIN}/passwd
	rm -f ${BIN}/chfn; ln -s passwd ${BIN}/chfn
	rm -f ${BIN}/chsh; ln -s passwd ${BIN}/chsh

includes:

manpages:
	install -c -m 444 -o bin -g bin passwd.man ${MAN1}/passwd.1
	rm -f ${MAN1}/chfn.1; ln -s passwd.1 ${MAN1}/chfn.1
	rm -f ${MAN1}/chsh.1; ln -s passwd.1 ${MAN1}/chsh.1

clean:
	rm -f passwd *.o *.d Makedep

.EXIT:
	@md -d -f -u Makedep *.d

-include Makedep
