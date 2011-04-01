// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/BoostAssertions.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace boost {

void assertion_failed( char const * expr,
                       char const * function,
                       char const * file,
                       long line)
{
  CF::Common::AssertionManager::do_assert ( false, expr, file, line, function);
}

} // namespace boost
