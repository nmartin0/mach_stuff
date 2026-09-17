# /usr/src/usr.bin/macintosh/Makefile

DESTDIR=

SUBDIR=	server patches

all:	${SUBDIR}

${SUBDIR}: FRC
	cd $@; make ${MFLAGS}

install: FRC
	-for i in ${SUBDIR}; do \
	  (cd $$i; make ${MFLAGS} DESTDIR=${DESTDIR} install); \
	done
	install -c -o bin -g bin -m 555 macintosh.sh ${DESTDIR}/usr/bin/macintosh

install.man: FRC
	@echo "*** need man page for macintosh"

clean:	FRC
	for i in ${SUBDIR}; do (cd $$i; make ${MFLAGS} clean); done

FRC:
