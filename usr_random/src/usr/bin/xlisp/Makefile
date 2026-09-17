# Makefile for xlisp

USRBIN=	${INSTALL_TREE}/usr/bin
MANDOC=	${INSTALL_TREE}/usr/man/doc

CFLAGS=	-MD -O

DOCS=	XlispOOP.doc XlispRef.doc xlisp.doc

SRCS=	unixstuff.c xlbfun.c xlcont.c xldbug.c xldmem.c xleval.c xlfio.c \
	xlftab.c xlglob.c xlimage.c xlinit.c xlio.c xlisp.c xljump.c \
	xllist.c xlmath.c xlobj.c xlpp.c xlprin.c xlread.c xlstr.c \
	xlstruct.c xlsubr.c xlsym.c xlsys.c osfun.c

OBJS=	unixstuff.o xlbfun.o xlcont.o xldbug.o xldmem.o xleval.o xlfio.o \
	xlftab.o xlglob.o xlimage.o xlinit.o xlio.o xlisp.o xljump.o \
	xllist.o xlmath.o xlobj.o xlpp.o xlprin.o xlread.o xlstr.o \
	xlstruct.o xlsubr.o xlsym.o xlsys.o osfun.o

compile: xlisp

xlisp:	${OBJS}
	cc ${OBJS} -o xlisp -lcurses -ltermcap

install:
	install -c -s -o bin -g bin -m 755 xlisp ${USRBIN}

includes:

manpages:
	-[ -d ${MANDOC}/xlisp ] || mkdir ${MANDOC}/xlisp
	chown bin.bin ${MANDOC}/xlisp
	chmod 555 ${MANDOC}/xlisp
	install -c -o bin -g bin -m 444 ${DOCS} ${MANDOC}/xlisp

clean:
	rm -f ${OBJS} xlisp Makedep

xlftab.o: osdefs.h osptrs.h

.EXIT:
	@md -d -f -u Makedep *.d

-include Makedep
