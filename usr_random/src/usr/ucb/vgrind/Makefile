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

# Makefile for MACH3.0+BSD4.3 /usr/ucb/vgrind

USRUCB=	${INSTALL_TREE}/usr/ucb
USRLIB=	${INSTALL_TREE}/usr/lib
MAN1=	${INSTALL_TREE}/usr/man/man1

CFLAGS=	-O -MD

ROBJS=	retest.o regexp.o

VOBJS=	regexp.o vfontedpr.o vgrindefs.o

compile: vfontedpr retest

vfontedpr: ${VOBJS}
	cc -o $@ ${VOBJS}

retest:	${ROBJS}
	cc -o $@ ${ROBJS}

install:
	install -c -m 555 -o bin -g bin vgrind.sh ${USRUCB}/vgrind
	install -c -s -m 555 -o bin -g bin vfontedpr ${USRLIB}
	-@[ -d ${USRLIB}/tmac ] || mkdir ${USRLIB}/tmac
	chown bin.bin ${USRLIB}/tmac
	chmod 555 ${USRLIB}/tmac
	install -c -m 555 -o bin -g bin tmac.vgrind ${USRLIB}/tmac

includes:

manpages:
	install -c -m 444 -o bin -g bin vgrind.man ${MAN1}/vgrind.1

clean:
	rm -f vfontedpr retest *.o *.d Makedep

.EXIT:
	@md -d -f -u Makedep *.d

-include Makedep
