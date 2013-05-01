// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

/// @file Assertions.hpp
/// @note This header gets included indirectly in common/Component.hpp
///       It should be as lean as possible!

#ifndef cf3_common_Assertions_hpp
#define cf3_common_Assertions_hpp

#include "common/CommonAPI.hpp"

#ifndef CF3_ENABLE_STDASSERT
  #include <cassert>
#endif

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace common {

/// Manager of behavior of assertions
class Common_API AssertionManager : public boost::noncopyable {
public:

  /// Constructor
  AssertionManager();

  /// Gets the instance of the manager
  static AssertionManager& instance ();

  /// If AssertionManager is not handling assertions and those are
  /// passed to the standard assert function
  /// Controlled by the build option CF3_ENABLE_STDASSERT
  static bool notHandlingAssertions ()
  {
    #ifdef CF3_ENABLE_STDASSERT
      return true;
    #else
      return false;
    #endif
  }

  /// Forward declaration of the function that inplements the always present assert
  static void do_assert (  bool condition,
                           const char * cond_str,
                           const char * file,
                           int line,
                           const char * func,
                           const char * desc = 0 );

  static void do_assert (  bool condition,
                           const char * cond_str,
                           const char * file,
                           int line,
                           const char * func,
                           const std::string& desc) { do_assert(condition,cond_str,file,line,func,desc.c_str()); }

  /// flag to  dynamically turn off assertions
  bool DoAssertions;
  /// assertions dump backtraces
  bool AssertionDumps;
  /// assertions throw exceptions
  bool AssertionThrows;

}; // class AssertionManager

} // namespace common

////////////////////////////////////////////////////////////////////////////////

#ifndef CF3_ENABLE_STDASSERT

/* using coolfluid assertion manager */

/// Assertion that always should be checked, even in optimized builds.
/// Use this for testing in non-speed-critical code.
#define cf3_always_assert(a) \
    if (a) {} else { ::cf3::common::AssertionManager::do_assert((a), #a, __FILE__, __LINE__, __FUNCTION__); }

/// Assertion that always should be checked, even in optimized builds.
/// Use this for testing in non-speed-critical code.
/// Adds a description of the assertion
#define cf3_always_assert_desc(msg,a) \
    if (a) {} else { ::cf3::common::AssertionManager::do_assert((a), #a, __FILE__, __LINE__, __FUNCTION__, msg); }

/// CF assertion
/// Assertions are off if compiled with DNDEBUG
#ifndef NDEBUG
  #define cf3_assert(a)        cf3_always_assert((a))
  #define cf3_assert_desc(m,a) cf3_always_assert_desc(m,(a))
#else
  #define cf3_assert(a)
  #define cf3_assert_desc(m,a)
#endif

#else // CF3_ENABLE_STDASSERT

/* using standard assertions */

/// CF assertion
/// Assertions are off if compiled with DNDEBUG
#ifndef NDEBUG
#define   cf3_assert(a)                 assert(a)
  #define cf3_assert_desc(m,a)          assert(a)
  #define cf3_always_assert(a)          assert(a)
  #define cf3_always_assert_desc(msg,a) assert(a)
#else
  #define cf3_assert(a)
  #define cf3_assert_desc(m,a)
  #define cf3_always_assert(a)
  #define cf3_always_assert_desc(msg,a)
#endif

#endif // CF3_ENABLE_STDASSERT

////////////////////////////////////////////////////////////////////////////////

} // cf3

#endif // cf3_common_Assertions_hpp
