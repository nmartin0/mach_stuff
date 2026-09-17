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

# Makefile for MACH3.0+BSD4.3 /usr/lib/man

USRLIB=	${INSTALL_TREE}/usr/lib

CFLAGS=	-O -MD

SRCS=	tmac.an tmac.an.new tmac.an6n tmac.an6t

compile:

install:
	-[ -d ${USRLIB}/tmac ] || mkdir ${USRLIB}/tmac
	-chmod 755 ${USRLIB}/tmac
	-chown root.bin ${USRLIB}/tmac
	for i in ${SRCS}; do \
	  (install -c -o bin -g bin -m 644 $$i ${USRLIB}/tmac/$$i); \
	done

includes:

manpages:

clean:
