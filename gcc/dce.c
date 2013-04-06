/* Dead-code elimination pass for the GNU compiler.
   Copyright (C) 2000 Free Software Foundation, Inc.
   Written by Jeffrey D. Oldham <oldham@codesourcery.com>.

This file is part of GNU CC.

GNU CC is free software; you can redistribute it and/or modify it
under the terms of the GNU General Public License as published by the
Free Software Foundation; either version 2, or (at your option) any
later version.

GNU CC is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
for more details.

You should have received a copy of the GNU General Public License
along with GNU CC; see the file COPYING.  If not, write to the Free
Software Foundation, 59 Temple Place - Suite 330, Boston, MA
02111-1307, USA.  */

/* Dead-code elimination is the removal of instructions which have no
   impact on the program's output.  "Dead instructions" have no impact
   on the program's output, while "necessary instructions" may have
   impact on the output.

   The algorithm consists of three phases:
   1) marking as necessary all instructions known to be necessary,
      e.g., writing a value to memory,
   2) propagating necessary instructions, e.g., the instructions
      giving values to operands in necessary instructions, and
   3) removing dead instructions (except replacing dead conditionals
      with unconditional jumps).

   Side Effects:
   The last step can require adding labels, deleting insns, and
   modifying basic block structures.  Some conditional jumps may be
   converted to unconditional jumps so the control-flow graph may be
   out-of-date.  

   Edges from some infinite loops to the exit block can be added to
   the control-flow graph.

   It Does Not Perform:
   We decided to not simultaneously perform jump optimization and dead
   loop removal during dead-code elimination.  Thus, all jump
   instructions originally present remain after dead-code elimination
   but 1) unnecessary conditional jump instructions are changed to
   unconditional jump instructions and 2) all unconditional jump
   instructions remain.

   Assumptions:
   1) SSA has been performed.
   2) The basic block and control-flow graph structures are accurate.
   3) The flow graph permits constructing an edge_list.
   4) note rtxes should be saved.

   Unfinished:
   When replacing unnecessary conditional jumps with unconditional
   jumps, the control-flow graph is not updated.  It should be.

   References:
   Building an Optimizing Compiler
   Robert Morgan
   Butterworth-Heinemann, 1998
   Section 8.9
*/

#include "config.h"
#include "system.h"

#include "rtl.h"
#include "hard-reg-set.h"
#include "basic-block.h"
#include "ssa.h"
#include "insn-config.h"
#include "recog.h"
#include "output.h"


/* A map from blocks to the edges on which they are control dependent.  */
typedef struct {
  /* An dynamically allocated array.  The Nth element corresponds to
     the block with index N + 2.  The Ith bit in the bitmap is set if
     that block is dependent on the Ith edge.  */
  bitmap *data;
  /* The number of elements in the array.  */
  int length;
} control_dependent_block_to_edge_map_s, *control_dependent_block_to_edge_map;

/* Local function prototypes.  */
static control_dependent_block_to_edge_map control_dependent_block_to_edge_map_create
  PARAMS((size_t num_basic_blocks));
static void set_control_dependent_block_to_edge_map_bit
  PARAMS ((control_dependent_block_to_edge_map c, basic_block bb,
	   int edge_index));
static void control_dependent_block_to_edge_map_free
  PARAMS ((control_dependent_block_to_edge_map c));
static void find_all_control_dependences
  PARAMS ((struct edge_list *el, int *pdom,
	   control_dependent_block_to_edge_map cdbte));
static void find_control_dependence
  PARAMS ((struct edge_list *el, int edge_index, int *pdom,
	   control_dependent_block_to_edge_map cdbte));
static basic_block find_pdom
  PARAMS ((int *pdom, basic_block block));
static int inherently_necessary_register_1
  PARAMS ((rtx *current_rtx, void *data));
static int inherently_necessary_register
  PARAMS ((rtx current_rtx));
static int find_inherently_necessary
  PARAMS ((rtx current_rtx));
static int propagate_necessity_through_operand
  PARAMS ((rtx *current_rtx, void *data));

/* Unnecessary insns are indicated using insns' in_struct bit.  */

/* Indicate INSN is dead-code; returns nothing.  */
#define KILL_INSN(INSN)		INSN_DEAD_CODE_P(INSN) = 1
/* Indicate INSN is necessary, i.e., not dead-code; returns nothing.  */
#define RESURRECT_INSN(INSN)	INSN_DEAD_CODE_P(INSN) = 0
/* Return nonzero if INSN is unnecessary.  */
#define UNNECESSARY_P(INSN)	INSN_DEAD_CODE_P(INSN)
static void mark_all_insn_unnecessary
  PARAMS ((void));
/* Execute CODE with free variable INSN for all unnecessary insns in
   an unspecified order, producing no output.  */
#define EXECUTE_IF_UNNECESSARY(INSN, CODE)	\
{								\
  rtx INSN;							\
								\
  for (INSN = get_insns (); INSN != NULL_RTX; INSN = NEXT_INSN (INSN))	\
    if (INSN_DEAD_CODE_P (INSN)) {				\
      CODE;							\
    }								\
}
/* Find the label beginning block BB.  */
static rtx find_block_label
  PARAMS ((basic_block bb));
/* Remove INSN, updating its basic block structure.  */
static void delete_insn_bb
  PARAMS ((rtx insn));

/* Recording which blocks are control dependent on which edges.  We
   expect each block to be control dependent on very few edges so we
   use a bitmap for each block recording its edges.  An array holds
   the bitmap.  Its position 0 entry holds the bitmap for block
   INVALID_BLOCK+1 so that all blocks, including the entry and exit
   blocks can participate in the data structure.  */

/* Create a control_dependent_block_to_edge_map, given the number
   NUM_BASIC_BLOCKS of non-entry, non-exit basic blocks, e.g.,
   n_basic_blocks.  This memory must be released using
   control_dependent_block_to_edge_map_free ().  */

static control_dependent_block_to_edge_map
control_dependent_block_to_edge_map_create (num_basic_blocks)
     size_t num_basic_blocks;
{
  int i;
  control_dependent_block_to_edge_map c
    = xmalloc (sizeof (control_dependent_block_to_edge_map_s));
  c->length = num_basic_blocks - (INVALID_BLOCK+1);
  c->data = xmalloc ((size_t) c->length*sizeof (bitmap));
  for (i = 0; i < c->length; ++i)
    c->data[i] = BITMAP_XMALLOC ();

  return c;
}

/* Indicate block BB is control dependent on an edge with index
   EDGE_INDEX in the mapping C of blocks to edges on which they are
   control-dependent.  */

static void
set_control_dependent_block_to_edge_map_bit (c, bb, edge_index)
     control_dependent_block_to_edge_map c;
     basic_block bb;
     int edge_index;
{
  if (bb->index - (INVALID_BLOCK+1) >= c->length)
    abort ();

  bitmap_set_bit (c->data[bb->index - (INVALID_BLOCK+1)],
		  edge_index);
}

/* Execute CODE for each edge (given number EDGE_NUMBER within the
   CODE) for which the block containing INSN is control dependent,
   returning no output.  CDBTE is the mapping of blocks to edges on
   which they are control-dependent.  */

#define EXECUTE_IF_CONTROL_DEPENDENT(CDBTE, INSN, EDGE_NUMBER, CODE) \
	EXECUTE_IF_SET_IN_BITMAP \
	  (CDBTE->data[BLOCK_NUM (INSN) - (INVALID_BLOCK+1)], 0, \
	  EDGE_NUMBER, CODE)

/* Destroy a control_dependent_block_to_edge_map C.  */

static void
control_dependent_block_to_edge_map_free (c)
     control_dependent_block_to_edge_map c;
{
  int i;
  for (i = 0; i < c->length; ++i)
    BITMAP_XFREE (c->data[i]);
  free ((PTR) c);
}

/* Record all blocks' control dependences on all edges in the edge
   list EL, ala Morgan, Section 3.6.  The mapping PDOM of blocks to
   their postdominators are used, and results are stored in CDBTE,
   which should be empty.  */

static void
find_all_control_dependences (el, pdom, cdbte)
   struct edge_list *el;
   int *pdom;
   control_dependent_block_to_edge_map cdbte;
{
  int i;

  for (i = 0; i < NUM_EDGES (el); ++i)
    find_control_dependence (el, i, pdom, cdbte);
}

/* Determine all blocks' control dependences on the given edge with
   edge_list EL index EDGE_INDEX, ala Morgan, Section 3.6.  The
   mapping PDOM of blocks to their postdominators are used, and
   results are stored in CDBTE, which is assumed to be initialized
   with zeros in each (block b', edge) position.  */

static void
find_control_dependence (el, edge_index, pdom, cdbte)
   struct edge_list *el;
   int edge_index;
   int *pdom;
   control_dependent_block_to_edge_map cdbte;
{
  basic_block current_block;
  basic_block ending_block;

  if (INDEX_EDGE_PRED_BB (el, edge_index) == EXIT_BLOCK_PTR)
    abort ();
  ending_block = 
    (INDEX_EDGE_PRED_BB (el, edge_index) == ENTRY_BLOCK_PTR) 
    ? BASIC_BLOCK (0) 
    : find_pdom (pdom, INDEX_EDGE_PRED_BB (el, edge_index));

  for (current_block = INDEX_EDGE_SUCC_BB (el, edge_index);
       current_block != ending_block && current_block != EXIT_BLOCK_PTR;
       current_block = find_pdom (pdom, current_block))
    {
      set_control_dependent_block_to_edge_map_bit (cdbte,
						   current_block,
						   edge_index);
    }
}

/* Find the immediate postdominator PDOM of the specified basic block
   BLOCK.  This function is necessary because some blocks have
   negative numbers.  */

static basic_block
find_pdom (pdom, block)
     int *pdom;
     basic_block block;
{
  if (!block)
    abort ();
  if (block->index == INVALID_BLOCK)
    abort ();

  if (block == ENTRY_BLOCK_PTR)
    return BASIC_BLOCK (0);
  else if (block == EXIT_BLOCK_PTR || pdom[block->index] == EXIT_BLOCK)
    return EXIT_BLOCK_PTR;
  else
    return BASIC_BLOCK (pdom[block->index]);
}

/* Determine if the given CURRENT_RTX uses a hard register not
   converted to SSA.  Returns nonzero only if it uses such a hard
   register.  DATA is not used.

   The program counter (PC) is not considered inherently necessary
   since code should be position-independent and thus not depend on
   particular PC values.  */

static int
inherently_necessary_register_1 (current_rtx, data)
     rtx *current_rtx;
     void *data ATTRIBUTE_UNUSED;
{
  rtx x = *current_rtx;

  if (x == NULL_RTX)
    return 0;
  switch (GET_CODE (x))
    {
    case CLOBBER:
      /* Do not traverse the rest of the clobber.  */
      return -1;		
      break;
    case PC:
      return 0;
      break;
    case REG:
      if (CONVERT_REGISTER_TO_SSA_P (REGNO (x)) || x == pc_rtx)
	return 0;
      else
	return !0;
      break;
    default:
      return 0;
      break;
    }
}

/* Return nonzero if the insn CURRENT_RTX is inherently necessary.  */

static int
inherently_necessary_register (current_rtx)
     rtx current_rtx;
{
  return for_each_rtx (&current_rtx,
		       &inherently_necessary_register_1, NULL);
}

/* Mark X as inherently necessary if appropriate.  For example,
   function calls and storing values into memory are inherently
   necessary.  This function is to be used with for_each_rtx ().
   Return nonzero iff inherently necessary.  */

static int
find_inherently_necessary (x)
     rtx x;
{
  rtx pattern;
  if (x == NULL_RTX)
    return 0;
  else if (inherently_necessary_register (x))
    return !0;
  else
    switch (GET_CODE (x))
      {  
      case CALL_INSN:
      case CODE_LABEL:
      case NOTE:
      case BARRIER:
	return !0;
	break;
      case JUMP_INSN:
	return JUMP_TABLE_DATA_P (x) || computed_jump_p (x) != 0;
	break;
      case INSN:
	pattern = PATTERN (x);
	switch (GET_CODE (pattern))
	  {
	  case SET:
	  case PRE_DEC:
	  case PRE_INC:
	  case POST_DEC:
	  case POST_INC:
	    return GET_CODE (SET_DEST (pattern)) == MEM;
	  case CALL:
	  case RETURN:
	  case USE:
	  case CLOBBER:
	    return !0;
	    break;
	  case ASM_INPUT:
	    /* We treat assembler instructions as inherently
	       necessary, and we hope that its operands do not need to
	       be propagated.  */
	    return !0;
	    break;
	  default:
	    return 0;
	  }
      default:
	/* Found an impossible insn type.  */
	abort();
	break;
      }
}

/* Propagate necessity through REG and SUBREG operands of CURRENT_RTX.
   This function is called with for_each_rtx () on necessary
   instructions.  The DATA must be a varray of unprocessed
   instructions.  */

static int
propagate_necessity_through_operand (current_rtx, data)
     rtx *current_rtx;
     void *data;
{
  rtx x = *current_rtx;
  varray_type *unprocessed_instructions = (varray_type *) data;

  if (x == NULL_RTX)
    return 0;
  switch ( GET_CODE (x))
    {
    case REG:
      if (CONVERT_REGISTER_TO_SSA_P (REGNO (x)))
	{
	  rtx insn = VARRAY_RTX (ssa_definition, REGNO (x));
	  if (insn != NULL_RTX && UNNECESSARY_P (insn))
	    {
	      RESURRECT_INSN (insn);
	      VARRAY_PUSH_RTX (*unprocessed_instructions, insn);
	    }
	}
      return 0;

    default:
      return 0;
    }
}

/* Indicate all insns initially assumed to be unnecessary.  */

static void
mark_all_insn_unnecessary ()
{
  rtx insn;
  for (insn = get_insns (); insn != NULL_RTX; insn = NEXT_INSN (insn))
    KILL_INSN (insn);
}

/* Find the label beginning block BB, adding one if necessary.  */

static rtx
find_block_label (bb)
     basic_block bb;
{
  rtx insn = bb->head;
  if (LABEL_P (insn))
    return insn;
  else
    {
      rtx new_label = emit_label_before (gen_label_rtx (), insn);
      if (insn == bb->head)
	bb->head = new_label;
      return new_label;
    }
}

/* Remove INSN, updating its basic block structure.  */

static void
delete_insn_bb (insn)
     rtx insn;
{
  basic_block bb;
  if (!insn)
    abort ();
  bb = BLOCK_FOR_INSN (insn);
  if (!bb)
    abort ();
  if (bb->head == bb->end)
    {
      /* Delete the insn by converting it to a note.  */
      PUT_CODE (insn, NOTE);
      NOTE_LINE_NUMBER (insn) = NOTE_INSN_DELETED;
      return;
    }
  else if (insn == bb->head)
    bb->head = NEXT_INSN (insn);
  else if (insn == bb->end)
    bb->end = PREV_INSN (insn);
  delete_insn (insn);
}

/* Perform the dead-code elimination.  */

void
eliminate_dead_code ()
{
  int i;
  rtx insn;
  /* Necessary instructions with operands to explore.  */
  varray_type unprocessed_instructions;
  /* Map element (b,e) is nonzero if the block is control dependent on
     edge.  "cdbte" abbreviates control dependent block to edge.  */
  control_dependent_block_to_edge_map cdbte;
 /* Element I is the immediate postdominator of block I.  */
  int *pdom;
  struct edge_list *el;

  int max_insn_uid = get_max_uid ();

  /* Initialize the data structures.  */
  mark_all_insn_unnecessary ();
  VARRAY_RTX_INIT (unprocessed_instructions, 64,
		   "unprocessed instructions");
  cdbte = control_dependent_block_to_edge_map_create (n_basic_blocks);

  /* Prepare for use of BLOCK_NUM ().  */
  connect_infinite_loops_to_exit ();
   /* Be careful not to clear the added edges.  */
  compute_bb_for_insn (max_insn_uid);

  /* Compute control dependence.  */
  pdom = (int *) xmalloc (n_basic_blocks * sizeof (int));
  for (i = 0; i < n_basic_blocks; ++i)
    pdom[i] = INVALID_BLOCK;
  calculate_dominance_info (pdom, NULL, CDI_POST_DOMINATORS);
  /* Assume there is a path from each node to the exit block.  */
  for (i = 0; i < n_basic_blocks; ++i)
    if (pdom[i] == INVALID_BLOCK)
      pdom[i] = EXIT_BLOCK;
  el = create_edge_list();
  find_all_control_dependences (el, pdom, cdbte);

  /* Find inherently necessary instructions.  */
  for (insn = get_insns (); insn != NULL_RTX; insn = NEXT_INSN (insn))
    if (find_inherently_necessary (insn))
      {
	RESURRECT_INSN (insn);
	VARRAY_PUSH_RTX (unprocessed_instructions, insn);
      }

  /* Propagate necessity using the operands of necessary instructions.  */
  while (VARRAY_ACTIVE_SIZE (unprocessed_instructions) > 0)
    {
      rtx current_instruction;
      int edge_number;

      current_instruction = VARRAY_TOP_RTX (unprocessed_instructions);
      VARRAY_POP (unprocessed_instructions);

      /* Make corresponding control dependent edges necessary.  */
      /* Assume the only JUMP_INSN is the block's last insn.  It appears
	 that the last instruction of the program need not be a
	 JUMP_INSN.  */

      if (INSN_P (current_instruction)
	  && !JUMP_TABLE_DATA_P (current_instruction))
	{
	  /* Notes and labels contain no interesting operands.  */
	  EXECUTE_IF_CONTROL_DEPENDENT
	    (cdbte, current_instruction, edge_number,
	    {
	      rtx jump_insn = (INDEX_EDGE_PRED_BB (el, edge_number))->end;
	      if (GET_CODE (jump_insn) == JUMP_INSN &&
		  UNNECESSARY_P (jump_insn)) {
		RESURRECT_INSN (jump_insn);
		VARRAY_PUSH_RTX (unprocessed_instructions, jump_insn);
	      }
	    });

	  /* Propagate through the operands.  */
	  for_each_rtx (&current_instruction,
			&propagate_necessity_through_operand,
			(PTR) &unprocessed_instructions);

	}
    }

  /* Remove the unnecessary instructions.  */
  EXECUTE_IF_UNNECESSARY (insn,
  {
    if (any_condjump_p (insn))
      {
      /* Convert unnecessary conditional insn to an unconditional
	 jump to immediate postdominator block.  */
	rtx old_label = JUMP_LABEL (insn);
	int pdom_block_number =
	  find_pdom (pdom, BLOCK_FOR_INSN (insn))->index;

	/* Prevent the conditional jump's label from being deleted so
	   we do not have to modify the basic block structure.  */
	++LABEL_NUSES (old_label);

	if (pdom_block_number != EXIT_BLOCK
	    && pdom_block_number != INVALID_BLOCK)
	  {
	    rtx lbl = find_block_label (BASIC_BLOCK (pdom_block_number));
	    rtx new_jump = emit_jump_insn_before (gen_jump (lbl), insn);
	    
	    /* Let jump know that label is in use.  */
	    JUMP_LABEL (new_jump) = lbl;
	    ++LABEL_NUSES (lbl);

	    delete_insn_bb (insn);

	    /* A conditional branch is unnecessary if and only if any
	       block control-dependent on it is unnecessary.  Thus,
	       any phi nodes in these unnecessary blocks are also
	       removed and these nodes need not be updated.  */

	    /* A barrier must follow any unconditional jump.  Barriers
	       are not in basic blocks so this must occur after
	       deleting the conditional jump.  */
	    emit_barrier_after (new_jump);
	  }
	else
	  /* The block drops off the end of the function and the
	     ending conditional jump is not needed.  */
	  delete_insn_bb (insn);
      }
    else if (!JUMP_P (insn))
      delete_insn_bb (insn);
  });
  
  /* Release allocated memory.  */
  for (insn = get_insns (); insn != NULL_RTX; insn = NEXT_INSN (insn))
    RESURRECT_INSN (insn);
  if (VARRAY_ACTIVE_SIZE (unprocessed_instructions) != 0)
    abort ();
  VARRAY_FREE (unprocessed_instructions);
  control_dependent_block_to_edge_map_free (cdbte);
  free ((PTR) pdom);
  free_edge_list (el);
}
