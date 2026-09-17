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

# Makefile for MACH3.0+BSD4.3 /lib/pcccpp

LIB=	${INSTALL_TREE}/lib
MAN1=	${INSTALL_TREE}/usr/man/man1

CFLAGS=	-O -Dunix=1 -DFLEXNAMES

YYFIX=	:yyfix.${TARGET}

OBJS=	cpp.o cpy.o rodata.o

compile: pcccpp

pcccpp:	${OBJS}
	cc -o $@ ${OBJS}

cpy.c:	cpy.y
	yacc cpy.y
	sh ${YYFIX} yyexca yyact yypact yypgo yyr1 yyr2 yychk yydef
	mv y.tab.c cpy.c

cpy.o:	cpy.c yylex.c

rodata.o: cpy.c
	cc ${CFLAGS} -R -c rodata.c

install:
	install -c -s -m 555 -o bin -g bin pcccpp ${LIB}/pcccpp

includes:

manpages:
	-install -c -m 444 -o bin -g bin pcccpp.man ${MAN1}/pcccpp.1

clean:
	rm -f pcccpp *.o *.d cpy.c rodata.c Makedep

.EXIT:
	@md -d -f -u Makedep *.d

-include Makedep
