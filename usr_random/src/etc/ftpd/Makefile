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

# Makefile for MACH3.0+BSD4.3 /etc/ftpd

ETC=	${INSTALL_TREE}/etc
USRETC=	${INSTALL_TREE}/usr/etc
CONFIG=	${INSTALL_TREE}/etc/newconfig
INCLUDE=${INSTALL_TREE}/usr/include
MAN8=	${INSTALL_TREE}/usr/man/man8
FTP=	../../usr/ucb/ftp

VPATH=	.:${FTP}

CFLAGS=	-O -MD

OBJS=	ftpd.o ftpcmd.o glob.o vers.o

compile: ftpd

ftpd:	${OBJS}
	cc ${OBJS} -o $@

#vers.o: newvers.sh
#	-sh newvers.sh
#	cc ${CFLAGS} -c vers.c

install:
	install -c -s -m 544 -o root -g bin ftpd ${USRETC}
	rm -f ${ETC}/ftpd; ln -s ../usr/etc/ftpd ${ETC}/ftpd
	install -c -m 444 -o bin -g bin ftpusers ${CONFIG}

includes:
	@install -p -v -c -m 444 -o bin -g bin ftp.h ${INCLUDE}/arpa

manpages:
	install -c -m 444 -o bin -g bin ftpd.man ${MAN8}/ftpd.8

clean:
	rm -f ftpd *.o *.d Makedep

.EXIT:
	@md -d -f -u Makedep *.d

-include Makedep
