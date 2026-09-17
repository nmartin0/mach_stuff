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

# Makefile for MACH3.0+BSD4.3 /usr/lib/me

USRLIB=	${INSTALL_TREE}/usr/lib

MELIB=	${USRLIB}/me
TMLIB=	${USRLIB}/tmac

MESRCS=	acm.me chars.me deltext.me eqn.me float.me footnote.me \
	index.me letterhead.me local.me null.me refer.me sh.me tbl.me thesis.me
TMSRCS=	tmac.e

compile:

install:
	-[ -d ${MELIB} ] || mkdir ${MELIB}
	-[ -d ${TMLIB} ] || mkdir ${TMLIB}
	chown bin.bin ${MELIB} ${TMLIB}
	chmod 755 ${MELIB} ${TMLIB}
	for i in ${MESRCS}; do \
	  ./strip.sed < $$i > ${MELIB}/$$i; \
	  chown bin.bin ${MELIB}/$$i; \
	  chmod 444 ${MELIB}/$$i; \
	done
	for i in ${TMSRCS}; do \
	  ./strip.sed < $$i > ${TMLIB}/$$i; \
	  chown bin.bin ${TMLIB}/$$i; \
	  chmod 444 ${TMLIB}/$$i; \
	done
	install -c -o bin -g bin -m 444 revisions ${MELIB}

includes:

manpages:

clean:
