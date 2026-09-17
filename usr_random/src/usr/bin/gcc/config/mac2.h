/* Definitions of target machine for GNU compiler.
   mac2 (Apple Macintosh II) 68020/68030 version.
   Copyright (C) 1987, 1988 Free Software Foundation, Inc.

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

#include "m68k.h"

/* See m68k.h.  7 means 68020 with 68881.  */

#undef TARGET_DEFAULT
#define TARGET_DEFAULT 7

/* Define __HAVE_FPA__ or __HAVE_68881__ in preprocessor,
   This will control the use of inline 68881 insns in certain macros.
   Also inform the program which CPU this is for.  */

/* -m68881 is the default */
#define CPP_SPEC \
"%{!msoft-float:%{mfpa:-D__HAVE_FPA__ }%{!mfpa:-D__HAVE_68881__ }}\
%{m68000:-D__mc68010__}%{mc68000:-D__mc68010__}%{!mc68000:%{!m68000:-D__mc68020__}} \
%{!ansi:%{m68000:-Dmc68010}%{mc68000:-Dmc68010}%{!mc68000:%{!m68000:-Dmc68020}}}"

/* -fwritable-strings is the default */
#define CC1_SPEC "-fwritable-strings"

#define PTRDIFF_TYPE "int"
#define SIZE_TYPE "int"

/* These compiler options take an argument.  We ignore -target for now.  */
#define WORD_SWITCH_TAKES_ARG(STR)				\
 (!strcmp (STR, "Tdata") || !strcmp (STR, "include")		\
  || !strcmp (STR, "imacros") || !strcmp (STR, "target")	\
  || !strcmp (STR, "assert") || !strcmp (STR, "aux-info"))

/* Names to predefine in the preprocessor for this target machine.  */
#define CPP_PREDEFINES "-Dmc68000 -Dmac2 -Dunix -DMACH -DCMU -DCMUCS"

/* Specify library to handle `-a' basic block profiling.
   Control choice of libm.a (if user says -lm)
   based on fp arith default and options.  */

/* -m68881 is the default */
#define LIB_SPEC "%{g:-lg} %{!p:%{!pg:-lc}}%{p:-lc_p}%{pg:-lc_p} \
%{a:/usr/lib/bb_link.o} %{g:-lg} \
%{msoft-float:-L/usr/lib/fsoft}%{!msoft-float:%{!mfpa:-L/usr/lib/f68881}}\
%{mfpa:-L/usr/lib/ffpa}"

/* Provide required defaults for linker -e switche.  */
#define LINK_SPEC "%{!e*:-e start} %{static:-Bstatic} %{assert*}"

/* Every structure or union's size must be a multiple of 2 bytes.  */
#define STRUCTURE_SIZE_BOUNDARY 16

/* Generate calls to memcpy, memcmp and memset.  */
#define TARGET_MEM_FUNCTIONS
