// { dg-options "-std=gnu++0x" }

// 2007-10-12  Paolo Carlini  <pcarlini@suse.de>
//
// Copyright (C) 2007 Free Software Foundation, Inc.
//
// This file is part of the GNU ISO C++ Library.  This library is free
// software; you can redistribute it and/or modify it under the
// terms of the GNU General Public License as published by the
// Free Software Foundation; either version 2, or (at your option)
// any later version.

// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without Pred the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License along
// with this library; see the file COPYING.  If not, write to the Free
// Software Foundation, 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,
// USA.

// 25.3.6 Heap operations [lib.alg.heap.operations]

#include <algorithm>
#include <functional>
#include <testsuite_hooks.h>

int A[] = {9, 8, 6, 7, 7, 5, 5, 3, 6, 4, 1, 2, 3, 4};
int B[] = {1, 3, 2, 4, 4, 6, 3, 5, 5, 7, 7, 6, 8, 9};
const int N = sizeof(A) / sizeof(int);

void
test01()
{
  bool test __attribute__((unused)) = true;

  for (int i = 0; i <= N; ++i)
    {
      VERIFY( A + i == std::is_heap_until(A, A + i) );
      VERIFY( A + i == std::is_heap_until(A, A + i, std::less<int>()) );
      VERIFY( B + i == std::is_heap_until(B, B + i, std::greater<int>()) );
      VERIFY( B + (i < 2 ? i : 1) == std::is_heap_until(B, B + i) );      
    }
}

int
main()
{
  test01();
  return 0;
}
