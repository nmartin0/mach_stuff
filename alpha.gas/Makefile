# Copyright (C) 1988, 1989, 1991 Free Software Foundation, Inc.
# This file is part of GNU gas
# 
# GNU Make is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2, or (at your option)
# any later version.
#
# GNU Make is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# 
# You should have received a copy of the GNU General Public License
# along with GNU Make; see the file COPYING.  If not, write to
# the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.

#
# HISTORY
# 20-Oct-93  Mary Thompson (mrt) at Carnegie-Mellon University
#	Created. copied from Makefile for GnuMake.
#
# $Log:	Makefile,v $
# 
#
#
#


# Name under which to install GNU gas
instname = gas
# Directory to install `gas' in.
bindir = $(prefix)/usr/local/bin
# Directory to install the man page in.
mandir = $(prefix)/usr/local/man/manl
# Number to put on the man page filename.
manext = l


OFILES		= as.o xrealloc.o xmalloc.o hash.o hex-value.o \
		  atof-generic.o append.o messages.o expr.o \
		  app.o frags.o input-file.o input-scrub.o \
		  output-file.o subsegs.o symbols.o version.o \
		  flonum-const.o flonum-copy.o flonum-mult.o \
		  strstr.o bignum-copy.o obstack.o write.o \
		  read.o list.o ${ALPHA_OFILES}


# Set if building in a CMU env using gcc
ALPHA_CFLAGS	= -DEXPR_TAKES_BIGNUMS -DFIRST_32BIT_QUADRANT=1 -DMACHINE_BITS_PER_LONG=64
ALPHA_OFILES	= alpha.o atof-ieee.o atof-vax.o
LDLIBS		= -lcs -lc

# Set if building in an OSF/1 env using the DEC mips compiler
#ALPHA_CFLAGS	= -g3 -Dalpha -DEXPR_TAKES_BIGNUMS -DMACHINE_BITS_PER_LONG=64
#ALPHA_OFILES	= alpha.o atof-ieee.o atof-vax.o alloca.o

#Set if cross building on a DecStation using DEC mips compiler
#ALPHA_CFLAGS	= -g3 -Dalpha -Umips -DMIPS_HOST=1 -DCROSS_COMPILE_32_TO_64_BITS=1 -DEXPR_TAKES_BIGNUMS \
		   -DFIRST_32BIT_QUADRANT=1 -DMACHINE_BITS_PER_LONG=64
#ALPHA_OFILES	= alpha.o atof-ieee.o atof-vax.o alloca.o cross_32_to_64.o

CFLAGS			= -DUSE_SYSTEM_HDR ${ALPHA_CFLAGS} ${$@_CFLAGS}

xrealloc.o_CFLAGS	= -Derror=as_fatal
xmalloc.o_CFLAGS	= -Derror=as_fatal
hash.o_CFLAGS		= -Derror=as_fatal

.SUFFIXES:
.SUFFIXES: .o .c .h 

all: gas

gas: $(OFILES)
	$(CC) $(CFLAGS) $(OFILES) $(LDLIBS) -o gas.new
	mv -f gas.new gas

.PHONY: install
install: $(bindir)/$(instname) 

$(bindir)/$(instname): gas
	cp gas $@.new

.PHONY: clean
clean: 
	-rm -f gas *.o core


alpha.o:	as.h read.h flonum.h bignum.h md.h \
		obstack.h frags.h struc-symbol.h a_out.h expr.h \
		symbols.h a_out.h alpha-opcode.h
atof-ieee.o:	flonum.h bignum.h
atof-vax.o:	as.h read.h flonum.h bignum.h

as.o:		as.h struc-symbol.h a_out.h write.h
atof-generic.o:	flonum.h bignum.h
bignum-copy.o:	bignum.h
expr.o:		as.h flonum.h bignum.h read.h struc-symbol.h a_out.h \
		expr.h obstack.h symbols.h
flonum-const.o:	flonum.h bignum.h
flonum-copy.o:	flonum.h bignum.h
flonum-mult.o:	flonum.h bignum.h
frags.o:	as.h subsegs.h obstack.h frags.h struc-symbol.h a_out.h
hash.o:		hash.h
input-file.o:	input-file.h
input-scrub.o:	as.h read.h input-file.h
list.o:		list.c as.h expr.h flonum.h frags.h hash.h md.h read.h \
		struc-symbol.h write.h
messages.o:	as.h
obstack.o:	obstack.h
read.o:		as.h read.h md.h hash.h obstack.h frags.h flonum.h \
		bignum.h struc-symbol.h a_out.h expr.h symbols.h
subsegs.o:	as.h subsegs.h obstack.h frags.h struc-symbol.h \
		a_out.h write.h
symbols.o:	as.h hash.h obstack.h struc-symbol.h a_out.h \
		symbols.h frags.h
write.o:	as.h md.h subsegs.h obstack.h struc-symbol.h a_out.h \
		write.h symbols.h

