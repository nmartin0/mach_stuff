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

# Makefile for MACH3.0+BSD4.3 /usr/bin/spell

USRBIN=	${INSTALL_TREE}/usr/bin
USRLIB=	${INSTALL_TREE}/usr/lib
USRDICT=${INSTALL_TREE}/usr/dict
MAN1=	${INSTALL_TREE}/usr/man/man1

SPECIAL= special.4bsd

CFLAGS=	-O -MD

SRCS=	spell.c spellin.c spellout.c

OBJS=	spell.o spellin.o spellout.o

compile: spell hlist hlista hlistb hstop spellin spellout

spell:	spell.c
	cc ${CFLAGS} spell.c -o $@

spellin: spellin.c
	cc ${CFLAGS} spellin.c -o $@

spellout: spellout.c
	cc ${CFLAGS} spellout.c -o $@

hlist:	words spellin
	./spellin < words >hlist

hlista: american ${SPECIAL} ${LOCAL} hlist spellin
	(cat american ${SPECIAL} ${LOCAL}) | ./spellin hlist > hlista

hlistb: british ${SPECIAL} ${LOCAL} hlist spellin
	(cat british ${SPECIAL} ${LOCAL}) | ./spellin hlist > hlistb

hstop: stop spellin
	./spellin < stop >hstop

install:
	-@[ -d ${USRDICT}/special ] || mkdir ${USRDICT}/special
	chown bin.bin ${USRDICT}/special
	chmod 555 ${USRDICT}/special
	install -c -s -o bin -g bin -m 755 spell ${USRLIB}/spell
	install -c -s -o bin -g bin -m 755 spellin ${USRBIN}/spellin
	install -c -s -o bin -g bin -m 755 spellout ${USRBIN}/spellout
	install -c -o bin -g bin -m 755 spell.sh ${USRBIN}/spell
	install -c -o bin -g bin -m 444 hlista ${USRDICT}/hlista
	install -c -o bin -g bin -m 444 hlistb ${USRDICT}/hlistb
	install -c -o bin -g bin -m 444 hstop ${USRDICT}/hstop
	install -c -o bin -g bin -m 444 hlist ${USRDICT}/hlist
	install -c -o bin -g bin -m 444 words ${USRDICT}/words
	install -c -o bin -g bin -m 444 american ${USRDICT}/american
	install -c -o bin -g bin -m 444 british ${USRDICT}/british
	install -c -o bin -g bin -m 444 stop ${USRDICT}/stop
	install -c -o bin -g bin -m 444 README ${USRDICT}/README
	install -c -o bin -g bin -m 444 special.4bsd ${USRDICT}/special/4bsd
	install -c -o bin -g bin -m 444 special.math ${USRDICT}/special/math

includes:

manpages:
	install -c -o bin -g bin -m 444 spell.man ${DESTDIR}/usr/man/man1/spell.1
	rm -f ${DESTDIR}/usr/man/man1/spellin.1
	ln -s spell.1 ${DESTDIR}/usr/man/man1/spellin.1
	rm -f ${DESTDIR}/usr/man/man1/spellout.1
	ln -s spell.1 ${DESTDIR}/usr/man/man1/spellout.1

clean:
	rm -f ${OBJS} core spell spellin hlista hlistb hlist hstop spellout Makedep

.EXIT:
	@md -d -f -u Makedep *.d

-include Makedep
