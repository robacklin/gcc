/* Macros to support INSN_ADDRESSES
   Copyright (C) 2000 Free Software Foundation, Inc.

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

#ifndef _INSN_ADDR_H
#define _INSN_ADDR_H 1

#include "varray.h"

extern varray_type insn_addresses_;
extern int insn_current_address;

#define INSN_ADDRESSES_DEFN() varray_type insn_addresses_
#define INSN_ADDRESSES(id) VARRAY_INT (insn_addresses_, (id))
#define INSN_ADDRESSES_ALLOC(size) \
  VARRAY_INT_INIT (insn_addresses_, (size), "insn_addresses")
#define INSN_ADDRESSES_FREE() VARRAY_FREE (insn_addresses_)
#define INSN_ADDRESSES_SET_P() (insn_addresses_ != 0)
#define INSN_ADDRESSES_SIZE() VARRAY_SIZE (insn_addresses_)
#define INSN_ADDRESSES_NEW(insn,addr) do {				\
  int insn_uid__ = INSN_UID ((insn)), insn_addr__ = (addr);		\
									\
  if (INSN_ADDRESSES_SET_P()) {						\
    if (INSN_ADDRESSES_SIZE() <= insn_uid__)				\
      insn_addresses_ = VARRAY_GROW (insn_addresses_, insn_uid__ + 1);	\
    INSN_ADDRESSES (insn_uid__) = insn_addr__;				\
  }									\
} while (0)

#endif /* _INSN_ADDR_H */
