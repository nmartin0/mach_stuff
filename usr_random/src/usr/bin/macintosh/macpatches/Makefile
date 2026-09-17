# MacMach Operating System
# Copyright (c) 1992 Carnegie Mellon University
# All Rights Reserved.
#
# MacMach was developed by CMU with support from Apple Computer, Inc.
# Use of this software is constrained by the MacMach End-User license.

# Makefile for MACH3.0+BSD4.3 /usr/lib/macpatches

USRLIB=	${INSTALL_TREE}/usr/lib
MAN1=	${INSTALL_TREE}/usr/man/man1

ASFLAGS= -MD
CFLAGS=	-O -MD

OBJS=	start.o msg.o patches.o main.o exception.o mig_support.o \
	exc_user.o inst.o inst_subr.o inst_patch.o fault.o emul.o \
	emul_misc.o drvr_low.o rtc.o time.o scsi.o sony.o ether.o \
	ether_hash.o input.o clock.o memory.o th.o lock.o prio.o \
	macserver_user.o sched.o

GENS=	exc_user.c exc.h macserver_user.c macserver.h

compile: macpatches

macpatches: ${OBJS}
	ld -N -T 40880000 -o $@ ${OBJS} -lmach_sa

macserver_user.c macserver.h: macserver.defs
	mig -user macserver_user.c -header macserver.h -server /dev/null $?

exc_user.c exc.h: exc.defs
	mig -user exc_user.c -header exc.h -server /dev/null $?

install:
	install -c -s -m 444 -o bin -g bin macpatches ${USRLIB}

includes:

manpages:

clean:
	rm -f ${GENS} macpatches *.o *.d Makedep

.EXIT:
	@md -d -f -u Makedep *.d

-include Makedep
