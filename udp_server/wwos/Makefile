#
#   Makefile for UDP Server
#
#   Copyright (c) 1992 Chris Maeda
#
#   This program is free software; you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation; either version 2 of the License, or
#   (at your option) any later version.
#
#   This program is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#   GNU General Public License for more details.
#
#   You should have received a copy of the GNU General Public License
#   along with this program; if not, write to the Free Software
#   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
#
#   Chris Maeda (cmaeda@cs.cmu.edu)
#   School of Computer Science
#   Carnegie Mellon University
#   Pittsburgh PA  15213-3890
#   USA
#

#
# This makefile assumes that the mach include files and
# libraries can be found by the compiler.  The way to
# do this is site-specific so you're on your own.
#
# At CMU, you do this:
# "setpath -i0 /afs/cs.cmu.edu/project/mach3/latest/release/@sys"
# 
# You'll probably have to do something different.
#

CFLAGS = -g -I./timing

# The order matters here
MLIBS = -lnetname -lthreads -lmach

#
# essential network code
#
ES_SRVR = cksum.o netstuff.o uport.o server_funs.o

#
# standalone server code (mostly mig support)
#
SA_SRVR = startup.o server_loop.o udpServer.o $(ES_SRVR)

#
# This code uses some raw UX message stubs so we need to
# pull the message stubs out of the emulator object tree.
# (Usually the emulator is the only thing that deals with UX msg stubs.)
#
# This is where your UX tree lives.
UX_BASE  = /afs/cs/project/mach/member/cmaeda/inet/obj/@sys/ux
# This is where to find the stubs in the tree.
BSD1USER = $(UX_BASE)/emulator/bsd_1_user.o

#
# Brian's timing package.
#
TIMING = ./timing/@sys/timing.o

#
# These programs are used to get server-to-server performance numbers.
# (Use user_client and user_server for user-to-user performance numbers.)
#
T_CLIENT = tclient.o $(ES_SRVR)
T_SERVER = tserver.o $(ES_SRVR)

all: server user_server user_client tclient tserver
 
server: $(SA_SRVR)
	$(CC) $(CFLAGS) $(SA_SRVR) $(BSD1USER) $(MLIBS)
	mv a.out server

user_server: user_server.o udpUser.o
	$(CC) $(CFLAGS) user_server.o udpUser.o -lnetname -lmach
	mv a.out user_server

user_client: user_client.o udpUser.o
	$(CC) $(CFLAGS) user_client.o udpUser.o -lnetname -lmach
	mv a.out user_client

tclient: $(T_CLIENT)
	$(CC) $(CFLAGS) $(T_CLIENT) $(TIMING) $(BSD1USER) $(MLIBS)
	mv a.out tclient

tserver: $(T_SERVER)
	$(CC) $(CFLAGS) $(T_SERVER) $(BSD1USER) $(MACHTRAP) $(MLIBS)
	mv a.out tserver	

udpUser.o : udpUser.c
udpServer.o : udpServer.c

udpUser.c udpServer.c : udp.defs
	mig udp.defs

clean:	
	rm -f *.o server user_server user_client tclient tserver
	rm -f udpUser.c udpServer.c udp.h

	
