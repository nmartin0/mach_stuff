# Copyright (c) Open Software Foundation, Inc.   
# 
# 
# pmk1.1



#
# Makefile for the linux kernel.
#
# Note! Dependencies are done automagically by 'make dep', which also
# removes any old dependencies. DON'T put your own dependencies here
# unless it's something special (ie not a .c file).
#
# Note 2! The CFLAGS definitions are now in the main makefile...

all: kernel.o 

O_TARGET := kernel.o
O_OBJS   := process.o misc_c.o traps.o setup.o ksyms.o port_io.o misc.o signal.o ptrace.o \
	    syscalls.o 

.S.o:
	$(CC) -DASSEMBLER -traditional -c $< -o $*.o

include $(TOPDIR)/Rules.make

