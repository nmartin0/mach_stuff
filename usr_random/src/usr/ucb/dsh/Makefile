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

# Makefile for MACH3.0+BSD4.3 /usr/ucb/dsh and /usr/ucb/dbid

USRUCB=	${INSTALL_TREE}/usr/ucb
MAN1=	${INSTALL_TREE}/usr/man/man1

CFLAGS=	-O -MD

OBJS1=	dsh.o error.o getrc.o ipc.o getwd.o getbids.o

OBJS2=	getloadave.o getpath.o dbid.o error.o getrc.o ipc.o

compile: dsh dbid

dsh:	${OBJS1}
	cc ${OBJS1} -o $@

dbid:	${OBJS2}
	cc ${OBJS1} -o $@

install:
	install -c -s -m 2555 -o bin -g kmem dsh ${USRUCB}
	install -c -s -m 2555 -o bin -g kmem dbid ${USRUCB}

includes:

manpages:
	install -c -m 444 -o bin -g bin dsh.man ${MAN1}/dsh.1
	-install -c -m 444 -o bin -g bin dbid.man ${MAN1}/dbid.1

clean:
	rm -f dsh dbid *.o *.d Makedep

.EXIT:
	@md -d -f -u Makedep *.d

-include Makedep
