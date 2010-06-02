#ifndef CF_GooglePerfTools_ProfiledTestFixture_hpp
#define CF_GooglePerfTools_ProfiledTestFixture_hpp

#include <boost/filesystem.hpp>

#include <coolfluid_profiling_config.h>

using namespace CF;
using namespace CF::Common;

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Tools {

/// The classes related to Google perftools
namespace GooglePerf {

class GooglePerfToolsModule;

////////////////////////////////////////////////////////////////////////////////

/// Provides a boilerplate implementation for a test that is profiled using GooglePerfTools
/// The test takes one optional runtime argument specifying the directory in which to save the profiling results
/// By default, the name of the test command is used as the first part of the profile output name
#ifdef CF_BUILD_GooglePerfTools
class ProfiledTestFixture {
public:
  /// On construction, the GooglePerfTools dynamic plugin is loaded from the build dir
  ProfiledTestFixture();


  /// Start profiling. If ProfileName is provided, this is appended to the current prefix.
  /// Full profile name will be <prefix>-<ProfileName>.pprof
  void startProfiling(const std::string& ProfileName = "");

  /// Stop profiling
  void stopProfiling();

  /// Process the last profiling file, storing the result as a .svg file with the same
  /// base name as the profile file
  void procesProfilingFile();

protected:
  /// The profiling module that is used
  GooglePerfToolsModule& m_profiler;
  /// Start of the profile output name
  std::string m_prefix;
  /// Directory to store the profile data in
  boost::filesystem::path m_profile_dir;
private:
  /// Storage for the file being profiled to
  std::string m_current_filename;
  /// The full command that was ran
  std::string m_command;
};
#else

class ProfiledTestFixture {
public:
  void startProfiling() {}

  void stopProfiling() {}
};

#endif

////////////////////////////////////////////////////////////////////////////////

} // GooglePerf
} // Tools
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_GooglePerfTools_ProfiledTestFixture_hpp
