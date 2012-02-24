// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "common/BoostAssertions.hpp"
#include "common/Assertions.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace boost {

void assertion_failed( char const * expr,
                       char const * function,
                       char const * file,
                       long line)
{
  cf3::common::AssertionManager::do_assert ( false, expr, file, line, function);
}

void assertion_failed_msg( char const * expr,
                           char const * msg,
                           char const * function,
                           char const * file,
                           long line)
{
  cf3::common::AssertionManager::do_assert ( false, expr, file, line, function, msg);
}

} // namespace boost
