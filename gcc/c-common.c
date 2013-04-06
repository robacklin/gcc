/* Subroutines shared by all languages that are variants of C.
   Copyright (C) 1992, 1993, 1994, 1995, 1996, 1997, 1998, 1999, 2000, 2001
   Free Software Foundation, Inc.

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

#include "config.h"
#include "system.h"
#include "tree.h"
#include "flags.h"
#include "toplev.h"
#include "output.h"
#include "c-pragma.h"
#include "rtl.h"
#include "ggc.h"
#include "expr.h"
#include "c-common.h"
#include "tm_p.h"
#include "obstack.h"
#include "cpplib.h"
cpp_reader *parse_in;		/* Declared in c-lex.h.  */

#undef WCHAR_TYPE_SIZE
#define WCHAR_TYPE_SIZE TYPE_PRECISION (wchar_type_node)

/* We let tm.h override the types used here, to handle trivial differences
   such as the choice of unsigned int or long unsigned int for size_t.
   When machines start needing nontrivial differences in the size type,
   it would be best to do something here to figure out automatically
   from other information what type to use.  */

#ifndef SIZE_TYPE
#define SIZE_TYPE "long unsigned int"
#endif

#ifndef WCHAR_TYPE
#define WCHAR_TYPE "int"
#endif

#ifndef PTRDIFF_TYPE
#define PTRDIFF_TYPE "long int"
#endif

#ifndef WINT_TYPE
#define WINT_TYPE "unsigned int"
#endif

#ifndef INTMAX_TYPE
#define INTMAX_TYPE ((INT_TYPE_SIZE == LONG_LONG_TYPE_SIZE)	\
		     ? "int"					\
		     : ((LONG_TYPE_SIZE == LONG_LONG_TYPE_SIZE)	\
			? "long int"				\
			: "long long int"))
#endif

#ifndef UINTMAX_TYPE
#define UINTMAX_TYPE ((INT_TYPE_SIZE == LONG_LONG_TYPE_SIZE)	\
		     ? "unsigned int"				\
		     : ((LONG_TYPE_SIZE == LONG_LONG_TYPE_SIZE)	\
			? "long unsigned int"			\
			: "long long unsigned int"))
#endif

/* The following symbols are subsumed in the c_global_trees array, and
   listed here individually for documentation purposes.

   INTEGER_TYPE and REAL_TYPE nodes for the standard data types.

	tree short_integer_type_node;
	tree long_integer_type_node;
	tree long_long_integer_type_node;

	tree short_unsigned_type_node;
	tree long_unsigned_type_node;
	tree long_long_unsigned_type_node;

	tree boolean_type_node;
	tree boolean_false_node;
	tree boolean_true_node;

	tree ptrdiff_type_node;

	tree unsigned_char_type_node;
	tree signed_char_type_node;
	tree wchar_type_node;
	tree signed_wchar_type_node;
	tree unsigned_wchar_type_node;

	tree float_type_node;
	tree double_type_node;
	tree long_double_type_node;

	tree complex_integer_type_node;
	tree complex_float_type_node;
	tree complex_double_type_node;
	tree complex_long_double_type_node;

	tree intQI_type_node;
	tree intHI_type_node;
	tree intSI_type_node;
	tree intDI_type_node;
	tree intTI_type_node;

	tree unsigned_intQI_type_node;
	tree unsigned_intHI_type_node;
	tree unsigned_intSI_type_node;
	tree unsigned_intDI_type_node;
	tree unsigned_intTI_type_node;

	tree widest_integer_literal_type_node;
	tree widest_unsigned_literal_type_node;

   Nodes for types `void *' and `const void *'.

	tree ptr_type_node, const_ptr_type_node;

   Nodes for types `char *' and `const char *'.

	tree string_type_node, const_string_type_node;

   Type `char[SOMENUMBER]'.
   Used when an array of char is needed and the size is irrelevant.

	tree char_array_type_node;

   Type `int[SOMENUMBER]' or something like it.
   Used when an array of int needed and the size is irrelevant.

	tree int_array_type_node;

   Type `wchar_t[SOMENUMBER]' or something like it.
   Used when a wide string literal is created.

	tree wchar_array_type_node;

   Type `int ()' -- used for implicit declaration of functions.

	tree default_function_type;

   Function types `int (int)', etc.

	tree int_ftype_int;
	tree void_ftype;
	tree void_ftype_ptr;
	tree int_ftype_int;
	tree ptr_ftype_sizetype;

   A VOID_TYPE node, packaged in a TREE_LIST.

	tree void_list_node;

  The identifiers __FUNCTION__, __PRETTY_FUNCTION__, and __func__.

	tree function_id_node;
	tree pretty_function_id_node;
	tree func_id_node;

*/

tree c_global_trees[CTI_MAX];

/* Nonzero means don't recognize the non-ANSI builtin functions.  */

int flag_no_builtin;

/* Nonzero means don't recognize the non-ANSI builtin functions.
   -ansi sets this.  */

int flag_no_nonansi_builtin;

/* Nonzero means give `double' the same size as `float'.  */

int flag_short_double;

/* Nonzero means give `wchar_t' the same size as `short'.  */

int flag_short_wchar;

/* Nonzero means warn about possible violations of sequence point rules.  */

int warn_sequence_point;

/* The elements of `ridpointers' are identifier nodes for the reserved
   type names and storage classes.  It is indexed by a RID_... value.  */
tree *ridpointers;

tree (*make_fname_decl)                PARAMS ((tree, const char *, int));

/* If non-NULL, the address of a language-specific function that
   returns 1 for language-specific statement codes.  */
int (*lang_statement_code_p)           PARAMS ((enum tree_code));

/* If non-NULL, the address of a language-specific function that takes
   any action required right before expand_function_end is called.  */
void (*lang_expand_function_end)       PARAMS ((void));

/* If this variable is defined to a non-NULL value, it will be called
   after the file has been completely parsed.  */
void (*back_end_hook) PARAMS ((tree));

/* Nonzero means the expression being parsed will never be evaluated.
   This is a count, since unevaluated expressions can nest.  */
int skip_evaluation;

enum attrs {A_PACKED, A_NOCOMMON, A_COMMON, A_NORETURN, A_CONST, A_T_UNION,
	    A_NO_CHECK_MEMORY_USAGE, A_NO_INSTRUMENT_FUNCTION,
	    A_CONSTRUCTOR, A_DESTRUCTOR, A_MODE, A_SECTION, A_ALIGNED,
	    A_UNUSED, A_FORMAT, A_FORMAT_ARG, A_WEAK, A_ALIAS, A_MALLOC,
	    A_NO_LIMIT_STACK, A_PURE};

static void add_attribute		PARAMS ((enum attrs, const char *,
						 int, int, int));
static void init_attributes		PARAMS ((void));
static int default_valid_lang_attribute PARAMS ((tree, tree, tree, tree));
static int constant_fits_type_p		PARAMS ((tree, tree));

/* Keep a stack of if statements.  We record the number of compound
   statements seen up to the if keyword, as well as the line number
   and file of the if.  If a potentially ambiguous else is seen, that
   fact is recorded; the warning is issued when we can be sure that
   the enclosing if statement does not have an else branch.  */
typedef struct
{
  int compstmt_count;
  int line;
  const char *file;
  int needs_warning;
  tree if_stmt;
} if_elt;

static if_elt *if_stack;

/* Amount of space in the if statement stack.  */
static int if_stack_space = 0;

/* Stack pointer.  */
static int if_stack_pointer = 0;

/* Record the start of an if-then, and record the start of it
   for ambiguous else detection.  */

void
c_expand_start_cond (cond, compstmt_count)
     tree cond;
     int compstmt_count;
{
  tree if_stmt;

  /* Make sure there is enough space on the stack.  */
  if (if_stack_space == 0)
    {
      if_stack_space = 10;
      if_stack = (if_elt *)xmalloc (10 * sizeof (if_elt));
    }
  else if (if_stack_space == if_stack_pointer)
    {
      if_stack_space += 10;
      if_stack = (if_elt *)xrealloc (if_stack, if_stack_space * sizeof (if_elt));
    }

  if_stmt = build_stmt (IF_STMT, NULL_TREE, NULL_TREE, NULL_TREE);
  IF_COND (if_stmt) = cond;
  add_stmt (if_stmt);

  /* Record this if statement.  */
  if_stack[if_stack_pointer].compstmt_count = compstmt_count;
  if_stack[if_stack_pointer].file = input_filename;
  if_stack[if_stack_pointer].line = lineno;
  if_stack[if_stack_pointer].needs_warning = 0;
  if_stack[if_stack_pointer].if_stmt = if_stmt;
  if_stack_pointer++;
}

/* Called after the then-clause for an if-statement is processed.  */

void
c_finish_then ()
{
  tree if_stmt = if_stack[if_stack_pointer - 1].if_stmt;
  RECHAIN_STMTS (if_stmt, THEN_CLAUSE (if_stmt));
}

/* Record the end of an if-then.  Optionally warn if a nested
   if statement had an ambiguous else clause.  */

void
c_expand_end_cond ()
{
  if_stack_pointer--;
  if (if_stack[if_stack_pointer].needs_warning)
    warning_with_file_and_line (if_stack[if_stack_pointer].file,
				if_stack[if_stack_pointer].line,
				"suggest explicit braces to avoid ambiguous `else'");
  last_expr_type = NULL_TREE;
}

/* Called between the then-clause and the else-clause
   of an if-then-else.  */

void
c_expand_start_else ()
{
  /* An ambiguous else warning must be generated for the enclosing if
     statement, unless we see an else branch for that one, too.  */
  if (warn_parentheses
      && if_stack_pointer > 1
      && (if_stack[if_stack_pointer - 1].compstmt_count
	  == if_stack[if_stack_pointer - 2].compstmt_count))
    if_stack[if_stack_pointer - 2].needs_warning = 1;

  /* Even if a nested if statement had an else branch, it can't be
     ambiguous if this one also has an else.  So don't warn in that
     case.  Also don't warn for any if statements nested in this else.  */
  if_stack[if_stack_pointer - 1].needs_warning = 0;
  if_stack[if_stack_pointer - 1].compstmt_count--;
}

/* Called after the else-clause for an if-statement is processed.  */

void
c_finish_else ()
{
  tree if_stmt = if_stack[if_stack_pointer - 1].if_stmt;
  RECHAIN_STMTS (if_stmt, ELSE_CLAUSE (if_stmt));
}

/* Make bindings for __FUNCTION__, __PRETTY_FUNCTION__, and __func__.  */

void
declare_function_name ()
{
  const char *name, *printable_name;

  if (current_function_decl == NULL)
    {
      name = "";
      printable_name = "top level";
    }
  else
    {
      /* Allow functions to be nameless (such as artificial ones).  */
      if (DECL_NAME (current_function_decl))
        name = IDENTIFIER_POINTER (DECL_NAME (current_function_decl));
      else
	name = "";
      printable_name = (*decl_printable_name) (current_function_decl, 2);

      /* ISO C99 defines __func__, which is a variable, not a string
	 constant, and which is not a defined symbol at file scope.  */
      (*make_fname_decl) (func_id_node, name, 0);
    }
  
  (*make_fname_decl) (function_id_node, name, 0);
  (*make_fname_decl) (pretty_function_id_node, printable_name, 1);
}

/* Given a chain of STRING_CST nodes,
   concatenate them into one STRING_CST
   and give it a suitable array-of-chars data type.  */

tree
combine_strings (strings)
     tree strings;
{
  register tree value, t;
  register int length = 1;
  int wide_length = 0;
  int wide_flag = 0;
  int wchar_bytes = TYPE_PRECISION (wchar_type_node) / BITS_PER_UNIT;
  int nchars;
  const int nchars_max = flag_isoc99 ? 4095 : 509;

  if (TREE_CHAIN (strings))
    {
      /* More than one in the chain, so concatenate.  */
      register char *p, *q;

      /* Don't include the \0 at the end of each substring,
	 except for the last one.
	 Count wide strings and ordinary strings separately.  */
      for (t = strings; t; t = TREE_CHAIN (t))
	{
	  if (TREE_TYPE (t) == wchar_array_type_node)
	    {
	      wide_length += (TREE_STRING_LENGTH (t) - wchar_bytes);
	      wide_flag = 1;
	    }
	  else
	    {
	      length += (TREE_STRING_LENGTH (t) - 1);
	      if (C_ARTIFICIAL_STRING_P (t) && !in_system_header)
		warning ("concatenation of string literals with __FUNCTION__ is deprecated.  This feature will be removed in future"); 
	    }
	}

      /* If anything is wide, the non-wides will be converted,
	 which makes them take more space.  */
      if (wide_flag)
	length = length * wchar_bytes + wide_length;

      p = alloca (length);

      /* Copy the individual strings into the new combined string.
	 If the combined string is wide, convert the chars to ints
	 for any individual strings that are not wide.  */

      q = p;
      for (t = strings; t; t = TREE_CHAIN (t))
	{
	  int len = (TREE_STRING_LENGTH (t)
		     - ((TREE_TYPE (t) == wchar_array_type_node)
			? wchar_bytes : 1));
	  if ((TREE_TYPE (t) == wchar_array_type_node) == wide_flag)
	    {
	      memcpy (q, TREE_STRING_POINTER (t), len);
	      q += len;
	    }
	  else
	    {
	      int i;
	      for (i = 0; i < len; i++)
		{
		  if (WCHAR_TYPE_SIZE == HOST_BITS_PER_SHORT)
		    ((short *) q)[i] = TREE_STRING_POINTER (t)[i];
		  else
		    ((int *) q)[i] = TREE_STRING_POINTER (t)[i];
		}
	      q += len * wchar_bytes;
	    }
	}
      if (wide_flag)
	{
	  int i;
	  for (i = 0; i < wchar_bytes; i++)
	    *q++ = 0;
	}
      else
	*q = 0;

      value = build_string (length, p);
    }
  else
    {
      value = strings;
      length = TREE_STRING_LENGTH (value);
      if (TREE_TYPE (value) == wchar_array_type_node)
	wide_flag = 1;
    }

  /* Compute the number of elements, for the array type.  */
  nchars = wide_flag ? length / wchar_bytes : length;

  if (pedantic && nchars - 1 > nchars_max && c_language == clk_c)
    pedwarn ("string length `%d' is greater than the length `%d' ISO C%d compilers are required to support",
	     nchars - 1, nchars_max, flag_isoc99 ? 99 : 89);

  /* Create the array type for the string constant.
     -Wwrite-strings says make the string constant an array of const char
     so that copying it to a non-const pointer will get a warning.
     For C++, this is the standard behavior.  */
  if (flag_const_strings
      && (! flag_traditional  && ! flag_writable_strings))
    {
      tree elements
	= build_type_variant (wide_flag ? wchar_type_node : char_type_node,
			      1, 0);
      TREE_TYPE (value)
	= build_array_type (elements,
			    build_index_type (build_int_2 (nchars - 1, 0)));
    }
  else
    TREE_TYPE (value)
      = build_array_type (wide_flag ? wchar_type_node : char_type_node,
			  build_index_type (build_int_2 (nchars - 1, 0)));

  TREE_CONSTANT (value) = 1;
  TREE_READONLY (value) = ! flag_writable_strings;
  TREE_STATIC (value) = 1;
  return value;
}

/* To speed up processing of attributes, we maintain an array of
   IDENTIFIER_NODES and the corresponding attribute types.  */

/* Array to hold attribute information.  */

static struct {enum attrs id; tree name; int min, max, decl_req;} attrtab[50];

static int attrtab_idx = 0;

/* Add an entry to the attribute table above.  */

static void
add_attribute (id, string, min_len, max_len, decl_req)
     enum attrs id;
     const char *string;
     int min_len, max_len;
     int decl_req;
{
  char buf[100];

  attrtab[attrtab_idx].id = id;
  attrtab[attrtab_idx].name = get_identifier (string);
  attrtab[attrtab_idx].min = min_len;
  attrtab[attrtab_idx].max = max_len;
  attrtab[attrtab_idx++].decl_req = decl_req;

  sprintf (buf, "__%s__", string);

  attrtab[attrtab_idx].id = id;
  attrtab[attrtab_idx].name = get_identifier (buf);
  attrtab[attrtab_idx].min = min_len;
  attrtab[attrtab_idx].max = max_len;
  attrtab[attrtab_idx++].decl_req = decl_req;
}

/* Initialize attribute table.  */

static void
init_attributes ()
{
  add_attribute (A_PACKED, "packed", 0, 0, 0);
  add_attribute (A_NOCOMMON, "nocommon", 0, 0, 1);
  add_attribute (A_COMMON, "common", 0, 0, 1);
  add_attribute (A_NORETURN, "noreturn", 0, 0, 1);
  add_attribute (A_NORETURN, "volatile", 0, 0, 1);
  add_attribute (A_UNUSED, "unused", 0, 0, 0);
  add_attribute (A_CONST, "const", 0, 0, 1);
  add_attribute (A_T_UNION, "transparent_union", 0, 0, 0);
  add_attribute (A_CONSTRUCTOR, "constructor", 0, 0, 1);
  add_attribute (A_DESTRUCTOR, "destructor", 0, 0, 1);
  add_attribute (A_MODE, "mode", 1, 1, 1);
  add_attribute (A_SECTION, "section", 1, 1, 1);
  add_attribute (A_ALIGNED, "aligned", 0, 1, 0);
  add_attribute (A_FORMAT, "format", 3, 3, 1);
  add_attribute (A_FORMAT_ARG, "format_arg", 1, 1, 1);
  add_attribute (A_WEAK, "weak", 0, 0, 1);
  add_attribute (A_ALIAS, "alias", 1, 1, 1);
  add_attribute (A_NO_INSTRUMENT_FUNCTION, "no_instrument_function", 0, 0, 1);
  add_attribute (A_NO_CHECK_MEMORY_USAGE, "no_check_memory_usage", 0, 0, 1);
  add_attribute (A_MALLOC, "malloc", 0, 0, 1);
  add_attribute (A_NO_LIMIT_STACK, "no_stack_limit", 0, 0, 1);
  add_attribute (A_PURE, "pure", 0, 0, 1);
}

/* Default implementation of valid_lang_attribute, below.  By default, there
   are no language-specific attributes.  */

static int
default_valid_lang_attribute (attr_name, attr_args, decl, type)
  tree attr_name ATTRIBUTE_UNUSED;
  tree attr_args ATTRIBUTE_UNUSED;
  tree decl ATTRIBUTE_UNUSED;
  tree type ATTRIBUTE_UNUSED;
{
  return 0;
}

/* Return a 1 if ATTR_NAME and ATTR_ARGS denote a valid language-specific
   attribute for either declaration DECL or type TYPE and 0 otherwise.  */

int (*valid_lang_attribute) PARAMS ((tree, tree, tree, tree))
     = default_valid_lang_attribute;

/* Process the attributes listed in ATTRIBUTES and PREFIX_ATTRIBUTES
   and install them in NODE, which is either a DECL (including a TYPE_DECL)
   or a TYPE.  PREFIX_ATTRIBUTES can appear after the declaration specifiers
   and declaration modifiers but before the declaration proper.  */

void
decl_attributes (node, attributes, prefix_attributes)
     tree node, attributes, prefix_attributes;
{
  tree decl = 0, type = 0;
  int is_type = 0;
  tree a;

  if (attrtab_idx == 0)
    init_attributes ();

  if (DECL_P (node))
    {
      decl = node;
      type = TREE_TYPE (decl);
      is_type = TREE_CODE (node) == TYPE_DECL;
    }
  else if (TYPE_P (node))
    type = node, is_type = 1;

#ifdef PRAGMA_INSERT_ATTRIBUTES
  /* If the code in c-pragma.c wants to insert some attributes then
     allow it to do so.  Do this before allowing machine back ends to
     insert attributes, so that they have the opportunity to override
     anything done here.  */
  PRAGMA_INSERT_ATTRIBUTES (node, & attributes, & prefix_attributes);
#endif

#ifdef INSERT_ATTRIBUTES
  INSERT_ATTRIBUTES (node, & attributes, & prefix_attributes);
#endif

  attributes = chainon (prefix_attributes, attributes);

  for (a = attributes; a; a = TREE_CHAIN (a))
    {
      tree name = TREE_PURPOSE (a);
      tree args = TREE_VALUE (a);
      int i;
      enum attrs id;

      for (i = 0; i < attrtab_idx; i++)
	if (attrtab[i].name == name)
	  break;

      if (i == attrtab_idx)
	{
	  if (! valid_machine_attribute (name, args, decl, type)
	      && ! (* valid_lang_attribute) (name, args, decl, type))
	    warning ("`%s' attribute directive ignored",
		     IDENTIFIER_POINTER (name));
	  else if (decl != 0)
	    type = TREE_TYPE (decl);
	  continue;
	}
      else if (attrtab[i].decl_req && decl == 0)
	{
	  warning ("`%s' attribute does not apply to types",
		   IDENTIFIER_POINTER (name));
	  continue;
	}
      else if (list_length (args) < attrtab[i].min
	       || list_length (args) > attrtab[i].max)
	{
	  error ("wrong number of arguments specified for `%s' attribute",
		 IDENTIFIER_POINTER (name));
	  continue;
	}

      id = attrtab[i].id;
      switch (id)
	{
	case A_PACKED:
	  if (is_type)
	    TYPE_PACKED (type) = 1;
	  else if (TREE_CODE (decl) == FIELD_DECL)
	    DECL_PACKED (decl) = 1;
	  /* We can't set DECL_PACKED for a VAR_DECL, because the bit is
	     used for DECL_REGISTER.  It wouldn't mean anything anyway.  */
	  else
	    warning ("`%s' attribute ignored", IDENTIFIER_POINTER (name));
	  break;

	case A_NOCOMMON:
	  if (TREE_CODE (decl) == VAR_DECL)
	    DECL_COMMON (decl) = 0;
	  else
	    warning ("`%s' attribute ignored", IDENTIFIER_POINTER (name));
	  break;

	case A_COMMON:
	  if (TREE_CODE (decl) == VAR_DECL)
	    DECL_COMMON (decl) = 1;
	  else
	    warning ("`%s' attribute ignored", IDENTIFIER_POINTER (name));
	  break;

	case A_NORETURN:
	  if (TREE_CODE (decl) == FUNCTION_DECL)
	    TREE_THIS_VOLATILE (decl) = 1;
	  else if (TREE_CODE (type) == POINTER_TYPE
		   && TREE_CODE (TREE_TYPE (type)) == FUNCTION_TYPE)
	    TREE_TYPE (decl) = type
	      = build_pointer_type
		(build_type_variant (TREE_TYPE (type),
				     TREE_READONLY (TREE_TYPE (type)), 1));
	  else
	    warning ("`%s' attribute ignored", IDENTIFIER_POINTER (name));
	  break;

	case A_MALLOC:
	  if (TREE_CODE (decl) == FUNCTION_DECL)
	    DECL_IS_MALLOC (decl) = 1;
	  /* ??? TODO: Support types.  */
	  else
	    warning ("`%s' attribute ignored", IDENTIFIER_POINTER (name));
	  break;

	case A_UNUSED:
	  if (is_type)
	    if (decl)
	      TREE_USED (decl) = 1;
	    else
	      TREE_USED (type) = 1;
	  else if (TREE_CODE (decl) == PARM_DECL
		   || TREE_CODE (decl) == VAR_DECL
		   || TREE_CODE (decl) == FUNCTION_DECL
		   || TREE_CODE (decl) == LABEL_DECL)
	    TREE_USED (decl) = 1;
	  else
	    warning ("`%s' attribute ignored", IDENTIFIER_POINTER (name));
	  break;

	case A_CONST:
	  if (TREE_CODE (decl) == FUNCTION_DECL)
	    TREE_READONLY (decl) = 1;
	  else if (TREE_CODE (type) == POINTER_TYPE
		   && TREE_CODE (TREE_TYPE (type)) == FUNCTION_TYPE)
	    TREE_TYPE (decl) = type
	      = build_pointer_type
		(build_type_variant (TREE_TYPE (type), 1,
				     TREE_THIS_VOLATILE (TREE_TYPE (type))));
	  else
	    warning ( "`%s' attribute ignored", IDENTIFIER_POINTER (name));
	  break;

	case A_PURE:
	  if (TREE_CODE (decl) == FUNCTION_DECL)
	    DECL_IS_PURE (decl) = 1;
	  /* ??? TODO: Support types.  */
	  else
	    warning ("`%s' attribute ignored", IDENTIFIER_POINTER (name));
	  break;


	case A_T_UNION:
	  if (is_type
	      && TREE_CODE (type) == UNION_TYPE
	      && (decl == 0
		  || (TYPE_FIELDS (type) != 0
		      && TYPE_MODE (type) == DECL_MODE (TYPE_FIELDS (type)))))
	    TYPE_TRANSPARENT_UNION (type) = 1;
	  else if (decl != 0 && TREE_CODE (decl) == PARM_DECL
		   && TREE_CODE (type) == UNION_TYPE
		   && TYPE_MODE (type) == DECL_MODE (TYPE_FIELDS (type)))
	    DECL_TRANSPARENT_UNION (decl) = 1;
	  else
	    warning ("`%s' attribute ignored", IDENTIFIER_POINTER (name));
	  break;

	case A_CONSTRUCTOR:
	  if (TREE_CODE (decl) == FUNCTION_DECL
	      && TREE_CODE (type) == FUNCTION_TYPE
	      && decl_function_context (decl) == 0)
	    {
	      DECL_STATIC_CONSTRUCTOR (decl) = 1;
	      TREE_USED (decl) = 1;
	    }
	  else
	    warning ("`%s' attribute ignored", IDENTIFIER_POINTER (name));
	  break;

	case A_DESTRUCTOR:
	  if (TREE_CODE (decl) == FUNCTION_DECL
	      && TREE_CODE (type) == FUNCTION_TYPE
	      && decl_function_context (decl) == 0)
	    {
	      DECL_STATIC_DESTRUCTOR (decl) = 1;
	      TREE_USED (decl) = 1;
	    }
	  else
	    warning ("`%s' attribute ignored", IDENTIFIER_POINTER (name));
	  break;

	case A_MODE:
	  if (TREE_CODE (TREE_VALUE (args)) != IDENTIFIER_NODE)
	    warning ("`%s' attribute ignored", IDENTIFIER_POINTER (name));
	  else
	    {
	      int j;
	      const char *p = IDENTIFIER_POINTER (TREE_VALUE (args));
	      int len = strlen (p);
	      enum machine_mode mode = VOIDmode;
	      tree typefm;

	      if (len > 4 && p[0] == '_' && p[1] == '_'
		  && p[len - 1] == '_' && p[len - 2] == '_')
		{
		  char *newp = (char *) alloca (len - 1);

		  strcpy (newp, &p[2]);
		  newp[len - 4] = '\0';
		  p = newp;
		}

	      /* Give this decl a type with the specified mode.
		 First check for the special modes.  */
	      if (! strcmp (p, "byte"))
		mode = byte_mode;
	      else if (!strcmp (p, "word"))
		mode = word_mode;
	      else if (! strcmp (p, "pointer"))
		mode = ptr_mode;
	      else
		for (j = 0; j < NUM_MACHINE_MODES; j++)
		  if (!strcmp (p, GET_MODE_NAME (j)))
		    mode = (enum machine_mode) j;

	      if (mode == VOIDmode)
		error ("unknown machine mode `%s'", p);
	      else if (0 == (typefm = type_for_mode (mode,
						     TREE_UNSIGNED (type))))
		error ("no data type for mode `%s'", p);
	      else
		{
		  if (TYPE_PRECISION (typefm) > (TREE_UNSIGNED (type)
						 ? TYPE_PRECISION(uintmax_type_node)
						 : TYPE_PRECISION(intmax_type_node))
		      && pedantic)
		    pedwarn ("type with more precision than %s",
			     TREE_UNSIGNED (type) ? "uintmax_t" : "intmax_t");
		  TREE_TYPE (decl) = type = typefm;
		  DECL_SIZE (decl) = DECL_SIZE_UNIT (decl) = 0;
		  layout_decl (decl, 0);
		}
	    }
	  break;

	case A_SECTION:
#ifdef ASM_OUTPUT_SECTION_NAME
	  if ((TREE_CODE (decl) == FUNCTION_DECL
	       || TREE_CODE (decl) == VAR_DECL)
	      && TREE_CODE (TREE_VALUE (args)) == STRING_CST)
	    {
	      if (TREE_CODE (decl) == VAR_DECL
		  && current_function_decl != NULL_TREE
		  && ! TREE_STATIC (decl))
		error_with_decl (decl,
		  "section attribute cannot be specified for local variables");
	      /* The decl may have already been given a section attribute from
		 a previous declaration.  Ensure they match.  */
	      else if (DECL_SECTION_NAME (decl) != NULL_TREE
		       && strcmp (TREE_STRING_POINTER (DECL_SECTION_NAME (decl)),
				  TREE_STRING_POINTER (TREE_VALUE (args))) != 0)
		error_with_decl (node,
				 "section of `%s' conflicts with previous declaration");
	      else
		DECL_SECTION_NAME (decl) = TREE_VALUE (args);
	    }
	  else
	    error_with_decl (node,
			   "section attribute not allowed for `%s'");
#else
	  error_with_decl (node,
		  "section attributes are not supported for this target");
#endif
	  break;

	case A_ALIGNED:
	  {
	    tree align_expr
	      = (args ? TREE_VALUE (args)
		 : size_int (BIGGEST_ALIGNMENT / BITS_PER_UNIT));
	    int i;

	    /* Strip any NOPs of any kind.  */
	    while (TREE_CODE (align_expr) == NOP_EXPR
		   || TREE_CODE (align_expr) == CONVERT_EXPR
		   || TREE_CODE (align_expr) == NON_LVALUE_EXPR)
	      align_expr = TREE_OPERAND (align_expr, 0);

	    if (TREE_CODE (align_expr) != INTEGER_CST)
	      {
		error ("requested alignment is not a constant");
		continue;
	      }

	    if ((i = tree_log2 (align_expr)) == -1)
	      error ("requested alignment is not a power of 2");
	    else if (i > HOST_BITS_PER_INT - 2)
	      error ("requested alignment is too large");
	    else if (is_type)
	      {
		/* If we have a TYPE_DECL, then copy the type, so that we
		   don't accidentally modify a builtin type.  See pushdecl.  */
		if (decl && TREE_TYPE (decl) != error_mark_node
		    && DECL_ORIGINAL_TYPE (decl) == NULL_TREE)
		  {
		    tree tt = TREE_TYPE (decl);
		    DECL_ORIGINAL_TYPE (decl) = tt;
		    tt = build_type_copy (tt);
		    TYPE_NAME (tt) = decl;
		    TREE_USED (tt) = TREE_USED (decl);
		    TREE_TYPE (decl) = tt;
		    type = tt;
		  }

		TYPE_ALIGN (type) = (1 << i) * BITS_PER_UNIT;
		TYPE_USER_ALIGN (type) = 1;
	      }
	    else if (TREE_CODE (decl) != VAR_DECL
		     && TREE_CODE (decl) != FIELD_DECL)
	      error_with_decl (decl,
			       "alignment may not be specified for `%s'");
	    else
	      {
		DECL_ALIGN (decl) = (1 << i) * BITS_PER_UNIT;
		DECL_USER_ALIGN (decl) = 1;
	      }
	  }
	  break;

	case A_FORMAT:
	  decl_handle_format_attribute (decl, args);
	  break;

	case A_FORMAT_ARG:
	  decl_handle_format_arg_attribute (decl, args);
	  break;

	case A_WEAK:
	  declare_weak (decl);
	  break;

	case A_ALIAS:
	  if ((TREE_CODE (decl) == FUNCTION_DECL && DECL_INITIAL (decl))
	      || (TREE_CODE (decl) != FUNCTION_DECL && ! DECL_EXTERNAL (decl)))
	    error_with_decl (decl,
			     "`%s' defined both normally and as an alias");
	  else if (decl_function_context (decl) == 0)
	    {
	      tree id;

	      id = TREE_VALUE (args);
	      if (TREE_CODE (id) != STRING_CST)
		{
		  error ("alias arg not a string");
		  break;
		}
	      id = get_identifier (TREE_STRING_POINTER (id));
	      /* This counts as a use of the object pointed to.  */
	      TREE_USED (id) = 1;

	      if (TREE_CODE (decl) == FUNCTION_DECL)
		DECL_INITIAL (decl) = error_mark_node;
	      else
		DECL_EXTERNAL (decl) = 0;
	      assemble_alias (decl, id);
	    }
	  else
	    warning ("`%s' attribute ignored", IDENTIFIER_POINTER (name));
	  break;

	case A_NO_CHECK_MEMORY_USAGE:
	  if (TREE_CODE (decl) != FUNCTION_DECL)
	    {
	      error_with_decl (decl,
			       "`%s' attribute applies only to functions",
			       IDENTIFIER_POINTER (name));
	    }
	  else if (DECL_INITIAL (decl))
	    {
	      error_with_decl (decl,
			       "can't set `%s' attribute after definition",
			       IDENTIFIER_POINTER (name));
	    }
	  else
	    DECL_NO_CHECK_MEMORY_USAGE (decl) = 1;
	  break;

	case A_NO_INSTRUMENT_FUNCTION:
	  if (TREE_CODE (decl) != FUNCTION_DECL)
	    {
	      error_with_decl (decl,
			       "`%s' attribute applies only to functions",
			       IDENTIFIER_POINTER (name));
	    }
	  else if (DECL_INITIAL (decl))
	    {
	      error_with_decl (decl,
			       "can't set `%s' attribute after definition",
			       IDENTIFIER_POINTER (name));
	    }
	  else
	    DECL_NO_INSTRUMENT_FUNCTION_ENTRY_EXIT (decl) = 1;
	  break;

        case A_NO_LIMIT_STACK:
	  if (TREE_CODE (decl) != FUNCTION_DECL)
	    {
	      error_with_decl (decl,
			       "`%s' attribute applies only to functions",
			       IDENTIFIER_POINTER (name));
	    }
	  else if (DECL_INITIAL (decl))
	    {
	      error_with_decl (decl,
			       "can't set `%s' attribute after definition",
			       IDENTIFIER_POINTER (name));
	    }
	  else
	    DECL_NO_LIMIT_STACK (decl) = 1;
	  break;
	}
    }
}

/* Split SPECS_ATTRS, a list of declspecs and prefix attributes, into two
   lists.  SPECS_ATTRS may also be just a typespec (eg: RECORD_TYPE).

   The head of the declspec list is stored in DECLSPECS.
   The head of the attribute list is stored in PREFIX_ATTRIBUTES.

   Note that attributes in SPECS_ATTRS are stored in the TREE_PURPOSE of
   the list elements.  We drop the containing TREE_LIST nodes and link the
   resulting attributes together the way decl_attributes expects them.  */

void
split_specs_attrs (specs_attrs, declspecs, prefix_attributes)
     tree specs_attrs;
     tree *declspecs, *prefix_attributes;
{
  tree t, s, a, next, specs, attrs;

  /* This can happen after an __extension__ in pedantic mode.  */
  if (specs_attrs != NULL_TREE 
      && TREE_CODE (specs_attrs) == INTEGER_CST)
    {
      *declspecs = NULL_TREE;
      *prefix_attributes = NULL_TREE;
      return;
    }

  /* This can happen in c++ (eg: decl: typespec initdecls ';').  */
  if (specs_attrs != NULL_TREE
      && TREE_CODE (specs_attrs) != TREE_LIST)
    {
      *declspecs = specs_attrs;
      *prefix_attributes = NULL_TREE;
      return;
    }

  /* Remember to keep the lists in the same order, element-wise.  */

  specs = s = NULL_TREE;
  attrs = a = NULL_TREE;
  for (t = specs_attrs; t; t = next)
    {
      next = TREE_CHAIN (t);
      /* Declspecs have a non-NULL TREE_VALUE.  */
      if (TREE_VALUE (t) != NULL_TREE)
	{
	  if (specs == NULL_TREE)
	    specs = s = t;
	  else
	    {
	      TREE_CHAIN (s) = t;
	      s = t;
	    }
	}
      else
	{
	  if (attrs == NULL_TREE)
	    attrs = a = TREE_PURPOSE (t);
	  else
	    {
	      TREE_CHAIN (a) = TREE_PURPOSE (t);
	      a = TREE_PURPOSE (t);
	    }
	  /* More attrs can be linked here, move A to the end.  */
	  while (TREE_CHAIN (a) != NULL_TREE)
	    a = TREE_CHAIN (a);
	}
    }

  /* Terminate the lists.  */
  if (s != NULL_TREE)
    TREE_CHAIN (s) = NULL_TREE;
  if (a != NULL_TREE)
    TREE_CHAIN (a) = NULL_TREE;

  /* All done.  */
  *declspecs = specs;
  *prefix_attributes = attrs;
}

/* Strip attributes from SPECS_ATTRS, a list of declspecs and attributes.
   This function is used by the parser when a rule will accept attributes
   in a particular position, but we don't want to support that just yet.

   A warning is issued for every ignored attribute.  */

tree
strip_attrs (specs_attrs)
     tree specs_attrs;
{
  tree specs, attrs;

  split_specs_attrs (specs_attrs, &specs, &attrs);

  while (attrs)
    {
      warning ("`%s' attribute ignored",
	       IDENTIFIER_POINTER (TREE_PURPOSE (attrs)));
      attrs = TREE_CHAIN (attrs);
    }

  return specs;
}

static int is_valid_printf_arglist PARAMS ((tree));
static rtx c_expand_builtin PARAMS ((tree, rtx, enum machine_mode, enum expand_modifier));
static rtx c_expand_builtin_printf PARAMS ((tree, rtx, enum machine_mode,
					    enum expand_modifier, int));
static rtx c_expand_builtin_fprintf PARAMS ((tree, rtx, enum machine_mode,
					     enum expand_modifier, int));

/* Print a warning if a constant expression had overflow in folding.
   Invoke this function on every expression that the language
   requires to be a constant expression.
   Note the ANSI C standard says it is erroneous for a
   constant expression to overflow.  */

void
constant_expression_warning (value)
     tree value;
{
  if ((TREE_CODE (value) == INTEGER_CST || TREE_CODE (value) == REAL_CST
       || TREE_CODE (value) == COMPLEX_CST)
      && TREE_CONSTANT_OVERFLOW (value) && pedantic)
    pedwarn ("overflow in constant expression");
}

/* Print a warning if an expression had overflow in folding.
   Invoke this function on every expression that
   (1) appears in the source code, and
   (2) might be a constant expression that overflowed, and
   (3) is not already checked by convert_and_check;
   however, do not invoke this function on operands of explicit casts.  */

void
overflow_warning (value)
     tree value;
{
  if ((TREE_CODE (value) == INTEGER_CST
       || (TREE_CODE (value) == COMPLEX_CST
	   && TREE_CODE (TREE_REALPART (value)) == INTEGER_CST))
      && TREE_OVERFLOW (value))
    {
      TREE_OVERFLOW (value) = 0;
      if (skip_evaluation == 0)
	warning ("integer overflow in expression");
    }
  else if ((TREE_CODE (value) == REAL_CST
	    || (TREE_CODE (value) == COMPLEX_CST
		&& TREE_CODE (TREE_REALPART (value)) == REAL_CST))
	   && TREE_OVERFLOW (value))
    {
      TREE_OVERFLOW (value) = 0;
      if (skip_evaluation == 0)
	warning ("floating point overflow in expression");
    }
}

/* Print a warning if a large constant is truncated to unsigned,
   or if -Wconversion is used and a constant < 0 is converted to unsigned.
   Invoke this function on every expression that might be implicitly
   converted to an unsigned type.  */

void
unsigned_conversion_warning (result, operand)
     tree result, operand;
{
  if (TREE_CODE (operand) == INTEGER_CST
      && TREE_CODE (TREE_TYPE (result)) == INTEGER_TYPE
      && TREE_UNSIGNED (TREE_TYPE (result))
      && skip_evaluation == 0
      && !int_fits_type_p (operand, TREE_TYPE (result)))
    {
      if (!int_fits_type_p (operand, signed_type (TREE_TYPE (result))))
	/* This detects cases like converting -129 or 256 to unsigned char.  */
	warning ("large integer implicitly truncated to unsigned type");
      else if (warn_conversion)
	warning ("negative integer implicitly converted to unsigned type");
    }
}

/* Nonzero if constant C has a value that is permissible
   for type TYPE (an INTEGER_TYPE).  */

static int
constant_fits_type_p (c, type)
     tree c, type;
{
  if (TREE_CODE (c) == INTEGER_CST)
    return int_fits_type_p (c, type);

  c = convert (type, c);
  return !TREE_OVERFLOW (c);
}     

/* Convert EXPR to TYPE, warning about conversion problems with constants.
   Invoke this function on every expression that is converted implicitly,
   i.e. because of language rules and not because of an explicit cast.  */

tree
convert_and_check (type, expr)
     tree type, expr;
{
  tree t = convert (type, expr);
  if (TREE_CODE (t) == INTEGER_CST)
    {
      if (TREE_OVERFLOW (t))
	{
	  TREE_OVERFLOW (t) = 0;

	  /* Do not diagnose overflow in a constant expression merely
	     because a conversion overflowed.  */
	  TREE_CONSTANT_OVERFLOW (t) = TREE_CONSTANT_OVERFLOW (expr);

	  /* No warning for converting 0x80000000 to int.  */
	  if (!(TREE_UNSIGNED (type) < TREE_UNSIGNED (TREE_TYPE (expr))
		&& TREE_CODE (TREE_TYPE (expr)) == INTEGER_TYPE
		&& TYPE_PRECISION (type) == TYPE_PRECISION (TREE_TYPE (expr))))
	    /* If EXPR fits in the unsigned version of TYPE,
	       don't warn unless pedantic.  */
	    if ((pedantic
		 || TREE_UNSIGNED (type)
		 || ! constant_fits_type_p (expr, unsigned_type (type)))
	        && skip_evaluation == 0)
	      warning ("overflow in implicit constant conversion");
	}
      else
	unsigned_conversion_warning (t, expr);
    }
  return t;
}

/* A node in a list that describes references to variables (EXPR), which are
   either read accesses if WRITER is zero, or write accesses, in which case
   WRITER is the parent of EXPR.  */
struct tlist
{
  struct tlist *next;
  tree expr, writer;
};

/* Used to implement a cache the results of a call to verify_tree.  We only
   use this for SAVE_EXPRs.  */
struct tlist_cache
{
  struct tlist_cache *next;
  struct tlist *cache_before_sp;
  struct tlist *cache_after_sp;
  tree expr;
};

/* Obstack to use when allocating tlist structures, and corresponding
   firstobj.  */
static struct obstack tlist_obstack;
static char *tlist_firstobj = 0;

/* Keep track of the identifiers we've warned about, so we can avoid duplicate
   warnings.  */
static struct tlist *warned_ids;
/* SAVE_EXPRs need special treatment.  We process them only once and then
   cache the results.  */
static struct tlist_cache *save_expr_cache;

static void add_tlist PARAMS ((struct tlist **, struct tlist *, tree, int));
static void merge_tlist PARAMS ((struct tlist **, struct tlist *, int));
static void verify_tree PARAMS ((tree, struct tlist **, struct tlist **, tree));
static int warning_candidate_p PARAMS ((tree));
static void warn_for_collisions PARAMS ((struct tlist *));
static void warn_for_collisions_1 PARAMS ((tree, tree, struct tlist *, int));
static struct tlist *new_tlist PARAMS ((struct tlist *, tree, tree));
static void verify_sequence_points PARAMS ((tree));

/* Create a new struct tlist and fill in its fields.  */
static struct tlist *
new_tlist (next, t, writer)
     struct tlist *next;
     tree t;
     tree writer;
{
  struct tlist *l;
  l = (struct tlist *) obstack_alloc (&tlist_obstack, sizeof *l);
  l->next = next;
  l->expr = t;
  l->writer = writer;
  return l;
}

/* Add duplicates of the nodes found in ADD to the list *TO.  If EXCLUDE_WRITER
   is nonnull, we ignore any node we find which has a writer equal to it.  */

static void
add_tlist (to, add, exclude_writer, copy)
     struct tlist **to;
     struct tlist *add;
     tree exclude_writer;
     int copy;
{
  while (add)
    {
      struct tlist *next = add->next;
      if (! copy)
	add->next = *to;
      if (! exclude_writer || add->writer != exclude_writer)
	*to = copy ? new_tlist (*to, add->expr, add->writer) : add;
      add = next;
    }
}

/* Merge the nodes of ADD into TO.  This merging process is done so that for
   each variable that already exists in TO, no new node is added; however if
   there is a write access recorded in ADD, and an occurrence on TO is only
   a read access, then the occurrence in TO will be modified to record the
   write.  */

static void
merge_tlist (to, add, copy)
     struct tlist **to;
     struct tlist *add;
     int copy;
{
  struct tlist **end = to;

  while (*end)
    end = &(*end)->next;

  while (add)
    {
      int found = 0;
      struct tlist *tmp2;
      struct tlist *next = add->next;

      for (tmp2 = *to; tmp2; tmp2 = tmp2->next)
	if (tmp2->expr == add->expr)
	  {
	    found = 1;
	    if (! tmp2->writer)
	      tmp2->writer = add->writer;
	  }
      if (! found)
	{
	  *end = copy ? add : new_tlist (NULL, add->expr, add->writer);
	  end = &(*end)->next;
	  *end = 0;
	}
      add = next;
    }
}

/* WRITTEN is a variable, WRITER is its parent.  Warn if any of the variable
   references in list LIST conflict with it, excluding reads if ONLY writers
   is nonzero.  */

static void
warn_for_collisions_1 (written, writer, list, only_writes)
     tree written, writer;
     struct tlist *list;
     int only_writes;
{
  struct tlist *tmp;

  /* Avoid duplicate warnings.  */
  for (tmp = warned_ids; tmp; tmp = tmp->next)
    if (tmp->expr == written)
      return;

  while (list)
    {
      if (list->expr == written
	  && list->writer != writer
	  && (! only_writes || list->writer))
	{
	  warned_ids = new_tlist (warned_ids, written, NULL_TREE);
	  warning ("operation on `%s' may be undefined",
		   IDENTIFIER_POINTER (DECL_NAME (list->expr)));
	}
      list = list->next;
    }
}

/* Given a list LIST of references to variables, find whether any of these
   can cause conflicts due to missing sequence points.  */

static void
warn_for_collisions (list)
     struct tlist *list;
{
  struct tlist *tmp;
  
  for (tmp = list; tmp; tmp = tmp->next)
    {
      if (tmp->writer)
	warn_for_collisions_1 (tmp->expr, tmp->writer, list, 0);
    }
}

/* Return nonzero if X is a tree that can be verified by the sequence poitn
   warnings.  */
static int
warning_candidate_p (x)
     tree x;
{
  return TREE_CODE (x) == VAR_DECL || TREE_CODE (x) == PARM_DECL;
}

/* Walk the tree X, and record accesses to variables.  If X is written by the
   parent tree, WRITER is the parent.
   We store accesses in one of the two lists: PBEFORE_SP, and PNO_SP.  If this
   expression or its only operand forces a sequence point, then everything up
   to the sequence point is stored in PBEFORE_SP.  Everything else gets stored
   in PNO_SP.
   Once we return, we will have emitted warnings if any subexpression before
   such a sequence point could be undefined.  On a higher level, however, the
   sequence point may not be relevant, and we'll merge the two lists.

   Example: (b++, a) + b;
   The call that processes the COMPOUND_EXPR will store the increment of B
   in PBEFORE_SP, and the use of A in PNO_SP.  The higher-level call that
   processes the PLUS_EXPR will need to merge the two lists so that
   eventually, all accesses end up on the same list (and we'll warn about the
   unordered subexpressions b++ and b.

   A note on merging.  If we modify the former example so that our expression
   becomes
     (b++, b) + a
   care must be taken not simply to add all three expressions into the final
   PNO_SP list.  The function merge_tlist takes care of that by merging the
   before-SP list of the COMPOUND_EXPR into its after-SP list in a special
   way, so that no more than one access to B is recorded.  */

static void
verify_tree (x, pbefore_sp, pno_sp, writer)
     tree x;
     struct tlist **pbefore_sp, **pno_sp;
     tree writer;
{
  struct tlist *tmp_before, *tmp_nosp, *tmp_list2, *tmp_list3;
  enum tree_code code;
  char class;

  /* X may be NULL if it is the operand of an empty statement expression
     ({ }).  */
  if (x == NULL)
    return;

 restart:
  code = TREE_CODE (x);
  class = TREE_CODE_CLASS (code);

  if (warning_candidate_p (x))
    {
      *pno_sp = new_tlist (*pno_sp, x, writer);
      return;
    }

  switch (code)
    {
    case CONSTRUCTOR:
      return;

    case COMPOUND_EXPR:
    case TRUTH_ANDIF_EXPR:
    case TRUTH_ORIF_EXPR:
      tmp_before = tmp_nosp = tmp_list3 = 0;
      verify_tree (TREE_OPERAND (x, 0), &tmp_before, &tmp_nosp, NULL_TREE);
      warn_for_collisions (tmp_nosp);
      merge_tlist (pbefore_sp, tmp_before, 0);
      merge_tlist (pbefore_sp, tmp_nosp, 0);
      verify_tree (TREE_OPERAND (x, 1), &tmp_list3, pno_sp, NULL_TREE);
      merge_tlist (pbefore_sp, tmp_list3, 0);
      return;

    case COND_EXPR:
      tmp_before = tmp_list2 = 0;
      verify_tree (TREE_OPERAND (x, 0), &tmp_before, &tmp_list2, NULL_TREE);
      warn_for_collisions (tmp_list2);
      merge_tlist (pbefore_sp, tmp_before, 0);
      merge_tlist (pbefore_sp, tmp_list2, 1);

      tmp_list3 = tmp_nosp = 0;
      verify_tree (TREE_OPERAND (x, 1), &tmp_list3, &tmp_nosp, NULL_TREE);
      warn_for_collisions (tmp_nosp);
      merge_tlist (pbefore_sp, tmp_list3, 0);

      tmp_list3 = tmp_list2 = 0;
      verify_tree (TREE_OPERAND (x, 2), &tmp_list3, &tmp_list2, NULL_TREE);
      warn_for_collisions (tmp_list2);
      merge_tlist (pbefore_sp, tmp_list3, 0);
      /* Rather than add both tmp_nosp and tmp_list2, we have to merge the
	 two first, to avoid warning for (a ? b++ : b++).  */
      merge_tlist (&tmp_nosp, tmp_list2, 0);
      add_tlist (pno_sp, tmp_nosp, NULL_TREE, 0);
      return;

    case PREDECREMENT_EXPR:
    case PREINCREMENT_EXPR:
    case POSTDECREMENT_EXPR:
    case POSTINCREMENT_EXPR:
      verify_tree (TREE_OPERAND (x, 0), pno_sp, pno_sp, x);
      return;

    case MODIFY_EXPR:
      tmp_before = tmp_nosp = tmp_list3 = 0;
      verify_tree (TREE_OPERAND (x, 1), &tmp_before, &tmp_nosp, NULL_TREE);
      verify_tree (TREE_OPERAND (x, 0), &tmp_list3, &tmp_list3, x);
      /* Expressions inside the LHS are not ordered wrt. the sequence points
	 in the RHS.  Example:
	   *a = (a++, 2)
	 Despite the fact that the modification of "a" is in the before_sp
	 list (tmp_before), it conflicts with the use of "a" in the LHS.
	 We can handle this by adding the contents of tmp_list3
	 to those of tmp_before, and redoing the collision warnings for that
	 list.  */
      add_tlist (&tmp_before, tmp_list3, x, 1);
      warn_for_collisions (tmp_before);
      /* Exclude the LHS itself here; we first have to merge it into the
	 tmp_nosp list.  This is done to avoid warning for "a = a"; if we
	 didn't exclude the LHS, we'd get it twice, once as a read and once
	 as a write.  */
      add_tlist (pno_sp, tmp_list3, x, 0);
      warn_for_collisions_1 (TREE_OPERAND (x, 0), x, tmp_nosp, 1);

      merge_tlist (pbefore_sp, tmp_before, 0);
      if (warning_candidate_p (TREE_OPERAND (x, 0)))
	merge_tlist (&tmp_nosp, new_tlist (NULL, TREE_OPERAND (x, 0), x), 0);
      add_tlist (pno_sp, tmp_nosp, NULL_TREE, 1);
      return;

    case CALL_EXPR:
      /* We need to warn about conflicts among arguments and conflicts between
	 args and the function address.  Side effects of the function address,
	 however, are not ordered by the sequence point of the call.  */
      tmp_before = tmp_nosp = tmp_list2 = tmp_list3 = 0;
      verify_tree (TREE_OPERAND (x, 0), &tmp_before, &tmp_nosp, NULL_TREE);
      if (TREE_OPERAND (x, 1))
	verify_tree (TREE_OPERAND (x, 1), &tmp_list2, &tmp_list3, NULL_TREE);
      merge_tlist (&tmp_list3, tmp_list2, 0);
      add_tlist (&tmp_before, tmp_list3, NULL_TREE, 0);
      add_tlist (&tmp_before, tmp_nosp, NULL_TREE, 0);
      warn_for_collisions (tmp_before);
      add_tlist (pbefore_sp, tmp_before, NULL_TREE, 0);
      return;

    case TREE_LIST:
      /* Scan all the list, e.g. indices of multi dimensional array.  */
      while (x)
	{
	  tmp_before = tmp_nosp = 0;
	  verify_tree (TREE_VALUE (x), &tmp_before, &tmp_nosp, NULL_TREE);
	  merge_tlist (&tmp_nosp, tmp_before, 0);
	  add_tlist (pno_sp, tmp_nosp, NULL_TREE, 0);
	  x = TREE_CHAIN (x);
	}
      return;

    case SAVE_EXPR:
      {
	struct tlist_cache *t;
	for (t = save_expr_cache; t; t = t->next)
	  if (t->expr == x)
	    break;

	if (! t)
	  {
	    t = (struct tlist_cache *) obstack_alloc (&tlist_obstack,
						      sizeof *t);
	    t->next = save_expr_cache;
	    t->expr = x;
	    save_expr_cache = t;

	    tmp_before = tmp_nosp = 0;
	    verify_tree (TREE_OPERAND (x, 0), &tmp_before, &tmp_nosp, NULL_TREE);
	    warn_for_collisions (tmp_nosp);

	    tmp_list3 = 0;
	    while (tmp_nosp)
	      {
		struct tlist *t = tmp_nosp;
		tmp_nosp = t->next;
		merge_tlist (&tmp_list3, t, 0);
	      }
	    t->cache_before_sp = tmp_before;
	    t->cache_after_sp = tmp_list3;
	  }
	merge_tlist (pbefore_sp, t->cache_before_sp, 1);
	add_tlist (pno_sp, t->cache_after_sp, NULL_TREE, 1);
	return;
      }
    default:
      break;
    }

  if (class == '1')
    {
      if (first_rtl_op (code) == 0)
	return;
      x = TREE_OPERAND (x, 0);
      writer = 0;
      goto restart;
    }

  switch (class)
    {
    case 'r':
    case '<':
    case '2':
    case 'b':
    case 'e':
    case 's':
    case 'x':
      {
	int lp;
	int max = first_rtl_op (TREE_CODE (x));
	for (lp = 0; lp < max; lp++)
	  {
	    tmp_before = tmp_nosp = 0;
	    verify_tree (TREE_OPERAND (x, lp), &tmp_before, &tmp_nosp, NULL_TREE);
	    merge_tlist (&tmp_nosp, tmp_before, 0);
	    add_tlist (pno_sp, tmp_nosp, NULL_TREE, 0);
	  }
	break;
      }
    }
}

/* Try to warn for undefined behaviour in EXPR due to missing sequence
   points.  */

static void
verify_sequence_points (expr)
     tree expr;
{
  struct tlist *before_sp = 0, *after_sp = 0;

  warned_ids = 0;
  save_expr_cache = 0;
  if (tlist_firstobj == 0)
    {
      gcc_obstack_init (&tlist_obstack);
      tlist_firstobj = obstack_alloc (&tlist_obstack, 0);
    }

  verify_tree (expr, &before_sp, &after_sp, 0);
  warn_for_collisions (after_sp);
  obstack_free (&tlist_obstack, tlist_firstobj);
}

tree
c_expand_expr_stmt (expr)
     tree expr;
{
  /* Do default conversion if safe and possibly important,
     in case within ({...}).  */
  if ((TREE_CODE (TREE_TYPE (expr)) == ARRAY_TYPE && lvalue_p (expr))
      || TREE_CODE (TREE_TYPE (expr)) == FUNCTION_TYPE)
    expr = default_conversion (expr);

  if (warn_sequence_point)
    verify_sequence_points (expr);

  if (TREE_TYPE (expr) != error_mark_node
      && !COMPLETE_OR_VOID_TYPE_P (TREE_TYPE (expr))
      && TREE_CODE (TREE_TYPE (expr)) != ARRAY_TYPE)
    error ("expression statement has incomplete type");

  last_expr_type = TREE_TYPE (expr); 
  return add_stmt (build_stmt (EXPR_STMT, expr));
}

/* Validate the expression after `case' and apply default promotions.  */

tree
check_case_value (value)
     tree value;
{
  if (value == NULL_TREE)
    return value;

  /* Strip NON_LVALUE_EXPRs since we aren't using as an lvalue.  */
  STRIP_TYPE_NOPS (value);
  /* In C++, the following is allowed:

       const int i = 3;
       switch (...) { case i: ... }

     So, we try to reduce the VALUE to a constant that way.  */
  if (c_language == clk_cplusplus)
    {
      value = decl_constant_value (value);
      STRIP_TYPE_NOPS (value);
      value = fold (value);
    }

  if (TREE_CODE (value) != INTEGER_CST
      && value != error_mark_node)
    {
      error ("case label does not reduce to an integer constant");
      value = error_mark_node;
    }
  else
    /* Promote char or short to int.  */
    value = default_conversion (value);

  constant_expression_warning (value);

  return value;
}

/* Return an integer type with BITS bits of precision,
   that is unsigned if UNSIGNEDP is nonzero, otherwise signed.  */

tree
type_for_size (bits, unsignedp)
     unsigned bits;
     int unsignedp;
{
  if (bits == TYPE_PRECISION (integer_type_node))
    return unsignedp ? unsigned_type_node : integer_type_node;

  if (bits == TYPE_PRECISION (signed_char_type_node))
    return unsignedp ? unsigned_char_type_node : signed_char_type_node;

  if (bits == TYPE_PRECISION (short_integer_type_node))
    return unsignedp ? short_unsigned_type_node : short_integer_type_node;

  if (bits == TYPE_PRECISION (long_integer_type_node))
    return unsignedp ? long_unsigned_type_node : long_integer_type_node;

  if (bits == TYPE_PRECISION (long_long_integer_type_node))
    return (unsignedp ? long_long_unsigned_type_node
	    : long_long_integer_type_node);

  if (bits == TYPE_PRECISION (widest_integer_literal_type_node))
    return (unsignedp ? widest_unsigned_literal_type_node
	    : widest_integer_literal_type_node);

  if (bits <= TYPE_PRECISION (intQI_type_node))
    return unsignedp ? unsigned_intQI_type_node : intQI_type_node;

  if (bits <= TYPE_PRECISION (intHI_type_node))
    return unsignedp ? unsigned_intHI_type_node : intHI_type_node;

  if (bits <= TYPE_PRECISION (intSI_type_node))
    return unsignedp ? unsigned_intSI_type_node : intSI_type_node;

  if (bits <= TYPE_PRECISION (intDI_type_node))
    return unsignedp ? unsigned_intDI_type_node : intDI_type_node;

  return 0;
}

/* Return a data type that has machine mode MODE.
   If the mode is an integer,
   then UNSIGNEDP selects between signed and unsigned types.  */

tree
type_for_mode (mode, unsignedp)
     enum machine_mode mode;
     int unsignedp;
{
  if (mode == TYPE_MODE (integer_type_node))
    return unsignedp ? unsigned_type_node : integer_type_node;

  if (mode == TYPE_MODE (signed_char_type_node))
    return unsignedp ? unsigned_char_type_node : signed_char_type_node;

  if (mode == TYPE_MODE (short_integer_type_node))
    return unsignedp ? short_unsigned_type_node : short_integer_type_node;

  if (mode == TYPE_MODE (long_integer_type_node))
    return unsignedp ? long_unsigned_type_node : long_integer_type_node;

  if (mode == TYPE_MODE (long_long_integer_type_node))
    return unsignedp ? long_long_unsigned_type_node : long_long_integer_type_node;

  if (mode == TYPE_MODE (widest_integer_literal_type_node))
    return unsignedp ? widest_unsigned_literal_type_node
                     : widest_integer_literal_type_node;

  if (mode == TYPE_MODE (intQI_type_node))
    return unsignedp ? unsigned_intQI_type_node : intQI_type_node;

  if (mode == TYPE_MODE (intHI_type_node))
    return unsignedp ? unsigned_intHI_type_node : intHI_type_node;

  if (mode == TYPE_MODE (intSI_type_node))
    return unsignedp ? unsigned_intSI_type_node : intSI_type_node;

  if (mode == TYPE_MODE (intDI_type_node))
    return unsignedp ? unsigned_intDI_type_node : intDI_type_node;

#if HOST_BITS_PER_WIDE_INT >= 64
  if (mode == TYPE_MODE (intTI_type_node))
    return unsignedp ? unsigned_intTI_type_node : intTI_type_node;
#endif

  if (mode == TYPE_MODE (float_type_node))
    return float_type_node;

  if (mode == TYPE_MODE (double_type_node))
    return double_type_node;

  if (mode == TYPE_MODE (long_double_type_node))
    return long_double_type_node;

  if (mode == TYPE_MODE (build_pointer_type (char_type_node)))
    return build_pointer_type (char_type_node);

  if (mode == TYPE_MODE (build_pointer_type (integer_type_node)))
    return build_pointer_type (integer_type_node);

#ifdef VECTOR_MODE_SUPPORTED_P
  if (mode == TYPE_MODE (V4SF_type_node) && VECTOR_MODE_SUPPORTED_P (mode))
    return V4SF_type_node;
  if (mode == TYPE_MODE (V4SI_type_node) && VECTOR_MODE_SUPPORTED_P (mode))
    return V4SI_type_node;
  if (mode == TYPE_MODE (V2SI_type_node) && VECTOR_MODE_SUPPORTED_P (mode))
    return V2SI_type_node;
  if (mode == TYPE_MODE (V4HI_type_node) && VECTOR_MODE_SUPPORTED_P (mode))
    return V4HI_type_node;
  if (mode == TYPE_MODE (V8QI_type_node) && VECTOR_MODE_SUPPORTED_P (mode))
    return V8QI_type_node;
#endif

  return 0;
}

/* Return an unsigned type the same as TYPE in other respects. */
tree
unsigned_type (type)
     tree type;
{
  tree type1 = TYPE_MAIN_VARIANT (type);
  if (type1 == signed_char_type_node || type1 == char_type_node)
    return unsigned_char_type_node;
  if (type1 == integer_type_node)
    return unsigned_type_node;
  if (type1 == short_integer_type_node)
    return short_unsigned_type_node;
  if (type1 == long_integer_type_node)
    return long_unsigned_type_node;
  if (type1 == long_long_integer_type_node)
    return long_long_unsigned_type_node;
  if (type1 == widest_integer_literal_type_node)
    return widest_unsigned_literal_type_node;
#if HOST_BITS_PER_WIDE_INT >= 64
  if (type1 == intTI_type_node)
    return unsigned_intTI_type_node;
#endif
  if (type1 == intDI_type_node)
    return unsigned_intDI_type_node;
  if (type1 == intSI_type_node)
    return unsigned_intSI_type_node;
  if (type1 == intHI_type_node)
    return unsigned_intHI_type_node;
  if (type1 == intQI_type_node)
    return unsigned_intQI_type_node;

  return signed_or_unsigned_type (1, type);
}

/* Return a signed type the same as TYPE in other respects.  */

tree
signed_type (type)
     tree type;
{
  tree type1 = TYPE_MAIN_VARIANT (type);
  if (type1 == unsigned_char_type_node || type1 == char_type_node)
    return signed_char_type_node;
  if (type1 == unsigned_type_node)
    return integer_type_node;
  if (type1 == short_unsigned_type_node)
    return short_integer_type_node;
  if (type1 == long_unsigned_type_node)
    return long_integer_type_node;
  if (type1 == long_long_unsigned_type_node)
    return long_long_integer_type_node;
  if (type1 == widest_unsigned_literal_type_node)
    return widest_integer_literal_type_node;
#if HOST_BITS_PER_WIDE_INT >= 64
  if (type1 == unsigned_intTI_type_node)
    return intTI_type_node;
#endif
  if (type1 == unsigned_intDI_type_node)
    return intDI_type_node;
  if (type1 == unsigned_intSI_type_node)
    return intSI_type_node;
  if (type1 == unsigned_intHI_type_node)
    return intHI_type_node;
  if (type1 == unsigned_intQI_type_node)
    return intQI_type_node;

  return signed_or_unsigned_type (0, type);
}

/* Return a type the same as TYPE except unsigned or
   signed according to UNSIGNEDP.  */

tree
signed_or_unsigned_type (unsignedp, type)
     int unsignedp;
     tree type;
{
  if (! INTEGRAL_TYPE_P (type)
      || TREE_UNSIGNED (type) == unsignedp)
    return type;

  if (TYPE_PRECISION (type) == TYPE_PRECISION (signed_char_type_node))
    return unsignedp ? unsigned_char_type_node : signed_char_type_node;
  if (TYPE_PRECISION (type) == TYPE_PRECISION (integer_type_node))
    return unsignedp ? unsigned_type_node : integer_type_node;
  if (TYPE_PRECISION (type) == TYPE_PRECISION (short_integer_type_node))
    return unsignedp ? short_unsigned_type_node : short_integer_type_node;
  if (TYPE_PRECISION (type) == TYPE_PRECISION (long_integer_type_node))
    return unsignedp ? long_unsigned_type_node : long_integer_type_node;
  if (TYPE_PRECISION (type) == TYPE_PRECISION (long_long_integer_type_node))
    return (unsignedp ? long_long_unsigned_type_node
	    : long_long_integer_type_node);
  if (TYPE_PRECISION (type) == TYPE_PRECISION (widest_integer_literal_type_node))
    return (unsignedp ? widest_unsigned_literal_type_node
	    : widest_integer_literal_type_node);
  return type;
}

/* Return the minimum number of bits needed to represent VALUE in a
   signed or unsigned type, UNSIGNEDP says which.  */

unsigned int
min_precision (value, unsignedp)
     tree value;
     int unsignedp;
{
  int log;

  /* If the value is negative, compute its negative minus 1.  The latter
     adjustment is because the absolute value of the largest negative value
     is one larger than the largest positive value.  This is equivalent to
     a bit-wise negation, so use that operation instead.  */

  if (tree_int_cst_sgn (value) < 0)
    value = fold (build1 (BIT_NOT_EXPR, TREE_TYPE (value), value));

  /* Return the number of bits needed, taking into account the fact
     that we need one more bit for a signed than unsigned type.  */

  if (integer_zerop (value))
    log = 0;
  else
    log = tree_floor_log2 (value);

  return log + 1 + ! unsignedp;
}

/* Print an error message for invalid operands to arith operation CODE.
   NOP_EXPR is used as a special case (see truthvalue_conversion).  */

void
binary_op_error (code)
     enum tree_code code;
{
  register const char *opname;

  switch (code)
    {
    case NOP_EXPR:
      error ("invalid truth-value expression");
      return;

    case PLUS_EXPR:
      opname = "+"; break;
    case MINUS_EXPR:
      opname = "-"; break;
    case MULT_EXPR:
      opname = "*"; break;
    case MAX_EXPR:
      opname = "max"; break;
    case MIN_EXPR:
      opname = "min"; break;
    case EQ_EXPR:
      opname = "=="; break;
    case NE_EXPR:
      opname = "!="; break;
    case LE_EXPR:
      opname = "<="; break;
    case GE_EXPR:
      opname = ">="; break;
    case LT_EXPR:
      opname = "<"; break;
    case GT_EXPR:
      opname = ">"; break;
    case LSHIFT_EXPR:
      opname = "<<"; break;
    case RSHIFT_EXPR:
      opname = ">>"; break;
    case TRUNC_MOD_EXPR:
    case FLOOR_MOD_EXPR:
      opname = "%"; break;
    case TRUNC_DIV_EXPR:
    case FLOOR_DIV_EXPR:
      opname = "/"; break;
    case BIT_AND_EXPR:
      opname = "&"; break;
    case BIT_IOR_EXPR:
      opname = "|"; break;
    case TRUTH_ANDIF_EXPR:
      opname = "&&"; break;
    case TRUTH_ORIF_EXPR:
      opname = "||"; break;
    case BIT_XOR_EXPR:
      opname = "^"; break;
    case LROTATE_EXPR:
    case RROTATE_EXPR:
      opname = "rotate"; break;
    default:
      opname = "unknown"; break;
    }
  error ("invalid operands to binary %s", opname);
}

/* Subroutine of build_binary_op, used for comparison operations.
   See if the operands have both been converted from subword integer types
   and, if so, perhaps change them both back to their original type.
   This function is also responsible for converting the two operands
   to the proper common type for comparison.

   The arguments of this function are all pointers to local variables
   of build_binary_op: OP0_PTR is &OP0, OP1_PTR is &OP1,
   RESTYPE_PTR is &RESULT_TYPE and RESCODE_PTR is &RESULTCODE.

   If this function returns nonzero, it means that the comparison has
   a constant value.  What this function returns is an expression for
   that value.  */

tree
shorten_compare (op0_ptr, op1_ptr, restype_ptr, rescode_ptr)
     tree *op0_ptr, *op1_ptr;
     tree *restype_ptr;
     enum tree_code *rescode_ptr;
{
  register tree type;
  tree op0 = *op0_ptr;
  tree op1 = *op1_ptr;
  int unsignedp0, unsignedp1;
  int real1, real2;
  tree primop0, primop1;
  enum tree_code code = *rescode_ptr;

  /* Throw away any conversions to wider types
     already present in the operands.  */

  primop0 = get_narrower (op0, &unsignedp0);
  primop1 = get_narrower (op1, &unsignedp1);

  /* Handle the case that OP0 does not *contain* a conversion
     but it *requires* conversion to FINAL_TYPE.  */

  if (op0 == primop0 && TREE_TYPE (op0) != *restype_ptr)
    unsignedp0 = TREE_UNSIGNED (TREE_TYPE (op0));
  if (op1 == primop1 && TREE_TYPE (op1) != *restype_ptr)
    unsignedp1 = TREE_UNSIGNED (TREE_TYPE (op1));

  /* If one of the operands must be floated, we cannot optimize.  */
  real1 = TREE_CODE (TREE_TYPE (primop0)) == REAL_TYPE;
  real2 = TREE_CODE (TREE_TYPE (primop1)) == REAL_TYPE;

  /* If first arg is constant, swap the args (changing operation
     so value is preserved), for canonicalization.  Don't do this if
     the second arg is 0.  */

  if (TREE_CONSTANT (primop0)
      && ! integer_zerop (primop1) && ! real_zerop (primop1))
    {
      register tree tem = primop0;
      register int temi = unsignedp0;
      primop0 = primop1;
      primop1 = tem;
      tem = op0;
      op0 = op1;
      op1 = tem;
      *op0_ptr = op0;
      *op1_ptr = op1;
      unsignedp0 = unsignedp1;
      unsignedp1 = temi;
      temi = real1;
      real1 = real2;
      real2 = temi;

      switch (code)
	{
	case LT_EXPR:
	  code = GT_EXPR;
	  break;
	case GT_EXPR:
	  code = LT_EXPR;
	  break;
	case LE_EXPR:
	  code = GE_EXPR;
	  break;
	case GE_EXPR:
	  code = LE_EXPR;
	  break;
	default:
	  break;
	}
      *rescode_ptr = code;
    }

  /* If comparing an integer against a constant more bits wide,
     maybe we can deduce a value of 1 or 0 independent of the data.
     Or else truncate the constant now
     rather than extend the variable at run time.

     This is only interesting if the constant is the wider arg.
     Also, it is not safe if the constant is unsigned and the
     variable arg is signed, since in this case the variable
     would be sign-extended and then regarded as unsigned.
     Our technique fails in this case because the lowest/highest
     possible unsigned results don't follow naturally from the
     lowest/highest possible values of the variable operand.
     For just EQ_EXPR and NE_EXPR there is another technique that
     could be used: see if the constant can be faithfully represented
     in the other operand's type, by truncating it and reextending it
     and see if that preserves the constant's value.  */

  if (!real1 && !real2
      && TREE_CODE (primop1) == INTEGER_CST
      && TYPE_PRECISION (TREE_TYPE (primop0)) < TYPE_PRECISION (*restype_ptr))
    {
      int min_gt, max_gt, min_lt, max_lt;
      tree maxval, minval;
      /* 1 if comparison is nominally unsigned.  */
      int unsignedp = TREE_UNSIGNED (*restype_ptr);
      tree val;

      type = signed_or_unsigned_type (unsignedp0, TREE_TYPE (primop0));

      /* If TYPE is an enumeration, then we need to get its min/max
	 values from it's underlying integral type, not the enumerated
	 type itself.  */
      if (TREE_CODE (type) == ENUMERAL_TYPE)
	type = type_for_size (TYPE_PRECISION (type), unsignedp0);

      maxval = TYPE_MAX_VALUE (type);
      minval = TYPE_MIN_VALUE (type);

      if (unsignedp && !unsignedp0)
	*restype_ptr = signed_type (*restype_ptr);

      if (TREE_TYPE (primop1) != *restype_ptr)
	primop1 = convert (*restype_ptr, primop1);
      if (type != *restype_ptr)
	{
	  minval = convert (*restype_ptr, minval);
	  maxval = convert (*restype_ptr, maxval);
	}

      if (unsignedp && unsignedp0)
	{
	  min_gt = INT_CST_LT_UNSIGNED (primop1, minval);
	  max_gt = INT_CST_LT_UNSIGNED (primop1, maxval);
	  min_lt = INT_CST_LT_UNSIGNED (minval, primop1);
	  max_lt = INT_CST_LT_UNSIGNED (maxval, primop1);
	}
      else
	{
	  min_gt = INT_CST_LT (primop1, minval);
	  max_gt = INT_CST_LT (primop1, maxval);
	  min_lt = INT_CST_LT (minval, primop1);
	  max_lt = INT_CST_LT (maxval, primop1);
	}

      val = 0;
      /* This used to be a switch, but Genix compiler can't handle that.  */
      if (code == NE_EXPR)
	{
	  if (max_lt || min_gt)
	    val = boolean_true_node;
	}
      else if (code == EQ_EXPR)
	{
	  if (max_lt || min_gt)
	    val = boolean_false_node;
	}
      else if (code == LT_EXPR)
	{
	  if (max_lt)
	    val = boolean_true_node;
	  if (!min_lt)
	    val = boolean_false_node;
	}
      else if (code == GT_EXPR)
	{
	  if (min_gt)
	    val = boolean_true_node;
	  if (!max_gt)
	    val = boolean_false_node;
	}
      else if (code == LE_EXPR)
	{
	  if (!max_gt)
	    val = boolean_true_node;
	  if (min_gt)
	    val = boolean_false_node;
	}
      else if (code == GE_EXPR)
	{
	  if (!min_lt)
	    val = boolean_true_node;
	  if (max_lt)
	    val = boolean_false_node;
	}

      /* If primop0 was sign-extended and unsigned comparison specd,
	 we did a signed comparison above using the signed type bounds.
	 But the comparison we output must be unsigned.

	 Also, for inequalities, VAL is no good; but if the signed
	 comparison had *any* fixed result, it follows that the
	 unsigned comparison just tests the sign in reverse
	 (positive values are LE, negative ones GE).
	 So we can generate an unsigned comparison
	 against an extreme value of the signed type.  */

      if (unsignedp && !unsignedp0)
	{
	  if (val != 0)
	    switch (code)
	      {
	      case LT_EXPR:
	      case GE_EXPR:
		primop1 = TYPE_MIN_VALUE (type);
		val = 0;
		break;

	      case LE_EXPR:
	      case GT_EXPR:
		primop1 = TYPE_MAX_VALUE (type);
		val = 0;
		break;

	      default:
		break;
	      }
	  type = unsigned_type (type);
	}

      if (!max_gt && !unsignedp0 && TREE_CODE (primop0) != INTEGER_CST)
	{
	  /* This is the case of (char)x >?< 0x80, which people used to use
	     expecting old C compilers to change the 0x80 into -0x80.  */
	  if (val == boolean_false_node)
	    warning ("comparison is always false due to limited range of data type");
	  if (val == boolean_true_node)
	    warning ("comparison is always true due to limited range of data type");
	}

      if (!min_lt && unsignedp0 && TREE_CODE (primop0) != INTEGER_CST)
	{
	  /* This is the case of (unsigned char)x >?< -1 or < 0.  */
	  if (val == boolean_false_node)
	    warning ("comparison is always false due to limited range of data type");
	  if (val == boolean_true_node)
	    warning ("comparison is always true due to limited range of data type");
	}

      if (val != 0)
	{
	  /* Don't forget to evaluate PRIMOP0 if it has side effects.  */
	  if (TREE_SIDE_EFFECTS (primop0))
	    return build (COMPOUND_EXPR, TREE_TYPE (val), primop0, val);
	  return val;
	}

      /* Value is not predetermined, but do the comparison
	 in the type of the operand that is not constant.
	 TYPE is already properly set.  */
    }
  else if (real1 && real2
	   && (TYPE_PRECISION (TREE_TYPE (primop0))
	       == TYPE_PRECISION (TREE_TYPE (primop1))))
    type = TREE_TYPE (primop0);

  /* If args' natural types are both narrower than nominal type
     and both extend in the same manner, compare them
     in the type of the wider arg.
     Otherwise must actually extend both to the nominal
     common type lest different ways of extending
     alter the result.
     (eg, (short)-1 == (unsigned short)-1  should be 0.)  */

  else if (unsignedp0 == unsignedp1 && real1 == real2
	   && TYPE_PRECISION (TREE_TYPE (primop0)) < TYPE_PRECISION (*restype_ptr)
	   && TYPE_PRECISION (TREE_TYPE (primop1)) < TYPE_PRECISION (*restype_ptr))
    {
      type = common_type (TREE_TYPE (primop0), TREE_TYPE (primop1));
      type = signed_or_unsigned_type (unsignedp0
				      || TREE_UNSIGNED (*restype_ptr),
				      type);
      /* Make sure shorter operand is extended the right way
	 to match the longer operand.  */
      primop0 = convert (signed_or_unsigned_type (unsignedp0, TREE_TYPE (primop0)),
			 primop0);
      primop1 = convert (signed_or_unsigned_type (unsignedp1, TREE_TYPE (primop1)),
			 primop1);
    }
  else
    {
      /* Here we must do the comparison on the nominal type
	 using the args exactly as we received them.  */
      type = *restype_ptr;
      primop0 = op0;
      primop1 = op1;

      if (!real1 && !real2 && integer_zerop (primop1)
	  && TREE_UNSIGNED (*restype_ptr))
	{
	  tree value = 0;
	  switch (code)
	    {
	    case GE_EXPR:
	      /* All unsigned values are >= 0, so we warn if extra warnings
		 are requested.  However, if OP0 is a constant that is
		 >= 0, the signedness of the comparison isn't an issue,
		 so suppress the warning.  */
	      if (extra_warnings && !in_system_header
		  && ! (TREE_CODE (primop0) == INTEGER_CST
			&& ! TREE_OVERFLOW (convert (signed_type (type),
						     primop0))))
		warning ("comparison of unsigned expression >= 0 is always true");
	      value = boolean_true_node;
	      break;

	    case LT_EXPR:
	      if (extra_warnings && !in_system_header
		  && ! (TREE_CODE (primop0) == INTEGER_CST
			&& ! TREE_OVERFLOW (convert (signed_type (type),
						     primop0))))
		warning ("comparison of unsigned expression < 0 is always false");
	      value = boolean_false_node;
	      break;

	    default:
	      break;
	    }

	  if (value != 0)
	    {
	      /* Don't forget to evaluate PRIMOP0 if it has side effects.  */
	      if (TREE_SIDE_EFFECTS (primop0))
		return build (COMPOUND_EXPR, TREE_TYPE (value),
			      primop0, value);
	      return value;
	    }
	}
    }

  *op0_ptr = convert (type, primop0);
  *op1_ptr = convert (type, primop1);

  *restype_ptr = boolean_type_node;

  return 0;
}

/* Prepare expr to be an argument of a TRUTH_NOT_EXPR,
   or validate its data type for an `if' or `while' statement or ?..: exp.

   This preparation consists of taking the ordinary
   representation of an expression expr and producing a valid tree
   boolean expression describing whether expr is nonzero.  We could
   simply always do build_binary_op (NE_EXPR, expr, boolean_false_node, 1),
   but we optimize comparisons, &&, ||, and !.

   The resulting type should always be `boolean_type_node'.  */

tree
truthvalue_conversion (expr)
     tree expr;
{
  if (TREE_CODE (expr) == ERROR_MARK)
    return expr;

#if 0 /* This appears to be wrong for C++.  */
  /* These really should return error_mark_node after 2.4 is stable.
     But not all callers handle ERROR_MARK properly.  */
  switch (TREE_CODE (TREE_TYPE (expr)))
    {
    case RECORD_TYPE:
      error ("struct type value used where scalar is required");
      return boolean_false_node;

    case UNION_TYPE:
      error ("union type value used where scalar is required");
      return boolean_false_node;

    case ARRAY_TYPE:
      error ("array type value used where scalar is required");
      return boolean_false_node;

    default:
      break;
    }
#endif /* 0 */

  switch (TREE_CODE (expr))
    {
    case EQ_EXPR:
    case NE_EXPR: case LE_EXPR: case GE_EXPR: case LT_EXPR: case GT_EXPR:
    case TRUTH_ANDIF_EXPR:
    case TRUTH_ORIF_EXPR:
    case TRUTH_AND_EXPR:
    case TRUTH_OR_EXPR:
    case TRUTH_XOR_EXPR:
    case TRUTH_NOT_EXPR:
      TREE_TYPE (expr) = boolean_type_node;
      return expr;

    case ERROR_MARK:
      return expr;

    case INTEGER_CST:
      return integer_zerop (expr) ? boolean_false_node : boolean_true_node;

    case REAL_CST:
      return real_zerop (expr) ? boolean_false_node : boolean_true_node;

    case ADDR_EXPR:
      /* If we are taking the address of a external decl, it might be zero
	 if it is weak, so we cannot optimize.  */
      if (DECL_P (TREE_OPERAND (expr, 0))
	  && DECL_EXTERNAL (TREE_OPERAND (expr, 0)))
	break;

      if (TREE_SIDE_EFFECTS (TREE_OPERAND (expr, 0)))
	return build (COMPOUND_EXPR, boolean_type_node,
		      TREE_OPERAND (expr, 0), boolean_true_node);
      else
	return boolean_true_node;

    case COMPLEX_EXPR:
      return build_binary_op ((TREE_SIDE_EFFECTS (TREE_OPERAND (expr, 1))
			       ? TRUTH_OR_EXPR : TRUTH_ORIF_EXPR),
			      truthvalue_conversion (TREE_OPERAND (expr, 0)),
			      truthvalue_conversion (TREE_OPERAND (expr, 1)),
			      0);

    case NEGATE_EXPR:
    case ABS_EXPR:
    case FLOAT_EXPR:
    case FFS_EXPR:
      /* These don't change whether an object is non-zero or zero.  */
      return truthvalue_conversion (TREE_OPERAND (expr, 0));

    case LROTATE_EXPR:
    case RROTATE_EXPR:
      /* These don't change whether an object is zero or non-zero, but
	 we can't ignore them if their second arg has side-effects.  */
      if (TREE_SIDE_EFFECTS (TREE_OPERAND (expr, 1)))
	return build (COMPOUND_EXPR, boolean_type_node, TREE_OPERAND (expr, 1),
		      truthvalue_conversion (TREE_OPERAND (expr, 0)));
      else
	return truthvalue_conversion (TREE_OPERAND (expr, 0));

    case COND_EXPR:
      /* Distribute the conversion into the arms of a COND_EXPR.  */
      return fold (build (COND_EXPR, boolean_type_node, TREE_OPERAND (expr, 0),
			  truthvalue_conversion (TREE_OPERAND (expr, 1)),
			  truthvalue_conversion (TREE_OPERAND (expr, 2))));

    case CONVERT_EXPR:
      /* Don't cancel the effect of a CONVERT_EXPR from a REFERENCE_TYPE,
	 since that affects how `default_conversion' will behave.  */
      if (TREE_CODE (TREE_TYPE (expr)) == REFERENCE_TYPE
	  || TREE_CODE (TREE_TYPE (TREE_OPERAND (expr, 0))) == REFERENCE_TYPE)
	break;
      /* fall through...  */
    case NOP_EXPR:
      /* If this is widening the argument, we can ignore it.  */
      if (TYPE_PRECISION (TREE_TYPE (expr))
	  >= TYPE_PRECISION (TREE_TYPE (TREE_OPERAND (expr, 0))))
	return truthvalue_conversion (TREE_OPERAND (expr, 0));
      break;

    case MINUS_EXPR:
      /* With IEEE arithmetic, x - x may not equal 0, so we can't optimize
	 this case.  */
      if (TARGET_FLOAT_FORMAT == IEEE_FLOAT_FORMAT
	  && TREE_CODE (TREE_TYPE (expr)) == REAL_TYPE)
	break;
      /* fall through...  */
    case BIT_XOR_EXPR:
      /* This and MINUS_EXPR can be changed into a comparison of the
	 two objects.  */
      if (TREE_TYPE (TREE_OPERAND (expr, 0))
	  == TREE_TYPE (TREE_OPERAND (expr, 1)))
	return build_binary_op (NE_EXPR, TREE_OPERAND (expr, 0),
				TREE_OPERAND (expr, 1), 1);
      return build_binary_op (NE_EXPR, TREE_OPERAND (expr, 0),
			      fold (build1 (NOP_EXPR,
					    TREE_TYPE (TREE_OPERAND (expr, 0)),
					    TREE_OPERAND (expr, 1))), 1);

    case BIT_AND_EXPR:
      if (integer_onep (TREE_OPERAND (expr, 1))
	  && TREE_TYPE (expr) != boolean_type_node)
	/* Using convert here would cause infinite recursion.  */
	return build1 (NOP_EXPR, boolean_type_node, expr);
      break;

    case MODIFY_EXPR:
      if (warn_parentheses && C_EXP_ORIGINAL_CODE (expr) == MODIFY_EXPR)
	warning ("suggest parentheses around assignment used as truth value");
      break;

    default:
      break;
    }

  if (TREE_CODE (TREE_TYPE (expr)) == COMPLEX_TYPE)
    {
      tree tem = save_expr (expr);
      return (build_binary_op
	      ((TREE_SIDE_EFFECTS (expr)
		? TRUTH_OR_EXPR : TRUTH_ORIF_EXPR),
	       truthvalue_conversion (build_unary_op (REALPART_EXPR, tem, 0)),
	       truthvalue_conversion (build_unary_op (IMAGPART_EXPR, tem, 0)),
	       0));
    }

  return build_binary_op (NE_EXPR, expr, integer_zero_node, 1);
}

static tree builtin_function_2 PARAMS ((const char *, const char *, tree, tree,
					int, enum built_in_class, int, int,
					int));

/* Make a variant type in the proper way for C/C++, propagating qualifiers
   down to the element type of an array.  */

tree
c_build_qualified_type (type, type_quals)
     tree type;
     int type_quals;
{
  /* A restrict-qualified pointer type must be a pointer to object or
     incomplete type.  Note that the use of POINTER_TYPE_P also allows
     REFERENCE_TYPEs, which is appropriate for C++.  Unfortunately,
     the C++ front-end also use POINTER_TYPE for pointer-to-member
     values, so even though it should be illegal to use `restrict'
     with such an entity we don't flag that here.  Thus, special case
     code for that case is required in the C++ front-end.  */
  if ((type_quals & TYPE_QUAL_RESTRICT)
      && (!POINTER_TYPE_P (type)
	  || !C_TYPE_OBJECT_OR_INCOMPLETE_P (TREE_TYPE (type))))
    {
      error ("invalid use of `restrict'");
      type_quals &= ~TYPE_QUAL_RESTRICT;
    }

  if (TREE_CODE (type) == ARRAY_TYPE)
    return build_array_type (c_build_qualified_type (TREE_TYPE (type),
						     type_quals),
			     TYPE_DOMAIN (type));
  return build_qualified_type (type, type_quals);
}

/* Apply the TYPE_QUALS to the new DECL.  */

void
c_apply_type_quals_to_decl (type_quals, decl)
     int type_quals;
     tree decl;
{
  if ((type_quals & TYPE_QUAL_CONST)
      || (TREE_TYPE (decl) 
	  && TREE_CODE (TREE_TYPE (decl)) == REFERENCE_TYPE))
    TREE_READONLY (decl) = 1;
  if (type_quals & TYPE_QUAL_VOLATILE)
    {
      TREE_SIDE_EFFECTS (decl) = 1;
      TREE_THIS_VOLATILE (decl) = 1;
    }
  if (type_quals & TYPE_QUAL_RESTRICT)
    {
      if (!TREE_TYPE (decl)
	  || !POINTER_TYPE_P (TREE_TYPE (decl))
	  || !C_TYPE_OBJECT_OR_INCOMPLETE_P (TREE_TYPE (TREE_TYPE (decl))))
	error ("invalid use of `restrict'");
      else if (flag_strict_aliasing)
	{
	  /* No two restricted pointers can point at the same thing.
	     However, a restricted pointer can point at the same thing
	     as an unrestricted pointer, if that unrestricted pointer
	     is based on the restricted pointer.  So, we make the
	     alias set for the restricted pointer a subset of the
	     alias set for the type pointed to by the type of the
	     decl.  */

	  HOST_WIDE_INT pointed_to_alias_set
	    = get_alias_set (TREE_TYPE (TREE_TYPE (decl)));

	  if (pointed_to_alias_set == 0)
	    /* It's not legal to make a subset of alias set zero.  */
	    ;
	  else
	    {
	      DECL_POINTER_ALIAS_SET (decl) = new_alias_set ();
	      record_alias_subset  (pointed_to_alias_set,
				    DECL_POINTER_ALIAS_SET (decl));
	    }
	}
    }
}


/* Return the typed-based alias set for T, which may be an expression
   or a type.  Return -1 if we don't do anything special.  */

HOST_WIDE_INT
lang_get_alias_set (t)
     tree t;
{
  tree u;
  
  /* We know nothing about vector types */
  if (TREE_CODE (t) == VECTOR_TYPE)
    return 0;          
  
  /* Permit type-punning when accessing a union, provided the access
     is directly through the union.  For example, this code does not
     permit taking the address of a union member and then storing
     through it.  Even the type-punning allowed here is a GCC
     extension, albeit a common and useful one; the C standard says
     that such accesses have implementation-defined behavior.  */
  for (u = t;
       TREE_CODE (u) == COMPONENT_REF || TREE_CODE (u) == ARRAY_REF;
       u = TREE_OPERAND (u, 0))
    if (TREE_CODE (u) == COMPONENT_REF
	&& TREE_CODE (TREE_TYPE (TREE_OPERAND (u, 0))) == UNION_TYPE)
      return 0;

  /* If this is a char *, the ANSI C standard says it can alias
     anything.  Note that all references need do this.  */
  if (TREE_CODE_CLASS (TREE_CODE (t)) == 'r'
      && TREE_CODE (TREE_TYPE (t)) == INTEGER_TYPE
      && TYPE_PRECISION (TREE_TYPE (t)) == TYPE_PRECISION (char_type_node))
    return 0;

  /* That's all the expressions we handle specially.  */
  if (! TYPE_P (t))
    return -1;

  /* The C standard specifically allows aliasing between signed and
     unsigned variants of the same type.  We treat the signed
     variant as canonical.  */
  if (TREE_CODE (t) == INTEGER_TYPE && TREE_UNSIGNED (t))
    {
      tree t1 = signed_type (t);

      /* t1 == t can happen for boolean nodes which are always unsigned.  */
      if (t1 != t)
	return get_alias_set (t1);
    }
  else if (POINTER_TYPE_P (t))
    {
      tree t1;

      /* Unfortunately, there is no canonical form of a pointer type.
	 In particular, if we have `typedef int I', then `int *', and
	 `I *' are different types.  So, we have to pick a canonical
	 representative.  We do this below.

	 Technically, this approach is actually more conservative that
	 it needs to be.  In particular, `const int *' and `int *'
	 chould be in different alias sets, according to the C and C++
	 standard, since their types are not the same, and so,
	 technically, an `int **' and `const int **' cannot point at
	 the same thing.

         But, the standard is wrong.  In particular, this code is
	 legal C++:

            int *ip;
            int **ipp = &ip;
            const int* const* cipp = &ip;

         And, it doesn't make sense for that to be legal unless you
	 can dereference IPP and CIPP.  So, we ignore cv-qualifiers on
	 the pointed-to types.  This issue has been reported to the
	 C++ committee.  */
      t1 = build_type_no_quals (t);
      if (t1 != t)
	return get_alias_set (t1);
    }
  /* It's not yet safe to use alias sets for classes in C++ because
     the TYPE_FIELDs list for a class doesn't mention base classes.  */
  else if (c_language == clk_cplusplus && AGGREGATE_TYPE_P (t))
    return 0;

  return -1;
}

/* Build tree nodes and builtin functions common to both C and C++ language
   frontends.  */

void
c_common_nodes_and_builtins ()
{
  int wchar_type_size;
  tree array_domain_type;
  tree temp;
  tree memcpy_ftype, memset_ftype, strlen_ftype;
  tree bzero_ftype, bcmp_ftype, puts_ftype, printf_ftype;
  tree fputs_ftype, fputc_ftype, fwrite_ftype, fprintf_ftype;
  tree endlink, int_endlink, double_endlink, unsigned_endlink;
  tree cstring_endlink, sizetype_endlink;
  tree ptr_ftype, ptr_ftype_unsigned;
  tree void_ftype_any, void_ftype_int, int_ftype_any;
  tree double_ftype_double, double_ftype_double_double;
  tree float_ftype_float, ldouble_ftype_ldouble;
  tree cfloat_ftype_cfloat, cdouble_ftype_cdouble, cldouble_ftype_cldouble;
  tree float_ftype_cfloat, double_ftype_cdouble, ldouble_ftype_cldouble;
  tree int_ftype_cptr_cptr_sizet, sizet_ftype_cstring_cstring;
  tree int_ftype_cstring_cstring, string_ftype_string_cstring;
  tree string_ftype_cstring_int, string_ftype_cstring_cstring;
  tree string_ftype_string_cstring_sizet, int_ftype_cstring_cstring_sizet;
  tree long_ftype_long;
  tree longlong_ftype_longlong;
  tree intmax_ftype_intmax;
  /* Either char* or void*.  */
  tree traditional_ptr_type_node;
  /* Either const char* or const void*.  */
  tree traditional_cptr_type_node;
  tree traditional_len_type_node;
  tree traditional_len_endlink;
  tree va_list_ref_type_node;
  tree va_list_arg_type_node;

  /* Define `int' and `char' first so that dbx will output them first.  */
  record_builtin_type (RID_INT, NULL_PTR, integer_type_node);
  record_builtin_type (RID_CHAR, "char", char_type_node);

  /* `signed' is the same as `int'.  FIXME: the declarations of "signed",
     "unsigned long", "long long unsigned" and "unsigned short" were in C++
     but not C.  Are the conditionals here needed?  */
  if (c_language == clk_cplusplus)
    record_builtin_type (RID_SIGNED, NULL_PTR, integer_type_node);
  record_builtin_type (RID_LONG, "long int", long_integer_type_node);
  record_builtin_type (RID_UNSIGNED, "unsigned int", unsigned_type_node);
  record_builtin_type (RID_MAX, "long unsigned int",
		       long_unsigned_type_node);
  if (c_language == clk_cplusplus)
    record_builtin_type (RID_MAX, "unsigned long", long_unsigned_type_node);
  record_builtin_type (RID_MAX, "long long int",
		       long_long_integer_type_node);
  record_builtin_type (RID_MAX, "long long unsigned int",
		       long_long_unsigned_type_node);
  if (c_language == clk_cplusplus)
    record_builtin_type (RID_MAX, "long long unsigned",
			 long_long_unsigned_type_node);
  record_builtin_type (RID_SHORT, "short int", short_integer_type_node);
  record_builtin_type (RID_MAX, "short unsigned int",
		       short_unsigned_type_node);
  if (c_language == clk_cplusplus)
    record_builtin_type (RID_MAX, "unsigned short",
			 short_unsigned_type_node);

  /* Define both `signed char' and `unsigned char'.  */
  record_builtin_type (RID_MAX, "signed char", signed_char_type_node);
  record_builtin_type (RID_MAX, "unsigned char", unsigned_char_type_node);

  /* These are types that type_for_size and type_for_mode use.  */
  pushdecl (build_decl (TYPE_DECL, NULL_TREE, intQI_type_node));
  pushdecl (build_decl (TYPE_DECL, NULL_TREE, intHI_type_node));
  pushdecl (build_decl (TYPE_DECL, NULL_TREE, intSI_type_node));
  pushdecl (build_decl (TYPE_DECL, NULL_TREE, intDI_type_node));
#if HOST_BITS_PER_WIDE_INT >= 64
  pushdecl (build_decl (TYPE_DECL, get_identifier ("__int128_t"), intTI_type_node));
#endif
  pushdecl (build_decl (TYPE_DECL, NULL_TREE, unsigned_intQI_type_node));
  pushdecl (build_decl (TYPE_DECL, NULL_TREE, unsigned_intHI_type_node));
  pushdecl (build_decl (TYPE_DECL, NULL_TREE, unsigned_intSI_type_node));
  pushdecl (build_decl (TYPE_DECL, NULL_TREE, unsigned_intDI_type_node));
#if HOST_BITS_PER_WIDE_INT >= 64
  pushdecl (build_decl (TYPE_DECL, get_identifier ("__uint128_t"), unsigned_intTI_type_node));
#endif

  /* Create the widest literal types.  */
  widest_integer_literal_type_node
    = make_signed_type (HOST_BITS_PER_WIDE_INT * 2);
  pushdecl (build_decl (TYPE_DECL, NULL_TREE,
			widest_integer_literal_type_node));

  widest_unsigned_literal_type_node
    = make_unsigned_type (HOST_BITS_PER_WIDE_INT * 2);
  pushdecl (build_decl (TYPE_DECL, NULL_TREE,
			widest_unsigned_literal_type_node));

  /* `unsigned long' is the standard type for sizeof.
     Note that stddef.h uses `unsigned long',
     and this must agree, even if long and int are the same size.  */
  c_size_type_node =
    TREE_TYPE (identifier_global_value (get_identifier (SIZE_TYPE)));
  signed_size_type_node = signed_type (c_size_type_node);
  if (flag_traditional)
    c_size_type_node = signed_size_type_node;
  set_sizetype (c_size_type_node);

  build_common_tree_nodes_2 (flag_short_double);

  record_builtin_type (RID_FLOAT, NULL_PTR, float_type_node);
  record_builtin_type (RID_DOUBLE, NULL_PTR, double_type_node);
  record_builtin_type (RID_MAX, "long double", long_double_type_node);

  pushdecl (build_decl (TYPE_DECL, get_identifier ("complex int"),
			complex_integer_type_node));
  pushdecl (build_decl (TYPE_DECL, get_identifier ("complex float"),
			complex_float_type_node));
  pushdecl (build_decl (TYPE_DECL, get_identifier ("complex double"),
			complex_double_type_node));
  pushdecl (build_decl (TYPE_DECL, get_identifier ("complex long double"),
			complex_long_double_type_node));

  record_builtin_type (RID_VOID, NULL_PTR, void_type_node);

  void_list_node = build_void_list_node ();

  /* Make a type to be the domain of a few array types
     whose domains don't really matter.
     200 is small enough that it always fits in size_t
     and large enough that it can hold most function names for the
     initializations of __FUNCTION__ and __PRETTY_FUNCTION__.  */
  array_domain_type = build_index_type (size_int (200));

  /* Make a type for arrays of characters.
     With luck nothing will ever really depend on the length of this
     array type.  */
  char_array_type_node
    = build_array_type (char_type_node, array_domain_type);

  /* Likewise for arrays of ints.  */
  int_array_type_node
    = build_array_type (integer_type_node, array_domain_type);

#ifdef MD_INIT_BUILTINS
  MD_INIT_BUILTINS;
#endif

  /* This is special for C++ so functions can be overloaded.  */
  wchar_type_node = get_identifier (flag_short_wchar
				    ? "short unsigned int"
				    : WCHAR_TYPE);
  wchar_type_node = TREE_TYPE (identifier_global_value (wchar_type_node));
  wchar_type_size = TYPE_PRECISION (wchar_type_node);
  if (c_language == clk_cplusplus)
    {
      if (TREE_UNSIGNED (wchar_type_node))
	wchar_type_node = make_unsigned_type (wchar_type_size);
      else
	wchar_type_node = make_signed_type (wchar_type_size);
      record_builtin_type (RID_WCHAR, "wchar_t", wchar_type_node);
    }
  else
    {
      signed_wchar_type_node = signed_type (wchar_type_node);
      unsigned_wchar_type_node = unsigned_type (wchar_type_node);
    }

  /* This is for wide string constants.  */
  wchar_array_type_node
    = build_array_type (wchar_type_node, array_domain_type);

  string_type_node = build_pointer_type (char_type_node);
  const_string_type_node
    = build_pointer_type (build_type_variant (char_type_node, 1, 0));

  wint_type_node =
    TREE_TYPE (identifier_global_value (get_identifier (WINT_TYPE)));

  intmax_type_node =
    TREE_TYPE (identifier_global_value (get_identifier (INTMAX_TYPE)));
  uintmax_type_node =
    TREE_TYPE (identifier_global_value (get_identifier (UINTMAX_TYPE)));

  default_function_type = build_function_type (integer_type_node, NULL_TREE);
  ptrdiff_type_node
    = TREE_TYPE (identifier_global_value (get_identifier (PTRDIFF_TYPE)));
  unsigned_ptrdiff_type_node = unsigned_type (ptrdiff_type_node);

  pushdecl (build_decl (TYPE_DECL, get_identifier ("__builtin_va_list"),
			va_list_type_node));

  pushdecl (build_decl (TYPE_DECL, get_identifier ("__builtin_ptrdiff_t"),
			ptrdiff_type_node));

  pushdecl (build_decl (TYPE_DECL, get_identifier ("__builtin_size_t"),
			sizetype));

  if (TREE_CODE (va_list_type_node) == ARRAY_TYPE)
    {
      va_list_arg_type_node = va_list_ref_type_node =
	build_pointer_type (TREE_TYPE (va_list_type_node));
    }
  else
    {
      va_list_arg_type_node = va_list_type_node;
      va_list_ref_type_node = build_reference_type (va_list_type_node);
    }
 
  endlink = void_list_node;
  int_endlink = tree_cons (NULL_TREE, integer_type_node, endlink);
  double_endlink = tree_cons (NULL_TREE, double_type_node, endlink);
  unsigned_endlink = tree_cons (NULL_TREE, unsigned_type_node, endlink);
  cstring_endlink = tree_cons (NULL_TREE, const_string_type_node, endlink);

  ptr_ftype = build_function_type (ptr_type_node, NULL_TREE);
  ptr_ftype_unsigned = build_function_type (ptr_type_node, unsigned_endlink);
  sizetype_endlink = tree_cons (NULL_TREE, TYPE_DOMAIN (sizetype), endlink);
  /* We realloc here because sizetype could be int or unsigned.  S'ok.  */
  ptr_ftype_sizetype = build_function_type (ptr_type_node, sizetype_endlink);

  int_ftype_any = build_function_type (integer_type_node, NULL_TREE);
  void_ftype_any = build_function_type (void_type_node, NULL_TREE);
  void_ftype = build_function_type (void_type_node, endlink);
  void_ftype_int = build_function_type (void_type_node, int_endlink);
  void_ftype_ptr
    = build_function_type (void_type_node,
 			   tree_cons (NULL_TREE, ptr_type_node, endlink));

  float_ftype_float
    = build_function_type (float_type_node,
			   tree_cons (NULL_TREE, float_type_node, endlink));

  double_ftype_double
    = build_function_type (double_type_node, double_endlink);

  ldouble_ftype_ldouble
    = build_function_type (long_double_type_node,
			   tree_cons (NULL_TREE, long_double_type_node,
				      endlink));

  double_ftype_double_double
    = build_function_type (double_type_node,
			   tree_cons (NULL_TREE, double_type_node,
				      double_endlink));

  cfloat_ftype_cfloat
    = build_function_type (complex_float_type_node,
			   tree_cons (NULL_TREE, complex_float_type_node,
				      endlink));
  cdouble_ftype_cdouble
    = build_function_type (complex_double_type_node,
			   tree_cons (NULL_TREE, complex_double_type_node,
				      endlink));
  cldouble_ftype_cldouble
    = build_function_type (complex_long_double_type_node,
			   tree_cons (NULL_TREE, complex_long_double_type_node,
				      endlink));

  float_ftype_cfloat
    = build_function_type (float_type_node,
			   tree_cons (NULL_TREE, complex_float_type_node,
				      endlink));
  double_ftype_cdouble
    = build_function_type (double_type_node,
			   tree_cons (NULL_TREE, complex_double_type_node,
				      endlink));
  ldouble_ftype_cldouble
    = build_function_type (long_double_type_node,
			   tree_cons (NULL_TREE, complex_long_double_type_node,
				      endlink));

  int_ftype_int
    = build_function_type (integer_type_node, int_endlink);

  long_ftype_long
    = build_function_type (long_integer_type_node,
			   tree_cons (NULL_TREE, long_integer_type_node,
				      endlink));

  longlong_ftype_longlong
    = build_function_type (long_long_integer_type_node,
			   tree_cons (NULL_TREE, long_long_integer_type_node,
				      endlink));

  intmax_ftype_intmax
    = build_function_type (intmax_type_node,
			   tree_cons (NULL_TREE, intmax_type_node,
				      endlink));

  int_ftype_cptr_cptr_sizet
    = build_function_type (integer_type_node,
			   tree_cons (NULL_TREE, const_ptr_type_node,
				      tree_cons (NULL_TREE,
						 const_ptr_type_node,
						 sizetype_endlink)));

  void_zero_node = build_int_2 (0, 0);
  TREE_TYPE (void_zero_node) = void_type_node;

  /* Prototype for strcpy/strcat.  */
  string_ftype_string_cstring
    = build_function_type (string_type_node,
			   tree_cons (NULL_TREE, string_type_node,
				      cstring_endlink));

  /* Prototype for strncpy/strncat.  */
  string_ftype_string_cstring_sizet
    = build_function_type (string_type_node,
			   tree_cons (NULL_TREE, string_type_node,
				      tree_cons (NULL_TREE,
						 const_string_type_node,
						 sizetype_endlink)));

  traditional_len_type_node = ((flag_traditional && 
				c_language != clk_cplusplus)
			       ? integer_type_node : sizetype);
  traditional_len_endlink = tree_cons (NULL_TREE, traditional_len_type_node,
				       endlink);

  /* Prototype for strcmp.  */
  int_ftype_cstring_cstring
    = build_function_type (integer_type_node,
			   tree_cons (NULL_TREE, const_string_type_node,
				      cstring_endlink));

  /* Prototype for strspn/strcspn.  */
  sizet_ftype_cstring_cstring
    = build_function_type (c_size_type_node,
			   tree_cons (NULL_TREE, const_string_type_node,
				      cstring_endlink));

  /* Prototype for strncmp.  */
  int_ftype_cstring_cstring_sizet
    = build_function_type (integer_type_node,
			   tree_cons (NULL_TREE, const_string_type_node,
				      tree_cons (NULL_TREE,
						 const_string_type_node,
						 sizetype_endlink)));

  /* Prototype for strstr, strpbrk, etc.  */
  string_ftype_cstring_cstring
    = build_function_type (string_type_node,
			   tree_cons (NULL_TREE, const_string_type_node,
				      cstring_endlink));

  /* Prototype for strchr.  */
  string_ftype_cstring_int
    = build_function_type (string_type_node,
			   tree_cons (NULL_TREE, const_string_type_node,
				      int_endlink));

  /* Prototype for strlen.  */
  strlen_ftype
    = build_function_type (traditional_len_type_node, cstring_endlink);

  traditional_ptr_type_node = ((flag_traditional && 
				c_language != clk_cplusplus)
			       ? string_type_node : ptr_type_node);
  traditional_cptr_type_node = ((flag_traditional && 
				 c_language != clk_cplusplus)
			       ? const_string_type_node : const_ptr_type_node);

  /* Prototype for memcpy.  */
  memcpy_ftype
    = build_function_type (traditional_ptr_type_node,
			   tree_cons (NULL_TREE, ptr_type_node,
				      tree_cons (NULL_TREE, const_ptr_type_node,
						 sizetype_endlink)));

  /* Prototype for memset.  */
  memset_ftype
    = build_function_type (traditional_ptr_type_node,
			   tree_cons (NULL_TREE, ptr_type_node,
				      tree_cons (NULL_TREE, integer_type_node,
						 sizetype_endlink)));

  /* Prototype for bzero.  */
  bzero_ftype
    = build_function_type (void_type_node,
			   tree_cons (NULL_TREE, traditional_ptr_type_node,
				      traditional_len_endlink));

  /* Prototype for bcmp.  */
  bcmp_ftype
    = build_function_type (integer_type_node,
			   tree_cons (NULL_TREE, traditional_cptr_type_node,
				      tree_cons (NULL_TREE,
						 traditional_cptr_type_node,
						 traditional_len_endlink)));

  /* Prototype for puts.  */
  puts_ftype
    = build_function_type (integer_type_node, cstring_endlink);

  /* Prototype for printf.  */
  printf_ftype
    = build_function_type (integer_type_node,
			   tree_cons (NULL_TREE, const_string_type_node,
				      NULL_TREE));

  /* These stdio prototypes are declared using void* in place of
     FILE*.  They are only used for __builtin_ style calls, regular
     style builtin prototypes omit the arguments and merge those
     provided by stdio.h.  */
  /* Prototype for fwrite.  */
  fwrite_ftype
    = build_function_type (c_size_type_node,
			   tree_cons (NULL_TREE, const_ptr_type_node,
				      tree_cons (NULL_TREE, c_size_type_node,
						 tree_cons (NULL_TREE, c_size_type_node,
							    tree_cons (NULL_TREE, ptr_type_node, endlink)))));

  /* Prototype for fputc.  */
  fputc_ftype
    = build_function_type (integer_type_node,
			   tree_cons (NULL_TREE, integer_type_node,
				      tree_cons (NULL_TREE, ptr_type_node, endlink)));

  /* Prototype for fputs.  */
  fputs_ftype
    = build_function_type (integer_type_node,
			   tree_cons (NULL_TREE, const_string_type_node,
				      tree_cons (NULL_TREE, ptr_type_node, endlink)));

  /* Prototype for fprintf.  */
  fprintf_ftype
    = build_function_type (integer_type_node,
			   tree_cons (NULL_TREE, ptr_type_node,
				      tree_cons (NULL_TREE,
						 const_string_type_node,
						 NULL_TREE)));

  builtin_function ("__builtin_constant_p", default_function_type,
		    BUILT_IN_CONSTANT_P, BUILT_IN_NORMAL, NULL_PTR);

  builtin_function ("__builtin_return_address", ptr_ftype_unsigned,
		    BUILT_IN_RETURN_ADDRESS, BUILT_IN_NORMAL, NULL_PTR);

  builtin_function ("__builtin_frame_address", ptr_ftype_unsigned,
		    BUILT_IN_FRAME_ADDRESS, BUILT_IN_NORMAL, NULL_PTR);

#ifdef EH_RETURN_DATA_REGNO
  builtin_function ("__builtin_eh_return_data_regno", int_ftype_int,
		    BUILT_IN_EH_RETURN_DATA_REGNO, BUILT_IN_NORMAL, NULL_PTR);
#endif

  builtin_function ("__builtin_alloca", ptr_ftype_sizetype,
		    BUILT_IN_ALLOCA, BUILT_IN_NORMAL, "alloca");
  builtin_function_2 ("__builtin_ffs", "ffs",
		      int_ftype_int, int_ftype_int,
		      BUILT_IN_FFS, BUILT_IN_NORMAL, 0, 1, 0);
  /* Define alloca as builtin, unless SMALL_STACK.  */
#ifndef SMALL_STACK
  builtin_function_2 (NULL_PTR, "alloca", NULL_TREE, ptr_ftype_sizetype,
		      BUILT_IN_ALLOCA, BUILT_IN_NORMAL, 0, 1, 0);
#endif
  /* Declare _exit and _Exit just to mark them as non-returning.  */
  builtin_function_2 (NULL_PTR, "_exit", NULL_TREE, void_ftype_int,
		      0, NOT_BUILT_IN, 0, 1, 1);
  builtin_function_2 (NULL_PTR, "_Exit", NULL_TREE, void_ftype_int,
		      0, NOT_BUILT_IN, 0, !flag_isoc99, 1);

  builtin_function_2 ("__builtin_index", "index",
		      string_ftype_cstring_int, string_ftype_cstring_int,
		      BUILT_IN_INDEX, BUILT_IN_NORMAL, 1, 1, 0);
  builtin_function_2 ("__builtin_rindex", "rindex",
		      string_ftype_cstring_int, string_ftype_cstring_int,
		      BUILT_IN_RINDEX, BUILT_IN_NORMAL, 1, 1, 0);

  /* The system prototypes for these functions have many
     variations, so don't specify parameters to avoid conflicts.
     The expand_* functions check the argument types anyway.  */
  builtin_function_2 ("__builtin_bzero", "bzero",
		      bzero_ftype, void_ftype_any,
		      BUILT_IN_BZERO, BUILT_IN_NORMAL, 1, 1, 0);
  builtin_function_2 ("__builtin_bcmp", "bcmp",
		      bcmp_ftype, int_ftype_any,
		      BUILT_IN_BCMP, BUILT_IN_NORMAL, 1, 1, 0);

  builtin_function_2 ("__builtin_abs", "abs",
		      int_ftype_int, int_ftype_int,
		      BUILT_IN_ABS, BUILT_IN_NORMAL, 0, 0, 0);
  builtin_function_2 ("__builtin_fabsf", "fabsf",
		      float_ftype_float, float_ftype_float,
		      BUILT_IN_FABS, BUILT_IN_NORMAL, 0, 0, 0);
  builtin_function_2 ("__builtin_fabs", "fabs",
		      double_ftype_double, double_ftype_double,
		      BUILT_IN_FABS, BUILT_IN_NORMAL, 0, 0, 0);
  builtin_function_2 ("__builtin_fabsl", "fabsl",
		      ldouble_ftype_ldouble, ldouble_ftype_ldouble,
		      BUILT_IN_FABS, BUILT_IN_NORMAL, 0, 0, 0);
  builtin_function_2 ("__builtin_labs", "labs",
		      long_ftype_long, long_ftype_long,
		      BUILT_IN_ABS, BUILT_IN_NORMAL, 0, 0, 0);
  builtin_function_2 ("__builtin_llabs", "llabs",
		      longlong_ftype_longlong, longlong_ftype_longlong,
		      BUILT_IN_ABS, BUILT_IN_NORMAL, 0, !flag_isoc99, 0);
  builtin_function_2 ("__builtin_imaxabs", "imaxabs",
		      intmax_ftype_intmax, intmax_ftype_intmax,
		      BUILT_IN_ABS, BUILT_IN_NORMAL, 0, !flag_isoc99, 0);

  builtin_function ("__builtin_saveregs", ptr_ftype, BUILT_IN_SAVEREGS,
		    BUILT_IN_NORMAL, NULL_PTR);
  builtin_function ("__builtin_classify_type", default_function_type,
		    BUILT_IN_CLASSIFY_TYPE, BUILT_IN_NORMAL, NULL_PTR);
  builtin_function ("__builtin_next_arg", ptr_ftype, BUILT_IN_NEXT_ARG,
		    BUILT_IN_NORMAL, NULL_PTR);
  builtin_function ("__builtin_args_info", int_ftype_int, BUILT_IN_ARGS_INFO,
		    BUILT_IN_NORMAL, NULL_PTR);
  builtin_function ("__builtin_setjmp",
		    build_function_type (integer_type_node,
					 tree_cons (NULL_TREE, ptr_type_node,
						    endlink)),
		    BUILT_IN_SETJMP, BUILT_IN_NORMAL, NULL_PTR);
  builtin_function ("__builtin_longjmp",
		    build_function_type (void_type_node,
					 tree_cons (NULL_TREE, ptr_type_node,
						    int_endlink)),
		    BUILT_IN_LONGJMP, BUILT_IN_NORMAL, NULL_PTR);
  builtin_function ("__builtin_trap", void_ftype, BUILT_IN_TRAP,
		    BUILT_IN_NORMAL, NULL_PTR);

  /* ISO C99 IEEE Unordered compares.  */
  builtin_function ("__builtin_isgreater", default_function_type,
		    BUILT_IN_ISGREATER, BUILT_IN_NORMAL, NULL_PTR);
  builtin_function ("__builtin_isgreaterequal", default_function_type,
		    BUILT_IN_ISGREATEREQUAL, BUILT_IN_NORMAL, NULL_PTR);
  builtin_function ("__builtin_isless", default_function_type,
		    BUILT_IN_ISLESS, BUILT_IN_NORMAL, NULL_PTR);
  builtin_function ("__builtin_islessequal", default_function_type,
		    BUILT_IN_ISLESSEQUAL, BUILT_IN_NORMAL, NULL_PTR);
  builtin_function ("__builtin_islessgreater", default_function_type,
		    BUILT_IN_ISLESSGREATER, BUILT_IN_NORMAL, NULL_PTR);
  builtin_function ("__builtin_isunordered", default_function_type,
		    BUILT_IN_ISUNORDERED, BUILT_IN_NORMAL, NULL_PTR);

  /* Untyped call and return.  */
  builtin_function ("__builtin_apply_args", ptr_ftype,
		    BUILT_IN_APPLY_ARGS, BUILT_IN_NORMAL, NULL_PTR);

  temp = tree_cons (NULL_TREE,
		    build_pointer_type (build_function_type (void_type_node,
							     NULL_TREE)),
		    tree_cons (NULL_TREE, ptr_type_node, sizetype_endlink));
  builtin_function ("__builtin_apply",
		    build_function_type (ptr_type_node, temp),
		    BUILT_IN_APPLY, BUILT_IN_NORMAL, NULL_PTR);
  builtin_function ("__builtin_return", void_ftype_ptr,
		    BUILT_IN_RETURN, BUILT_IN_NORMAL, NULL_PTR);

  /* Support for varargs.h and stdarg.h.  */
  builtin_function ("__builtin_varargs_start",
		    build_function_type (void_type_node,
					 tree_cons (NULL_TREE,
						    va_list_ref_type_node,
						    endlink)),
		    BUILT_IN_VARARGS_START, BUILT_IN_NORMAL, NULL_PTR);

  builtin_function ("__builtin_stdarg_start",
		    build_function_type (void_type_node,
					 tree_cons (NULL_TREE,
						    va_list_ref_type_node,
						    NULL_TREE)),
		    BUILT_IN_STDARG_START, BUILT_IN_NORMAL, NULL_PTR);

  builtin_function ("__builtin_va_end",
		    build_function_type (void_type_node,
					 tree_cons (NULL_TREE,
						    va_list_ref_type_node,
						    endlink)),
		    BUILT_IN_VA_END, BUILT_IN_NORMAL, NULL_PTR);

  builtin_function ("__builtin_va_copy",
		    build_function_type (void_type_node,
					 tree_cons (NULL_TREE,
						    va_list_ref_type_node,
						    tree_cons (NULL_TREE,
						      va_list_arg_type_node,
						      endlink))),
		    BUILT_IN_VA_COPY, BUILT_IN_NORMAL, NULL_PTR);

  /* ??? Ought to be `T __builtin_expect(T, T)' for any type T.  */
  builtin_function ("__builtin_expect",
		    build_function_type (long_integer_type_node,
					 tree_cons (NULL_TREE,
						    long_integer_type_node,
						    tree_cons (NULL_TREE,
							long_integer_type_node,
							endlink))),
		    BUILT_IN_EXPECT, BUILT_IN_NORMAL, NULL_PTR);

  /* Currently under experimentation.  */
  builtin_function_2 ("__builtin_memcpy", "memcpy",
		      memcpy_ftype, memcpy_ftype,
		      BUILT_IN_MEMCPY, BUILT_IN_NORMAL, 1, 0, 0);
  builtin_function_2 ("__builtin_memcmp", "memcmp",
		      int_ftype_cptr_cptr_sizet, int_ftype_cptr_cptr_sizet,
		      BUILT_IN_MEMCMP, BUILT_IN_NORMAL, 1, 0, 0);
  builtin_function_2 ("__builtin_memset", "memset",
		      memset_ftype, memset_ftype,
		      BUILT_IN_MEMSET, BUILT_IN_NORMAL, 1, 0, 0);
  built_in_decls[BUILT_IN_STRCMP] =
    builtin_function_2 ("__builtin_strcmp", "strcmp",
			int_ftype_cstring_cstring, int_ftype_cstring_cstring,
			BUILT_IN_STRCMP, BUILT_IN_NORMAL, 1, 0, 0);
  builtin_function_2 ("__builtin_strncmp", "strncmp",
		      int_ftype_cstring_cstring_sizet,
		      int_ftype_cstring_cstring_sizet,
		      BUILT_IN_STRNCMP, BUILT_IN_NORMAL, 1, 0, 0);
  builtin_function_2 ("__builtin_strstr", "strstr",
		      string_ftype_cstring_cstring, string_ftype_cstring_cstring,
		      BUILT_IN_STRSTR, BUILT_IN_NORMAL, 1, 0, 0);
  builtin_function_2 ("__builtin_strpbrk", "strpbrk",
		      string_ftype_cstring_cstring, string_ftype_cstring_cstring,
		      BUILT_IN_STRPBRK, BUILT_IN_NORMAL, 1, 0, 0);
  built_in_decls[BUILT_IN_STRCHR] =
    builtin_function_2 ("__builtin_strchr", "strchr",
			string_ftype_cstring_int, string_ftype_cstring_int,
			BUILT_IN_STRCHR, BUILT_IN_NORMAL, 1, 0, 0);
  builtin_function_2 ("__builtin_strrchr", "strrchr",
		      string_ftype_cstring_int, string_ftype_cstring_int,
		      BUILT_IN_STRRCHR, BUILT_IN_NORMAL, 1, 0, 0);
  builtin_function_2 ("__builtin_strcpy", "strcpy",
		      string_ftype_string_cstring, string_ftype_string_cstring,
		      BUILT_IN_STRCPY, BUILT_IN_NORMAL, 1, 0, 0);
  builtin_function_2 ("__builtin_strncpy", "strncpy",
		      string_ftype_string_cstring_sizet,
		      string_ftype_string_cstring_sizet,
		      BUILT_IN_STRNCPY, BUILT_IN_NORMAL, 1, 0, 0);
  built_in_decls[BUILT_IN_STRCAT] =
    builtin_function_2 ("__builtin_strcat", "strcat",
			string_ftype_string_cstring,
			string_ftype_string_cstring,
			BUILT_IN_STRCAT, BUILT_IN_NORMAL, 1, 0, 0);
  builtin_function_2 ("__builtin_strncat", "strncat",
		      string_ftype_string_cstring_sizet,
		      string_ftype_string_cstring_sizet,
		      BUILT_IN_STRNCAT, BUILT_IN_NORMAL, 1, 0, 0);
  builtin_function_2 ("__builtin_strspn", "strspn",
		      sizet_ftype_cstring_cstring, sizet_ftype_cstring_cstring,
		      BUILT_IN_STRSPN, BUILT_IN_NORMAL, 1, 0, 0);
  builtin_function_2 ("__builtin_strcspn", "strcspn",
		      sizet_ftype_cstring_cstring, sizet_ftype_cstring_cstring,
		      BUILT_IN_STRCSPN, BUILT_IN_NORMAL, 1, 0, 0);
  built_in_decls[BUILT_IN_STRLEN] =
    builtin_function_2 ("__builtin_strlen", "strlen",
			strlen_ftype, strlen_ftype,
			BUILT_IN_STRLEN, BUILT_IN_NORMAL, 1, 0, 0);

  builtin_function_2 ("__builtin_sqrtf", "sqrtf",
		      float_ftype_float, float_ftype_float,
		      BUILT_IN_FSQRT, BUILT_IN_NORMAL, 1, 0, 0);
  builtin_function_2 ("__builtin_sqrt", "sqrt",
		      double_ftype_double, double_ftype_double,
		      BUILT_IN_FSQRT, BUILT_IN_NORMAL, 1, 0, 0);
  builtin_function_2 ("__builtin_sqrtl", "sqrtl",
		      ldouble_ftype_ldouble, ldouble_ftype_ldouble,
		      BUILT_IN_FSQRT, BUILT_IN_NORMAL, 1, 0, 0);
  builtin_function_2 ("__builtin_sinf", "sinf",
		      float_ftype_float, float_ftype_float,
		      BUILT_IN_SIN, BUILT_IN_NORMAL, 1, 0, 0);
  builtin_function_2 ("__builtin_sin", "sin",
		      double_ftype_double, double_ftype_double,
		      BUILT_IN_SIN, BUILT_IN_NORMAL, 1, 0, 0);
  builtin_function_2 ("__builtin_sinl", "sinl",
		      ldouble_ftype_ldouble, ldouble_ftype_ldouble,
		      BUILT_IN_SIN, BUILT_IN_NORMAL, 1, 0, 0);
  builtin_function_2 ("__builtin_cosf", "cosf",
		      float_ftype_float, float_ftype_float,
		      BUILT_IN_COS, BUILT_IN_NORMAL, 1, 0, 0);
  builtin_function_2 ("__builtin_cos", "cos",
		      double_ftype_double, double_ftype_double,
		      BUILT_IN_COS, BUILT_IN_NORMAL, 1, 0, 0);
  builtin_function_2 ("__builtin_cosl", "cosl",
		      ldouble_ftype_ldouble, ldouble_ftype_ldouble,
		      BUILT_IN_COS, BUILT_IN_NORMAL, 1, 0, 0);

  /* ISO C99 complex arithmetic functions.  */
  builtin_function_2 ("__builtin_conjf", "conjf",
		      cfloat_ftype_cfloat, cfloat_ftype_cfloat,
		      BUILT_IN_CONJ, BUILT_IN_NORMAL, 0, !flag_isoc99, 0);
  builtin_function_2 ("__builtin_conj", "conj",
		      cdouble_ftype_cdouble, cdouble_ftype_cdouble,
		      BUILT_IN_CONJ, BUILT_IN_NORMAL, 0, !flag_isoc99, 0);
  builtin_function_2 ("__builtin_conjl", "conjl",
		      cldouble_ftype_cldouble, cldouble_ftype_cldouble,
		      BUILT_IN_CONJ, BUILT_IN_NORMAL, 0, !flag_isoc99, 0);
  builtin_function_2 ("__builtin_crealf", "crealf",
		      float_ftype_cfloat, float_ftype_cfloat,
		      BUILT_IN_CREAL, BUILT_IN_NORMAL, 0, !flag_isoc99, 0);
  builtin_function_2 ("__builtin_creal", "creal",
		      double_ftype_cdouble, double_ftype_cdouble,
		      BUILT_IN_CREAL, BUILT_IN_NORMAL, 0, !flag_isoc99, 0);
  builtin_function_2 ("__builtin_creall", "creall",
		      ldouble_ftype_cldouble, ldouble_ftype_cldouble,
		      BUILT_IN_CREAL, BUILT_IN_NORMAL, 0, !flag_isoc99, 0);
  builtin_function_2 ("__builtin_cimagf", "cimagf",
		      float_ftype_cfloat, float_ftype_cfloat,
		      BUILT_IN_CIMAG, BUILT_IN_NORMAL, 0, !flag_isoc99, 0);
  builtin_function_2 ("__builtin_cimag", "cimag",
		      double_ftype_cdouble, double_ftype_cdouble,
		      BUILT_IN_CIMAG, BUILT_IN_NORMAL, 0, !flag_isoc99, 0);
  builtin_function_2 ("__builtin_cimagl", "cimagl",
		      ldouble_ftype_cldouble, ldouble_ftype_cldouble,
		      BUILT_IN_CIMAG, BUILT_IN_NORMAL, 0, !flag_isoc99, 0);

  built_in_decls[BUILT_IN_PUTCHAR] =
    builtin_function ("__builtin_putchar", int_ftype_int,
		      BUILT_IN_PUTCHAR, BUILT_IN_NORMAL, "putchar");
  built_in_decls[BUILT_IN_PUTS] =
    builtin_function ("__builtin_puts", puts_ftype,
		      BUILT_IN_PUTS, BUILT_IN_NORMAL, "puts");
  builtin_function_2 ("__builtin_printf", "printf",
		      printf_ftype, printf_ftype,
		      BUILT_IN_PRINTF, BUILT_IN_FRONTEND, 1, 0, 0);
  builtin_function_2 ("__builtin_fprintf", "fprintf",
		      fprintf_ftype, fprintf_ftype,
		      BUILT_IN_FPRINTF, BUILT_IN_FRONTEND, 1, 0, 0);
  built_in_decls[BUILT_IN_FWRITE] =
    builtin_function ("__builtin_fwrite", fwrite_ftype,
		      BUILT_IN_FWRITE, BUILT_IN_NORMAL, "fwrite");
  built_in_decls[BUILT_IN_FPUTC] =
    builtin_function ("__builtin_fputc", fputc_ftype,
		      BUILT_IN_FPUTC, BUILT_IN_NORMAL, "fputc");
  /* Declare the __builtin_ style with arguments and the regular style
     without them.  We rely on stdio.h to supply the arguments for the
     regular style declaration since we had to use void* instead of
     FILE* in the __builtin_ prototype supplied here.  */
  built_in_decls[BUILT_IN_FPUTS] =
    builtin_function_2 ("__builtin_fputs", "fputs",
			fputs_ftype, int_ftype_any,
			BUILT_IN_FPUTS, BUILT_IN_NORMAL, 1, 0, 0);

  /* Declare these functions non-returning
     to avoid spurious "control drops through" warnings.  */
  builtin_function_2 (NULL_PTR, "abort",
		      NULL_TREE, ((c_language == clk_cplusplus)
				  ? void_ftype : void_ftype_any),
		      0, NOT_BUILT_IN, 0, 0, 1);

  builtin_function_2 (NULL_PTR, "exit",
		      NULL_TREE, ((c_language == clk_cplusplus)
				  ? void_ftype_int : void_ftype_any),
		      0, NOT_BUILT_IN, 0, 0, 1);

#if 0
  /* Support for these has not been written in either expand_builtin
     or build_function_call.  */
  builtin_function ("__builtin_div", default_ftype, BUILT_IN_DIV,
		    BUILT_IN_NORMAL, NULL_PTR);
  builtin_function ("__builtin_ldiv", default_ftype, BUILT_IN_LDIV,
		    BUILT_IN_NORMAL, NULL_PTR);
  builtin_function ("__builtin_ffloor", double_ftype_double, BUILT_IN_FFLOOR,
		    BUILT_IN_NORMAL, NULL_PTR);
  builtin_function ("__builtin_fceil", double_ftype_double, BUILT_IN_FCEIL,
		    BUILT_IN_NORMAL, NULL_PTR);
  builtin_function ("__builtin_fmod", double_ftype_double_double,
		    BUILT_IN_FMOD, BUILT_IN_NORMAL, NULL_PTR);
  builtin_function ("__builtin_frem", double_ftype_double_double,
		    BUILT_IN_FREM, BUILT_IN_NORMAL, NULL_PTR);
  builtin_function ("__builtin_getexp", double_ftype_double, BUILT_IN_GETEXP,
		    BUILT_IN_NORMAL, NULL_PTR);
  builtin_function ("__builtin_getman", double_ftype_double, BUILT_IN_GETMAN,
		    BUILT_IN_NORMAL, NULL_PTR);
#endif

  main_identifier_node = get_identifier ("main");

  /* ??? Perhaps there's a better place to do this.  But it is related
     to __builtin_va_arg, so it isn't that off-the-wall.  */
  lang_type_promotes_to = simple_type_promotes_to;
}

tree
build_va_arg (expr, type)
     tree expr, type;
{
  return build1 (VA_ARG_EXPR, type, expr);
}


/* Possibly define a builtin function with one or two names.  BUILTIN_NAME
   is an __builtin_-prefixed name; NAME is the ordinary name; one or both
   of these may be NULL (though both being NULL is useless).
   BUILTIN_TYPE is the type of the __builtin_-prefixed function;
   TYPE is the type of the function with the ordinary name.  These
   may differ if the ordinary name is declared with a looser type to avoid
   conflicts with headers.  FUNCTION_CODE and CLASS are as for
   builtin_function.  If LIBRARY_NAME_P is nonzero, NAME is passed as
   the LIBRARY_NAME parameter to builtin_function when declaring BUILTIN_NAME.
   If NONANSI_P is nonzero, the name NAME is treated as a non-ANSI name; if
   NORETURN_P is nonzero, the function is marked as non-returning.
   Returns the declaration of BUILTIN_NAME, if any, otherwise
   the declaration of NAME.  Does not declare NAME if flag_no_builtin,
   or if NONANSI_P and flag_no_nonansi_builtin.  */

static tree
builtin_function_2 (builtin_name, name, builtin_type, type, function_code,
		    class, library_name_p, nonansi_p, noreturn_p)
     const char *builtin_name;
     const char *name;
     tree builtin_type;
     tree type;
     int function_code;
     enum built_in_class class;
     int library_name_p;
     int nonansi_p;
     int noreturn_p;
{
  tree bdecl = NULL_TREE;
  tree decl = NULL_TREE;
  if (builtin_name != 0)
    {
      bdecl = builtin_function (builtin_name, builtin_type, function_code,
				class, library_name_p ? name : NULL_PTR);
      if (noreturn_p)
	{
	  TREE_THIS_VOLATILE (bdecl) = 1;
	  TREE_SIDE_EFFECTS (bdecl) = 1;
	}
    }
  if (name != 0 && !flag_no_builtin && !(nonansi_p && flag_no_nonansi_builtin))
    {
      decl = builtin_function (name, type, function_code, class, NULL_PTR);
      if (nonansi_p)
	DECL_BUILT_IN_NONANSI (decl) = 1;
      if (noreturn_p)
	{
	  TREE_THIS_VOLATILE (decl) = 1;
	  TREE_SIDE_EFFECTS (decl) = 1;
	}
    }
  return (bdecl != 0 ? bdecl : decl);
}

/* Nonzero if the type T promotes to int.  This is (nearly) the
   integral promotions defined in ISO C99 6.3.1.1/2.  */

bool
c_promoting_integer_type_p (t)
     tree t;
{
  switch (TREE_CODE (t))
    {
    case INTEGER_TYPE:
      return (TYPE_MAIN_VARIANT (t) == char_type_node
	      || TYPE_MAIN_VARIANT (t) == signed_char_type_node
	      || TYPE_MAIN_VARIANT (t) == unsigned_char_type_node
	      || TYPE_MAIN_VARIANT (t) == short_integer_type_node
	      || TYPE_MAIN_VARIANT (t) == short_unsigned_type_node
	      || TYPE_PRECISION (t) < TYPE_PRECISION (integer_type_node));

    case ENUMERAL_TYPE:
      /* ??? Technically all enumerations not larger than an int
	 promote to an int.  But this is used along code paths
	 that only want to notice a size change.  */
      return TYPE_PRECISION (t) < TYPE_PRECISION (integer_type_node);

    case BOOLEAN_TYPE:
      return 1;

    default:
      return 0;
    }
}

/* Given a type, apply default promotions wrt unnamed function arguments
   and return the new type.  Return NULL_TREE if no change.  */
/* ??? There is a function of the same name in the C++ front end that
   does something similar, but is more thorough and does not return NULL
   if no change.  We could perhaps share code, but it would make the
   self_promoting_type property harder to identify.  */

tree
simple_type_promotes_to (type)
     tree type;
{
  if (TYPE_MAIN_VARIANT (type) == float_type_node)
    return double_type_node;

  if (c_promoting_integer_type_p (type))
    {
      /* Traditionally, unsignedness is preserved in default promotions.
         Also preserve unsignedness if not really getting any wider.  */
      if (TREE_UNSIGNED (type)
          && (flag_traditional
              || TYPE_PRECISION (type) == TYPE_PRECISION (integer_type_node)))
        return unsigned_type_node;
      return integer_type_node;
    }

  return NULL_TREE;
}

/* Return 1 if PARMS specifies a fixed number of parameters
   and none of their types is affected by default promotions.  */

int
self_promoting_args_p (parms)
     tree parms;
{
  register tree t;
  for (t = parms; t; t = TREE_CHAIN (t))
    {
      register tree type = TREE_VALUE (t);

      if (TREE_CHAIN (t) == 0 && type != void_type_node)
	return 0;

      if (type == 0)
	return 0;

      if (TYPE_MAIN_VARIANT (type) == float_type_node)
	return 0;

      if (c_promoting_integer_type_p (type))
	return 0;
    }
  return 1;
}

/* Recursively examines the array elements of TYPE, until a non-array
   element type is found.  */

tree
strip_array_types (type)
     tree type;
{
  while (TREE_CODE (type) == ARRAY_TYPE)
    type = TREE_TYPE (type);

  return type;
}

/* Recognize certain built-in functions so we can make tree-codes
   other than CALL_EXPR.  We do this when it enables fold-const.c
   to do something useful.  */
/* ??? By rights this should go in builtins.c, but only C and C++
   implement build_{binary,unary}_op.  Not exactly sure what bits
   of functionality are actually needed from those functions, or
   where the similar functionality exists in the other front ends.  */

tree
expand_tree_builtin (function, params, coerced_params)
     tree function, params, coerced_params;
{
  enum tree_code code;

  if (DECL_BUILT_IN_CLASS (function) != BUILT_IN_NORMAL)
    return NULL_TREE;

  switch (DECL_FUNCTION_CODE (function))
    {
    case BUILT_IN_ABS:
    case BUILT_IN_FABS:
      if (coerced_params == 0)
	return integer_zero_node;
      return build_unary_op (ABS_EXPR, TREE_VALUE (coerced_params), 0);

    case BUILT_IN_CONJ:
      if (coerced_params == 0)
	return integer_zero_node;
      return build_unary_op (CONJ_EXPR, TREE_VALUE (coerced_params), 0);

    case BUILT_IN_CREAL:
      if (coerced_params == 0)
	return integer_zero_node;
      return build_unary_op (REALPART_EXPR, TREE_VALUE (coerced_params), 0);

    case BUILT_IN_CIMAG:
      if (coerced_params == 0)
	return integer_zero_node;
      return build_unary_op (IMAGPART_EXPR, TREE_VALUE (coerced_params), 0);

    case BUILT_IN_ISGREATER:
      if (TARGET_FLOAT_FORMAT == IEEE_FLOAT_FORMAT)
	code = UNLE_EXPR;
      else
	code = LE_EXPR;
      goto unordered_cmp;

    case BUILT_IN_ISGREATEREQUAL:
      if (TARGET_FLOAT_FORMAT == IEEE_FLOAT_FORMAT)
	code = UNLT_EXPR;
      else
	code = LT_EXPR;
      goto unordered_cmp;

    case BUILT_IN_ISLESS:
      if (TARGET_FLOAT_FORMAT == IEEE_FLOAT_FORMAT)
	code = UNGE_EXPR;
      else
	code = GE_EXPR;
      goto unordered_cmp;

    case BUILT_IN_ISLESSEQUAL:
      if (TARGET_FLOAT_FORMAT == IEEE_FLOAT_FORMAT)
	code = UNGT_EXPR;
      else
	code = GT_EXPR;
      goto unordered_cmp;

    case BUILT_IN_ISLESSGREATER:
      if (TARGET_FLOAT_FORMAT == IEEE_FLOAT_FORMAT)
	code = UNEQ_EXPR;
      else
	code = EQ_EXPR;
      goto unordered_cmp;

    case BUILT_IN_ISUNORDERED:
      if (TARGET_FLOAT_FORMAT != IEEE_FLOAT_FORMAT)
	return integer_zero_node;
      code = UNORDERED_EXPR;
      goto unordered_cmp;

    unordered_cmp:
      {
	tree arg0, arg1;

	if (params == 0
	    || TREE_CHAIN (params) == 0)
	  {
	    error ("too few arguments to function `%s'",
		   IDENTIFIER_POINTER (DECL_NAME (function)));
	    return error_mark_node;
	  }
	else if (TREE_CHAIN (TREE_CHAIN (params)) != 0)
	  {
	    error ("too many arguments to function `%s'",
		   IDENTIFIER_POINTER (DECL_NAME (function)));
	    return error_mark_node;
	  }

	arg0 = TREE_VALUE (params);
	arg1 = TREE_VALUE (TREE_CHAIN (params));
	arg0 = build_binary_op (code, arg0, arg1, 0);
	if (code != UNORDERED_EXPR)
	  arg0 = build_unary_op (TRUTH_NOT_EXPR, arg0, 0);
	return arg0;
      }
      break;

    default:
      break;
    }

  return NULL_TREE;
}

/* Returns non-zero if CODE is the code for a statement.  */

int
statement_code_p (code)
     enum tree_code code;
{
  switch (code)
    {
    case EXPR_STMT:
    case COMPOUND_STMT:
    case DECL_STMT:
    case IF_STMT:
    case FOR_STMT:
    case WHILE_STMT:
    case DO_STMT:
    case RETURN_STMT:
    case BREAK_STMT:
    case CONTINUE_STMT:
    case SCOPE_STMT:
    case SWITCH_STMT:
    case GOTO_STMT:
    case LABEL_STMT:
    case ASM_STMT:
    case CASE_LABEL:
      return 1;

    default:
      if (lang_statement_code_p)
	return (*lang_statement_code_p) (code);
      return 0;
    }
}

/* Walk the statemen tree, rooted at *tp.  Apply FUNC to all the
   sub-trees of *TP in a pre-order traversal.  FUNC is called with the
   DATA and the address of each sub-tree.  If FUNC returns a non-NULL
   value, the traversal is aborted, and the value returned by FUNC is
   returned.  If FUNC sets WALK_SUBTREES to zero, then the subtrees of
   the node being visited are not walked.

   We don't need a without_duplicates variant of this one because the
   statement tree is a tree, not a graph.  */

tree 
walk_stmt_tree (tp, func, data)
     tree *tp;
     walk_tree_fn func;
     void *data;
{
  enum tree_code code;
  int walk_subtrees;
  tree result;
  int i, len;

#define WALK_SUBTREE(NODE)				\
  do							\
    {							\
      result = walk_stmt_tree (&(NODE), func, data);	\
      if (result)					\
	return result;					\
    }							\
  while (0)

  /* Skip empty subtrees.  */
  if (!*tp)
    return NULL_TREE;

  /* Skip subtrees below non-statement nodes.  */
  if (!statement_code_p (TREE_CODE (*tp)))
    return NULL_TREE;

  /* Call the function.  */
  walk_subtrees = 1;
  result = (*func) (tp, &walk_subtrees, data);

  /* If we found something, return it.  */
  if (result)
    return result;

  /* Even if we didn't, FUNC may have decided that there was nothing
     interesting below this point in the tree.  */
  if (!walk_subtrees)
    return NULL_TREE;

  /* FUNC may have modified the tree, recheck that we're looking at a
     statement node.  */
  code = TREE_CODE (*tp);
  if (!statement_code_p (code))
    return NULL_TREE;

  /* Walk over all the sub-trees of this operand.  Statement nodes never
     contain RTL, and we needn't worry about TARGET_EXPRs.  */
  len = TREE_CODE_LENGTH (code);

  /* Go through the subtrees.  We need to do this in forward order so
     that the scope of a FOR_EXPR is handled properly.  */
  for (i = 0; i < len; ++i)
    WALK_SUBTREE (TREE_OPERAND (*tp, i));

  /* Finally visit the chain.  This can be tail-recursion optimized if
     we write it this way.  */
  return walk_stmt_tree (&TREE_CHAIN (*tp), func, data);

#undef WALK_SUBTREE
}

/* Used to compare case labels.  K1 and K2 are actually tree nodes
   representing case labels, or NULL_TREE for a `default' label.
   Returns -1 if K1 is ordered before K2, -1 if K1 is ordered after
   K2, and 0 if K1 and K2 are equal.  */

int
case_compare (k1, k2)
     splay_tree_key k1;
     splay_tree_key k2;
{
  /* Consider a NULL key (such as arises with a `default' label) to be
     smaller than anything else.  */
  if (!k1)
    return k2 ? -1 : 0;
  else if (!k2)
    return k1 ? 1 : 0;

  return tree_int_cst_compare ((tree) k1, (tree) k2);
}

/* Process a case label for the range LOW_VALUE ... HIGH_VALUE.  If
   LOW_VALUE and HIGH_VALUE are both NULL_TREE then this case label is
   actually a `default' label.  If only HIGH_VALUE is NULL_TREE, then
   case label was declared using the usual C/C++ syntax, rather than
   the GNU case range extension.  CASES is a tree containing all the
   case ranges processed so far; COND is the condition for the
   switch-statement itself.  Returns the CASE_LABEL created, or
   ERROR_MARK_NODE if no CASE_LABEL is created.  */

tree
c_add_case_label (cases, cond, low_value, high_value)
     splay_tree cases;
     tree cond;
     tree low_value;
     tree high_value;
{
  tree type;
  tree label;
  tree case_label;
  splay_tree_node node;

  /* Create the LABEL_DECL itself.  */
  label = build_decl (LABEL_DECL, NULL_TREE, NULL_TREE);
  DECL_CONTEXT (label) = current_function_decl;

  /* If there was an error processing the switch condition, bail now
     before we get more confused.  */
  if (!cond || cond == error_mark_node)
    {
      /* Add a label anyhow so that the back-end doesn't think that
	 the beginning of the switch is unreachable.  */
      if (!cases->root)
	add_stmt (build_case_label (NULL_TREE, NULL_TREE, label));
      return error_mark_node;
    }

  if ((low_value && TREE_TYPE (low_value) 
       && POINTER_TYPE_P (TREE_TYPE (low_value))) 
      || (high_value && TREE_TYPE (high_value)
	  && POINTER_TYPE_P (TREE_TYPE (high_value))))
    error ("pointers are not permitted as case values");

  /* Case ranges are a GNU extension.  */
  if (high_value && pedantic)
    {
      if (c_language == clk_cplusplus)
	pedwarn ("ISO C++ forbids range expressions in switch statements");
      else
	pedwarn ("ISO C forbids range expressions in switch statements");
    }

  type = TREE_TYPE (cond);
  if (low_value)
    {
      low_value = check_case_value (low_value);
      low_value = convert_and_check (type, low_value);
    }
  if (high_value)
    {
      high_value = check_case_value (high_value);
      high_value = convert_and_check (type, high_value);
    }

  /* If an error has occurred, bail out now.  */
  if (low_value == error_mark_node || high_value == error_mark_node)
    {
      if (!cases->root)
	add_stmt (build_case_label (NULL_TREE, NULL_TREE, label));
      return error_mark_node;
    }

  /* If the LOW_VALUE and HIGH_VALUE are the same, then this isn't
     really a case range, even though it was written that way.  Remove
     the HIGH_VALUE to simplify later processing.  */
  if (tree_int_cst_equal (low_value, high_value))
    high_value = NULL_TREE;
  if (low_value && high_value 
      && !tree_int_cst_lt (low_value, high_value)) 
    warning ("empty range specified");

  /* Look up the LOW_VALUE in the table of case labels we already
     have.  */
  node = splay_tree_lookup (cases, (splay_tree_key) low_value);
  /* If there was not an exact match, check for overlapping ranges.
     There's no need to do this if there's no LOW_VALUE or HIGH_VALUE;
     that's a `default' label and the only overlap is an exact match.  */
  if (!node && (low_value || high_value))
    {
      splay_tree_node low_bound;
      splay_tree_node high_bound;

      /* Even though there wasn't an exact match, there might be an
	 overlap between this case range and another case range.
	 Since we've (inductively) not allowed any overlapping case
	 ranges, we simply need to find the greatest low case label
	 that is smaller that LOW_VALUE, and the smallest low case
	 label that is greater than LOW_VALUE.  If there is an overlap
	 it will occur in one of these two ranges.  */
      low_bound = splay_tree_predecessor (cases,
					  (splay_tree_key) low_value);
      high_bound = splay_tree_successor (cases,
					 (splay_tree_key) low_value);

      /* Check to see if the LOW_BOUND overlaps.  It is smaller than
	 the LOW_VALUE, so there is no need to check unless the
	 LOW_BOUND is in fact itself a case range.  */
      if (low_bound
	  && CASE_HIGH ((tree) low_bound->value)
	  && tree_int_cst_compare (CASE_HIGH ((tree) low_bound->value),
				    low_value) >= 0)
	node = low_bound;
      /* Check to see if the HIGH_BOUND overlaps.  The low end of that
	 range is bigger than the low end of the current range, so we
	 are only interested if the current range is a real range, and
	 not an ordinary case label.  */
      else if (high_bound 
	       && high_value
	       && (tree_int_cst_compare ((tree) high_bound->key,
					 high_value)
		   <= 0))
	node = high_bound;
    }
  /* If there was an overlap, issue an error.  */
  if (node)
    {
      tree duplicate = CASE_LABEL_DECL ((tree) node->value);

      if (high_value)
	{
	  error ("duplicate (or overlapping) case value");
	  error_with_decl (duplicate, 
			   "this is the first entry overlapping that value");
	}
      else if (low_value)
	{
	  error ("duplicate case value") ;
	  error_with_decl (duplicate, "previously used here");
	}
      else
	{
	  error ("multiple default labels in one switch");
	  error_with_decl (duplicate, "this is the first default label");
	}
      if (!cases->root)
	add_stmt (build_case_label (NULL_TREE, NULL_TREE, label));
    }

  /* Add a CASE_LABEL to the statement-tree.  */
  case_label = add_stmt (build_case_label (low_value, high_value, label));
  /* Register this case label in the splay tree.  */
  splay_tree_insert (cases, 
		     (splay_tree_key) low_value,
		     (splay_tree_value) case_label);

  return case_label;
}

/* Mark P (a stmt_tree) for GC.  The use of a `void *' for the
   parameter allows this function to be used as a GC-marking
   function.  */

void
mark_stmt_tree (p)
     void *p;
{
  stmt_tree st = (stmt_tree) p;

  ggc_mark_tree (st->x_last_stmt);
  ggc_mark_tree (st->x_last_expr_type);
}

/* Mark LD for GC.  */

void
c_mark_lang_decl (c)
     struct c_lang_decl *c;
{
  ggc_mark_tree (c->saved_tree);
}

/* Mark F for GC.  */

void
mark_c_language_function (f)
     struct language_function *f;
{
  if (!f)
    return;

  mark_stmt_tree (&f->x_stmt_tree);
  ggc_mark_tree (f->x_scope_stmt_stack);
}

/* Hook used by expand_expr to expand language-specific tree codes.  */

rtx
c_expand_expr (exp, target, tmode, modifier)
     tree exp;
     rtx target;
     enum machine_mode tmode;
     enum expand_modifier modifier;
{
  switch (TREE_CODE (exp))
    {
    case STMT_EXPR:
      {
	tree rtl_expr;
	rtx result;

	/* Since expand_expr_stmt calls free_temp_slots after every
	   expression statement, we must call push_temp_slots here.
	   Otherwise, any temporaries in use now would be considered
	   out-of-scope after the first EXPR_STMT from within the
	   STMT_EXPR.  */
	push_temp_slots ();
	rtl_expr = expand_start_stmt_expr ();
	expand_stmt (STMT_EXPR_STMT (exp));
	expand_end_stmt_expr (rtl_expr);
	result = expand_expr (rtl_expr, target, tmode, modifier);
	pop_temp_slots ();
	return result;
      }
      break;
      
    case CALL_EXPR:
      {
	if (TREE_CODE (TREE_OPERAND (exp, 0)) == ADDR_EXPR
	    && (TREE_CODE (TREE_OPERAND (TREE_OPERAND (exp, 0), 0))
		== FUNCTION_DECL)
	    && DECL_BUILT_IN (TREE_OPERAND (TREE_OPERAND (exp, 0), 0))
	    && (DECL_BUILT_IN_CLASS (TREE_OPERAND (TREE_OPERAND (exp, 0), 0))
		== BUILT_IN_FRONTEND))
	  return c_expand_builtin (exp, target, tmode, modifier);
	else
	  abort();
      }
      break;

    default:
      abort ();
    }

  abort ();
  return NULL;
}

/* Hook used by safe_from_p to handle language-specific tree codes.  */

int
c_safe_from_p (target, exp)
     rtx target;
     tree exp;
{
  /* We can see statements here when processing the body of a
     statement-expression.  For a declaration statement declaring a
     variable, look at the variable's initializer.  */
  if (TREE_CODE (exp) == DECL_STMT) 
    {
      tree decl = DECL_STMT_DECL (exp);

      if (TREE_CODE (decl) == VAR_DECL
	  && DECL_INITIAL (decl)
	  && !safe_from_p (target, DECL_INITIAL (decl), /*top_p=*/0))
	return 0;
    }

  /* For any statement, we must follow the statement-chain.  */
  if (statement_code_p (TREE_CODE (exp)) && TREE_CHAIN (exp))
    return safe_from_p (target, TREE_CHAIN (exp), /*top_p=*/0);

  /* Assume everything else is safe.  */
  return 1;
}

/* Hook used by unsafe_for_reeval to handle language-specific tree codes.  */

int
c_unsafe_for_reeval (exp)
     tree exp;
{
  /* Statement expressions may not be reevaluated.  */
  if (TREE_CODE (exp) == STMT_EXPR)
    return 2;

  /* Walk all other expressions.  */
  return -1;
}

/* Tree code classes. */

#define DEFTREECODE(SYM, NAME, TYPE, LENGTH) TYPE,

static char c_tree_code_type[] = {
  'x',
#include "c-common.def"
};
#undef DEFTREECODE

/* Table indexed by tree code giving number of expression
   operands beyond the fixed part of the node structure.
   Not used for types or decls.  */

#define DEFTREECODE(SYM, NAME, TYPE, LENGTH) LENGTH,

static int c_tree_code_length[] = {
  0,
#include "c-common.def"
};
#undef DEFTREECODE

/* Names of tree components.
   Used for printing out the tree and error messages.  */
#define DEFTREECODE(SYM, NAME, TYPE, LEN) NAME,

static const char *c_tree_code_name[] = {
  "@@dummy",
#include "c-common.def"
};
#undef DEFTREECODE

/* Adds the tree codes specific to the C front end to the list of all
   tree codes. */

void
add_c_tree_codes ()
{
  memcpy (tree_code_type + (int) LAST_AND_UNUSED_TREE_CODE,
	  c_tree_code_type,
	  (int)LAST_C_TREE_CODE - (int)LAST_AND_UNUSED_TREE_CODE);
  memcpy (tree_code_length + (int) LAST_AND_UNUSED_TREE_CODE,
	  c_tree_code_length,
	  (LAST_C_TREE_CODE - (int)LAST_AND_UNUSED_TREE_CODE) * sizeof (int));
  memcpy (tree_code_name + (int) LAST_AND_UNUSED_TREE_CODE,
	  c_tree_code_name,
	  (LAST_C_TREE_CODE - (int)LAST_AND_UNUSED_TREE_CODE) * sizeof (char *));
  lang_unsafe_for_reeval = c_unsafe_for_reeval;
}

#define CALLED_AS_BUILT_IN(NODE) \
   (!strncmp (IDENTIFIER_POINTER (DECL_NAME (NODE)), "__builtin_", 10))

static rtx
c_expand_builtin (exp, target, tmode, modifier)
     tree exp;
     rtx target;
     enum machine_mode tmode;
     enum expand_modifier modifier;
{
  tree type = TREE_TYPE (exp);
  tree fndecl = TREE_OPERAND (TREE_OPERAND (exp, 0), 0);
  tree arglist = TREE_OPERAND (exp, 1);
  enum built_in_function fcode = DECL_FUNCTION_CODE (fndecl);
  enum tree_code code = TREE_CODE (exp);
  const int ignore = (target == const0_rtx
		      || ((code == NON_LVALUE_EXPR || code == NOP_EXPR
			   || code == CONVERT_EXPR || code == REFERENCE_EXPR
			   || code == COND_EXPR)
			  && TREE_CODE (type) == VOID_TYPE));

  if (! optimize && ! CALLED_AS_BUILT_IN (fndecl))
    return expand_call (exp, target, ignore);

  switch (fcode)
    {
    case BUILT_IN_PRINTF:
      target = c_expand_builtin_printf (arglist, target, tmode,
					modifier, ignore);
      if (target)
	return target;
      break;

    case BUILT_IN_FPRINTF:
      target = c_expand_builtin_fprintf (arglist, target, tmode,
					 modifier, ignore);
      if (target)
	return target;
      break;

    default:			/* just do library call, if unknown builtin */
      error ("built-in function `%s' not currently supported",
	     IDENTIFIER_POINTER (DECL_NAME (fndecl)));
    }

  /* The switch statement above can drop through to cause the function
     to be called normally.  */
  return expand_call (exp, target, ignore);
}

/* Check an arglist to *printf for problems.  The arglist should start
   at the format specifier, with the remaining arguments immediately
   following it. */
static int
is_valid_printf_arglist (arglist)
  tree arglist;
{
  /* Save this value so we can restore it later. */
  const int SAVE_pedantic = pedantic;
  int diagnostic_occurred = 0;

  /* Set this to a known value so the user setting won't affect code
     generation.  */
  pedantic = 1;
  /* Check to make sure there are no format specifier errors. */
  check_function_format (&diagnostic_occurred,
			 maybe_get_identifier("printf"),
			 NULL_TREE, arglist);

  /* Restore the value of `pedantic'. */
  pedantic = SAVE_pedantic;

  /* If calling `check_function_format_ptr' produces a warning, we
     return false, otherwise we return true. */
  return ! diagnostic_occurred;
}

/* If the arguments passed to printf are suitable for optimizations,
   we attempt to transform the call. */
static rtx
c_expand_builtin_printf (arglist, target, tmode, modifier, ignore)
     tree arglist;
     rtx target;
     enum machine_mode tmode;
     enum expand_modifier modifier;
     int ignore;
{
  tree fn_putchar = built_in_decls[BUILT_IN_PUTCHAR],
    fn_puts = built_in_decls[BUILT_IN_PUTS];
  tree fn, format_arg, stripped_string;

  /* If the return value is used, or the replacement _DECL isn't
     initialized, don't do the transformation. */
  if (!ignore || !fn_putchar || !fn_puts)
    return 0;

  /* Verify the required arguments in the original call. */
  if (arglist == 0
      || (TREE_CODE (TREE_TYPE (TREE_VALUE (arglist))) != POINTER_TYPE))
    return 0;
  
  /* Check the specifier vs. the parameters. */
  if (!is_valid_printf_arglist (arglist))
    return 0;
  
  format_arg = TREE_VALUE (arglist);
  stripped_string = format_arg;
  STRIP_NOPS (stripped_string);
  if (stripped_string && TREE_CODE (stripped_string) == ADDR_EXPR)
    stripped_string = TREE_OPERAND (stripped_string, 0);

  /* If the format specifier isn't a STRING_CST, punt.  */
  if (TREE_CODE (stripped_string) != STRING_CST)
    return 0;
  
  /* OK!  We can attempt optimization.  */

  /* If the format specifier was "%s\n", call __builtin_puts(arg2). */
  if (strcmp (TREE_STRING_POINTER (stripped_string), "%s\n") == 0)
    {
      arglist = TREE_CHAIN (arglist);
      fn = fn_puts;
    }
  /* If the format specifier was "%c", call __builtin_putchar (arg2). */
  else if (strcmp (TREE_STRING_POINTER (stripped_string), "%c") == 0)
    {
      arglist = TREE_CHAIN (arglist);
      fn = fn_putchar;
    }
  else
    {
     /* We can't handle anything else with % args or %% ... yet. */
      if (strchr (TREE_STRING_POINTER (stripped_string), '%'))
	return 0;
      
      /* If the resulting constant string has a length of 1, call
         putchar.  Note, TREE_STRING_LENGTH includes the terminating
         NULL in its count.  */
      if (TREE_STRING_LENGTH (stripped_string) == 2)
        {
	  /* Given printf("c"), (where c is any one character,)
             convert "c"[0] to an int and pass that to the replacement
             function. */
	  arglist = build_int_2 (TREE_STRING_POINTER (stripped_string)[0], 0);
	  arglist = build_tree_list (NULL_TREE, arglist);
	  
	  fn = fn_putchar;
        }
      /* If the resulting constant was "string\n", call
         __builtin_puts("string").  Ensure "string" has at least one
         character besides the trailing \n.  Note, TREE_STRING_LENGTH
         includes the terminating NULL in its count.  */
      else if (TREE_STRING_LENGTH (stripped_string) > 2
	       && TREE_STRING_POINTER (stripped_string)
	       [TREE_STRING_LENGTH (stripped_string) - 2] == '\n')
        {
	  /* Create a NULL-terminated string that's one char shorter
	     than the original, stripping off the trailing '\n'.  */
	  const int newlen = TREE_STRING_LENGTH (stripped_string) - 1;
	  char *newstr = (char *) alloca (newlen);
	  memcpy (newstr, TREE_STRING_POINTER (stripped_string), newlen - 1);
	  newstr[newlen - 1] = 0;
	  
	  arglist = combine_strings (build_string (newlen, newstr));
	  arglist = build_tree_list (NULL_TREE, arglist);
	  fn = fn_puts;
	}
      else
	/* We'd like to arrange to call fputs(string) here, but we
           need stdout and don't have a way to get it ... yet.  */
	return 0;
    }
  
  return expand_expr (build_function_call (fn, arglist),
		      (ignore ? const0_rtx : target),
		      tmode, modifier);
}

/* If the arguments passed to fprintf are suitable for optimizations,
   we attempt to transform the call. */
static rtx
c_expand_builtin_fprintf (arglist, target, tmode, modifier, ignore)
     tree arglist;
     rtx target;
     enum machine_mode tmode;
     enum expand_modifier modifier;
     int ignore;
{
  tree fn_fputc = built_in_decls[BUILT_IN_FPUTC],
    fn_fputs = built_in_decls[BUILT_IN_FPUTS];
  tree fn, format_arg, stripped_string;

  /* If the return value is used, or the replacement _DECL isn't
     initialized, don't do the transformation. */
  if (!ignore || !fn_fputc || !fn_fputs)
    return 0;

  /* Verify the required arguments in the original call. */
  if (arglist == 0
      || (TREE_CODE (TREE_TYPE (TREE_VALUE (arglist))) != POINTER_TYPE)
      || (TREE_CHAIN (arglist) == 0)
      || (TREE_CODE (TREE_TYPE (TREE_VALUE (TREE_CHAIN (arglist)))) !=
	  POINTER_TYPE))
    return 0;
  
  /* Check the specifier vs. the parameters. */
  if (!is_valid_printf_arglist (TREE_CHAIN (arglist)))
    return 0;
  
  format_arg = TREE_VALUE (TREE_CHAIN (arglist));
  stripped_string = format_arg;
  STRIP_NOPS (stripped_string);
  if (stripped_string && TREE_CODE (stripped_string) == ADDR_EXPR)
    stripped_string = TREE_OPERAND (stripped_string, 0);

  /* If the format specifier isn't a STRING_CST, punt.  */
  if (TREE_CODE (stripped_string) != STRING_CST)
    return 0;
  
  /* OK!  We can attempt optimization.  */

  /* If the format specifier was "%s", call __builtin_fputs(arg3, arg1). */
  if (strcmp (TREE_STRING_POINTER (stripped_string), "%s") == 0)
    {
      tree newarglist = build_tree_list (NULL_TREE, TREE_VALUE (arglist));
      arglist = tree_cons (NULL_TREE,
			   TREE_VALUE (TREE_CHAIN (TREE_CHAIN (arglist))),
			   newarglist);
      fn = fn_fputs;
    }
  /* If the format specifier was "%c", call __builtin_fputc (arg3, arg1). */
  else if (strcmp (TREE_STRING_POINTER (stripped_string), "%c") == 0)
    {
      tree newarglist = build_tree_list (NULL_TREE, TREE_VALUE (arglist));
      arglist = tree_cons (NULL_TREE,
			   TREE_VALUE (TREE_CHAIN (TREE_CHAIN (arglist))),
			   newarglist);
      fn = fn_fputc;
    }
  else
    {
     /* We can't handle anything else with % args or %% ... yet. */
      if (strchr (TREE_STRING_POINTER (stripped_string), '%'))
	return 0;
      
      /* When "string" doesn't contain %, replace all cases of
         fprintf(stream,string) with fputs(string,stream).  The fputs
         builtin will take take of special cases like length==1.  */
      arglist = tree_cons (NULL_TREE, TREE_VALUE (TREE_CHAIN (arglist)),
			   build_tree_list (NULL_TREE, TREE_VALUE (arglist)));
      fn = fn_fputs;
    }
  
  return expand_expr (build_function_call (fn, arglist),
		      (ignore ? const0_rtx : target),
		      tmode, modifier);
}


/* Given a boolean expression ARG, return a tree representing an increment
   or decrement (as indicated by CODE) of ARG.  The front end must check for
   invalid cases (e.g., decrement in C++).  */
tree
boolean_increment (code, arg)
     enum tree_code code;
     tree arg;
{
  tree val;
  tree true_res = (c_language == clk_cplusplus
		   ? boolean_true_node
		   : c_bool_true_node);
  arg = stabilize_reference (arg);
  switch (code)
    {
    case PREINCREMENT_EXPR:
      val = build (MODIFY_EXPR, TREE_TYPE (arg), arg, true_res);
      break;
    case POSTINCREMENT_EXPR:
      val = build (MODIFY_EXPR, TREE_TYPE (arg), arg, true_res);
      arg = save_expr (arg);
      val = build (COMPOUND_EXPR, TREE_TYPE (arg), val, arg);
      val = build (COMPOUND_EXPR, TREE_TYPE (arg), arg, val);
      break;
    case PREDECREMENT_EXPR:
      val = build (MODIFY_EXPR, TREE_TYPE (arg), arg, invert_truthvalue (arg));
      break;
    case POSTDECREMENT_EXPR:
      val = build (MODIFY_EXPR, TREE_TYPE (arg), arg, invert_truthvalue (arg));
      arg = save_expr (arg);
      val = build (COMPOUND_EXPR, TREE_TYPE (arg), val, arg);
      val = build (COMPOUND_EXPR, TREE_TYPE (arg), arg, val);
      break;
    default:
      abort ();
    }
  TREE_SIDE_EFFECTS (val) = 1;
  return val;
}


/* Do the parts of lang_init common to C and C++.  */
void
c_common_lang_init ()
{
  /* If still "unspecified", make it match -fbounded-pointers.  */
  if (flag_bounds_check < 0)
    flag_bounds_check = flag_bounded_pointers;

  /* Special format checking options don't work without -Wformat; warn if
     they are used.  */
  if (warn_format_y2k && !warn_format)
    warning ("-Wformat-y2k ignored without -Wformat");
  if (warn_format_extra_args && !warn_format)
    warning ("-Wformat-extra-args ignored without -Wformat");
  if (warn_format_nonliteral && !warn_format)
    warning ("-Wformat-nonliteral ignored without -Wformat");
  if (warn_format_security && !warn_format)
    warning ("-Wformat-security ignored without -Wformat");
  if (warn_missing_format_attribute && !warn_format)
    warning ("-Wmissing-format-attribute ignored without -Wformat");
}
