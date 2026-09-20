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

O_TARGET := chr_dev.o
O_OBJS   := console.o 

ifdef CONFIG_GRF_CONSOLE	
        O_OBJS += grf.o	hil.o gkd.o
endif

include $(TOPDIR)/Rules.make
