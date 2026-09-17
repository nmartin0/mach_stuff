# Makefile for uemacs.

USRBIN=	${INSTALL_TREE}/usr/bin
USRLIB=	${INSTALL_TREE}/usr/lib
MAN1=	${INSTALL_TREE}/usr/man/man1

CFLAGS=	-O -MD -traditional

OFILES=	basic.o bind.o buffer.o char.o crypt.o display.o eval.o \
	exec.o file.o fileio.o input.o isearch.o line.o lock.o main.o \
	mouse.o random.o region.o search.o tcap.o unix.o window.o word.o

CFILES=	basic.c bind.c buffer.c char.c crypt.c display.c eval.c \
	exec.c file.c fileio.c input.c isearch.c line.c lock.c main.c \
	mouse.c random.c region.c search.c tcap.c unix.c window.c word.c

HFILES=	estruct.h edef.h efunc.h epath.h ebind.h evar.h etype.h

LFILES=	emacs.hlp emacs.rc emacs.tut emacs.txt menu.cmd fortran.rc \
	azbmap.cmd azmap.cmd bpage.cmd bufdel.cmd cpage.cmd encode.cmd \
	epage.cmd epson.cmd error.cmd func.cmd iwrap.cmd menu.cmd \
	newpage.cmd opage.cmd platin.cmd pos.cmd ppage.cmd scan.cmd \
	sentence.cmd shell.cmd smooth.cmd table.cmd wpage.cmd

compile: uemacs

uemacs:	$(OFILES)
	$(CC) $(CFLAGS) $(OFILES) -ltermcap -lc -o uemacs

$(OFILES): $(HFILES)

install:
	install -c -s -o bin -g bin -m 755 uemacs ${USRBIN}/uemacs
	rm -rf ${USRLIB}/uemacs
	mkdir ${USRLIB}/uemacs
	chmod 755 ${USRLIB}/uemacs
	install -c -m 444 -o bin -g bin ${LFILES} ${USRLIB}/uemacs

includes:

manpages:

clean:
	rm -f *.o uemacs Makedep

.EXIT:
	@md -d -f -u Makedep *.d

-include Makedep
