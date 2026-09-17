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

# Makefile for MACH3.0+BSD4.3 /usr/ucb/script

USRUCB=	${INSTALL_TREE}/usr/ucb
MAN1=	${INSTALL_TREE}/usr/man/man1

CFLAGS=	-O -MD

compile: script

script:	script.o
	cc $? -o $@

install:
	install -c -s -m 555 -o bin -g bin script ${USRUCB}

includes:

manpages:
	install -c -m 444 -o bin -g bin script.man ${MAN1}/script.1

clean:
	rm -f script *.o *.d Makedep

.EXIT:
	@md -d -f -u Makedep *.d

-include Makedep
