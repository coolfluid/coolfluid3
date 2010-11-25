// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_GooglePerfTools_ProfiledTestFixture_hpp
#define CF_GooglePerfTools_ProfiledTestFixture_hpp

#include <boost/filesystem.hpp>
#include <boost/test/test_observer.hpp>

#include <coolfluid_profiling_config.h>

#include "Common/CodeProfiler.hpp"
#include "Tools/Testing/LibTesting.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Tools {
namespace Testing {

////////////////////////////////////////////////////////////////////////////////


/// Any test using this fixture (or a derivative) will be profiled
class Testing_API ProfiledTestFixture {
public:

  ProfiledTestFixture();
  ~ProfiledTestFixture();

  /// Start profiling when a test starts
  void test_unit_start( boost::unit_test::test_unit const& );

  /// terminate library when a test ends and process the file
  void test_unit_finish( boost::unit_test::test_unit const& );

protected:
  /// Start of the profile output name
  std::string m_prefix;
  /// Directory to store the profile data in
  boost::filesystem::path m_profile_dir;
private:
  /// Storage for the file being profiled to
  std::string m_current_filename;
  /// The full command that was ran
  std::string m_command;
  /// @c true if GooglePerfTools was found
  bool m_using_google_perf;
};

////////////////////////////////////////////////////////////////////////////////

} // Testing
} // Tools
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_GooglePerfTools_ProfiledTestFixture_hpp
