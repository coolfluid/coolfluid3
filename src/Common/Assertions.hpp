// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Common_Assertions_hpp
#define CF_Common_Assertions_hpp

// disable boost assertions if compiled with -DNDEBUG
#ifdef NDEBUG
  #define BOOST_DISABLE_ASSERTS
#endif

#define BOOST_ENABLE_ASSERT_HANDLER
#include <boost/assert.hpp>

#include "Common/CommonAPI.hpp"

#ifndef CF_ENABLE_STDASSERT
  #include <cassert>
#endif

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Common {

/// Manager of behavior of assertions
class Common_API AssertionManager : public boost::noncopyable {
public:

  /// Constructor
  AssertionManager();

  /// Gets the instance of the manager
  static AssertionManager& instance ();

  /// If AssertionManager is not handling assertions and those are
  /// passed to the standard assert function
  /// Controlled by the build option CF_ENABLE_STDASSERT
  static bool notHandlingAssertions ()
  {
    #ifdef CF_ENABLE_STDASSERT
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

} // namespace Common

////////////////////////////////////////////////////////////////////////////////

#ifndef CF_ENABLE_STDASSERT

/* using coolfluid assertion manager */

/// Assertion that always should be checked, even in optimized builds.
/// Use this for testing in non-speed-critical code.
#define cf_always_assert(a) \
    if (a) {} else { ::CF::Common::AssertionManager::do_assert((a), #a, __FILE__, __LINE__, __FUNCTION__); }

/// Assertion that always should be checked, even in optimized builds.
/// Use this for testing in non-speed-critical code.
/// Adds a description of the assertion
#define cf_always_assert_desc(msg,a) \
    if (a) {} else { ::CF::Common::AssertionManager::do_assert((a), #a, __FILE__, __LINE__, __FUNCTION__, msg); }

/// CF assertion
/// Assertions are off if compiled with DNDEBUG
#ifndef NDEBUG
  #define cf_assert(a)        cf_always_assert((a))
  #define cf_assert_desc(m,a) cf_always_assert_desc(m,(a))
#else
  #define cf_assert(a)
  #define cf_assert_desc(m,a)
#endif

#else // CF_ENABLE_STDASSERT

/* using standard assertions */

/// CF assertion
/// Assertions are off if compiled with DNDEBUG
#ifndef NDEBUG
#define   cf_assert(a)                 assert(a)
  #define cf_assert_desc(m,a)          assert(a)
  #define cf_always_assert(a)          assert(a)
  #define cf_always_assert_desc(msg,a) assert(a)
#else
  #define cf_assert(a)
  #define cf_assert_desc(m,a)
  #define cf_always_assert(a)
  #define cf_always_assert_desc(msg,a)
#endif

#endif // CF_ENABLE_STDASSERT

////////////////////////////////////////////////////////////////////////////////

} // CF

////////////////////////////////////////////////////////////////////////////////

namespace boost {

inline void assertion_failed(char const * expr,
                             char const * function,
                             char const * file,
                             long line)
{
  CF::Common::AssertionManager::do_assert ( false, expr, file, line, function);
}

} // namespace boost

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Common_Assertions_hpp
