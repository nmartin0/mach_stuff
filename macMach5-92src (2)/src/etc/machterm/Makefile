# Makefile for machterm.

DESTDIR=

SYS=	../../mach_kernel

BINDIR=${DESTDIR}/etc

MANDIR=${DESTDIR}/usr/man/man8

CFLAGS= -O -MD

SRCS=	main.c machterm.c

OBJS=	main.o machterm.o

LIBS=	-ltermcap

all:	machterm .depend

machterm: ${OBJS}
	cc -o $@ ${OBJS} ${LIBS}

machterm.c: ${SYS}/kernel/mac2dev/machterm.c
	ln -s ${SYS}/kernel/mac2dev/machterm.c machterm.c

install: FRC
	install -c -s -o root -g bin -m 4555 machterm ${BINDIR}/machterm

install.man: FRC
	@echo "*** need man page for machterm"
#	install -c -o bin -g bin -m 444 machterm.man ${MANDIR}/machterm.8

clean:	FRC
	rm -f .depend ${OBJS} machterm

.depend: ${SRCS}
	md -d -f -u .depend *.d 2>/dev/null

FRC:

-include .depend

