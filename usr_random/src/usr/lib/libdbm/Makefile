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

# Makefile for MACH3.0+BSD4.3 /usr/lib/libdbm

USRLIB=	${INSTALL_TREE}/usr/lib
INCLUDE=${INSTALL_TREE}/usr/include
MAN3=	${INSTALL_TREE}/usr/man/man3

CFLAGS=	-O -MD

compile: libdbm.a libdbm_p.a

libdbm.a: dbm.c
	cc -c ${CFLAGS} dbm.c
	ld -r -o libdbm.a -x dbm.o
	rm -f dbm.o

libdbm_p.a: dbm.c
	cc -p -c ${CFLAGS} dbm.c
	ld -r -o libdbm_p.a -X dbm.o
	rm -f dbm.o

install:
	install -c -o bin -g bin -m 644 libdbm.a ${USRLIB}
	install -c -o bin -g bin -m 644 libdbm_p.a ${USRLIB}

includes:
	@install -p -v -c -o bin -g bin -m 444 dbm.h ${INCLUDE}

manpages:
	install -c -m 444 -o bin -g bin dbm.man ${MAN3}/dbm.3
	rm -f ${MAN3}/dbminit.3; ln -s dbm.3 ${MAN3}/dbminit.3
	rm -f ${MAN3}/delete.3; ln -s dbm.3 ${MAN3}/delete.3
	rm -f ${MAN3}/fetch.3; ln -s dbm.3 ${MAN3}/fetch.3
	rm -f ${MAN3}/firstkey.3; ln -s dbm.3 ${MAN3}/firstkey.3
	rm -f ${MAN3}/nextkey.3; ln -s dbm.3 ${MAN3}/nextkey.3
	rm -f ${MAN3}/store.3; ln -s dbm.3 ${MAN3}/store.3

clean:
	rm -f libdbm.a libdbm_p.a *.o *.d Makedep

.EXIT:
	@md -d -f -u Makedep *.d

-include Makedep
