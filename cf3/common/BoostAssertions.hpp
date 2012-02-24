// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_common_BoostAssertions_hpp
#define cf3_common_BoostAssertions_hpp

// build system should define BOOST_ENABLE_ASSERT_HANDLER

#include <boost/assert.hpp>

////////////////////////////////////////////////////////////////////////////////

/// @brief external boost library namespace
namespace boost {

/// @brief Callback function to provide our own boost_assertion handling
void assertion_failed( char const * expr,
                       char const * function,
                       char const * file,
                       long line);

/// @brief Callback function to provide our own boost_assertion handling
void assertion_failed_msg( char const * expr,
                           char const * msg,
                           char const * function,
                           char const * file,
                           long line);

} // namespace boost

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_common_BoostAssertions_hpp
