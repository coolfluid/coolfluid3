#ifndef CF_GooglePerfTools_GooglePerfTools_hpp
#define CF_GooglePerfTools_GooglePerfTools_hpp

////////////////////////////////////////////////////////////////////////////////

#include <boost/filesystem.hpp>

#include "Common/OwnedObject.hpp"
#include "Common/ConcreteProvider.hpp"
#include "Common/ModuleRegister.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Tools {

/// The classes related to Google perftools
namespace GooglePerf {

////////////////////////////////////////////////////////////////////////////////

/// Google perf-tools module
/// This module starts CPU profiling using Google perftools when loaded.
/// Usage: Add libGooglePerfTools to the Simulator.Modules.Libs list. This will
/// create a file called "perftools-profile.pprof" in the current output directory,
/// which contains profiling data that can be analyzed using i.e.:
///   pprof --gv <coolfluid-solver binary> perftools-profile.pprof
/// More examples are given on the google perftools website:
/// http://google-perftools.googlecode.com/svn/trunk/doc/cpuprofile.html
/// @author Bart Janssens
class GooglePerfToolsModule : public Common::ModuleRegister<GooglePerfToolsModule>
{
public:
  /// Default constructor constructs as uninited
  GooglePerfToolsModule();

  /// Static function that returns the module name.
  /// Must be implemented for the ModuleRegister template
  /// @return name of the module
  static std::string getModuleName()
  {
    return "GooglePerfTools";
  }

  /// Static function that returns the description of the module.
  /// Must be implemented for the ModuleRegister template
  /// @return descripton of the module
  static std::string getModuleDescription()
  {
    return "This module implements profiling using Google perftools.";
  }

  /// Gets the Class name
  static std::string type_name() { return "GooglePerfToolsModule"; }

  /// Start profiling
  virtual void initiate();

  /// Stop profiling
  virtual void terminate();

  /// Set the path to the profiling file
  void setFilePath(const boost::filesystem::path& path);

private:
  boost::filesystem::path m_path;

}; // end GooglePerfToolsModule

////////////////////////////////////////////////////////////////////////////////

} // GooglePerf
} // Tools
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_GooglePerfTools_GooglePerfTools_hpp
