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

# Makefile for MACH3.0+BSD4.3 /usr/bin/roff

DESTDIR=${INSTALL_TREE}
USRBIN=	${INSTALL_TREE}/usr/bin
USRLIB=	${INSTALL_TREE}/usr/lib
MAN1=	${INSTALL_TREE}/usr/man/man1

SUBDIR=	nroff troff nroff_term troff_font

compile:
	for i in ${SUBDIR}; do (cd $$i; make all); done

install:
	-[ -d ${USRLIB}/term ] || mkdir ${USRLIB}/term
	-chmod 555 ${USRLIB}/term
	-chown bin.bin ${USRLIB}/term
	for i in ${SUBDIR}; do \
	  (cd $$i; make ${MFLAGS} DESTDIR=${DESTDIR} install); \
	done
	-[ -d ${USRLIB}/tmac ] || mkdir ${USRLIB}/tmac
	-chmod 555 ${USRLIB}/tmac
	-chown bin.bin ${USRLIB}/tmac
	install -c -m 444 -o bin -g bin tmac/* ${USRLIB}/tmac
	-[ -d ${USRLIB}/ms ] || mkdir ${USRLIB}/ms
	-chmod 755 ${USRLIB}/ms
	-chown bin.bin ${USRLIB}/ms
	install -c -o bin -g bin -m 444 ms/* ${USRLIB}/ms

includes:

manpages:
	install -c -o bin -g bin -m 444 troff.man ${DESTDIR}/usr/man/man1/troff.1
	install -c -o bin -g bin -m 444 nroff.man ${DESTDIR}/usr/man/man1/nroff.1

clean:
	for i in ${SUBDIR}; do (cd $$i; make ${MFLAGS} clean); done
