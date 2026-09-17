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

# Makefile for MACH3.0+BSD4.3 /usr/lib/libmachterm.a

USRLIB=	${INSTALL_TREE}/usr/lib
MAN3=	${INSTALL_TREE}/usr/man/man3
MANDOC=	${INSTALL_TREE}/usr/man/doc

CFLAGS=	-O -MD

OBJS=	machterm.o machterm_6x13_bold.o machterm_6x13_font.o \
	machterm_7x13_bold.o machterm_7x13_font.o machterm_9x15_bold.o \
	machterm_9x15_font.o machterm_buffer.o machterm_setup.o \
	small_machterm.o small_machterm_setup.o

compile: libmachterm.a

libmachterm.a: ${OBJS}
	ar cu $@ ${OBJS}

install:
	install -c -m 755 -o bin -g bin libmachterm.a ${USRLIB}
	ranlib ${USRLIB}/libmachterm.a

includes:

manpages:
	install -c -m 444 -o bin -g bin machterm.man ${MAN3}/machterm.3
	install -c -m 444 -o bin -g bin machterm.doc ${MANDOC}

clean:
	rm -f libmachterm.a *.o *.d Makedep

.EXIT:
	@md -d -f -u Makedep *.d

-include Makedep

