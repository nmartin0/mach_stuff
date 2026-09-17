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

# Makefile for MACH3.0+BSD4.3 /usr/lib/libthreads.a

USRLIB=	${INSTALL_TREE}/usr/lib
INCLUDE=${INSTALL_TREE}/usr/include
MAN3=	${INSTALL_TREE}/usr/man/man3
MANDOC=	${INSTALL_TREE}/usr/man/doc

ASFLAGS= -MD
CFLAGS=	-O -I. -MD -DMACH_IPC_COMPAT=0

VPATH=	${target_machine}:.

OBJS=	cprocs.o cthreads.o malloc.o \
	mig_support.o stack.o sync.o \
	thread.o lock.o csw.o

compile: libthreads.a

libthreads.a: ${OBJS}
	ar cu libthreads.a ${OBJS}

.c.o:
	cc -S ${CFLAGS} -c $*.c
	@awk -f ${target_machine}/cthread_inline.awk $*.s >$*.S
	@cc -c $*.S
	@rm $*.s $*.S

threadtest: threadtest.c libthreads.a
	cc -O -c threadtest.c
	ld -o threadtest \
	  /lib/crt0.o \
	  threadtest.o \
	  /usr/lib/libthreads.a \
	  /usr/lib/libmach.a \
	  /lib/libc.a
	rm threadtest.o

install:
	install -c -m 444 -o bin -g bin libthreads.a ${USRLIB}/libthreads.a
	ranlib ${USRLIB}/libthreads.a

includes:
	@install -p -v -c -m 444 -o bin -g bin cthreads.h ${INCLUDE}
	@install -p -v -c -m 444 -o bin -g bin \
	  ${target_machine}/cthreads.h ${INCLUDE}/${target_machine}

manpages:
	-[ -d ${MANDOC}/mach ] || mkdir ${MANDOC}/mach
	chown bin.bin ${MANDOC}/mach
	chmod 555 ${MANDOC}/mach
	install -c -m 444 -o bin -g bin threads.doc ${MANDOC}/mach

clean:
	rm -f *.s *.S *.o libthreads.a threadtest Makedep

.EXIT:
	@md -d -f -u Makedep *.d

-include Makedep
