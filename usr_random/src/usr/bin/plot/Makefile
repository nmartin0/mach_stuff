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

# Makefile for MACH3.0+BSD4.3 /usr/bin/plot

USRBIN=	${INSTALL_TREE}/usr/bin
MAN1=	${INSTALL_TREE}/usr/man/man1

CFLAGS=	-O -MD

OBJS=	atoplot.o chrtab.o crtdriver.o crtplot.o debug.o driver.o plottoa.o

STD=	tek t4013 t300 t300s t450 aedplot bgplot crtplot dumbplot gigiplot \
	hpplot hp7221plot implot atoplot plottoa grnplot

compile: ${STD} debug

tek:	driver.o
	cc ${CFLAGS} -o $@ driver.o -l4014 -lm

t4013:	driver.o
	cc ${CFLAGS} -o $@ driver.o -l4013 -lm

t300:	driver.o
	cc ${CFLAGS} -o $@ driver.o -l300 -lm

t300s:	driver.o
	cc ${CFLAGS} -o $@ driver.o -l300s -lm

t450:	driver.o
	cc ${CFLAGS} -o $@ driver.o -l450 -lm

aedplot: driver.o
	cc ${CFLAGS} -o $@ driver.o -lplotaed

bgplot: driver.o
	cc ${CFLAGS} -o $@ driver.o -lplotbg -lm

crtplot: crtdriver.o crtplot.o
	cc ${CFLAGS} -o $@ crtdriver.o crtplot.o -lcurses -ltermcap -lm

dumbplot: driver.o
	cc ${CFLAGS} -o $@ driver.o -lplotdumb -ltermcap -lm

gigiplot: driver.o
	cc ${CFLAGS} -o $@ driver.o -lplotgigi -lm

hpplot:	driver.o
	cc ${CFLAGS} -o $@ driver.o -lplot2648 -lm

hp7221plot: driver.o
	cc ${CFLAGS} -o $@ driver.o -lplot7221 -lm

implot:	driver.o
	cc ${CFLAGS} -o $@ driver.o -lplotimagen -lm

atoplot: atoplot.o
	cc ${CFLAGS} -o $@ atoplot.o -lplot -lm

plottoa: plottoa.o
	cc ${CFLAGS} -o $@ plottoa.o

grnplot: driver.o
	cc ${CFLAGS} -o $@ driver.o -lplotgrn -lm

debug:	debug.o
	cc ${CFLAGS} -o $@ debug.o

install:
	install -c -s -o bin -g bin -m 555 ${STD} ${USRBIN}
	install -c -o bin -g bin -m 555 plot.sh ${USRBIN}/plot

includes:

manpages:
	install -c -o bin -g bin -m 444 plot.man ${DESTDIR}/usr/man/man1/plot.1

clean:
	rm -f ${STD} debug *.o *.d Makedep

.EXIT:
	@md -d -f -u Makedep *.d

-include Makedep
