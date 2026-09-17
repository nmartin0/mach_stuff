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

# Makefile for MACH3.0+BSD4.3 /bin/as and /bin/gas

BIN=	${INSTALL_TREE}/bin
MAN1=	${INSTALL_TREE}/usr/man/man1

compile: gas.${target_machine}

gas.mac2:
	make -f Makefile.orig G0="-I. -O" O3="-Dm68851" a68
	mv a68 gas.mac2

install:
	install -c -s -m 555 -o bin -g bin gas.${target_machine} ${BIN}/gas
	rm -f ${BIN}/as; ln -s gas ${BIN}/as

includes:

manpages:
	-install -c -m 444 -o bin -g bin gas.man ${MAN1}/gas.1
	rm -f ${MAN1}/as.1; ln -s gas.1 ${MAN1}/as.1

clean:
	make -f Makefile.orig clean
	rm -f gas.${target_machine}

