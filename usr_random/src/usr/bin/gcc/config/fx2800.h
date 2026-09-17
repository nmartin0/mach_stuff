/* Target definitions for GNU compiler for Alliant FX/2800 w/Concentrix 2.x
   Copyright (C) 1991 Free Software Foundation, Inc.

   Written by Ron Guilmette (rfg@ncd.com).

This file is part of GNU CC.

GNU CC is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.

GNU CC is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with GNU CC; see the file COPYING.  If not, write to
the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.  */

/* The Alliant fx2800 running Concentrix 2.1 is weird.  This is basically
   a BSD 4.3 based operating system, but it runs svr4 ELF format object
   files and it somehow puts BSD stabs records into the ELF files for
   symbolic debug information.  It also seems to use the assembler from
   the svr4 reference port, but that assembler has probably been hacked
   slightly to allow the BSD stabs to be smuggled in.

   Given that we need to use svr4-style pseudo-ops, and given that the
   target object file format is ELF, we just include "i860v4.h" here so
   as to pick-up a set of defines appropriate for for an svr4/ELF target
   and then we just override the few things that are special for the FX/2800
   and Concentrix.  */

#include "i860v4.h"

#undef DWARF_DEBUGGING_INFO
#define DBX_DEBUGGING_INFO

/* Provide a set of pre-definitions and pre-assertions appropriate for
   the i860 running Concentrix 2.1.  */

#undef CPP_PREDEFINES
#define CPP_PREDEFINES "-Di860 -Dunix -DBSD4_3 -Dalliant -Asystem(unix) -Acpu(i860) -Amachine(i860)"

/* Undefine some things defined in i860.h because the native C compiler
   on the FX/2800 emits code to do these operations inline.  For GCC,
   we will use the default implementation of these things... i.e.
   generating calls to libgcc1.c routines.  */

#undef DIVSI3_LIBCALL
#undef UDIVSI3_LIBCALL
#undef REMSI3_LIBCALL
#undef UREMSI3_LIBCALL
