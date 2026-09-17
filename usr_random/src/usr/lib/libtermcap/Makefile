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

# Makefile for MACH3.0+BSD4.3 /usr/lib/libtermcap.a

USRLIB=	${INSTALL_TREE}/usr/lib
MAN3=	${INSTALL_TREE}/usr/man/man3

CFLAGS=	-O -MD -DCM_N -DCM_GT -DCM_B -DCM_D

OBJS=	termcap.o tgoto.o tputs.o

compile: profiled libtermcap.a libtermcap_p.a

libtermcap.a: ${OBJS}
	ar cu $@ ${OBJS}

libtermcap_p.a: ${OBJS}
	cd profiled; ar cr ../libtermcap_p.a ${OBJS}

profiled:
	mkdir profiled

.c.o:
	@cc ${CFLAGS} -c -p $*.c
	@ld -x -r -o profiled/$*.o $*.o
	cc ${CFLAGS} -c $*.c
	@ld -X -r $*.o
	@mv a.out $*.o

test:
	ranlib libtermcap.a
	cc ${CFLAGS} -o tc1 tc1.c libtermcap.a
	cc ${CFLAGS} -o tc2 tc2.c libtermcap.a
	cc ${CFLAGS} -o tc3 tc3.c libtermcap.a

install:
	install -c -m 444 -o bin -g bin libtermcap.a ${USRLIB}/libtermcap.a
	ranlib ${USRLIB}/libtermcap.a
	@rm -f ${USRLIB}/libtermlib.a
	ln -s libtermcap.a ${USRLIB}/libtermlib.a
	install -c -m 444 -o bin -g bin libtermcap_p.a ${USRLIB}/libtermcap_p.a
	ranlib ${USRLIB}/libtermcap_p.a
	@rm -f ${USRLIB}/libtermlib_p.a
	ln -s libtermcap_p.a ${USRLIB}/libtermlib_p.a

includes:

manpages:
	install -c -m 444 -o bin -g bin termcap.man ${MAN3}/termcap.3
	rm -f ${MAN3}/termlib.3; ln -s termcap.3 ${MAN3}/termlib.3

clean:
	rm -rf profiled
	rm -f ${OBJS} libtermcap.a libtermcap_p.a Makedep

.EXIT:
	@md -d -f -u Makedep *.d

-include Makedep
