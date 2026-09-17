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

# Makefile for MACH3.0+BSD4.3 /usr/bin/struct

USRBIN=	${INSTALL_TREE}/usr/bin
USRLIB=	${INSTALL_TREE}/usr/lib
MAN1=	${INSTALL_TREE}/usr/man/man1

CFLAGS=	-O -MD

YFLAGS=	-d

BSRCS=	beauty.c tree.c lextab.c bdef.c

BOBJS=	beauty.o tree.o lextab.o bdef.o

SSRCS=	0.alloc.c 0.args.c 0.def.c 0.extr.c 0.graph.c 0.list.c 0.parts.c \
	0.string.c 1.finish.c 1.form.c 1.fort.c 1.hash.c 1.init.c 1.line.c \
	1.main.c 1.node.c 1.recog.c 1.tables.c 2.dfs.c 2.dom.c 2.head.c \
	2.inarc.c 2.main.c 2.tree.c 3.branch.c 3.flow.c 3.loop.c 3.main.c \
	3.reach.c 3.then.c 4.brace.c 4.form.c 4.main.c 4.out.c main.c

SOBJS=	0.alloc.o 0.args.o 0.def.o 0.extr.o 0.graph.o 0.list.o 0.parts.o \
	0.string.o 1.finish.o 1.form.o 1.fort.o 1.hash.o 1.init.o 1.line.o \
	1.main.o 1.node.o 1.recog.o 1.tables.o 2.dfs.o 2.dom.o 2.head.o \
	2.inarc.o 2.main.o 2.tree.o 3.branch.o 3.flow.o 3.loop.o 3.main.o \
	3.reach.o 3.then.o 4.brace.o 4.form.o 4.main.o 4.out.o main.o

SRCS=	${BSRCS} ${SSRCS}

OBJS=	${BOBJS} ${SOBJS}

compile: beautify structure

beautify: ${BOBJS}
	cc ${CFLAGS} ${BOBJS} -o $@ -lln

structure: ${SOBJS}
	cc ${CFLAGS} ${SOBJS} -o $@

install:
	-[ -d ${USRLIB}/struct ] || mkdir ${USRLIB}/struct
	-chmod 755 ${USRLIB}/struct
	-chown root.bin ${USRLIB}/struct
	install -c -s -o bin -g bin -m 755 structure $(DESTDIR)/usr/lib/struct/structure
	install -c -s -o bin -g bin -m 755 beautify $(DESTDIR)/usr/lib/struct/beautify
	install -c -o bin -g bin -m 755 struct $(DESTDIR)/usr/bin/struct

includes:

manpages:
	install -c -m 444 -o bin -g bin struct.man ${MAN1}/struct.1

clean:
	rm -f ${OBJS} core beauty.c structure beautify y.tab.h lextab.c Makedep

.EXIT:
	@md -d -f -u Makedep *.d

-include Makedep
