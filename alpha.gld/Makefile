# Copyright (C) 1988, 1989, 1991 Free Software Foundation, Inc.
# This file is part of GNU gld
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


# Name under which to install GNU gld
instname = gld
# Directory to install `gld' in.
bindir = $(prefix)/usr/local/bin
# Directory to install the man page in.
mandir = $(prefix)/usr/local/man/manl
# Number to put on the man page filename.
manext = l


OFILES		= gld.o cplus-dem.o

# choose if buiding in CMU env
CFLAGS			= -O0 -Dalpha -Umips -DCMUCS -Dnounderscore
# needs openp
LDLIBS			= -lcs

# choose if building in OSF/1 env using DEC MIPS compiler and building
# a loader to generate excutables to run under Mach OS
#CFLAGS			= -I. -g3 -Dalpha -Umips -Dnounderscore
#EXTRA_OFILES		= alloca.o

# choose if building in OSF/1 env using DEC MIPS compiler and building
# a loader to generate excutables to run under OSF/1
#CFLAGS			= -I. -g3 -Dalpha -Umips -DWANT_COFF
#EXTRA_OFILES		= alloca.o
#OBJECTS		= crt0.o osf_ld_symbols.o __main.o

#set if cross building from DecStation running CMUCS environment
#CFLAGS			= -I. -g3 -Dalpha -Umips -Dnounderscore -DCROSS_COMPILE_32_TO_64_BITS
#EXTRA_OFILES			= alloca.o cross_32_to_64.o
#LDLIBS			= -lcs

#set if cross building from DecStation running Ultrix using DEC MIPS compiler
#CFLAGS			= -I. -g3 -Dalpha -Umips -Dnounderscore -DCROSS_COMPILE_32_TO_64_BITS
#EXTRA_OFILES			= alloca.o cross_32_to_64.o

.SUFFIXES:
.SUFFIXES: .o .c .h .s

all: gld $(OBJECTS)

gld: $(OFILES) $(EXTRA_OFILES) 
	$(CC) $(CFLAGS) $(OFILES) $(EXTRA_OFILES) $(LDLIBS) -o gld.new
	mv -f gld.new gld

.PHONY: install
install: $(bindir)/$(instname) 

$(bindir)/$(instname): gld
	cp gld $@.new

.PHONY: clean
clean: 
	-rm -f gld *.o core


