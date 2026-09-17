# /usr/src/usr/bin/macintosh/patches/Makefile

DESTDIR=

CFLAGS=	-O -MD -I. -I../common

SYS=	../../../../mach_kernel/kernel

LINKS=	mac2dev mac2os mac2if mac2extra

SRCS=	

OBJS=	msg.o patches.o main.o exception.o \
	mig_support.o macserver_user.o exc_user.o \
	inst.o inst_subr.o inst_patch.o fault.o \
	emul.o emul_misc.o drvr_low.o rtc.o time.o \
	scsi.o sony.o ether.o ether_hash.o \
	input.o clock.o \
	memory.o th.o lock.o prio.o sched.o

LIBS=	/usr/lib/libmach_sa.a

RTOBJ=	start.o

all:	macpatches .depend

links:	FRC
	for i in ${LINKS}; do rm -f $$i; ln -s ${SYS}/$$i $$i; done

macpatches: links macserver.h ${RTOBJ} ${OBJS}
	ld -N -T 40880000 -o macpatches ${RTOBJ} ${OBJS} ${LIBS}

macserver_user.c macserver.h: mac2extra/macserver.defs
	mig $? -header macserver.h -user macserver_user.c -server /dev/null

exc_user.c: ${SYS}/mach/exc.defs
	mig $? -header /dev/null -user exc_user.c -server /dev/null

install: FRC
	install -c -s -o bin -g bin -m 555 macpatches ${DESTDIR}/usr/lib

clean:	FRC
	rm -f .depend ${OBJS} ${RTOBJ} core macpatches \#* *~
	rm -f macserver_user.c exc_user.c
	rm -f ${LINKS}

.depend: ${SRCS}
	md -d -f -u .depend *.d 2>/dev/null

-include .depend

FRC:
