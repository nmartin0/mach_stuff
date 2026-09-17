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

# Makefile for MACH3.0+BSD4.3 /usr/bin/eqn and /usr/bin/neqn

MAN1=	${INSTALL_TREE}/usr/man/man1

compile:
	cd eqn; make compile
	cd neqn; make compile

install:
	cd eqn; make install
	cd neqn; make install

includes:
	cd eqn; make includes
	cd neqn; make includes

manpages:
	install -c -m 444 -o bin -g bin eqn.man ${MAN1}/eqn.1
	rm -f ${MAN1}/checkeq.1; ln -s eqn.1 ${MAN1}/checkeq.1
	rm -f ${MAN1}/neqn.1; ln -s eqn.1 ${MAN1}/neqn.1

clean:
	cd eqn; make clean
	cd neqn; make clean
