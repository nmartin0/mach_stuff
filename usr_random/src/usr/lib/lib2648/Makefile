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

# Makefile for MACH3.0+BSD4.3 /usr/lib/lib2648.a

USRLIB=	${INSTALL_TREE}/usr/lib
MAN3=	${INSTALL_TREE}/usr/man/man3

CFLAGS=	-DTRACE -O -MD

OBJS=	agoto.o aminmax.o aon.o areaclear.o beep.o bitcopy.o cleara.o \
	clearg.o curon.o dispmsg.o draw.o drawbox.o dumpmat.o \
	emptyrow.o error.o escseq.o gdefault.o gon.o kon.o line.o mat.o \
	message.o minmax.o motion.o move.o movecurs.o newmat.o outchar.o \
	outstr.o printg.o rawchar.o rbon.o rdchar.o readline.o set.o \
	setmat.o sync.o texton.o ttyinit.o update.o video.o zermat.o \
	zoomn.o zoomon.o zoomout.o

compile: lib2648.a

lib2648.a: ${OBJS}
	ar cr lib2648.a `lorder ${OBJS} | tsort`

install:
	install -c -o bin -g bin -m 644 lib2648.a ${USRLIB}
	ranlib ${USRLIB}/lib2648.a

includes:

manpages:
	install -c -m 444 -o bin -g bin lib2648.man ${MAN3}/lib2648.3

clean:
	rm -f lib2648.a *.o *.d Makedep

.EXIT:
	@md -d -f -u Makedep *.d

-include Makedep
