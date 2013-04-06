/* Static Single Assignment (SSA) definitions for GNU C-Compiler
   Copyright (C) 2000 Free Software Foundation, Inc.
   Written by Jeffrey D. Oldham <oldham@codesourcery.com>.

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


/* Main SSA routines.  */
extern void convert_to_ssa		PARAMS ((void));
extern void convert_from_ssa		PARAMS ((void));
typedef int (*successor_phi_fn)         PARAMS ((rtx, int, int, void *));
extern int for_each_successor_phi       PARAMS ((basic_block bb,
						 successor_phi_fn,
						 void *));

/* Optimizations.  */
/* In dce.c */
extern void eliminate_dead_code		PARAMS ((void));

/* SSA definitions and uses.  */
/* This flag is set when the CFG is in SSA form.  */
extern int in_ssa_form;

/* Element I is the single instruction that sets register I.  */
extern varray_type ssa_definition;

/* Element I is an INSN_LIST of instructions that use register I.  */
extern varray_type ssa_uses;


/* Specify which hard registers should be converted.  */

/* All pseudo-registers (having register number >=
   FIRST_PSEUDO_REGISTER) and hard registers satisfying
   CONVERT_HARD_REGISTER_TO_SSA_P are converted to SSA form.  */

/* Given a hard register number REG_NO, return nonzero if and only if
   the register should be converted to SSA.  */

#ifndef CONVERT_HARD_REGISTER_TO_SSA_P
#define CONVERT_HARD_REGISTER_TO_SSA_P(REG_NO) (0) /* default of no hard registers */
#endif /* CONVERT_HARD_REGISTER_TO_SSA_P  */

/* Given a register number REG_NO, return nonzero if and only if the
   register should be converted to SSA.  */

#define CONVERT_REGISTER_TO_SSA_P(REG_NO)	\
	((!HARD_REGISTER_NUM_P (REG_NO)) || \
	 (CONVERT_HARD_REGISTER_TO_SSA_P (REG_NO)))
