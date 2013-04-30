// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_Tools_Prowl_LibProwl_hpp
#define cf3_Tools_Prowl_LibProwl_hpp

////////////////////////////////////////////////////////////////////////////////

#include "common/Library.hpp"

////////////////////////////////////////////////////////////////////////////////

/// Define the macro Prowl_API
/// @note build system defines COOLFLUID_TOOLS_PROWL_EXPORTS when compiling
/// Prowl files
#ifdef COOLFLUID_TOOLS_PROWL_EXPORTS
#   define Tools_Prowl_API      CF3_EXPORT_API
#   define Tools_Prowl_TEMPLATE
#else
#   define Tools_Prowl_API      CF3_IMPORT_API
#   define Tools_Prowl_TEMPLATE CF3_TEMPLATE_EXTERN
#endif

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace Tools {

/// @brief Classes for %Prowl push notifications on Apple iOS devices
///
/// %Prowl is an iOS app that can receive push notifications (http://www.prowlapp.com/),
/// sent from different sources (most notably <a href="http://growl.info/">Growl</a>).
/// @author Willem Deconinck
namespace Prowl {

////////////////////////////////////////////////////////////////////////////////

/// @brief Defines the initialization and termination of the library Prowl
///
/// %Prowl is an iOS app that can receive push notifications (http://www.prowlapp.com/),
/// sent from different sources (most notably <a href="http://growl.info/">Growl</a>).
/// @author Willem Deconinck
class Tools_Prowl_API LibProwl : public common::Library
{
public:

  
  

  /// Constructor
  LibProwl ( const std::string& name) : common::Library(name) {   }

public: // functions

  /// @return string of the library namespace
  static std::string library_namespace() { return "cf3.Tools.Prowl"; }

  /// Static function that returns the module name.
  /// Must be implemented for Library registration
  /// @return name of the library
  static std::string library_name() { return "Prowl"; }

  /// Static function that returns the description of the module.
  /// Must be implemented for Library registration
  /// @return description of the library

  static std::string library_description()
  {
    return "This library implements the Prowl notification API.";
  }

  /// Gets the Class name
  static std::string type_name() { return "LibProwl"; }

}; // end LibProwl

////////////////////////////////////////////////////////////////////////////////

} // Prowl
} // Tools
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_Tools_Prowl_LibProwl_hpp
