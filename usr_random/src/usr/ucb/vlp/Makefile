# Makefile for vlp.

MAN1=	${INSTALL_TREE}/usr/man/man1

compile:
	cd vlp.${TARGET}; make

install:
	cd vlp.${TARGET}; make DESTDIR=${INSTALL_TREE} install

includes:

manpages:
	install -c -m 444 -o bin -g bin vlp.man ${MAN1}/vlp.1

clean:
	cd vlp.${TARGET}; make clean
