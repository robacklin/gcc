/* Define constants for communication with c-parse.y.
   Copyright (C) 1987, 1992, 1998, 1999, 2000 Free Software Foundation, Inc.

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

#ifndef _C_LEX_H
#define _C_LEX_H

extern tree make_pointer_declarator PARAMS ((tree, tree));
extern void position_after_white_space PARAMS ((void));

extern int c_lex PARAMS ((tree *));
extern const char *init_c_lex PARAMS ((const char *));

extern void save_and_forget_protocol_qualifiers PARAMS ((void));
extern void forget_protocol_qualifiers PARAMS ((void));
extern void remember_protocol_qualifiers PARAMS ((void));
extern tree is_class_name PARAMS ((tree));

extern int indent_level;

struct cpp_reader;
extern struct cpp_reader* parse_in;

#endif
