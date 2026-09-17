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

# Makefile for /usr/bin/gcc which is /bin/cc

BIN=	${INSTALL_TREE}/bin
LIB=	${INSTALL_TREE}/lib
USR=	${INSTALL_TREE}/usr
USRBIN=	${INSTALL_TREE}/usr/bin
INFODIR= ${USR}/man/info

INFO =	cpp.info cpp.info-1 cpp.info-2 \
	gcc.info gcc.info-1 gcc.info-2 gcc.info-3 gcc.info-4 gcc.info-5 \
	gcc.info-6 gcc.info-7 gcc.info-8 gcc.info-9 gcc.info-10

compile: Makefile
	@echo "*** INSTALL_TREE=\"${INSTALL_TREE}\""
	-@if [ "${HOST}" = "${TARGET}" ]; then \
	  make -f Makefile CFLAGS=-O prefix=${USR} all.internal ;\
	else \
	  make -f Makefile CFLAGS=-O prefix=${USR} all.cross ;\
	fi

Makefile:
	-@if [ "${HOST}" = "${TARGET}" ]; then \
	  sh configure ${TARGET} ;\
	else \
	  sh configure --host=${HOST} --target=${TARGET} ;\
	fi

install:
	rm -f ${USRBIN}/gcc-${TARGET}
	-@if [ "${HOST}" = "${TARGET}" ]; then \
	  make -f Makefile prefix=${USR} install-common ;\
	  make -f Makefile prefix=${USR} install-libgcc ;\
	else \
	  make -f Makefile prefix=${USR} install-common ;\
	  make -f Makefile prefix=${USR} install-cross-tools ;\
	  make -f Makefile prefix=${USR} install-libgcc ;\
	fi
	@rm -f ${BIN}/cc
	ln -s ../usr/bin/gcc ${BIN}/cc
	@rm -f ${LIB}/cpp
	ln -s ../usr/lib/gcc-lib/${TARGET}/2.1/cpp ${LIB}/cpp
	@rm -f ${LIB}/libgcc.a
	ln -s ../usr/lib/gcc-lib/${TARGET}/2.1/libgcc.a ${LIB}/libgcc.a

includes:

manpages:
	make -f Makefile prefix=${USR} install-man
#	make -f Makefile doc
#	install -c -o bin -g bin -m 444 ${INFO} ${INFODIR}

clean:
	-make -f Makefile realclean
