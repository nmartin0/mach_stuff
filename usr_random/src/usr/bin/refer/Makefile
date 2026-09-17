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

# Makefile for MACH3.0+BSD4.3 /usr/bin/refer

USRBIN=	${INSTALL_TREE}/usr/bin
USRLIB=	${INSTALL_TREE}/usr/lib
USRDICT=${INSTALL_TREE}/usr/dict
MAN1=	${INSTALL_TREE}/usr/man/man1

CFLAGS=	-O -MD

DLSRCS=	deliv1.c deliv2.c

DLOBJS=	deliv1.o deliv2.o

HTSRCS=	deliv2.c glue4.c glue5.c hunt1.c hunt2.c hunt3.c hunt5.c hunt6.c \
	hunt7.c hunt8.c hunt9.c refer3.c shell.c tick.c

HTOBJS=	deliv2.o glue4.o glue5.o hunt1.o hunt2.o hunt3.o hunt5.o hunt6.o \
	hunt7.o hunt8.o hunt9.o refer3.o shell.o tick.o

INSRCS=	inv1.c inv2.c inv3.c inv5.c inv6.c deliv2.c

INOBJS=	inv1.o inv2.o inv3.o inv5.o inv6.o deliv2.o

MKSRCS=	mkey1.c mkey2.c mkey3.c deliv2.c

MKOBJS=	mkey1.o mkey2.o mkey3.o deliv2.o

RFSRCS=	deliv2.c glue1.c glue2.c glue3.c glue4.c glue5.c hunt2.c hunt3.c \
	hunt5.c hunt6.c hunt7.c hunt8.c hunt9.c mkey3.c refer0.c refer1.c \
	refer2.c refer4.c refer5.c refer6.c refer7.c refer8.c shell.c

RFOBJS=	deliv2.o glue1.o glue2.o glue3.o glue4.o glue5.o hunt2.o hunt3.o \
	hunt5.o hunt6.o hunt7.o hunt8.o hunt9.o mkey3.o refer0.o refer1.o \
	refer2.o refer4.o refer5.o refer6.o refer7.o refer8.o shell.o

WHSRCS=	what1.c what2.c what3.c what4.c shell.c mkey3.c

WHOBJS=	what1.o what2.o what3.o what4.o shell.o mkey3.o

SRCS=	addbib.c lookbib.c ${DLSRCS} ${HTSRCS} ${INSRCS} ${MKSRCS} ${RFSRCS} \
	${WHSRCS}

OBJS=	addbib.o lookbib.o sortbib.o ${DLOBJS} ${HTOBJS} ${INOBJS} ${MKOBJS} \
	${RFOBJS} ${WHOBJS}

compile: addbib deliv hunt inv lookbib mkey refer sortbib whatabout

addbib: addbib.o
	cc addbib.o -o $@

deliv: ${DLOBJS} ${LIBC}
	cc ${DLOBJS} -o $@

hunt: ${HTOBJS} ${LIBC}
	cc ${HTOBJS} -o $@

inv: ${INOBJS} ${LIBC}
	cc ${INOBJS} -o $@

lookbib: lookbib.o
	cc lookbib.o -o $@

mkey: ${MKOBJS}
	cc ${MKOBJS} -o $@

refer: ${RFOBJS}
	cc ${RFOBJS} -o $@

# refpart doesn't compile
#
#RPSRCS=	deliv2.c glue4.c glue5.c mkey3.c refer0.c refer1.c refer2.c \
#	refer3.c refer4.c refer5.c refer6.c refer7.c refer8.c
#RPOBJS=	deliv2.o glue4.o glue5.o mkey3.o refer0.o refer1.o refer2.o \
#	refer3.o refer4.o refer5.o refer6.o refer7.o refer8.o
#refpart: ${RPOBJS} ${LIBC}
#	cc ${CFLAGS} ${RPOBJS} -o $@

sortbib: sortbib.o
	cc  sortbib.o -o $@

whatabout: ${WHOBJS} ${LIBC}
	cc ${WHOBJS} -o $@

install:
	-@[ -d ${USRDICT} ] || mkdir ${USRDICT}
	chown bin.bin ${USRDICT}
	chmod 555 ${USRDICT}
	-@[ -d ${USRLIB}/refer ] || mkdir ${USRLIB}/refer
	chown bin.bin ${USRLIB}/refer
	chmod 555 ${USRLIB}/refer
	-@[ -d ${USRLIB}/tmac ] || mkdir ${USRLIB}/tmac
	chown bin.bin ${USRLIB}/tmac
	chmod 555 ${USRLIB}/tmac
	install -c -s -o bin -g bin -m 755 mkey ${USRLIB}/refer/mkey
	install -c -s -o bin -g bin -m 755 inv ${USRLIB}/refer/inv
	install -c -s -o bin -g bin -m 755 hunt ${USRLIB}/refer/hunt
	install -c -s -o bin -g bin -m 755 refer ${USRBIN}/refer
	install -c -s -o bin -g bin -m 755 addbib ${USRBIN}/addbib
	install -c -s -o bin -g bin -m 755 sortbib ${USRBIN}/sortbib
	install -c -s -o bin -g bin -m 755 lookbib ${USRBIN}/lookbib
	install -c -o bin -g bin -m 755 roffbib.sh ${USRBIN}/roffbib
	install -c -o bin -g bin -m 755 indxbib.sh ${USRBIN}/indxbib
	install -c -o bin -g bin -m 755 tmac.bib ${USRLIB}/tmac
	tar -chf - papers | (cd ${USRDICT}; tar -xvf -)
	find ${USRDICT}/papers -type f -exec chmod 644 {} \;
	find ${USRDICT}/papers -type f -exec chown root.bin {} \;
	chmod 755 ${USRDICT}/papers
	chown root.bin ${USRDICT}/papers
	cd ${USRDICT}/papers; LPATH=${USRLIB}; export LPATH; /bin/sh runinv

includes:

manpages:
	install -c -m 444 -o bin -g bin addbib.man ${MAN1}/addbib.1
	install -c -m 444 -o bin -g bin lookbib.man ${MAN1}/lookbib.1
	install -c -m 444 -o bin -g bin refer.man ${MAN1}/refer.1
	install -c -m 444 -o bin -g bin roffbib.man ${MAN1}/roffbib.1
	install -c -m 444 -o bin -g bin sortbib.man ${MAN1}/sortbib.1
	rm -f ${MAN1}/indxbib.1; ln -s lookbib.1 ${MAN1}/indxbib.1

clean:
	rm -f ${OBJS} core addbib deliv hunt inv lookbib mkey refer \
		refpart sortbib whatabout Makedep

.EXIT:
	@md -d -f -u Makedep *.d

-include Makedep
