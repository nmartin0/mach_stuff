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

# Makefile for MACH3.0+BSD4.3 /usr/games/boggle

USRG=	${INSTALL_TREE}/usr/games
USRGHD= ${INSTALL_TREE}/usr/games/hide
USRGLB= ${INSTALL_TREE}/usr/games/lib
MAN6=	${INSTALL_TREE}/usr/man/man6

CFLAGS=	-O -MD

compile: boggle bogdict

boggle:	boggle.o
	cc $? -o $@

bogdict: comp sfile
	sed -f sfile ${INSTALL_TREE}/usr/dict/words | uniq | ./comp >bogdict

comp:	comp.o
	cc $? -o $@

install:
	install -c -s -m 540 -o games -g bin boggle ${USRGHD}
	rm -f ${USRG}/boggle; ln -s dm ${USRG}/boggle
	install -c -m 440 -o games -g bin bogdict ${USRGLB}

includes:

manpages:
	install -c -m 444 -o bin -g bin boggle.man ${MAN6}/boggle.6

clean:
	rm -f boggle bogdict comp *.o *.d Makedep

.EXIT:
	@md -d -f -u Makedep *.d

-include Makedep
