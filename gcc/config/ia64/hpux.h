/* Definitions of target machine GNU compiler.  IA-64 version.
   Copyright (C) 1999, 2000, 2001 Free Software Foundation, Inc.
   Contributed by Steve Ellcey <sje@cup.hp.com> and
                  Reva Cuthbertson <reva@cup.hp.com>

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
the Free Software Foundation, 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.  */

/* This macro is a C statement to print on `stderr' a string describing the
   particular machine description choice.  */

#define TARGET_VERSION fprintf (stderr, " (IA-64) HP-UX");

#undef CPP_PREDEFINES
#define CPP_PREDEFINES "\
  -D__IA64__ -D__ia64 -D__ia64__ -D__hpux -D__hpux__ -Dhpux -Dunix \
  -D__BIG_ENDIAN__ -D_LONGLONG -D__ELF__ \
  -Asystem=hpux -Asystem=posix -Asystem=unix -Acpu=ia64 -Amachine=ia64 \
  -D_UINT128_T"

/* -D__fpreg=long double is needed to compensate for the lack of __fpreg
   which is a primitive type in HP C but does not exist in GNU C.  Same
   for __float80 and __float128.  These types appear in HP-UX header
   files and so must have some definition.  */

#undef CPP_SPEC
#define CPP_SPEC "\
  %{mcpu=itanium:-D__itanium__} \
  -D__LP64__ -D__LONG_MAX__=9223372036854775807L \
  %{!ansi:%{!std=c*:%{!std=i*: -D_HPUX_SOURCE -D__STDC_EXT__}}} \
  -D__fpreg=long\\ double \
  -D__float80=long\\ double \
  -D__float128=long\\ double"

#undef ASM_SPEC
#define ASM_SPEC "-x %{mconstant-gp} %{mauto-pic}"

#undef ENDFILE_SPEC

#undef STARTFILE_SPEC
#ifdef CROSS_COMPILE
#define STARTFILE_SPEC "%{!shared:crt0%O%s}"
#else
#define STARTFILE_SPEC "/usr/ccs/lib/hpux64/crt0%O"
#endif

#undef LINK_SPEC
#define LINK_SPEC "\
  +Accept TypeMismatch \
  %{shared:-b} \
  %{!shared: \
    -u main \
    %{!static: \
      %{rdynamic:-export-dynamic}} \
      %{static:-static}}"

#undef  LIB_SPEC
#define LIB_SPEC "%{!shared:%{!symbolic:-lc}}"

#define DONT_USE_BUILTIN_SETJMP
#define JMP_BUF_SIZE  (8 * 76)

#undef CONST_SECTION_ASM_OP
#define CONST_SECTION_ASM_OP    "\t.section\t.rodata,\t\"a\",\t\"progbits\""

#undef BITS_BIG_ENDIAN
#define BITS_BIG_ENDIAN 1

#undef TARGET_DEFAULT
#define TARGET_DEFAULT (MASK_DWARF2_ASM | MASK_BIG_ENDIAN)

/* We need this macro to output DWARF2 information correctly.  The macro
   is defined in dwarf2out.c, but it will not do section relative offsets
   which messes up our ability to debug using gdb.  */

#undef ASM_OUTPUT_DWARF_OFFSET
#define ASM_OUTPUT_DWARF_OFFSET(FILE,LABEL)				\
 do {									\
	fprintf ((FILE), "\t%s\t", UNALIGNED_OFFSET_ASM_OP);		\
	fprintf ((FILE), "@secrel(");                                   \
	assemble_name (FILE, LABEL);					\
	fprintf ((FILE), ")");                                          \
  } while (0)
