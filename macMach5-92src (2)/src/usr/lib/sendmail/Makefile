# Makefile for sendmail.

DESTDIR=

SUBDIR=	lib src aux

all:	${SUBDIR}

${SUBDIR}: FRC
	cd $@; make ${MFLAGS}

install: FRC
	-[ -d ${DESTDIR}/usr/spool/mqueue ] || mkdir ${DESTDIR}/usr/spool/mqueue
	chown root.wheel ${DESTDIR}/usr/spool/mqueue
	chmod 775 ${DESTDIR}/usr/spool/mqueue
	for i in ${SUBDIR}; do \
	  (cd $$i; make ${MFLAGS} DESTDIR=${DESTDIR} install); \
	done

install.man: FRC
	install -c -o bin -g bin -m 444 sendmail.man \
	  ${DESTDIR}/usr/man/man8/sendmail.8
	install -c -o bin -g bin -m 444 newaliases.man \
	  ${DESTDIR}/usr/man/man1/newaliases.1
	install -c -o bin -g bin -m 444 rmail.man \
	  ${DESTDIR}/usr/man/man1/rmail.1
	install -c -o bin -g bin -m 444 binmail.man \
	  ${DESTDIR}/usr/man/man1/binmail.1

clean:	FRC
	for i in ${SUBDIR}; do (cd $$i; make ${MFLAGS} clean); done

FRC:
