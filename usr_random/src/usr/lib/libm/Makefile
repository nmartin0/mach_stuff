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

# Makefile for MACH3.0+BSD4.3 /usr/lib/libm.a

USRLIB=	${INSTALL_TREE}/usr/lib
INCLUDE=${INSTALL_TREE}/usr/include
MAN3=	${INSTALL_TREE}/usr/man/man3

CFLAGS=	-O -MD

VPATH=	${target_machine}:src

MANS=	math asinh sin sinh hypot ieee erf exp floor lgamma j0 sinh sqrt infnan

OBJS=	acosh.o asincos.o asinh.o atan.o atan2.o atanh.o cabs.o cbrt.o \
	cosh.o erf.o exp.o exp__E.o expm1.o floor.o lgamma.o j0.o j1.o jn.o \
	log.o log10.o log1p.o log__L.o pow.o sincos.o sinh.o support.o \
	tan.o tanh.o

compile: profiled libm.a libm_p.a

libm.a:	${OBJS}
	ar cru libm.a ${OBJS}

libm_p.a: ${OBJS}
	cd profiled; ar cru ../libm_p.a ${OBJS}

profiled:
	mkdir profiled

.c.o:
	@cc -p ${CFLAGS} -c $*.c
	@ld -X -o profiled/$*.o -r $*.o
	cc ${CFLAGS} -c $*.c
	@ld -x -r $*.o
	@mv a.out $*.o

.S.o:
	@cc -DPROF ${ASFLAGS} -c $*.S
	@ld -X -o profiled/$*.o -r $*.o
	cc ${ASFLAGS} -c $*.S
	@ld -x -r $*.o
	@mv a.out $*.o

install:
	install -c -o bin -g bin -m 444 libm.a ${USRLIB}/libm.a
	ranlib ${USRLIB}/libm.a
	install -c -o bin -g bin -m 444 libm_p.a ${USRLIB}/libm_p.a
	ranlib ${USRLIB}/libm_p.a

includes:
	@install -p -v -c -m 444 -o bin -g bin math.h ${INCLUDE}

manpages:
	@for i in ${MANS}; do \
	  install -v -c -m 444 -o bin -g bin man/$$i.man ${MAN3}/$$i.3 ;\
	done
	rm -f ${MAN3}/acosh.3; ln -s asinh.3 ${MAN3}/acosh.3
	rm -f ${MAN3}/atanh.3; ln -s asinh.3 ${MAN3}/atanh.3
	rm -f ${MAN3}/acos.3; ln -s sin.3 ${MAN3}/acos.3
	rm -f ${MAN3}/asin.3; ln -s sin.3 ${MAN3}/asin.3
	rm -f ${MAN3}/atan.3; ln -s sin.3 ${MAN3}/atan.3
	rm -f ${MAN3}/atan2.3; ln -s sin.3 ${MAN3}/atan2.3
	rm -f ${MAN3}/cos.3; ln -s sin.3 ${MAN3}/cos.3
	rm -f ${MAN3}/tan.3; ln -s sin.3 ${MAN3}/tan.3
	rm -f ${MAN3}/cabs.3; ln -s hypot.3 ${MAN3}/cabs.3
	rm -f ${MAN3}/copysign.3; ln -s ieee.3 ${MAN3}/copysign.3
	rm -f ${MAN3}/drem.3; ln -s ieee.3 ${MAN3}/drem.3
	rm -f ${MAN3}/finite.3; ln -s ieee.3 ${MAN3}/finite.3
	rm -f ${MAN3}/logb.3; ln -s ieee.3 ${MAN3}/logb.3
	rm -f ${MAN3}/scalb.3; ln -s ieee.3 ${MAN3}/scalb.3
	rm -f ${MAN3}/erfc.3; ln -s erf.3 ${MAN3}/erfc.3
	rm -f ${MAN3}/expm1.3; ln -s exp.3 ${MAN3}/expm1.3
	rm -f ${MAN3}/log.3; ln -s exp.3 ${MAN3}/log.3
	rm -f ${MAN3}/log10.3; ln -s exp.3 ${MAN3}/log10.3
	rm -f ${MAN3}/log1p.3; ln -s exp.3 ${MAN3}/log1p.3
	rm -f ${MAN3}/pow.3; ln -s exp.3 ${MAN3}/pow.3
	rm -f ${MAN3}/ceil.3; ln -s floor.3 ${MAN3}/ceil.3
	rm -f ${MAN3}/fabs.3; ln -s floor.3 ${MAN3}/fabs.3
	rm -f ${MAN3}/rint.3; ln -s floor.3 ${MAN3}/rint.3
	rm -f ${MAN3}/gamma.3; ln -s lgamma.3 ${MAN3}/gamma.3
	rm -f ${MAN3}/j1.3; ln -s j0.3 ${MAN3}/j1.3
	rm -f ${MAN3}/jn.3; ln -s j0.3 ${MAN3}/jn.3
	rm -f ${MAN3}/y0.3; ln -s j0.3 ${MAN3}/y0.3
	rm -f ${MAN3}/y1.3; ln -s j0.3 ${MAN3}/y1.3
	rm -f ${MAN3}/yn.3; ln -s j0.3 ${MAN3}/yn.3
	rm -f ${MAN3}/cosh.3; ln -s sinh.3 ${MAN3}/cosh.3
	rm -f ${MAN3}/tanh.3; ln -s sinh.3 ${MAN3}/tanh.3
	rm -f ${MAN3}/cbrt.3; ln -s sqrt.3 ${MAN3}/cbrt.3

clean:
	rm -f ${OBJS} libm.a libm_p.a Makedep
	rm -rf profiled

.EXIT:
	@md -d -f -u Makedep *.d

-include Makedep
