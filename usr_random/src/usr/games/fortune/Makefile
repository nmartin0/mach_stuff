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

# Makefile for MACH3.0+BSD4.3 /usr/games/fortune

USRG=	${INSTALL_TREE}/usr/games
USRGHD= ${INSTALL_TREE}/usr/games/hide
USRGLB= ${INSTALL_TREE}/usr/games/lib
MAN6=	${INSTALL_TREE}/usr/man/man6

CFLAGS=	-O -MD

OBJS=	fortune.o rnd.o

compile: fortune fortunes.dat

fortune: ${OBJS}
	cc ${OBJS} -o $@

fortunes.dat: fortunes strfile
	./strfile -r fortunes

fortunes: sort.scene scene sort.obscene obscene
	(cat scene; echo "%-"; cat obscene) > fortunes

strfile: strfile.o rnd.o
	cc strfile.o rnd.o -o $@

sort.scene: strfile unstr
	./strfile -oi scene
	mv scene Oscene
	./unstr -o scene

sort.obscene: strfile unstr
	./strfile -oi obscene
	mv obscene Oobscene
	./unstr -o obscene

unstr:	unstr.c
	cc -o $@ unstr.c

install:
	install -c -s -m 540 -o games -g bin fortune ${USRGHD}
	rm -f ${USRG}/fortune; ln -s dm ${USRG}/fortune
	install -c -m 440 -o games -g bin fortunes.dat ${USRGLB}

includes:

manpages:
	install -c -m 444 -o bin -g bin fortune.man ${MAN6}/fortune.6

clean:
	rm -f fortune strfile unstr *.o *.d Makedep
	rm -f fortunes.dat scene.dat Oscene obscene.dat Oobscene

.EXIT:
	@md -d -f -u Makedep *.d

-include Makedep
