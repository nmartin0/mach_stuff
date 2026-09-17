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

# Makefile for MACH3.0+BSD4.3 /usr/bin/sup

USRBIN=	${INSTALL_TREE}/usr/bin
ETC=	${INSTALL_TREE}/etc
USRETC=	${INSTALL_TREE}/usr/etc
MAN1=	${INSTALL_TREE}/usr/man/man1
MAN8=	${INSTALL_TREE}/usr/man/man8
MANDOC=	${INSTALL_TREE}/usr/man/doc

CFLAGS=	-O -MD -UCMU

SRCS=	supcmain.c supcvers.c supcparse.c supcname.c supcmisc.c supcmeat.c \
	scm.c scmio.c stree.c log.c supmsg.c supscan.c stree.c scan.c \
	doprnt.c vsprintf.c

SUP_OBJS= supcmain.o supcvers.o supcparse.o supcname.o supcmisc.o supcmeat.o \
	scm.o scmio.o stree.o log.o supmsg.o doprnt.o vsprintf.o \
	salloc.o nxtarg.o skipto.o path.o run.o ffilecopy.o errmsg.o \
	filecopy.o

SUPFILESRV_OBJS= supfilesrv.o scan.o scm.o scmio.o stree.o log.o supmsg.o \
	doprnt.o vsprintf.o salloc.o nxtarg.o skipto.o expand.o errmsg.o \
	atoo.o

SUPSCAN_OBJS= supscan.o stree.o scan.o salloc.o nxtarg.o skipto.o expand.o \
	atoo.o

OBJS=	${SUP_OBJS} ${SUPFILESRV_OBJS} ${SUPSCAN_OBJS}

compile: sup supfilesrv supscan

sup:	${SUP_OBJS}
	${CC} ${CFLAGS} -o $@ ${SUP_OBJS} -lcmucs

supfilesrv: ${SUPFILESRV_OBJS}
	${CC} ${CFLAGS} -o $@ ${SUPFILESRV_OBJS} -lcmucs

supscan: ${SUPSCAN_OBJS}
	${CC} ${CFLAGS} -o $@ ${SUPSCAN_OBJS} -lcmucs

install:
	install -c -s -m 555 -o bin -g bin sup ${USRBIN}
	install -c -s -m 555 -o bin -g bin supscan ${USRBIN}
	install -c -s -m 555 -o bin -g bin supfilesrv ${USRETC}
	@rm -f ${ETC}/supfilesrv
	ln -s ../usr/etc/supfilesrv ${ETC}/supfilesrv

includes:

manpages:
	install -c -m 444 -o bin -g bin sup.man ${MAN1}/sup.1
	install -c -m 444 -o bin -g bin supservers.man ${MAN8}/supservers.8
	rm -f ${MAN8}/supfilesrv.8
	ln -s supservers.8 ${MAN8}/supfilesrv.8
	install -c -m 444 -o bin -g bin sup.doc ${MANDOC}

clean:
	rm -f ${OBJS} core sup supfilesrv supscan Makedep

.EXIT:
	@md -d -f -u Makedep *.d

-include Makedep
