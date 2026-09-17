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

# Makefile for MACH3.0+BSD4.3 /usr/bin/efl

USRBIN=	${INSTALL_TREE}/usr/bin
MAN1=	${INSTALL_TREE}/usr/man/man1

CFLAGS=	-O -MD -traditional

OBJS=	addr.o alloc.o blklab.o dcl.o dclgen.o error.o exec.o field.o free.o \
	gram.o icfile.o init.o io.o lex.o main.o misc.o mk.o namgen.o pass2.o \
	print.o simple.o struct.o symtab.o tailor.o temp.o

compile:

efl:	${OBJS}
	cc ${OBJS} -o $@
# gram.c can no longer be made on a pdp11 because of yacc limits
gram.c: gram.head gram.dcl gram.expr gram.exec tokdefs
	(sed < tokdefs "s/#define/%token/" ; \
	cat gram.head gram.dcl gram.expr gram.exec) > gram.in
	${YACC} ${YFLAGS} gram.in
	(echo "#define YYDEBUG"; cat y.tab.c) > gram.c
	rm -f gram.in y.tab.c

lex.c: fixuplex
	lex lex.l
	sh fixuplex
	mv lex.yy.c lex.c

tokdefs: tokens
	grep -n . < tokens | sed "s/\([^:]*\):\(.*\)/#define \2 \1/" > tokdefs

TEST:
	./efl "system=gcos" efltest/Hard.e >z1 2>z2
	cmp z1 efltest/Hard.out
	./efl "system=gcos" efltest/Band.e >z1 2>z2
	cmp z1 efltest/Band.out
	./efl "system=gcos" efltest/Buram.e >z1 2>z2
	cmp z1 efltest/Buram.out
	./efl "system=gcos" efltest/Dgl.e >z1 2>z2
	cmp z1 efltest/Dgl.out
	rm -f z1 z2
	@echo TEST OK

install:
	echo 'echo "*** efl not ported"' >${USRBIN}/efl
	chown bin.bin ${USRBIN}/efl
	chmod 555 ${USRBIN}/efl
#	install -c -s -m 555 -o bin -g bin efl ${USRBIN}

includes:

manpages:
	install -c -m 444 -o bin -g bin efl.man ${MAN1}/efl.1

clean:
	rm -f efl gram.c lex.c tokdefs *.o *.d Makedep

.EXIT:
	@md -d -f -u Makedep *.d

-include Makedep
