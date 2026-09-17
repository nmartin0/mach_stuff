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

# Makefile for MACH3.0+BSD4.3 /etc/termcap

ETC=	${INSTALL_TREE}/etc
USRLIB=	${INSTALL_TREE}/usr/lib
MAN5=	${INSTALL_TREE}/usr/man/man5

compile: termcap

termcap: reorder termcap.src
	ex - termcap.src < reorder

install:
	install -c -m 444 -o bin -g bin map3270 ${ETC}
	install -c -m 444 -o bin -g bin termcap ${ETC}
	-@[ -d ${USRLIB}/tabset ] || mkdir ${USRLIB}/tabset
	chown bin.bin ${USRLIB}/tabset
	chmod 555 ${USRLIB}/tabset
	install -c -m 444 -o bin -g bin tabset/* ${USRLIB}/tabset

includes:

manpages:
	install -c -m 444 -o bin -g bin termcap.man ${MAN5}/termcap.5
	install -c -m 444 -o bin -g bin map3270.man ${MAN5}/termcap.5

clean:
	rm -f termcap
