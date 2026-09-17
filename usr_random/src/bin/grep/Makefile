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

# Makefile for MACH3.0+BSD4.3 /bin/grep /usr/bin/egrep /usr/bin/fgrep

BIN=	${INSTALL_TREE}/bin
USRBIN=	${INSTALL_TREE}/usr/bin
USRLIB=	${INSTALL_TREE}/usr/lib
MAN1=	${INSTALL_TREE}/usr/man/man1

# -DEGREPSTD=path	location of std egrep (normally /usr/bin/egrep).
EGREPSTD=\"${USRLIB}/old.egrep\"

# -DGREPSTD=path	location of std grep (normally /bin/grep).
GREPSTD=\"${USRLIB}/old.bin.grep\"

# -DFGREPSTD=path	location of std fgrep (normally /usr/bin/fgrep).
FGREPSTD=\"${USRLIB}/old.fgrep\"

# -Dstrrchr=rindex, -Dstrchr=index	for troglodytes.
# -DSLOWSYS		invoke xread() for system time quirk on PDP, others? 
# -DNOKANJI		default is for Japanese Unix.  undef only for raw
#			 parity-marked search capability, not standard w/grep.
# -DCHINESE		for systems using EUC Chinese2 codes


CFLAGS=	-O -MD -Dstrrchr=rindex -Dstrchr=index -DNOKANJI \
	-DEGREPSTD=${EGREPSTD} -DGREPSTD=${GREPSTD} -DFGREPSTD=${FGREPSTD}

compile: egrep old.ucb.grep old.bin.grep old.fgrep old.egrep


egrep: egrep.o
	cc $? -o $@

oegrep: oegrep.o
	cc $? -o $@

old.ucb.grep: old.ucb.grep.o
	cc $? -o $@

old.bin.grep: old.bin.grep.o
	cc $? -o $@

old.fgrep: old.fgrep.o
	cc $? -o $@

old.egrep: old.egrep.c old.egrep.o
	cc old.egrep.o -o $@

old.egrep.c: old.egrep.y
	yacc old.egrep.y
	mv y.tab.c old.egrep.c

install:
	install -c -s -m 555 -o bin -g bin egrep ${USRBIN}
	rm -f ${BIN}/grep; ln -s ../usr/bin/egrep ${BIN}/grep
	rm -f ${USRBIN}/fgrep; ln -s egrep ${USRBIN}/fgrep
	install -c -s -m 555 -o bin -g bin old.bin.grep ${USRLIB}
	install -c -s -m 555 -o bin -g bin old.ucb.grep ${USRLIB}
	install -c -s -m 555 -o bin -g bin old.fgrep ${USRLIB}
	install -c -s -m 555 -o bin -g bin old.egrep ${USRLIB}

includes:

manpages:
	install -c -m 444 -o bin -g bin grep.man ${MAN1}/grep.1
	rm -f ${MAN1}/egrep.1; ln -s grep.1 ${MAN1}/egrep.1
	rm -f ${MAN1}/fgrep.1; ln -s grep.1 ${MAN1}/fgrep.1

clean:
	rm -f egrep old.ucb.grep old.bin.grep old.fgrep old.egrep
	rm -f old.egrep.c *.o *.d Makedep

.EXIT:
	@md -d -f -u Makedep *.d

-include Makedep
