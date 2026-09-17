# Makefile for sendmail

USRSPL=	${INSTALL_TREE}/usr/spool
MAN1=	${INSTALL_TREE}/usr/man/man1
MAN8=	${INSTALL_TREE}/usr/man/man8

SUBDIR=	lib src aux

compile: ${SUBDIR}

${SUBDIR}: FRC
	cd $@; make

install:
	-[ -d ${USRSPL}/mqueue ] || mkdir ${USRSPL}/mqueue
	chown root.wheel ${USRSPL}/mqueue
	chmod 775 ${DESTDIR}/usr/spool/mqueue
	for i in ${SUBDIR}; do \
	  (cd $$i; make DESTDIR=${INSTALL_TREE} install); \
	done

includes:

manpages:
	install -c -o bin -g bin -m 444 sendmail.man ${MAN8}/sendmail.8
	install -c -o bin -g bin -m 444 newaliases.man ${MAN1}/newaliases.1
	install -c -o bin -g bin -m 444 rmail.man ${MAN1}/rmail.1
	install -c -o bin -g bin -m 444 binmail.man ${MAN1}/binmail.1

clean:
	for i in ${SUBDIR}; do (cd $$i; make clean); done

FRC:

