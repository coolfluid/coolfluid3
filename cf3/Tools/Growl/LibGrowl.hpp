// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_Tools_Growl_LibGrowl_hpp
#define cf3_Tools_Growl_LibGrowl_hpp

////////////////////////////////////////////////////////////////////////////////

#include "common/Library.hpp"

////////////////////////////////////////////////////////////////////////////////

/// Define the macro Tools_Growl_API
/// @note build system defines COOLFLUID_TOOLS_GROWL_EXPORTS when compiling
/// Growl files
#ifdef COOLFLUID_TOOLS_GROWL_EXPORTS
#   define Tools_Growl_API      CF3_EXPORT_API
#   define Tools_Growl_TEMPLATE
#else
#   define Tools_Growl_API      CF3_IMPORT_API
#   define Tools_Growl_TEMPLATE CF3_TEMPLATE_EXTERN
#endif

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace Tools {

/// @brief Classes for %Growl desktop notifications using the GNTP protocol
///
/// @see Growl::Notifier
/// @author Willem Deconinck
namespace Growl {

////////////////////////////////////////////////////////////////////////////////

/// @brief Defines the initialization and termination of the library %Growl
///
/// @see Growl::Notifier
/// @author Willem Deconinck
class Tools_Growl_API LibGrowl : public common::Library
{
public:

  
  

  /// Constructor
  LibGrowl ( const std::string& name) : common::Library(name) {   }

public: // functions

  /// @return string of the library namespace
  static std::string library_namespace() { return "cf3.Tools.Growl"; }

  /// Static function that returns the library name.
  /// Must be implemented for Library registration
  /// @return name of the library
  static std::string library_name() { return "Growl"; }

  /// Static function that returns the description of the library.
  /// Must be implemented for Library registration
  /// @return description of the library

  static std::string library_description()
  {
    return "This library implements the Growl notification API.";
  }

  /// Gets the Class name
  static std::string type_name() { return "LibGrowl"; }

}; // end LibGrowl

////////////////////////////////////////////////////////////////////////////////

} // Growl
} // Tools
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_Tools_Growl_LibGrowl_hpp
