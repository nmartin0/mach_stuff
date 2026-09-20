# Copyright (c) 1991-1998 Open Software Foundation, Inc. 
#  
# 
# 
# MkLinux
#

#
# Makefile for the linux server on top of the OSF/Mach3 micro-kernel.
#
# Note! Dependencies are done automagically by 'make dep', which also
# removes any old dependencies. DON'T put your own dependencies here
# unless it's something special (ie not a .c file).
#
# Note 2! The CFLAGS definitions are now in the main makefile...

O_TARGET := chr_dev.o
O_OBJS   := adb.o char_dev.o atixlmouse.o busmouse.o console.o lp.o mem.o \
	misc.o mouse.o \
	msbusmouse.o psaux.o sunmouse.o \
	st.o tpqic02.o

ifeq ($(CONFIG_SERIAL),y)
O_OBJS += serial.o
else
  ifeq ($(CONFIG_SERIAL),m)
  MX_OBJS += serial.o
  endif
endif

ifeq ($(CONFIG_MACMOUSE),y)
O_OBJS += adbmouse.o
else
  ifeq ($(CONFIG_MACMOUSE),m)
  MX_OBJS += adbmouse.o
  endif
endif

ifeq ($(CONFIG_VIDEO_DEV),y)
O_OBJS += videodev.o
else
  ifeq ($(CONFIG_VIDEO_DEV),m)
    MX_OBJS += videodev.o
  endif
endif

ifeq ($(CONFIG_VIDEO_PLANB),y)
O_OBJS += planb.o
else
  ifeq ($(CONFIG_VIDEO_PLANB),m)
  MX_OBJS += planb.o
  endif
endif

include $(TOPDIR)/Rules.make
