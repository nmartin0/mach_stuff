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

# Makefile for MACH3.0+BSD4.3 /usr/bin/diction

USRBIN=	${INSTALL_TREE}/usr/bin
USRLIB=	${INSTALL_TREE}/usr/lib
MAN1=	${INSTALL_TREE}/usr/man/man1

CFLAGS=	-O -MD

compile: dprog style1 style2 style3

dprog: diction.c
	cc ${CFLAGS} -DDICT=\"${USRLIB}/dict.d\" diction.c -o $@

style1: nwords.o
	cc nwords.o -o $@ -ll

style2: end.o
	cc end.o -o $@ -ll

style3: part.o pscan.o outp.o
	cc part.o pscan.o outp.o -o $@ -ll

install:
	install -c -s -m 555 -o bin -g bin style1 ${USRLIB}
	install -c -s -m 555 -o bin -g bin style2 ${USRLIB}
	install -c -s -m 555 -o bin -g bin style3 ${USRLIB}
	install -c -s -m 555 -o bin -g bin dprog ${USRLIB}
	install -c -m 555 -o bin -g bin style.sh ${USRBIN}/style
	install -c -m 555 -o bin -g bin diction.sh ${USRBIN}/diction
	install -c -m 555 -o bin -g bin explain.sh ${USRBIN}/explain
	install -c -m 555 -o bin -g bin dict.d.src ${USRLIB}/dict.d
	install -c -m 555 -o bin -g bin explain.d.src ${USRLIB}/explain.d

#	-@[ -d ${INSTALL_TREE}/usr/dict ] || mkdir ${DESTDIR}/usr/dict
#	-@chown root.bin ${INSTALL_TREE}/usr/dict
#	-@chmod 555 ${INSTALL_TREE}/usr/dict

includes:

manpages:
	install -c -m 444 -o bin -g bin diction.man ${MAN1}/diction.1

clean:
	rm -f part.c nwords.c end.c style1 style2 style3 dprog *.o *.d Makedep

.EXIT:
	@md -d -f -u Makedep *.d

-include Makedep
