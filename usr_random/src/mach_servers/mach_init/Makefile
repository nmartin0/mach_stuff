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

# Makefile for MACH3.0+BSD4.3 /mach_servers/mach_init

SERVERS=${INSTALL_TREE}/mach_servers
ETC=	${INSTALL_TREE}/etc
USRETC=	${INSTALL_TREE}/usr/etc
MAN8=	${INSTALL_TREE}/usr/man/man8

CFLAGS=	-O -MD

compile: mach_init test_service waitfor

mach_init: main.o service.o
	cc ${CFLAGS} main.o service.o -o $@ -lcmucs -lservice -lmach -lthreads

test_service: test_service.o
	cc $? -o $@ -lcmucs -lservice -lmach -lthreads

waitfor: waitfor.o
	cc $? -o $@ -lcmucs -lservice -lmach -lthreads

install:
	install -c -s -m 544 -o root -g bin mach_init ${SERVERS}
	install -c -s -m 544 -o root -g bin test_service ${USRETC}
	@rm -f ${ETC}/test_service
	ln -s ../usr/etc/test_service ${ETC}/test_service
	install -c -s -m 544 -o root -g bin waitfor ${USRETC}
	@rm -f ${ETC}/waitfor
	ln -s ../usr/erc/waitfor ${ETC}/waitfor

includes:

manpages:
	-install -c -m 444 -o bin -g bin mach_init.man ${MAN8}/mach_init.8
	-install -c -m 444 -o bin -g bin test_service.man ${MAN8}/test_service.8
	install -c -m 444 -o bin -g bin waitfor.man ${MAN8}/waitfor.8

clean:
	rm -f mach_init test_service waitfor *.o *.d Makedep

.EXIT:
	@md -d -f -u Makedep *.d

-include Makedep
