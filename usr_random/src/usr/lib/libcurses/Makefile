# Mach Operating System
# Copyright (c) 1992 Carnegie Mellon University
# All Rights Reserved.
# 
# Permission to use, copy, modify and distribute this software and its
# documentation is hereby granted, provided that both the copyright
# notice and this permission notice appear in all copies of the
# software, derivative works or modified versions, and any portions
# thereof, and that both notices appear in supporting documentation.
# 
# CARNEGIE MELLON ALLOWS FREE USE OF THIS SOFTWARE IN ITS "AS IS"
# CONDITION.  CARNEGIE MELLON DISCLAIMS ANY LIABILITY OF ANY KIND FOR
# ANY DAMAGES WHATSOEVER RESULTING FROM THE USE OF THIS SOFTWARE.
# 
# Carnegie Mellon requests users of this software to return to
# 
#  Software Distribution Coordinator  or  Software.Distribution@CS.CMU.EDU
#  School of Computer Science
#  Carnegie Mellon University
#  Pittsburgh PA 15213-3890
# 
# any improvements or extensions that they make and grant Carnegie Mellon 
# the rights to redistribute these changes.

# Makefile for MACH3.0+BSD4.3 /usr/lib/libcurses.a

USRLIB=	${INSTALL_TREE}/usr/lib
INCLUDE=${INSTALL_TREE}/usr/include
MAN3=	${INSTALL_TREE}/usr/man/man3

CFLAGS=	-O -MD

OBJS=	addbytes.o addch.o addstr.o box.o clear.o clrtobot.o clrtoeol.o \
	cr_put.o cr_tty.o curses.o delch.o deleteln.o delwin.o endwin.o \
	erase.o fullname.o getch.o getstr.o idlok.o id_subwins.o initscr.o \
	insch.o insertln.o longname.o move.o mvprintw.o mvscanw.o mvwin.o \
	newwin.o overlay.o overwrite.o printw.o putchar.o refresh.o scanw.o \
	scroll.o toucholap.o standout.o touchwin.o tstp.o unctrl.o

compile: profiled libcurses.a libcurses_p.a

libcurses.a: ${OBJS}
	ar cu $@ ${OBJS}

libcurses_p.a: ${OBJS}
	cd profiled; ar cr ../libcurses_p.a ${OBJS}

profiled:
	mkdir profiled

.c.o:
	@cc ${CFLAGS} -c -p $*.c
	@ld -x -r -o profiled/$*.o $*.o
	cc ${CFLAGS} -c $*.c
	@ld -X -r $*.o
	@mv a.out $*.o

test:
	ranlib libcurses.a
	cc ${CFLAGS} -o tc1 tc1.c libcurses.a
	cc ${CFLAGS} -o tc2 tc2.c libcurses.a
	cc ${CFLAGS} -o tc3 tc3.c libcurses.a

install:
	install -c -m 444 -o bin -g bin libcurses.a ${USRLIB}/libcurses.a
	ranlib ${USRLIB}/libcurses.a
	install -c -m 444 -o bin -g bin libcurses_p.a ${USRLIB}/libcurses_p.a
	ranlib ${USRLIB}/libcurses_p.a

includes:
	@install -p -v -c -m 444 -o bin -g bin curses.h ${INCLUDE}

manpages:
	install -c -m 444 -o bin -g bin curses.man ${MAN3}/curses.3

clean:
	rm -rf profiled
	rm -f ${OBJS} libcurses.a libcurses_p.a Makedep

.EXIT:
	@md -d -f -u Makedep *.d

-include Makedep
