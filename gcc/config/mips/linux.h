/* Definitions for MIPS running Linux-based GNU systems with ELF format.
   Copyright (C) 1998, 1999, 2000, 2001 Free Software Foundation, Inc.

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

#undef TARGET_VERSION
#if TARGET_ENDIAN_DEFAULT == 0
#define TARGET_VERSION fprintf (stderr, " (MIPSel GNU/Linux with ELF)");
#else
#define TARGET_VERSION fprintf (stderr, " (MIPS GNU/Linux with ELF)");
#endif

#undef MD_EXEC_PREFIX
#undef MD_STARTFILE_PREFIX

/* Required to keep collect2.c happy */
#undef OBJECT_FORMAT_COFF

/* If we don't set MASK_ABICALLS, we can't default to PIC. */
#undef TARGET_DEFAULT
#define TARGET_DEFAULT (MASK_ABICALLS|MASK_GAS)


/* Handle #pragma weak and #pragma pack.  */
#undef HANDLE_SYSV_PRAGMA
#define HANDLE_SYSV_PRAGMA 1

/* Use more efficient ``thunks'' to implement C++ vtables. */
#undef DEFAULT_VTABLE_THUNKS
#define DEFAULT_VTABLE_THUNKS 1

/* Don't assume anything about the header files.  */
#define NO_IMPLICIT_EXTERN_C

/* Generate calls to memcpy, etc., not bcopy, etc.  */
#define TARGET_MEM_FUNCTIONS

/* Specify predefined symbols in preprocessor.  */
#undef CPP_PREDEFINES
#if TARGET_ENDIAN_DEFAULT == 0
#define CPP_PREDEFINES "-DMIPSEL -D_MIPSEL -Dunix -Dmips -D_mips \
-DR3000 -D_R3000 -Dlinux -Asystem=posix -Acpu=mips \
-Amachine=mips -D__ELF__ -D__PIC__ -D__pic__"
#else
#define CPP_PREDEFINES "-DMIPSEB -D_MIPSEB -Dunix -Dmips -D_mips \
-DR3000 -D_R3000 -Dlinux -Asystem=posix -Acpu=mips \
-Amachine=mips -D__ELF__ -D__PIC__ -D__pic__"
#endif

#undef SUBTARGET_CPP_SIZE_SPEC
#define SUBTARGET_CPP_SIZE_SPEC "\
%{mabi=32: -D__SIZE_TYPE__=unsigned\\ int -D__PTRDIFF_TYPE__=int} \
%{mabi=n32: -D__SIZE_TYPE__=unsigned\\ int -D__PTRDIFF_TYPE__=int} \
%{mabi=64: -D__SIZE_TYPE__=long\\ unsigned\\ int -D__PTRDIFF_TYPE__=long\\ int} \
%{!mabi*: -D__SIZE_TYPE__=unsigned\\ int -D__PTRDIFF_TYPE__=int}"

/* We must make -mips3 do what -mlong64 used to do.  */
/* ??? If no mipsX option given, but a mabi=X option is, then should set
   _MIPS_ISA based on the mabi=X option.  */
/* ??? If no mabi=X option give, but a mipsX option is, then should set
   _MIPS_SIM based on the mipsX option.  */
/* ??? Same for _MIPS_SZINT.  */
/* ??? Same for _MIPS_SZPTR.  */
/* ??? Same for __SIZE_TYPE and __PTRDIFF_TYPE.  */
#undef SUBTARGET_CPP_SPEC
#define SUBTARGET_CPP_SPEC "\
%{mfp32: -D_MIPS_FPSET=16} \
%{mfp64: -D_MIPS_FPSET=32} \
%{!mfp*: -D_MIPS_FPSET=32} \
%{mips1: -D_MIPS_ISA=_MIPS_ISA_MIPS1} \
%{mips2: -D_MIPS_ISA=_MIPS_ISA_MIPS2} \
%{mips3: -D_MIPS_ISA=_MIPS_ISA_MIPS3} \
%{mips4: -D_MIPS_ISA=_MIPS_ISA_MIPS4} \
%{!mips*: -D_MIPS_ISA=_MIPS_ISA_MIPS1} \
%{mabi=32: -D_MIPS_SIM=_MIPS_SIM_ABI32}	\
%{mabi=n32: -D_ABIN32=2 -D_MIPS_SIM=_ABIN32} \
%{mabi=64: -D_ABI64=3 -D_MIPS_SIM=_ABI64} \
%{!mabi*: -D_MIPS_SIM=_MIPS_SIM_ABI32}	\
%{!mint64: -D_MIPS_SZINT=32}%{mint64: -D_MIPS_SZINT=64} \
%{mabi=32: -D_MIPS_SZLONG=32} \
%{mabi=n32: -D_MIPS_SZLONG=32} \
%{mabi=64: -D_MIPS_SZLONG=64} \
%{!mabi*: -D_MIPS_SZLONG=32} \
%{mabi=32: -D_MIPS_SZPTR=32} \
%{mabi=n32: -D_MIPS_SZPTR=32} \
%{mabi=64: -D_MIPS_SZPTR=64} \
%{!mabi*: -D_MIPS_SZPTR=32} \
%{!mips*: -U__mips -D__mips} \
%{mabi=32: -U__mips64} \
%{mabi=n32: -D__mips64} \
%{mabi=64: -U__mips64} \
%{!mabi*: -U__mips64} \
%{fno-PIC:-U__PIC__ -U__pic__} %{fno-pic:-U__PIC__ -U__pic__} \
%{fPIC:-D__PIC__ -D__pic__} %{fpic:-D__PIC__ -D__pic__} \
%{pthread:-D_REENTRANT}"

/* The GNU C++ standard library requires that these macros be defined.  */
#undef CPLUSPLUS_CPP_SPEC
#define CPLUSPLUS_CPP_SPEC "\
-D__LANGUAGE_C_PLUS_PLUS -D_LANGUAGE_C_PLUS_PLUS \
-D_GNU_SOURCE %(cpp) \
"

/* Provide a STARTFILE_SPEC appropriate for GNU/Linux.  Here we add
   the GNU/Linux magical crtbegin.o file (see crtstuff.c) which
   provides part of the support for getting C++ file-scope static
   object constructed before entering `main'. */

#undef  STARTFILE_SPEC
#define STARTFILE_SPEC \
  "%{!shared: \
     %{pg:gcrt1.o%s} %{!pg:%{p:gcrt1.o%s} %{!p:crt1.o%s}}}\
   crti.o%s %{!shared:crtbegin.o%s} %{shared:crtbeginS.o%s}"

/* Provide a ENDFILE_SPEC appropriate for GNU/Linux.  Here we tack on
   the GNU/Linux magical crtend.o file (see crtstuff.c) which
   provides part of the support for getting C++ file-scope static
   object constructed before entering `main', followed by a normal
   GNU/Linux "finalizer" file, `crtn.o'.  */

#undef  ENDFILE_SPEC
#define ENDFILE_SPEC \
  "%{!shared:crtend.o%s} %{shared:crtendS.o%s} crtn.o%s"

/* From iris5.h */
/* -G is incompatible with -KPIC which is the default, so only allow objects
   in the small data section if the user explicitly asks for it.  */
#undef MIPS_DEFAULT_GVALUE
#define MIPS_DEFAULT_GVALUE 0

#undef LIB_SPEC
/* Taken from sparc/linux.h.  */
#define LIB_SPEC \
  "%{shared: -lc} \
   %{!shared: %{mieee-fp:-lieee} %{pthread:-lpthread} \
     %{profile:-lc_p} %{!profile: -lc}}"

/* Borrowed from sparc/linux.h */
#undef LINK_SPEC
#define LINK_SPEC \
 "%(endian_spec) \
  %{shared:-shared} \
  %{!shared: \
    %{!ibcs: \
      %{!static: \
        %{rdynamic:-export-dynamic} \
        %{!dynamic-linker:-dynamic-linker /lib/ld.so.1}} \
        %{static:-static}}}"


#undef SUBTARGET_ASM_SPEC
#define SUBTARGET_ASM_SPEC "\
%{mabi=64: -64} \
%{!fno-PIC:%{!fno-pic:-KPIC}} \
%{fno-PIC:-non_shared} %{fno-pic:-non_shared}"

/* We don't need those nonsenses.  */
#undef INVOKE__main
#undef CTOR_LIST_BEGIN
#undef CTOR_LIST_END
#undef DTOR_LIST_BEGIN
#undef DTOR_LIST_END

/* The MIPS assembler has different syntax for .set. We set it to
   .dummy to trap any errors.  */
#undef SET_ASM_OP
#define SET_ASM_OP "\t.dummy\t"

#undef  ASM_OUTPUT_SOURCE_LINE
#define ASM_OUTPUT_SOURCE_LINE(FILE, LINE)				\
do									\
  {									\
    static int sym_lineno = 1;						\
    fprintf (FILE, "%sLM%d:\n\t%s 68,0,%d,%sLM%d",			\
	     LOCAL_LABEL_PREFIX, sym_lineno, ASM_STABN_OP,		\
	     LINE, LOCAL_LABEL_PREFIX, sym_lineno);			\
    putc ('-', FILE);							\
    assemble_name (FILE,						\
		   XSTR (XEXP (DECL_RTL (current_function_decl), 0), 0));\
    putc ('\n', FILE);							\
    sym_lineno++;							\
  }									\
while (0)

/* This is how we tell the assembler that two symbols have the
   same value.  */
#undef ASM_OUTPUT_DEF
#define ASM_OUTPUT_DEF(FILE,LABEL1,LABEL2)				\
  do {									\
	fprintf ((FILE), "\t");						\
	assemble_name (FILE, LABEL1);					\
	fprintf (FILE, "=");						\
	assemble_name (FILE, LABEL2);					\
	fprintf (FILE, "\n");						\
 } while (0)

#undef ASM_OUTPUT_DEFINE_LABEL_DIFFERENCE_SYMBOL
#define ASM_OUTPUT_DEFINE_LABEL_DIFFERENCE_SYMBOL(FILE, SY, HI, LO)    	\
  do {									\
	fputc ('\t', FILE);						\
	assemble_name (FILE, SY);					\
	fputc ('=', FILE);						\
	assemble_name (FILE, HI);					\
	fputc ('-', FILE);						\
	assemble_name (FILE, LO);					\
  } while (0)

#undef ASM_DECLARE_FUNCTION_NAME
#define ASM_DECLARE_FUNCTION_NAME(STREAM, NAME, DECL)			\
  do {									\
    if (!flag_inhibit_size_directive)					\
      {									\
	fputs ("\t.ent\t", STREAM);					\
	assemble_name (STREAM, NAME);					\
	putc ('\n', STREAM);						\
      }									\
    fprintf (STREAM, "\t%s\t ", TYPE_ASM_OP);				\
    assemble_name (STREAM, NAME);					\
    putc (',', STREAM);							\
    fprintf (STREAM, TYPE_OPERAND_FMT, "function");			\
    putc ('\n', STREAM);						\
    assemble_name (STREAM, NAME);					\
    fputs (":\n", STREAM);						\
  } while (0)

#undef ASM_DECLARE_FUNCTION_SIZE
#define ASM_DECLARE_FUNCTION_SIZE(STREAM, NAME, DECL)			\
  do {									\
    if (!flag_inhibit_size_directive)					\
      {									\
	fputs ("\t.end\t", STREAM);					\
	assemble_name (STREAM, NAME);					\
	putc ('\n', STREAM);						\
      }									\
  } while (0)

/* Tell function_prologue in mips.c that we have already output the .ent/.end
   pseudo-ops.  */
#define FUNCTION_NAME_ALREADY_DECLARED

/* Output #ident as a .ident.  */
#undef ASM_OUTPUT_IDENT
#define ASM_OUTPUT_IDENT(FILE, NAME) \
  fprintf (FILE, "\t%s\t\"%s\"\n", IDENT_ASM_OP, NAME);
