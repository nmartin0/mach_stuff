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

# Makefile for MACH3.0+BSD4.3 /usr/games/jive and /usr/games/valspeak

USRG=	${INSTALL_TREE}/usr/games
USRGHD= ${INSTALL_TREE}/usr/games/hide
MAN6=	${INSTALL_TREE}/usr/man/man6

CFLAGS=	-O -MD

compile: jive valspeak

valspeak: valspeak.o
	cc $? -o $@ -ll

jive:	jive.o
	cc $? -o $@ -ll

valspeak.c: valspeak.l
	lex valspeak.l
	mv lex.yy.c valspeak.c

jive.c:	jive.l
	lex jive.l
	mv lex.yy.c jive.c

install:
	install -c -s -m 540 -o games -g bin jive ${USRGHD}
	rm -f ${USRG}/jive; ln -s dm ${USRG}/jive
	install -c -s -m 540 -o games -g bin valspeak ${USRGHD}
	rm -f ${USRG}/valspeak; ln -s dm ${USRG}/valspeak

includes:

manpages:
	install -c -m 444 -o bin -g bin jive.man ${MAN6}/jive.6
	install -c -m 444 -o bin -g bin valspeak.man ${MAN6}/valspeak.6

clean:
	rm -f jive valspeak jive.c valspeak.c *.o *.d Makedep

.EXIT:
	@md -d -f -u Makedep *.d

-include Makedep
