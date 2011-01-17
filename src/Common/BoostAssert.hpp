// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Common_BoostAssert_hpp
#define CF_Common_BoostAssert_hpp

#ifndef CF_hpp
#error The header BoostAssert.hpp shouldnt be included directly rather by including CF.hpp instead
#endif

// disable boost assertions if compiled with -DNDEBUG
#ifdef NDEBUG
  #define BOOST_DISABLE_ASSERTS
#endif


#define BOOST_ENABLE_ASSERT_HANDLER
#include <boost/assert.hpp> 

namespace boost {

//////////////////////////////////////////////////////////////////////////////

inline void assertion_failed(char const * expr, char const * function, char const * file, long line)
{
  CF::AssertionManager::do_assert ( false, expr, file, line, function);
}

//////////////////////////////////////////////////////////////////////////////

} // namespace boost


#endif // CF_Common_BoostAssert_hpp
