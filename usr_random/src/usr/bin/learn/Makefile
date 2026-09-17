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

# Makefile for MACH3.0+BSD4.3 /usr/bin/learn

USRBIN=	${INSTALL_TREE}/usr/bin
USRLIB=	${INSTALL_TREE}/usr/lib
MAN1=	${INSTALL_TREE}/usr/man/man1

CFLAGS=	-O -MD

OBJS=	copy.o dounit.o getlesson.o learn.o list.o mem.o makpipe.o maktee.o \
	mysys.o selsub.o selunit.o start.o whatnow.o wrapup.o

compile: learn lrntee lcount getline.o getnum.o

learn:	${OBJS}
	cc ${OBJS} -o $@

lcount: lcount.c
	cc ${CFLAGS} lcount.c -o $@

lrntee: lrntee.c
	cc ${CFLAGS} lrntee.c -o $@

install:
	-@[ -d ${USRLIB}/learn ] || mkdir ${USRLIB}/learn
	@chown bin.bin ${USRLIB}/learn
	@chmod 555 ${USRLIB}/learn
	-@[ -d ${USRLIB}/learn/bin ] || mkdir ${USRLIB}/learn/bin
	@chown bin.bin ${USRLIB}/learn/bin
	@chmod 555 ${USRLIB}/learn/bin
	-@[ -d ${USRLIB}/learn/C ] || mkdir ${USRLIB}/learn/C
	@chown bin.bin ${USRLIB}/learn/C
	@chmod 555 ${USRLIB}/learn/C
	install -c -s -m 555 -o bin -g bin learn ${USRBIN}
	install -c -s -o bin -g bin -m 555 learn ${USRLIB}/learn/bin
	install -c -s -o bin -g bin -m 555 lrntee ${USRLIB}/learn/bin
	install -c -s -o bin -g bin -m 555 lcount ${USRLIB}/learn/bin
	install -c -o bin -g bin -m 444 getline.o ${USRLIB}/learn/bin
	install -c -o bin -g bin -m 444 getnum.o ${USRLIB}/learn/bin
	cd ${USRLIB}/learn/C; \
	  rm -f getline.o; ln -s ../bin/getline.o getline.o; \
	  rm -f getnum.o; ln -s ../bin/getnum.o getnum.o
	install -c -o bin -g bin -m 444 getline.c ${USRLIB}/learn/C
	install -c -o bin -g bin -m 444 getnum.c ${USRLIB}/learn/C

includes:

manpages:
	install -c -m 444 -o bin -g bin learn.man ${MAN1}/learn.1

clean:
	rm -f learn lrntee lcount *.o *.d Makedep

.EXIT:
	@md -d -f -u Makedep *.d

-include Makedep
