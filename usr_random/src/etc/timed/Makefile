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

# Makefile for MACH3.0+BSD4.3 /etc/timed and /etc/timedc

ETC=	${INSTALL_TREE}/etc
USRETC=	${INSTALL_TREE}/usr/etc
INCLUDE=${INSTALL_TREE}/usr/include
MAN8=	${INSTALL_TREE}/usr/man/man8

# optional flags are: MEASURE TESTING DEBUG
CFLAGS=	-O -MD -I.

VPATH=	.:${target_machine}

OBJS1=	acksend.o candidate.o correct.o master.o networkdelta.o readmsg.o \
	slave.o timed.o byteorder.o measure.o cksum.o ocsum.o

OBJS2=	cmds.o cmdtab.o timedc.o byteorder.o measure.o cksum.o ocsum.o

compile: timed timedc

timed:	${OBJS1}
	cc ${OBJS1} -o $@

timedc:	${OBJS2}
	cc ${OBJS2} -o $@

install:
	install -c -s -m 544 -o root -g bin timed ${ETC}
	rm -f ${ETC}/timed; ln -s ../usr/etc/timed ${ETC}/timed
	install -c -s -m 544 -o root -g bin timedc ${ETC}
	rm -f ${ETC}/timedc; ln -s ../usr/etc/timedc ${ETC}/timedc

includes:
	@install -p -v -c -m 444 -o bin -g bin timed.h ${INCLUDE}/protocols

manpages:
	install -c -m 444 -o bin -g bin timed.man ${MAN8}/timed.8
	install -c -m 444 -o bin -g bin timedc.man ${MAN8}/timedc.8

clean:
	rm -f timed timedc *.o *.d Makedep

.EXIT:
	@md -d -f -u Makedep *.d

-include Makedep
