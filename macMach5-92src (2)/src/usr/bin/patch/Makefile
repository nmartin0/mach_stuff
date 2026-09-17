# $Header: Makefile.SH,v 2.0.1.2 88/06/22 20:43:40 lwall Locked $
#
# $Log:	Makefile.SH,v $
# Revision 2.0.1.2  88/06/22  20:43:40  lwall
# patch12: config.h now depends on config.h.SH
# 
# Revision 2.0.1.1  88/06/03  15:00:48  lwall
# patch10: upgraded to match some new metaconfig stuff
# 
# Revision 2.0  86/09/17  15:36:15  lwall
# Baseline for netwide release.
# 
# Revision 1.2  86/09/08  14:07:42  lwall
# Split up patch.c.
# 
# Revision 1.1  86/08/01  20:18:35  lwall
# Initial revision
# 

CC = cc
bin = ${DESTDIR}/usr/bin
mansrc = ${DESTDIR}/usr/man/man1
manext = 1
CFLAGS =  -O
LDFLAGS = 
SMALL = 
LARGE =  


public = patch
private = 
manpages = patch.man
util = Makefile

c = patch.c pch.c inp.c version.c util.c

obj = patch.o pch.o inp.o util.o version.o

lintflags = -phbvxac

addedbyconf = Makefile.old bsd config.h config.sh eunice loc pdp11 usg v7

# grrr
SHELL = /bin/sh

.c.o:
	$(CC) -c $(CFLAGS) $(LARGE) $*.c

all: $(public) $(private) $(util)
	@echo Done.

patch: $(obj)
	$(CC) $(LDFLAGS) $(obj) $(libs) -o patch

config.h: config.h.SH
	sh config.h.SH

install: FRC
	install -c -s -o bin -g bin -m 555 patch ${DESTDIR}/usr/bin/patch

install.man: FRC
	install -c -o bin -g bin -m 444 patch.man ${DESTDIR}/usr/man/man1/patch.1

clean:	FRC
	rm -f *.o *.orig core patch

realclean:
	rm -f patch *.o *.orig *~ core $(addedbyconf)

# The following lint has practically everything turned on.  Unfortunately,
# you have to wade through a lot of mumbo jumbo that can't be suppressed.
# If the source file has a /*NOSTRICT*/ somewhere, ignore the lint message
# for that spot.

lint:
	lint $(lintflags) $(defs) $(c) > patch.fuzz

patch.o: config.h common.h patch.c inp.h pch.h util.h version.h
pch.o: config.h common.h pch.c pch.h util.h
inp.o: config.h common.h inp.c inp.h util.h
util.o: config.h common.h util.c util.h
version.o: config.h common.h version.c version.h patchlevel.h util.h

FRC:
