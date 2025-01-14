// { dg-options "-std=gnu++11" }

// Copyright (C) 2015 Free Software Foundation, Inc.
//
// This file is part of the GNU ISO C++ Library.  This library is free
// software; you can redistribute it and/or modify it under the
// terms of the GNU General Public License as published by the
// Free Software Foundation; either version 3, or (at your option)
// any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along
// with this library; see the file COPYING3.  If not see
// <http://www.gnu.org/licenses/>.

#include <testsuite_hooks.h>
#include <testsuite_regex.h>
#include <testsuite_allocator.h>

using namespace __gnu_test;
using namespace std;

// Costumized allocator.
void
test01()
{
  bool test __attribute__((unused)) = true;

  {
    match_results<const char*, __gnu_test::tracker_allocator<char>> m;
    VERIFY(tracker_allocator_counter::get_allocation_count() == 0);
    VERIFY(tracker_allocator_counter::get_deallocation_count() == 0);
    VERIFY(regex_match("asdf", m, regex("asdf")));
    VERIFY(tracker_allocator_counter::get_allocation_count() > 0);
  }
  VERIFY(tracker_allocator_counter::get_allocation_count()
	 == tracker_allocator_counter::get_deallocation_count());
}

int
main()
{
  test01();
  return 0;
}
