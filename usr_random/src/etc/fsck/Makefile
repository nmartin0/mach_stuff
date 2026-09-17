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

# Makefile for MACH3.0+BSD4.3 /etc/fsck

ETC=	${INSTALL_TREE}/etc
MAN8=	${INSTALL_TREE}/usr/man/man8

CFLAGS=	-O -MD

OBJS=	dir.o inode.o main.o pass1.o pass1b.o pass2.o pass3.o pass4.o \
	pass5.o setup.o utilities.o ufs_subr.o ufs_tables.o

compile: fsck

fsck:	${OBJS}
	cc ${CFLAGS} ${OBJS} -o $@

install:
	install -c -s -m 544 -o root -g bin fsck ${ETC}

includes:

manpages:
	install -c -m 444 -o bin -g bin fsck.man ${MAN8}/fsck.8

clean:
	rm -f fsck *.o *.d Makedep

.EXIT:
	@md -d -f -u Makedep *.d

-include Makedep
