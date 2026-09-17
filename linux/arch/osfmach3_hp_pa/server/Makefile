# Copyright (c) Open Software Foundation, Inc.   
# 
# 
# pmk1.1


#
# Makefile for the linux i386-specific parts of the OSF/Mach3 server.
#
# Note! Dependencies are done automagically by 'make dep', which also
# removes any old dependencies. DON'T put your own dependencies here
# unless it's something special (ie not a .c file).
#
# Note 2! The CFLAGS definition is now in the main makefile...

O_TARGET := server.o
O_OBJS	 := parent_osf1_syscalls.o trap.o

.S.o:
	$(CC) -DASSEMBLER -traditional -c $< -o $*.o

include $(TOPDIR)/Rules.make

