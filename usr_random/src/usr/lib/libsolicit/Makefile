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

# Makefile for MACH3.0+BSD4.3 /usr/lib/libsolicit.a

USRLIB=	${INSTALL_TREE}/usr/lib
INCLUDE=${INSTALL_TREE}/usr/include
MAN3=	${INSTALL_TREE}/usr/man/man3

CFLAGS=	-O -MD

OBJS=	search_items.o select_item.o solicit_one_or_more_items.o \
	solicit_password.o solicit_text.o solicit_yesno.o \
	sort_items.o count_items.o add_item.o

compile: libsolicit.a

libsolicit.a: ${OBJS}
	ar cu $@ ${OBJS}

install:
	install -c -m 755 -o bin -g bin libsolicit.a ${USRLIB}
	ranlib ${USRLIB}/libsolicit.a

includes:
	@install -p -v -c -m 444 -o bin -g bin solicit.h ${INCLUDE}

manpages:
	install -c -m 644 -o bin -g bin solicit.man ${MAN3}/solicit.3
	rm -f ${MAN3}/count_items.3; ln -s solicit.3 ${MAN3}/count_items.3
	rm -f ${MAN3}/search_items.3; ln -s solicit.3 ${MAN3}/search_items.3
	rm -f ${MAN3}/select_item.3; ln -s solicit.3 ${MAN3}/select_item.3
	rm -f ${MAN3}/sort_items.3; ln -s solicit.3 ${MAN3}/sort_items.3
	rm -f ${MAN3}/solicit_item.3; ln -s solicit.3 ${MAN3}/solicit_item.3
	rm -f ${MAN3}/solicit_items.3; ln -s solicit.3 ${MAN3}/solicit_items.3
	rm -f ${MAN3}/solicit_password.3; ln -s solicit.3 ${MAN3}/solicit_password.3
	rm -f ${MAN3}/solicit_text.3; ln -s solicit.3 ${MAN3}/solicit_text.3
	rm -f ${MAN3}/solicit_yesno.3; ln -s solicit.3 ${MAN3}/solicit_yesno.3

clean:
	rm -f libsolicit.a *.o *.d Makedep

.EXIT:
	@md -d -f -u Makedep *.d

-include Makedep
