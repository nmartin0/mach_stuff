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

# Makefile for MACH3.0+BSD4.3 /usr/bin/bison

USRBIN=	${INSTALL_TREE}/usr/bin
USRLIB=	${INSTALL_TREE}/usr/lib
INFO=	${INSTALL_TREE}/usr/man/info
MAN1=	${INSTALL_TREE}/usr/man/man1

INFS=	bison.info bison.info-1 bison.info-2 bison.info-3 bison.info-4

PFILES=	-DXPFILE=\\\"${USRLIB}/bison.simple\\\" \
	-DXPFILE1=\\\"${USRLIB}/bison.hairy\\\"

compile:
	make -f Makefile.orig CFLAGS="-O" PFILES="${PFILES}" bison

install:
	install -c -s -m 555 -o bin -g bin bison ${USRBIN}
	awk -f mergedir.awk dir=${USRLIB} bison.simple >bison.simple.rm
	install -m 444 -o bin -g bin bison.simple.rm ${USRLIB}/bison.simple
	install -c -m 444 -o bin -g bin bison.hairy ${USRLIB}

includes:

manpages:
	-install -c -m 444 -o bin -g bin bison.man ${MAN1}/bison.1
	install -c -m 444 -o bin -g bin ${INFS} ${INFO}

clean:
	make -f Makefile.orig clean
