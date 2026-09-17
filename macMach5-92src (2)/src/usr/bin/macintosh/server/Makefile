# /usr/src/usr/bin/macintosh/server/Makefile

DESTDIR=

CFLAGS=	-O -MD -I. -I../common -DNO_DATA_SUPPLY

SYS=	../../../../mach_kernel/kernel

LINKS=	mac2os mac2extra

SRCS=	catch_exception.c debug.c macserver_user.c main.c console_fix.c \
	opset.c pager_mappings.c pager_routines.c \
	pager_support.c server.c signal.c memory_object_change_completed.c

OBJS=	catch_exception.o debug.o macserver_user.o main.o console_fix.o \
	opset.o pager_mappings.o pager_routines.o \
	pager_support.o server.o signal.o memory_object_change_completed.o

all:	macserver .depend

links:	FRC
	for i in ${LINKS}; do rm -f $$i; ln -s ${SYS}/$$i $$i; done

macserver: links ${OBJS}
	cc -o macserver ${OBJS} -lthreads -lmach

macserver_user.c macserver.h: mac2extra/macserver.defs
	mig -header macserver.h -user macserver_user.c -server /dev/null $?

install: FRC
	install -c -s -o bin -g bin -m 555 macserver ${DESTDIR}/usr/lib

clean:	FRC
	rm -f .depend ${OBJS} core macserver \#* *~
	rm -f mac2test macserver_user.c macserver.h mac.h
	rm -f ${LINKS}

.depend: ${SRCS}
	md -d -f -u .depend *.d 2>/dev/null

-include .depend

FRC:
