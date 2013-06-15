// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

/// @file Use coolfluid assertions instead of Eigen assertions.
///
/// This file should be included before including any Eigen files

#ifndef cf3_common_EigenAssertions_hpp
#define cf3_common_EigenAssertions_hpp

#include "common/Assertions.hpp"


/// @macro Translate eigen assertion into coolfluid assertion
/// @author Willem Deconinck
#ifdef NDEBUG   // disable Eigen assertions if compiled with -DNDEBUG
  #define cf_eigen_assertion_failed(x)
#else
  #define cf_eigen_assertion_failed(x) \
    do { \
      if(!Eigen::internal::copy_bool(x)) \
        cf3::common::AssertionManager::do_assert(false, EIGEN_MAKESTRING(x), __FILE__, __LINE__, __PRETTY_FUNCTION__); \
    } while(false)
#endif

/// set eigen assertions to cf assertions
#ifdef eigen_assert
  #warning "common/EigenAssertions.hpp" must be included before any other Eigen header for eigen to coolfluid assertions
#else // eigen_assert can be overridden
  #define eigen_assert(x) cf_eigen_assertion_failed(x)
#endif

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_common_EigenAssertions_hpp
