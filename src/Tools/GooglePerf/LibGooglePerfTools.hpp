// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Tools_GooglePerfTools_LibGooglePerfTools_hpp
#define CF_Tools_GooglePerfTools_LibGooglePerfTools_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Common/CLibrary.hpp"

////////////////////////////////////////////////////////////////////////////////

/// Define the macro GooglePerfTools_API
/// @note build system defines COOLFLUID_GOOGLE_PERFTOOLS_EXPORTS when compiling
/// GooglePerfTools files
#ifdef COOLFLUID_GOOGLE_PERFTOOLS_EXPORTS
#   define GooglePerfTools_API      CF_EXPORT_API
#   define GooglePerfTools_TEMPLATE
#else
#   define GooglePerfTools_API      CF_IMPORT_API
#   define GooglePerfTools_TEMPLATE CF_TEMPLATE_EXTERN
#endif

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Tools {

/// The classes related to Google perftools
namespace GooglePerf {

////////////////////////////////////////////////////////////////////////////////

/// Google perf-tools library
/// This module starts CPU profiling using Google perftools when loaded.
/// Usage: Add libGooglePerfTools to the Simulator.Modules.Libs list. This will
/// create a file called "perftools-profile.pprof" in the current output directory,
/// which contains profiling data that can be analyzed using i.e.:
///   pprof --gv <coolfluid-solver binary> perftools-profile.pprof
/// More examples are given on the google perftools website:
/// http://google-perftools.googlecode.com/svn/trunk/doc/cpuprofile.html
/// @author Bart Janssens
class GooglePerfTools_API LibGooglePerfTools : public Common::CLibrary
{
public:

  typedef boost::shared_ptr<LibGooglePerfTools> Ptr;
  typedef boost::shared_ptr<LibGooglePerfTools const> ConstPtr;

  /// Constructor
  LibGooglePerfTools ( const std::string& name) : Common::CLibrary(name) { BuildComponent<none>().build(this); }

public: // functions

  /// @return string of the library namespace
  static std::string library_namespace() { return "CF.Tools.GooglePerf"; }

  /// Static function that returns the module name.
  /// Must be implemented for CLibrary registration
  /// @return name of the library
  static std::string library_name() {  return "GooglePerfTools"; }

  /// Static function that returns the description of the module.
  /// Must be implemented for CLibrary registration
  /// @return description of the library

  static std::string library_description()
  {
    return "This module implements profiling using Google perftools.";
  }

  /// Gets the Class name
  static std::string type_name() { return "LibGooglePerfTools"; }

  /// initiate library
  virtual void initiate();

  /// terminate library
  virtual void terminate();

}; // LibGooglePerfTools

////////////////////////////////////////////////////////////////////////////////

} // GooglePerf
} // Tools
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Tools_GooglePerfTools_LibGooglePerfTools_hpp
