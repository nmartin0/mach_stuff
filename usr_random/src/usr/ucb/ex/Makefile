# Makefile for /usr/ucb/ex

VERSION=3.7

USRUCB=	${INSTALL_TREE}/usr/ucb
USRLIB=	${INSTALL_TREE}/usr/lib
USR=	${INSTALL_TREE}/usr
MAN1=	${INSTALL_TREE}/usr/man/man1

#
# Ex is very large - this version will not fit on PDP-11's without overlay
# software.  Things that can be turned off to save
# space include LISPCODE (-l flag, showmatch and lisp options), UCVISUAL
# (visual \ nonsense on upper case only terminals), CHDIR (the undocumented
# chdir command.)  CRYPT includes the code to edit encrypted files (the -x
# option, like ed.)  VMUNIX makes ex considerably larger, raising many limits
# and improving speed and simplicity of maintenance.  It is suitable only
# for a VAX or other large machine, and then probably only in a paged system.
#
# Don't define VFORK unless your system has the VFORK system call,
# which is like fork but the two processes have only one data space until the
# child execs. This speeds up ex by saving the memory copy.
#
# If your system expands tabs to 4 spaces you should -DTABS=4 below
#
OPTIONS=	-DCRYPT -DLISPCODE -DCHDIR -DUCVISUAL -DVFORK -DVMUNIX \
		-DFASTTAG -DUNIX_SBRK

CFLAGS=		-DTABS=8 -O ${OPTIONS}

LDFLAGS=	-z				# or -i or -n

SRCS=	ex.c ex_addr.c ex_cmds.c ex_cmds2.c ex_cmdsub.c ex_data.c ex_get.c \
	ex_io.c ex_put.c ex_re.c ex_set.c ex_subr.c ex_tagio.c ex_temp.c \
	ex_tty.c ex_unix.c ex_v.c ex_vadj.c ex_vget.c ex_vmain.c ex_voper.c \
	ex_vops.c ex_vops2.c ex_vops3.c ex_vput.c ex_vwind.c printf.c

OBJS=	ex.o ex_addr.o ex_cmds.o ex_cmds2.o ex_cmdsub.o ex_data.o ex_extern.o \
	ex_get.o ex_io.o ex_put.o ex_re.o ex_set.o ex_subr.o ex_tagio.o \
	ex_temp.o ex_tty.o ex_unix.o ex_v.o ex_vadj.o ex_vget.o ex_vmain.o \
	ex_voper.o ex_vops.o ex_vops2.o ex_vops3.o ex_vput.o ex_vwind.o \
	printf.o strings.o

compile: ex exrecover expreserve

ex:	${OBJS}
	cc -o $@ ${LDFLAGS} ${OBJS} -ltermcap

.c.o:
	cc -E ${CFLAGS} $*.c | xstr -c -
	cc ${CFLAGS} -c x.c 
	mv x.o $*.o

ex_vars.h: ex_data.c
	csh makeoptions ${CFLAGS}

strings.o: strings
	xstr
	cc -c -S xs.c
	ed - <rofix xs.s
	${AS} -o strings.o xs.s
	rm xs.s

exrecover: exrecover.o ex_extern.o
	cc ${CFLAGS} exrecover.o ex_extern.o -o $@

exrecover.o: exrecover.c
	cc ${CFLAGS} -c -O exrecover.c

expreserve: expreserve.c
	cc ${CFLAGS} expreserve.c -o $@

expreserve.o: expreserve.c
	cc ${CFLAGS} -c -O expreserve.c

install:
	install -c -s -o bin -g bin -m 1755 ex ${USRUCB}/ex
	install -c -s -o root -g bin -m 4755 exrecover ${USRLIB}/ex${VERSION}recover
	install -c -s -o root -g bin -m 4755 expreserve ${USRLIB}/ex${VERSION}preserve
	rm -f ${USRUCB}/edit; ln -s ex ${USRUCB}/edit
	rm -f ${USRUCB}/e; ln -s ex ${USRUCB}/e
	rm -f ${USRUCB}/vi; ln -s ex ${USRUCB}/vi
	rm -f ${USRUCB}/view; ln -s ex ${USRUCB}/view
	-[ -d ${USR}/preserve ] || mkdir ${USR}/preserve
	chown root.bin ${USR}/preserve
	chmod 770 ${USR}/preserve

includes:

manpages:
	install -c -o bin -g bin -m 444 ex.man ${MAN1}/ex.1
	install -c -o bin -g bin -m 444 vi.man ${MAN1}/vi.1
	rm -f ${MAN1}/edit.1; ln -s ex.1 ${MAN1}/edit.1

# if we don't have ex we can't make it so don't rm ex_vars.h
clean:
	rm -f a.out ex exrecover expreserve strings core errs trace
	rm -f *.o x*.[cs] Makedep

.EXIT:
	@md -d -f -u Makedep *.d

-include Makedep
