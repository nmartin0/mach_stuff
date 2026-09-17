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

# Makefile for MACH3.0+BSD4.3 /usr/lib/learn

USRLIB=	${INSTALL_TREE}/usr/lib

LESSONS=C editor eqn files macros morefiles
FILES=	Linfo Xinfo

compile:

install:
	-[ -d ${USRLIB}/learn ] || mkdir ${USRLIB}/learn
	-[ -d ${USRLIB}/learn/log ] || mkdir ${USRLIB}/learn/log
	-[ -d ${USRLIB}/learn/bin ] || mkdir ${USRLIB}/learn/bin
	install -c -o bin -g bin -m 644 Linfo ${USRLIB}/learn
	install -c -o bin -g bin -m 644 Xinfo ${USRLIB}/learn
	tar chf - ${LESSONS} | (cd ${USRLIB}/learn; tar xpf -);
	chown -R bin.bin ${USRLIB}/learn
	find ${USRLIB}/learn -type d -exec chmod 755 {} \;
	find ${USRLIB}/learn -type f -exec chmod 644 {} \;

includes:

manpages:

clean:
