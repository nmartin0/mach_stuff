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

# Makefile for /usr/lib/libreadline.a

USRLIB=	${INSTALL_TREE}/usr/lib
MAN2=	${INSTALL_TREE}/usr/man/man2
MAN3=	${INSTALL_TREE}/usr/man/man3

ASFLAGS= -MD
CFLAGS=	-O -MD

OBJS=	 readline.o history.o funmap.o keymaps.o

compile: libreadline.a

libreadline.a: ${OBJS}
	ar cu $@ ${OBJS}

install:
	install -c -m 755 -o bin -g bin libreadline.a ${USRLIB}
	ranlib ${USRLIB}/libreadline.a

includes:

manpages:
	-install -c -m 644 -o bin -g bin readline.man ${MAN2}/readline.3

clean:
	rm -f libreadline.a *.o *.d Makedep

.EXIT:
	@md -d -f -u Makedep *.d

-include Makedep
